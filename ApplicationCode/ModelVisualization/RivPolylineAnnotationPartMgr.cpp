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

#include "RimPolylinesAnnotation.h"
#include "RimAnnotationInViewCollection.h"
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

static RimSimWellInViewCollection* simWellInViewCollection() { return nullptr; }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivPolylineAnnotationPartMgr::RivPolylineAnnotationPartMgr(RimPolylinesAnnotation* annotation)
: m_rimAnnotation(annotation)
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
void RivPolylineAnnotationPartMgr::buildPolygonAnnotationParts(const caf::DisplayCoordTransform* displayXf)
{
    clearAllGeometry();

    if (!m_rimAnnotation->isEmpty())
    {
        const auto& points = m_rimAnnotation->polyLinesData();

        auto linesInDisplayCoords =  points->polyLines();

        for (auto& line : linesInDisplayCoords)
        {
            for ( cvf::Vec3d& point : line)
            {
                point = displayXf->transformToDisplayCoord(point);
            }
        }

        cvf::ref<cvf::DrawableGeo> drawableGeo = RivPolylineGenerator::createLineAlongPolylineDrawable(linesInDisplayCoords);
        cvf::ref<cvf::Part> part = new cvf::Part;
        //part->setName("RivAnnotationPartMgr: text " + cvfString);
        part->setDrawable(drawableGeo.p());

        caf::MeshEffectGenerator colorEffgen(cvf::Color3f::RED);
        cvf::ref<cvf::Effect> eff = colorEffgen.generateCachedEffect();

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::MeshLines);  

        cvf::ref<RivPolylinesAnnotationSourceInfo> sourceInfo = new RivPolylinesAnnotationSourceInfo(m_rimAnnotation);
        part->setSourceInfo(sourceInfo.p());

        m_part = part;
    }
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
void RivPolylineAnnotationPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                                           const caf::DisplayCoordTransform * displayXf)
{
    if (m_rimAnnotation.isNull()) return;
    if (m_rimAnnotation->isEmpty()) return;

    buildPolygonAnnotationParts(displayXf);
    model->addPart(m_part.p());
}

