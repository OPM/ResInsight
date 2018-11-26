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



#include "RivAnnotationsPartMgr.h"

#include "RiaApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"

#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimPolylineAnnotation.h"
#include "RimAnnotationInViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimSimWellInViewCollection.h"
#include "RimSimWellInView.h"

#include "RivTextAnnotationPartMgr.h"
#include "RivReachCircleAnnotationPartMgr.h"
#include "RivPolylineAnnotationPartMgr.h"
#include "RivPipeGeometryGenerator.h"
#include "RivPartPriority.h"
#include "RivSimWellPipeSourceInfo.h"

#include "cafEffectGenerator.h"

#include "cvfArrowGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"
#include "cafDisplayCoordTransform.h"
#include "RivSectionFlattner.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivAnnotationsPartMgr::RivAnnotationsPartMgr(Rim3dView* view)
: m_rimView(view)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivAnnotationsPartMgr::~RivAnnotationsPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationsPartMgr::appendGeometryPartsToModel(cvf::ModelBasicList*              model,
                                                       const caf::DisplayCoordTransform* displayCoordTransform)
{
     createAnnotationPartManagers();

    for (auto& partMgr : m_textAnnotationPartMgrs)
    {
        partMgr->appendDynamicGeometryPartsToModel(model, displayCoordTransform);
    }
    for (auto& partMgr : m_reachCircleAnnotationPartMgrs)
    {
        partMgr->appendDynamicGeometryPartsToModel(model, displayCoordTransform);
    }
    for (auto& partMgr : m_polylineAnnotationPartMgrs)
    {
        partMgr->appendDynamicGeometryPartsToModel(model, displayCoordTransform);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationsPartMgr::createAnnotationPartManagers()
{
    RimProject* proj        = RiaApplication::instance()->project();
    auto        textAnnotations = proj->textAnnotations();
    auto        reachCircleAnnotations = proj->reachCircleAnnotations();
    auto        polylineAnnotations = proj->polylineAnnotations();

    clearGeometryCache();

    if (m_textAnnotationPartMgrs.size() != textAnnotations.size())
    {
        for (auto annotation : textAnnotations)
        {
            auto* apm = new RivTextAnnotationPartMgr(annotation);
            m_textAnnotationPartMgrs.push_back(apm);
            //m_mapFromViewToIndex[wellPath] = wppm;
        }
    }
    if (m_reachCircleAnnotationPartMgrs.size() != reachCircleAnnotations.size())
    {
        for (auto annotation : reachCircleAnnotations)
        {
            auto* apm = new RivReachCircleAnnotationPartMgr(annotation);
            m_reachCircleAnnotationPartMgrs.push_back(apm);
            // m_mapFromViewToIndex[wellPath] = wppm;
        }
    }
    if (m_polylineAnnotationPartMgrs.size() != polylineAnnotations.size())
    {
        for (auto annotation : polylineAnnotations)
        {
            auto* apm = new RivPolylineAnnotationPartMgr(annotation);
            m_polylineAnnotationPartMgrs.push_back(apm);
            // m_mapFromViewToIndex[wellPath] = wppm;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivAnnotationsPartMgr::clearGeometryCache()
{
    m_textAnnotationPartMgrs.clear();
    m_reachCircleAnnotationPartMgrs.clear();
    m_polylineAnnotationPartMgrs.clear();
}
