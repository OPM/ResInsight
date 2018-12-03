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

#include "RivPolylineAnnotationPartMgr.h"

#include "RiaBoundingBoxTools.h"

#include "Rim3dView.h"
#include "RimAnnotationCollection.h"
#include "RimPolylinesAnnotation.h"
#include "RimPolylinesAnnotationInView.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationLineAppearance.h"

#include "RigPolyLinesData.h"

#include "RivPolylineGenerator.h"
#include "RivPartPriority.h"
#include "RivPolylinesAnnotationSourceInfo.h"


#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"
#include "cafDisplayCoordTransform.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivPolylineAnnotationPartMgr::RivPolylineAnnotationPartMgr(Rim3dView* view, RimPolylinesAnnotationInView* annotationInView)
: m_rimView(view), m_rimAnnotationInView(annotationInView)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivPolylineAnnotationPartMgr::~RivPolylineAnnotationPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPolylineAnnotationPartMgr::buildPolylineAnnotationParts(const caf::DisplayCoordTransform* displayXf)
{
    clearAllGeometry();

    auto rimAnnotation = m_rimAnnotationInView->sourceAnnotation();
    if (!rimAnnotation->isEmpty() && rimAnnotation->isActive())
    {
        auto        lineColor     = rimAnnotation->appearance()->color();
        auto        isDashedLine  = rimAnnotation->appearance()->isDashed();
        auto        lineThickness = rimAnnotation->appearance()->thickness();

        auto* collection = annotationCollection();
        if (!collection) return;

        auto linesInDomain = getPolylinesPointsInDomain(collection->snapAnnotations(), collection->annotationPlaneZ());
        auto linesInDisplay = transformPolylinesPointsToDisplay(linesInDomain, displayXf);

        cvf::ref<cvf::DrawableGeo> drawableGeo = RivPolylineGenerator::createLineAlongPolylineDrawable(linesInDisplay);
        cvf::ref<cvf::Part> part = new cvf::Part;
        //part->setName("RivAnnotationPartMgr: text " + cvfString);
        part->setDrawable(drawableGeo.p());

        caf::MeshEffectGenerator effgen(lineColor);
        effgen.setLineWidth(lineThickness);
        if (isDashedLine) effgen.setLineStipple(true);
        cvf::ref<cvf::Effect> eff = effgen.generateCachedEffect();

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::MeshLines);  

        cvf::ref<RivPolylinesAnnotationSourceInfo> sourceInfo = new RivPolylinesAnnotationSourceInfo(rimAnnotation);
        part->setSourceInfo(sourceInfo.p());

        m_part = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<RivPolylineAnnotationPartMgr::Vec3d>>
    RivPolylineAnnotationPartMgr::getPolylinesPointsInDomain(bool snapToPlaneZ, double planeZ)
{
    auto polylines = m_rimAnnotationInView->sourceAnnotation()->polyLinesData()->polyLines();
    if (!snapToPlaneZ) return polylines;

    std::vector<std::vector<Vec3d>> polylinesInDisplay;
    for (const auto& pts : polylines)
    {
        std::vector<Vec3d> polyline;
        for (const auto& pt : pts)
        {
            auto ptInDisp = pt;
            ptInDisp.z() = planeZ;
            polyline.push_back(ptInDisp);
        }
        polylinesInDisplay.push_back(polyline);
    }
    return polylinesInDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RivPolylineAnnotationPartMgr::transformPolylinesPointsToDisplay(
    const std::vector<std::vector<Vec3d>>& pointsInDomain,
    const caf::DisplayCoordTransform* displayXf)
{
    std::vector<std::vector<Vec3d>> pointsInDisplay;
    for (const auto& pts : pointsInDomain)
    {
        std::vector<Vec3d> polyline;
        for (const auto& pt : pts)
        {
            polyline.push_back(displayXf->transformToDisplayCoord(pt));
        }
        pointsInDisplay.push_back(polyline);

    }
    return pointsInDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivPolylineAnnotationPartMgr::isPolylinesInBoundingBox(const cvf::BoundingBox& boundingBox)
{
    auto coll = annotationCollection();
    if (!coll) return false;

    auto effectiveBoundingBox = RiaBoundingBoxTools::inflate(boundingBox, 3);
    for (const auto& pts : getPolylinesPointsInDomain(coll->snapAnnotations(), coll->annotationPlaneZ()))
    {
        for (const auto& pt : pts)
        {
            if (effectiveBoundingBox.contains(pt)) return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPolylineAnnotationPartMgr::clearAllGeometry()
{
    m_part = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection* RivPolylineAnnotationPartMgr::annotationCollection() const
{
    std::vector<RimAnnotationInViewCollection*> colls;
    m_rimView->descendantsIncludingThisOfType(colls);
    return !colls.empty() ? colls.front() : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPolylineAnnotationPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                                                     const caf::DisplayCoordTransform * displayXf,
                                                                     const cvf::BoundingBox& boundingBox)
{
    auto rimAnnotation = m_rimAnnotationInView->sourceAnnotation();
    if (!rimAnnotation) return;
    if (rimAnnotation->isEmpty()) return;
    if (!m_rimAnnotationInView->isVisible()) return;

    // Check bounding box
    if (!isPolylinesInBoundingBox(boundingBox)) return;

    buildPolylineAnnotationParts(displayXf);

    if ( m_part.notNull() )
    {
        model->addPart(m_part.p());
    }
}

