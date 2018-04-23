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

#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimCellRangeFilterCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "Rim3dView.h"
#include "RimViewController.h"

#include "RivCellSetEnum.h"
#include "RivFemElmVisibilityCalculator.h"
#include "RivGeoMechPartMgr.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivReservoirViewPartMgr.h"

#include "cvfModelBasicList.h"
#include "cvfStructGridGeometryGenerator.h"

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
   this->appendPartsToModel(-1, model);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::appendPartsToModel(int timeStepIndex, cvf::ModelBasicList* model)
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs(timeStepIndex);
    for (size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx)
    {
        RivGeoMechPartMgr*  partMgr = getUpdatedPartMgr(visiblePartMgrs[pmIdx]);

        partMgr->appendGridPartsToModel(model);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::updateCellResultColor(int timeStepIndex, RimGeoMechCellColors* cellResultColors)
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs(timeStepIndex);
    for (size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx)
    {
        RivGeoMechPartMgr*  partMgr = m_partMgrCache->partMgr(visiblePartMgrs[pmIdx]);
        partMgr->updateCellResultColor(timeStepIndex, cellResultColors);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::updateStaticCellColors(int timeStepIndex)
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs(timeStepIndex);
    for (size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx)
    {
        RivGeoMechPartMgr*  partMgr = m_partMgrCache->partMgr(visiblePartMgrs[pmIdx]);
        partMgr->updateCellColor(cvf::Color4f(cvf::Color3f::ORANGE));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::scheduleGeometryRegen(RivCellSetEnum geometryType)
{
    this->scheduleRegenOfDirectlyDependentGeometry(geometryType);

    int frameCount = 0;
    if (m_geomechView->geoMechCase() && m_geomechView->geoMechCase()->geoMechData())
    {
        frameCount = m_geomechView->geoMechCase()->geoMechData()->femPartResults()->frameCount();
    }

    for (int fIdx = -1; fIdx < frameCount; ++fIdx)
    {
        RivGeoMechPartMgrCache::Key geomToRegen(geometryType, fIdx);
        m_partMgrCache->scheduleRegeneration(geomToRegen);
    }
}

void RivGeoMechVizLogic::scheduleRegenOfDirectlyDependentGeometry(RivCellSetEnum geometryType)
{
    if (geometryType == RANGE_FILTERED)
    {
        this->scheduleGeometryRegen(PROPERTY_FILTERED);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RivGeoMechPartMgrCache::Key> RivGeoMechVizLogic::keysToVisiblePartMgrs(int timeStepIndex) const
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs;
    if (m_geomechView->viewController() && m_geomechView->viewController()->isVisibleCellsOveridden())
    {
        visiblePartMgrs.push_back(RivGeoMechPartMgrCache::Key(OVERRIDDEN_CELL_VISIBILITY, -1));
    }
    else if ( m_geomechView->isGridVisualizationMode() )
    {
        if ( timeStepIndex >= 0 && m_geomechView->geoMechPropertyFilterCollection()->hasActiveFilters() )
        {
            visiblePartMgrs.push_back(RivGeoMechPartMgrCache::Key(PROPERTY_FILTERED, timeStepIndex));
        }
        else if ( m_geomechView->rangeFilterCollection()->hasActiveFilters() )
        {
            visiblePartMgrs.push_back(RivGeoMechPartMgrCache::Key(RANGE_FILTERED, -1));
        }
        else
        {
            visiblePartMgrs.push_back(RivGeoMechPartMgrCache::Key(ALL_CELLS, -1));
        }
    }
    return visiblePartMgrs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::ref<RivGeoMechPartMgrCache> RivGeoMechVizLogic::partMgrCache() const
{
    return m_partMgrCache;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgr* RivGeoMechVizLogic::getUpdatedPartMgr(RivGeoMechPartMgrCache::Key pMgrKey)
{
    if (!m_partMgrCache->isNeedingRegeneration(pMgrKey))
    {
         return m_partMgrCache->partMgr(pMgrKey);
    }

    RivGeoMechPartMgr* partMgrToUpdate = m_partMgrCache->partMgr(pMgrKey);
    int partCount = 0;
    RigGeoMechCaseData* caseData = nullptr;
    if ( m_geomechView->geoMechCase() )
    {
        caseData = m_geomechView->geoMechCase()->geoMechData();
        partCount = caseData->femParts()->partCount();
    }

    if (partMgrToUpdate->initializedFemPartCount() != partCount)
    {
        partMgrToUpdate->clearAndSetReservoir(caseData);
    }

    for (int femPartIdx = 0; femPartIdx < partCount; ++femPartIdx)
    {
        cvf::ref<cvf::UByteArray> elmVisibility =  partMgrToUpdate->cellVisibility(femPartIdx);
        partMgrToUpdate->setTransform(m_geomechView->scaleTransform());

        if (pMgrKey.geometryType() == RANGE_FILTERED)
        {
            cvf::CellRangeFilter cellRangeFilter;
            m_geomechView->rangeFilterCollection()->compoundCellRangeFilter(&cellRangeFilter, femPartIdx);
            RivFemElmVisibilityCalculator::computeRangeVisibility(  elmVisibility.p(), 
                                                                    caseData->femParts()->part(femPartIdx), 
                                                                    cellRangeFilter);
        }
        else if (pMgrKey.geometryType() == PROPERTY_FILTERED)
        {
            RivGeoMechPartMgr* rangefiltered = nullptr;
            if (m_geomechView->rangeFilterCollection()->hasActiveFilters())
            {
                rangefiltered = getUpdatedPartMgr(RivGeoMechPartMgrCache::Key(RANGE_FILTERED, -1));
            }
            else
            {
                rangefiltered = getUpdatedPartMgr(RivGeoMechPartMgrCache::Key(ALL_CELLS, -1));
            }
            cvf::ref<cvf::UByteArray> rangeFiltVisibility = rangefiltered->cellVisibility(femPartIdx);

            RivFemElmVisibilityCalculator::computePropertyVisibility(elmVisibility.p(),
                                                                     caseData->femParts()->part(femPartIdx),
                                                                     pMgrKey.frameIndex(),
                                                                     rangeFiltVisibility.p(),
                                                                     m_geomechView->geoMechPropertyFilterCollection()
                                                                     );
        }
        else if (pMgrKey.geometryType() == OVERRIDDEN_CELL_VISIBILITY)
        {
            RivFemElmVisibilityCalculator::computeOverriddenCellVisibility(elmVisibility.p(), 
                                                                           caseData->femParts()->part(femPartIdx),
                                                                           m_geomechView->viewController());
        }

        else if (pMgrKey.geometryType() == ALL_CELLS)
        {
            RivFemElmVisibilityCalculator::computeAllVisible(elmVisibility.p(), caseData->femParts()->part(femPartIdx));
        }
        else
        {
            CVF_ASSERT(false); // Unsupported CellSet Enum
        }

        partMgrToUpdate->setCellVisibility(femPartIdx, elmVisibility.p());
    }

    m_partMgrCache->setGenerationFinished(pMgrKey);

    return partMgrToUpdate;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStepIndex)
{
    if (!m_geomechView->geoMechCase()) return;

    size_t gridCount = m_geomechView->geoMechCase()->geoMechData()->femParts()->partCount();
    
    if (gridCount == 0) return;

    RigFemPart* part = m_geomechView->geoMechCase()->geoMechData()->femParts()->part(0);
    int elmCount = part->elementCount();

    totalVisibility->resize(elmCount);
    totalVisibility->setAll(false);

    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs(timeStepIndex);
    for (size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx)
    {
        RivGeoMechPartMgr*  partMgr = m_partMgrCache->partMgr(visiblePartMgrs[pmIdx]);

        cvf::ref<cvf::UByteArray> visibility =  partMgr->cellVisibility(0);
        for (int elmIdx = 0; elmIdx < elmCount; ++ elmIdx)
        {
            (*totalVisibility)[elmIdx] |= (*visibility)[elmIdx];
        }
    }
}


