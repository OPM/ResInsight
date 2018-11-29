/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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



#include "RivReachCircleAnnotationPartMgr.h"

#include "RiaBoundingBoxTools.h"

#include "Rim3dView.h"
#include "RimAnnotationInViewCollection.h"
#include "RimReachCircleAnnotation.h"

#include "RivPolylineGenerator.h"
#include "RivPartPriority.h"
#include "RivReachCircleAnnotationSourceInfo.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cafDisplayCoordTransform.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReachCircleAnnotationPartMgr::RivReachCircleAnnotationPartMgr(Rim3dView* view, RimReachCircleAnnotation* annotation)
: m_rimView(view), m_rimAnnotation(annotation)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReachCircleAnnotationPartMgr::~RivReachCircleAnnotationPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReachCircleAnnotationPartMgr::buildParts(const caf::DisplayCoordTransform* displayXf, bool doFlatten, double xOffset)
{
    clearAllGeometry();

    cvf::ref<RivReachCircleAnnotationSourceInfo> sourceInfo = new RivReachCircleAnnotationSourceInfo(m_rimAnnotation);

    Vec3d centerPositionInDomain = m_rimAnnotation->centerPoint();

    auto    lineColor      = m_rimAnnotation->appearance()->color();
    auto    isDashedLine   = m_rimAnnotation->appearance()->isDashed();
    auto    lineThickness = m_rimAnnotation->appearance()->thickness();

    // Circle part
    auto* collection = annotationCollection();
    if(collection)
    {
        std::vector<Vec3d> pointsInDomain = computeCirclePointsInDomain(collection->snapAnnotations(), collection->annotationPlaneZ());
        std::vector<Vec3d> points = transformCirclePointsToDisplay(pointsInDomain, displayXf);

        cvf::ref<cvf::DrawableGeo> drawableGeo = RivPolylineGenerator::createLineAlongPolylineDrawable(points);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawableGeo.p());

        caf::MeshEffectGenerator effgen(lineColor);
        effgen.setLineWidth(lineThickness);
        if (isDashedLine) effgen.setLineStipple(true);                      // Currently, dashed lines are not supported
        cvf::ref<cvf::Effect>    eff = effgen.generateUnCachedEffect();

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::MeshLines);
        part->setSourceInfo(sourceInfo.p());

        m_circlePart = part;
    }

    // Center point part
    {
        auto centerPos = m_rimAnnotation->centerPoint();
        double symbolSize = 20;
        double xMin = centerPos.x() - symbolSize / 2.0;
        double xMax = xMin + symbolSize;
        double yMin = centerPos.y() - symbolSize / 2.0;
        double yMax = yMin + symbolSize;
        double z = centerPos.z();
        std::vector<Vec3d> line1 = { {xMin, yMin, z}, {xMax, yMax, z} };
        std::vector<Vec3d> line2 = { {xMax, yMin, z}, {xMin, yMax, z} };
        std::vector<std::vector<Vec3d>> symbol = { line1, line2 };
        cvf::ref<cvf::DrawableGeo> drawableGeo = RivPolylineGenerator::createLineAlongPolylineDrawable(symbol);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawableGeo.p());

        caf::MeshEffectGenerator effgen(lineColor);
        effgen.setLineWidth(2);
        cvf::ref<cvf::Effect> eff = effgen.generateUnCachedEffect();

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::MeshLines);
        part->setSourceInfo(sourceInfo.p());

        m_centerPointPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReachCircleAnnotationPartMgr::clearAllGeometry()
{
    m_circlePart  = nullptr;
    m_centerPointPart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReachCircleAnnotationPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList*              model,
                                                                        const caf::DisplayCoordTransform* displayXf,
                                                                        const cvf::BoundingBox& boundingBox)
{
    if (m_rimAnnotation.isNull()) return;
    if (!m_rimAnnotation->isActive()) return;

    // Check bounding box
    if (!isCircleInBoundingBox(boundingBox)) return;

    if (!validateAnnotation(m_rimAnnotation)) return;

    buildParts(displayXf, false, 0.0);
    model->addPart(m_circlePart.p());
    model->addPart(m_centerPointPart.p());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivReachCircleAnnotationPartMgr::validateAnnotation(const RimReachCircleAnnotation* annotation) const
{
    return m_rimAnnotation->centerPoint() != cvf::Vec3d::ZERO && m_rimAnnotation->radius() > 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivReachCircleAnnotationPartMgr::isCircleInBoundingBox(const cvf::BoundingBox& boundingBox)
{
    auto coll = annotationCollection();
    if (!coll) return false;

    auto effectiveBoundingBox = RiaBoundingBoxTools::inflate(boundingBox, 3);
    auto points = computeCirclePointsInDomain(coll->snapAnnotations(), coll->annotationPlaneZ());
    for (const auto& pt : points)
    {
        if (effectiveBoundingBox.contains(pt)) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RivReachCircleAnnotationPartMgr::computeCirclePointsInDomain(bool snapToPlaneZ, double planeZ)
{
    int  numPoints = 36;
    auto centerPos = m_rimAnnotation->centerPoint();
    auto radius = m_rimAnnotation->radius();

    if (snapToPlaneZ)
    {
        centerPos.z() = planeZ;
    }

    std::vector<Vec3d> points;
    for (int i = 0; i < numPoints; i++)
    {
        double rad = 2 * cvf::PI_D * (double)i / (double)numPoints;
        Vec3d  pt(centerPos.x() + cos(rad) * radius, centerPos.y() + sin(rad) * radius, centerPos.z());
        points.push_back(pt);
    }
    points.push_back(points.front());
    return points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RivReachCircleAnnotationPartMgr::Vec3d>
    RivReachCircleAnnotationPartMgr::transformCirclePointsToDisplay(const std::vector<Vec3d>& pointsInDomain,
                                                                    const caf::DisplayCoordTransform* displayXf)
{
    std::vector<Vec3d> pointsInDisplay;
    for (const auto& pt : pointsInDomain) pointsInDisplay.push_back(displayXf->transformToDisplayCoord(pt));
    return pointsInDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection* RivReachCircleAnnotationPartMgr::annotationCollection() const
{
    std::vector<RimAnnotationInViewCollection*> colls;
    m_rimView->descendantsIncludingThisOfType(colls);
    return !colls.empty() ? colls.front() : nullptr;
}
