/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RivGeoMechVizLogic.h"

#include "RimGeoMechView.h"
#include "cvfModelBasicList.h"
#include "RimGeoMechResultSlot.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivGeoMechPartMgr.h"
#include "RivReservoirViewPartMgr.h"
#include "RimGeoMechCase.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RimCellRangeFilterCollection.h"

#include "RivCellSetEnum.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechVizLogic::RivGeoMechVizLogic(RimGeoMechView * geomView)
{
    CVF_ASSERT(geomView);
    m_geomechView = geomView;
    m_partMgrCache = new RivGeoMechPartMgrCache;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechVizLogic::~RivGeoMechVizLogic()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::appendNoAnimPartsToModel(cvf::ModelBasicList* model)
{
    RivGeoMechPartMgrCache::Key pMgrKey = currentPartMgrKey();

    RivGeoMechPartMgr* currentGeoMechPartMgr = m_partMgrCache->partMgr(pMgrKey);
    RigGeoMechCaseData* caseData = m_geomechView->geoMechCase()->geoMechData();
    int partCount = caseData->femParts()->partCount();

    if (m_partMgrCache->needsRegeneration(pMgrKey))
    {
        if (currentGeoMechPartMgr->initializedFemPartCount() != partCount)
        {
            currentGeoMechPartMgr->clearAndSetReservoir(caseData);
        }

        for (int femPartIdx = 0; femPartIdx < partCount; ++femPartIdx)
        {
            cvf::ref<cvf::UByteArray> elmVisibility =  currentGeoMechPartMgr->cellVisibility(femPartIdx);
            currentGeoMechPartMgr->setTransform(m_geomechView->scaleTransform());

            if (pMgrKey.geometryType() == RANGE_FILTERED)
            {
                cvf::CellRangeFilter cellRangeFilter;
                m_geomechView->rangeFilterCollection()->compoundCellRangeFilter(&cellRangeFilter, femPartIdx);
                RivElmVisibilityCalculator::computeRangeVisibility(elmVisibility.p(), caseData->femParts()->part(femPartIdx), cellRangeFilter);
            }
            else
            {
                RivElmVisibilityCalculator::computeAllVisible(elmVisibility.p(), caseData->femParts()->part(femPartIdx));
            }

            currentGeoMechPartMgr->setCellVisibility(femPartIdx, elmVisibility.p());
        }

        m_partMgrCache->generationFinished(pMgrKey);
    }
  
    currentGeoMechPartMgr->appendGridPartsToModel(model);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::appendPartsToModel( int timeStepIndex, cvf::ModelBasicList* model)
{
    appendNoAnimPartsToModel(model);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::updateCellResultColor(size_t timeStepIndex, RimGeoMechResultSlot* cellResultSlot)
{
    RivGeoMechPartMgrCache::Key pMgrKey = currentPartMgrKey();
    RivGeoMechPartMgr*  partMgr = m_partMgrCache->partMgr(pMgrKey);
    partMgr->updateCellResultColor(timeStepIndex, cellResultSlot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::updateStaticCellColors()
{
    RivGeoMechPartMgrCache::Key pMgrKey = currentPartMgrKey();
    RivGeoMechPartMgr*  partMgr = m_partMgrCache->partMgr(pMgrKey);
    partMgr->updateCellColor(cvf::Color4f(cvf::Color3f::ORANGE));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::scheduleGeometryRegen(RivCellSetEnum geometryType)
{
    switch (geometryType)
    {
        case RANGE_FILTERED:
        m_partMgrCache->scheduleRegeneration(RivGeoMechPartMgrCache::Key(RANGE_FILTERED, 0));
        break;
        case RANGE_FILTERED_INACTIVE:
        break;
        case PROPERTY_FILTERED:
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgrCache::Key RivGeoMechVizLogic::currentPartMgrKey()
{
    RivGeoMechPartMgrCache::Key pMgrKey;
    
    if (m_geomechView->rangeFilterCollection()->hasActiveFilters())
    {
        pMgrKey.set(RANGE_FILTERED, 0);
    }
    else
    {
        pMgrKey.set(ALL_CELLS, 0);
    }

    return pMgrKey;
}


