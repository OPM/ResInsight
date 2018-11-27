/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicPointTangentManipulatorPartMgr.h"

#include "RivPartPriority.h"

#include "cafEffectGenerator.h"
#include "cafLine.h"
#include "cafSelectionManager.h"

#include "cvfBoxGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRay.h"
#include "cvfPlane.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfHitItem.h"

#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulatorPartMgr::RicPointTangentManipulatorPartMgr() 
    : m_tangentOnStartManipulation(cvf::Vec3d::UNDEFINED),
    m_originOnStartManipulation(cvf::Vec3d::UNDEFINED),
    m_currentHandleIndex(cvf::UNDEFINED_SIZE_T),
    m_handleSize(1.0)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulatorPartMgr::~RicPointTangentManipulatorPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setOrigin(const cvf::Vec3d& origin)
{
    if (isManipulatorActive()) return;

    m_origin = origin;
    if (m_originOnStartManipulation.isUndefined()) m_originOnStartManipulation = origin;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setTangent(const cvf::Vec3d& tangent)
{
    if(isManipulatorActive()) return;

    m_tangent = tangent;
    if (m_tangentOnStartManipulation.isUndefined()) m_tangentOnStartManipulation = m_tangent;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setHandleSize(double handleSize)
{
    m_handleSize = handleSize;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::originAndTangent(cvf::Vec3d* origin, cvf::Vec3d* tangent)
{
    *origin = m_origin;
    *tangent = m_tangent;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPointTangentManipulatorPartMgr::isManipulatorActive() const
{
    return m_currentHandleIndex != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::appendPartsToModel(cvf::ModelBasicList* model)
{
    if (!m_handleParts.size())
    {
        recreateAllGeometryAndParts();
    }

    for (size_t i = 0; i < m_handleParts.size(); i++)
    {
        model->addPart(m_handleParts.at(i));
    }

    for (auto activeModePart: m_activeDragModeParts)
    {
        model->addPart(activeModePart.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::tryToActivateManipulator(const cvf::HitItem* hitItem)
{
    endManipulator();

    if (!hitItem) return;

    const cvf::Part* pickedPart = hitItem->part();
    const cvf::Vec3d intersectionPoint = hitItem->intersectionPoint();

    if (!pickedPart) return;

    for (size_t i = 0; i < m_handleParts.size(); i++)
    {
        if (pickedPart == m_handleParts.at(i))
        {
            m_initialPickPoint = intersectionPoint;
            m_tangentOnStartManipulation = m_tangent;
            m_originOnStartManipulation = m_origin;
            m_currentHandleIndex = i;
        }
    }

}


//--------------------------------------------------------------------------------------------------
/// Calculate new origin and tangent based on the new ray position
/// Clear geometry to trigger regeneration  
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::updateManipulatorFromRay(const cvf::Ray* newMouseRay)
{
    if (!isManipulatorActive()) return;

    if ( m_handleIds[m_currentHandleIndex] ==  HORIZONTAL_PLANE )
    {
        cvf::Plane plane;
        plane.setFromPointAndNormal(m_origin, cvf::Vec3d::Z_AXIS);
        cvf::Vec3d newIntersection;
        newMouseRay->planeIntersect(plane, &newIntersection);

        cvf::Vec3d newOrigin = m_originOnStartManipulation + (newIntersection - m_initialPickPoint);

        m_origin = newOrigin;
    }
    else if ( m_handleIds[m_currentHandleIndex] ==  VERTICAL_AXIS )
    {
        cvf::Plane plane;
        cvf::Vec3d planeNormal = (newMouseRay->direction() ^ cvf::Vec3d::Z_AXIS) ^ cvf::Vec3d::Z_AXIS;
        double length = planeNormal.length();

        if (length < 1e-5) return;

        planeNormal /= length;
        plane.setFromPointAndNormal(m_initialPickPoint, planeNormal );
        cvf::Vec3d newIntersection;
        newMouseRay->planeIntersect(plane, &newIntersection);

        cvf::Vec3d newOrigin = m_originOnStartManipulation;
        newOrigin.z() += (newIntersection.z() - m_initialPickPoint.z());

        m_origin = newOrigin;
    }
    //m_tangent = newTangent;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::endManipulator()
{
    m_currentHandleIndex = cvf::UNDEFINED_SIZE_T;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::clearAllGeometryAndParts()
{
    m_handleIds.clear();
    m_handleParts.clear();
    m_activeDragModeParts.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::recreateAllGeometryAndParts()
{
    createAllHandleParts();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::createAllHandleParts()
{
    createHorizontalPlaneHandle();
    createVerticalAxisHandle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void  RicPointTangentManipulatorPartMgr::createHorizontalPlaneHandle()
{
    using namespace cvf;
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(6);
    
    vertexArray->set(0,  {-1, -1, 0} );
    vertexArray->set(1,  { 1, -1, 0});
    vertexArray->set(2,  { 1,  1, 0});
    vertexArray->set(3,  {-1, -1, 0});
    vertexArray->set(4,  { 1,  1, 0});
    vertexArray->set(5,  {-1,  1, 0});

    Vec3f origin(m_origin);
    for (cvf::Vec3f& vx: *vertexArray)
    {
        vx *= 0.5*m_handleSize;
        vx += origin;
    }

    ref<DrawableGeo> geo = createTriangelDrawableGeo(vertexArray.p());

    HandleType handleId = HORIZONTAL_PLANE;
    cvf::Color4f color =  cvf::Color4f(1.0f, 0.0f, 1.0f, 0.5f);
    cvf::String partName("PointTangentManipulator Horizontal Plane Handle");

    addHandlePart(geo.p(), color,  handleId, partName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void  RicPointTangentManipulatorPartMgr::createVerticalAxisHandle()
{
    using namespace cvf;
    
    cvf::ref< cvf::GeometryBuilderTriangles> geomBuilder = new cvf::GeometryBuilderTriangles;
    cvf::GeometryUtils::createBox({-0.3f, -0.3f, -1.0f}, { 0.3f,  0.3f, 1.0f}, geomBuilder.p());
    
    cvf::ref<cvf::Vec3fArray> vertexArray = geomBuilder->vertices();
    cvf::ref<cvf::UIntArray> indexArray = geomBuilder->triangles();

    Vec3f origin(m_origin);
    for (cvf::Vec3f& vx: *vertexArray)
    {
        vx *= 0.5*m_handleSize;
        vx += origin;
    }

    ref<DrawableGeo> geo = createIndexedTriangelDrawableGeo(vertexArray.p(), indexArray.p());

    HandleType handleId = VERTICAL_AXIS;
    cvf::Color4f color =  cvf::Color4f(0.0f, 0.2f, 0.8f, 0.5f);
    cvf::String partName("PointTangentManipulator Vertical Axis Handle");

    addHandlePart(geo.p(), color,  handleId, partName);
}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void  RicPointTangentManipulatorPartMgr::createAzimuthHandle()
{
    using namespace cvf;

    cvf::ref< cvf::GeometryBuilderTriangles> geomBuilder = new cvf::GeometryBuilderTriangles;
    cvf::GeometryUtils::createDisc(1.3, 1.1, 16, geomBuilder.p());

    cvf::ref<cvf::Vec3fArray> vertexArray = geomBuilder->vertices();
    cvf::ref<cvf::UIntArray> indexArray = geomBuilder->triangles();

    Vec3f origin(m_origin);
    for (cvf::Vec3f& vx: *vertexArray)
    {
        vx *= 0.5*m_handleSize;
        vx += origin;
    }

    ref<DrawableGeo> geo = createIndexedTriangelDrawableGeo(vertexArray.p(), indexArray.p());

    HandleType handleId = AZIMUTH;
    cvf::Color4f color =  cvf::Color4f(0.0f, 0.2f, 0.8f, 0.5f);
    cvf::String partName("PointTangentManipulator Azimuth Handle");

    addHandlePart(geo.p(), color,  handleId, partName);
}

#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createIndexedTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray, 
                                                                                               cvf::UIntArray* triangleIndices)
{
    using namespace cvf;
    ref<DrawableGeo> geo = new DrawableGeo;
    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES, triangleIndices);

    geo->setVertexArray(triangleVertexArray);
    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray)
{
    using namespace cvf;
    ref<DrawableGeo> geo = new DrawableGeo;

    geo->setVertexArray(triangleVertexArray);
    ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect(cvf::PT_TRIANGLES);
    primSet->setIndexCount(triangleVertexArray->size());

    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::addHandlePart(cvf::DrawableGeo* geo, 
                                                      const cvf::Color4f& color, 
                                                      HandleType handleId, 
                                                      const cvf::String& partName)
{
    cvf::ref<cvf::Part> handlePart = createPart(geo, color, partName);

    m_handleParts.push_back(handlePart.p());
    m_handleIds.push_back(handleId);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::addActiveModePart(cvf::DrawableGeo* geo, 
                                                          const cvf::Color4f& color, 
                                                          HandleType handleId, 
                                                          const cvf::String& partName)
{
    cvf::ref<cvf::Part> handlePart = createPart(geo, color, partName);

    m_activeDragModeParts.push_back(handlePart.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RicPointTangentManipulatorPartMgr::createPart(cvf::DrawableGeo* geo,
                                                                  const cvf::Color4f& color,
                                                                  const cvf::String& partName)
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName(partName);
    part->setDrawable(geo);
    part->updateBoundingBox();

    caf::SurfaceEffectGenerator surfaceGen(color, caf::PO_1);
    cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
    part->setEffect(eff.p());
    if (color.a() < 1.0) part->setPriority(RivPartPriority::Transparent);

    return part;
}






