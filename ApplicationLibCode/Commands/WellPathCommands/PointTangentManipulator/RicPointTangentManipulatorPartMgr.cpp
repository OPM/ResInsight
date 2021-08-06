/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
#include "cvfHitItem.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPlane.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRay.h"

#include "cvfGeometryTools.h"
#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulatorPartMgr::RicPointTangentManipulatorPartMgr()
    : m_tangentOnStartManipulation( cvf::Vec3d::UNDEFINED )
    , m_originOnStartManipulation( cvf::Vec3d::UNDEFINED )
    , m_activeHandle( HandleType::NONE )
    , m_handleSize( 1.0 )
    , m_isGeometryUpdateNeeded( true )
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
void RicPointTangentManipulatorPartMgr::setOrigin( const cvf::Vec3d& origin )
{
    if ( isManipulatorActive() ) return;

    m_origin = origin;
    if ( m_originOnStartManipulation.isUndefined() ) m_originOnStartManipulation = origin;

    m_isGeometryUpdateNeeded = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setTangent( const cvf::Vec3d& tangent )
{
    if ( isManipulatorActive() ) return;

    m_tangent = tangent;
    if ( m_tangentOnStartManipulation.isUndefined() ) m_tangentOnStartManipulation = m_tangent;

    m_isGeometryUpdateNeeded = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setHandleSize( double handleSize )
{
    m_handleSize = handleSize;

    m_isGeometryUpdateNeeded = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::originAndTangent( cvf::Vec3d* origin, cvf::Vec3d* tangent )
{
    *origin  = m_origin;
    *tangent = m_tangent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setPolyline( const std::vector<cvf::Vec3d>& polyline )
{
    m_polyline = polyline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPointTangentManipulatorPartMgr::isManipulatorActive() const
{
    return m_activeHandle != HandleType::NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::appendPartsToModel( cvf::ModelBasicList* model )
{
    if ( m_handleParts.empty() )
    {
        recreateAllGeometryAndParts();
    }

    if ( m_isGeometryUpdateNeeded )
    {
        createGeometryOnly();
    }

    for ( auto& idPartIt : m_handleParts )
    {
        model->addPart( idPartIt.second.p() );
    }

    for ( auto activeModePart : m_activeDragModeParts )
    {
        model->addPart( activeModePart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::tryToActivateManipulator( const cvf::HitItem* hitItem )
{
    endManipulator();

    if ( !hitItem ) return;

    const cvf::Part* pickedPart        = hitItem->part();
    const cvf::Vec3d intersectionPoint = hitItem->intersectionPoint();

    if ( !pickedPart ) return;

    for ( auto& idPartIt : m_handleParts )
    {
        if ( pickedPart == idPartIt.second.p() )
        {
            m_initialPickPoint           = intersectionPoint;
            m_tangentOnStartManipulation = m_tangent;
            m_originOnStartManipulation  = m_origin;
            m_activeHandle               = idPartIt.first;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Calculate new origin and tangent based on the new ray position
/// Clear geometry to trigger regeneration
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::updateManipulatorFromRay( const cvf::Ray* newMouseRay )
{
    if ( !isManipulatorActive() ) return;

    if ( m_activeHandle == HandleType::PRESCRIBED_POLYLINE )
    {
        cvf::Plane plane;
        plane.setFromPointAndNormal( m_origin, newMouseRay->direction() );
        cvf::Vec3d newIntersection;
        newMouseRay->planeIntersect( plane, &newIntersection );

        const cvf::Vec3d newOrigin = m_originOnStartManipulation + ( newIntersection - m_initialPickPoint );

        double     closestDistance = std::numeric_limits<double>::max();
        cvf::Vec3d closestPoint;

        for ( size_t i = 1; i < m_polyline.size(); i++ )
        {
            const auto& p1 = m_polyline[i];
            const auto& p2 = m_polyline[i - 1];

            double     normalizedIntersection;
            const auto pointOnLine = cvf::GeometryTools::projectPointOnLine( p1, p2, newOrigin, &normalizedIntersection );

            const double candidateDistance = pointOnLine.pointDistanceSquared( newOrigin );
            if ( candidateDistance < closestDistance )
            {
                closestDistance = candidateDistance;
                closestPoint    = pointOnLine;
            }
        }

        m_origin = closestPoint;
    }
    else if ( m_activeHandle == HandleType::HORIZONTAL_PLANE )
    {
        cvf::Plane plane;
        plane.setFromPointAndNormal( m_origin, cvf::Vec3d::Z_AXIS );
        cvf::Vec3d newIntersection;
        newMouseRay->planeIntersect( plane, &newIntersection );

        cvf::Vec3d newOrigin = m_originOnStartManipulation + ( newIntersection - m_initialPickPoint );

        m_origin = newOrigin;
    }
    else if ( m_activeHandle == HandleType::VERTICAL_AXIS )
    {
        cvf::Plane plane;
        cvf::Vec3d planeNormal = ( newMouseRay->direction() ^ cvf::Vec3d::Z_AXIS ) ^ cvf::Vec3d::Z_AXIS;
        double     length      = planeNormal.length();

        if ( length < 1e-5 ) return;

        planeNormal /= length;
        plane.setFromPointAndNormal( m_initialPickPoint, planeNormal );
        cvf::Vec3d newIntersection;
        newMouseRay->planeIntersect( plane, &newIntersection );

        cvf::Vec3d newOrigin = m_originOnStartManipulation;
        newOrigin.z() += ( newIntersection.z() - m_initialPickPoint.z() );

        m_origin = newOrigin;
    }

    m_isGeometryUpdateNeeded = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::endManipulator()
{
    m_activeHandle = HandleType::NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::recreateAllGeometryAndParts()
{
    if ( m_polyline.empty() )
    {
        createHorizontalPlaneHandle();
        createVerticalAxisHandle();
    }
    else
    {
        createPolylineHandle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::createGeometryOnly()
{
    if ( m_polyline.empty() )
    {
        m_handleParts[HandleType::HORIZONTAL_PLANE]->setDrawable( createHorizontalPlaneGeo().p() );
        m_handleParts[HandleType::VERTICAL_AXIS]->setDrawable( createVerticalAxisGeo().p() );
    }
    else
    {
        m_handleParts[HandleType::PRESCRIBED_POLYLINE]->setDrawable( createPolylineGeo().p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::createHorizontalPlaneHandle()
{
    using namespace cvf;

    ref<cvf::DrawableGeo> geo = createHorizontalPlaneGeo();

    HandleType   handleId = HandleType::HORIZONTAL_PLANE;
    cvf::Color4f color    = cvf::Color4f( 1.0f, 0.0f, 1.0f, 0.7f );
    cvf::String  partName( "PointTangentManipulator Horizontal Plane Handle" );

    addHandlePart( geo.p(), color, handleId, partName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createHorizontalPlaneGeo()
{
    using namespace cvf;

    cvf::ref<cvf::Vec3fArray> vertexArray         = new cvf::Vec3fArray( 16 );
    cvf::ref<cvf::Vec3fArray> triangleVertexArray = new cvf::Vec3fArray( 16 * 3 );

    float cos_pi_12  = cvf::Math::cos( 0.5 * cvf::PI_F / 12 );
    float sin_pi_12  = cvf::Math::sin( 0.5 * cvf::PI_F / 12 );
    float cos_3pi_12 = cvf::Math::cos( 3 * cvf::PI_F / 12 );
    float sin_3pi_12 = cvf::Math::sin( 3 * cvf::PI_F / 12 );
    float cos_5pi_12 = cvf::Math::cos( 5.5 * cvf::PI_F / 12 );
    float sin_5pi_12 = cvf::Math::sin( 5.5 * cvf::PI_F / 12 );

    vertexArray->set( 0, { 1.25, 0, 0 } );
    vertexArray->set( 1, { cos_pi_12, sin_pi_12, 0 } );
    vertexArray->set( 2, { cos_3pi_12, sin_3pi_12, 0 } );
    vertexArray->set( 3, { cos_5pi_12, sin_5pi_12, 0 } );
    vertexArray->set( 4, { 0, 1.25, 0 } );
    vertexArray->set( 5, { -cos_5pi_12, sin_5pi_12, 0 } );
    vertexArray->set( 6, { -cos_3pi_12, sin_3pi_12, 0 } );
    vertexArray->set( 7, { -cos_pi_12, sin_pi_12, 0 } );
    vertexArray->set( 8, { -1.25, 0, 0 } );
    vertexArray->set( 9, { -cos_pi_12, -sin_pi_12, 0 } );
    vertexArray->set( 10, { -cos_3pi_12, -sin_3pi_12, 0 } );
    vertexArray->set( 11, { -cos_5pi_12, -sin_5pi_12, 0 } );
    vertexArray->set( 12, { 0, -1.25, 0 } );
    vertexArray->set( 13, { cos_5pi_12, -sin_5pi_12, 0 } );
    vertexArray->set( 14, { cos_3pi_12, -sin_3pi_12, 0 } );
    vertexArray->set( 15, { cos_pi_12, -sin_pi_12, 0 } );

    Vec3f origin( m_origin );
    for ( cvf::Vec3f& vx : *vertexArray )
    {
        vx *= 0.4 * m_handleSize;
        vx += origin;
    }

    unsigned tVxIdx = 0;
    for ( unsigned vxIdx = 0; vxIdx < vertexArray->size(); ++vxIdx )
    {
        cvf::Vec3f vx = ( *vertexArray )[vxIdx];

        unsigned idx2 = vxIdx + 1;
        if ( idx2 >= vertexArray->size() ) idx2 = 0;

        cvf::Vec3f vx2 = ( *vertexArray )[idx2];

        triangleVertexArray->set( tVxIdx, origin );
        tVxIdx++;
        triangleVertexArray->set( tVxIdx, vx );
        tVxIdx++;
        triangleVertexArray->set( tVxIdx, vx2 );
        tVxIdx++;
    }

    return createTriangelDrawableGeo( triangleVertexArray.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::createVerticalAxisHandle()
{
    using namespace cvf;
    cvf::ref<cvf::DrawableGeo> geo = createVerticalAxisGeo();

    HandleType   handleId = HandleType::VERTICAL_AXIS;
    cvf::Color4f color    = cvf::Color4f( 0.0f, 0.7f, 0.8f, 0.7f );
    cvf::String  partName( "PointTangentManipulator Vertical Axis Handle" );

    addHandlePart( geo.p(), color, handleId, partName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createVerticalAxisGeo()
{
    using namespace cvf;

    cvf::ref<cvf::GeometryBuilderTriangles> geomBuilder = new cvf::GeometryBuilderTriangles;

    cvf::GeometryUtils::createObliqueCylinder( 0.3f, 0.05f, 1.0f, 0.0f, 0.0, 12, true, true, true, 1, geomBuilder.p() );

    float s = 0.5 * m_handleSize;
    Vec3f origin( m_origin );

    geomBuilder->transformVertexRange( 0, geomBuilder->vertexCount() - 1, cvf::Mat4f::fromScaling( { s, s, s } ) );
    geomBuilder->transformVertexRange( 0, geomBuilder->vertexCount() - 1, cvf::Mat4f::fromTranslation( origin ) );

    unsigned vxArraySizeFirstCylinder = geomBuilder->vertexCount();

    cvf::GeometryUtils::createObliqueCylinder( 0.05f, 0.3f, 1.0f, 0.0f, 0.0, 12, true, true, true, 1, geomBuilder.p() );

    geomBuilder->transformVertexRange( vxArraySizeFirstCylinder,
                                       geomBuilder->vertexCount() - 1,
                                       cvf::Mat4f::fromTranslation( { 0.0f, 0.0f, -1.0f } ) );
    geomBuilder->transformVertexRange( vxArraySizeFirstCylinder,
                                       geomBuilder->vertexCount() - 1,
                                       cvf::Mat4f::fromScaling( { s, s, s } ) );
    geomBuilder->transformVertexRange( vxArraySizeFirstCylinder,
                                       geomBuilder->vertexCount() - 1,
                                       cvf::Mat4f::fromTranslation( origin ) );

    cvf::ref<cvf::Vec3fArray> vertexArray = geomBuilder->vertices();
    cvf::ref<cvf::UIntArray>  indexArray  = geomBuilder->triangles();

    return createIndexedTriangelDrawableGeo( vertexArray.p(), indexArray.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::createPolylineHandle()
{
    cvf::ref<cvf::DrawableGeo> geo = createPolylineGeo();

    HandleType   handleId = HandleType::PRESCRIBED_POLYLINE;
    cvf::Color4f color    = cvf::Color4f( 0.8f, 0.7f, 0.8f, 0.7f );
    cvf::String  partName( "PointTangentManipulator Polyline Handle" );

    addHandlePart( geo.p(), color, handleId, partName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createPolylineGeo()
{
    using namespace cvf;

    cvf::ref<cvf::GeometryBuilderTriangles> geomBuilder = new cvf::GeometryBuilderTriangles;

    double radius = m_handleSize * 0.3;
    cvf::GeometryUtils::createSphere( radius, 10, 10, geomBuilder.p() );

    Vec3f origin( m_origin );

    geomBuilder->transformVertexRange( 0, geomBuilder->vertexCount() - 1, cvf::Mat4f::fromTranslation( origin ) );

    cvf::ref<cvf::Vec3fArray> vertexArray = geomBuilder->vertices();
    cvf::ref<cvf::UIntArray>  indexArray  = geomBuilder->triangles();

    return createIndexedTriangelDrawableGeo( vertexArray.p(), indexArray.p() );
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
cvf::ref<cvf::DrawableGeo>
    RicPointTangentManipulatorPartMgr::createIndexedTriangelDrawableGeo( cvf::Vec3fArray* triangleVertexArray,
                                                                         cvf::UIntArray*  triangleIndices )
{
    using namespace cvf;
    ref<DrawableGeo>             geo     = new DrawableGeo;
    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt( PT_TRIANGLES, triangleIndices );

    geo->setVertexArray( triangleVertexArray );
    geo->addPrimitiveSet( primSet.p() );
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createTriangelDrawableGeo( cvf::Vec3fArray* triangleVertexArray )
{
    using namespace cvf;
    ref<DrawableGeo> geo = new DrawableGeo;

    geo->setVertexArray( triangleVertexArray );
    ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect( cvf::PT_TRIANGLES );
    primSet->setIndexCount( triangleVertexArray->size() );

    geo->addPrimitiveSet( primSet.p() );
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::addHandlePart( cvf::DrawableGeo*   geo,
                                                       const cvf::Color4f& color,
                                                       HandleType          handleId,
                                                       const cvf::String&  partName )
{
    cvf::ref<cvf::Part> handlePart = createPart( geo, color, partName );
    m_handleParts[handleId]        = handlePart;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::addActiveModePart( cvf::DrawableGeo*   geo,
                                                           const cvf::Color4f& color,
                                                           HandleType          handleId,
                                                           const cvf::String&  partName )
{
    cvf::ref<cvf::Part> handlePart = createPart( geo, color, partName );

    m_activeDragModeParts.push_back( handlePart.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RicPointTangentManipulatorPartMgr::createPart( cvf::DrawableGeo*   geo,
                                                                   const cvf::Color4f& color,
                                                                   const cvf::String&  partName )
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName( partName );
    part->setDrawable( geo );
    part->updateBoundingBox();

    caf::SurfaceEffectGenerator surfaceGen( color, caf::PO_1 );
    cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();
    part->setEffect( eff.p() );
    if ( color.a() < 1.0 ) part->setPriority( RivPartPriority::Transparent );

    return part;
}
