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

#include "Rim3dView.h"
#include "RimCellFilterCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPartCollection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
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
RivGeoMechVizLogic::RivGeoMechVizLogic( RimGeoMechView* geomView )
{
    CVF_ASSERT( geomView );
    m_geomechView  = geomView;
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
void RivGeoMechVizLogic::appendNoAnimPartsToModel( cvf::ModelBasicList* model )
{
    this->appendPartsToModel( -1, model );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::appendPartsToModel( int viewerStepIndex, cvf::ModelBasicList* model )
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs( viewerStepIndex );
    for ( size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx )
    {
        RivGeoMechPartMgr* partMgr = getUpdatedPartMgr( visiblePartMgrs[pmIdx] );

        partMgr->appendGridPartsToModel( model );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::updateCellResultColor( int viewerStepIndex, int timeStepIndex, int frameIndex, RimGeoMechCellColors* cellResultColors )
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs( viewerStepIndex );
    for ( size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx )
    {
        RivGeoMechPartMgr* partMgr = m_partMgrCache->partMgr( visiblePartMgrs[pmIdx] );
        partMgr->updateCellResultColor( timeStepIndex, frameIndex, cellResultColors );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::updateStaticCellColors( int viewerStepIndex )
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs( viewerStepIndex );
    for ( size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx )
    {
        RivGeoMechPartMgr* partMgr = m_partMgrCache->partMgr( visiblePartMgrs[pmIdx] );
        auto               color   = staticCellColor();
        partMgr->updateCellColor( cvf::Color4f( color ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::scheduleGeometryRegen( RivCellSetEnum geometryType )
{
    this->scheduleRegenOfDirectlyDependentGeometry( geometryType );

    bool resultsOk = ( m_geomechView->geoMechCase() && m_geomechView->geoMechCase()->geoMechData() &&
                       m_geomechView->geoMechCase()->geoMechData()->femPartResults() );

    int stepCount = 0;
    if ( resultsOk )
    {
        stepCount = m_geomechView->geoMechCase()->geoMechData()->femPartResults()->totalSteps();
    }

    for ( int stepIdx = -1; stepIdx < stepCount; stepIdx++ )
    {
        RivGeoMechPartMgrCache::Key geomToRegen( geometryType, stepIdx );
        m_partMgrCache->scheduleRegeneration( geomToRegen );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::scheduleGeometryRegenOfVisiblePartMgrs( int viewerStepIndex )
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs( viewerStepIndex );
    for ( size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx )
    {
        m_partMgrCache->scheduleRegeneration( visiblePartMgrs[pmIdx] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::scheduleRegenOfDirectlyDependentGeometry( RivCellSetEnum geometryType )
{
    if ( geometryType == RANGE_FILTERED )
    {
        this->scheduleGeometryRegen( PROPERTY_FILTERED );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RivGeoMechPartMgrCache::Key> RivGeoMechVizLogic::keysToVisiblePartMgrs( int viewerStepIndex ) const
{
    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs;
    if ( m_geomechView->viewController() && m_geomechView->viewController()->isVisibleCellsOveridden() )
    {
        visiblePartMgrs.push_back( RivGeoMechPartMgrCache::Key( OVERRIDDEN_CELL_VISIBILITY, -1 ) );
    }
    else if ( m_geomechView->isGridVisualizationMode() )
    {
        if ( viewerStepIndex >= 0 && m_geomechView->geoMechPropertyFilterCollection()->hasActiveFilters() )
        {
            visiblePartMgrs.push_back( RivGeoMechPartMgrCache::Key( PROPERTY_FILTERED, viewerStepIndex ) );
        }
        else if ( m_geomechView->cellFilterCollection()->hasActiveFilters() )
        {
            visiblePartMgrs.push_back( RivGeoMechPartMgrCache::Key( RANGE_FILTERED, -1 ) );
        }
        else
        {
            visiblePartMgrs.push_back( RivGeoMechPartMgrCache::Key( ALL_CELLS, -1 ) );
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
cvf::Color3f RivGeoMechVizLogic::staticCellColor()
{
    return cvf::Color3f::ORANGE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgr* RivGeoMechVizLogic::getUpdatedPartMgr( RivGeoMechPartMgrCache::Key pMgrKey )
{
    if ( !m_partMgrCache->isNeedingRegeneration( pMgrKey ) )
    {
        return m_partMgrCache->partMgr( pMgrKey );
    }

    RivGeoMechPartMgr*  partMgrToUpdate = m_partMgrCache->partMgr( pMgrKey );
    int                 partCount       = 0;
    RigGeoMechCaseData* caseData        = nullptr;
    int                 timeStepIdx     = -1;
    int                 frameIdx        = -1;

    if ( m_geomechView->geoMechCase() )
    {
        caseData  = m_geomechView->geoMechCase()->geoMechData();
        partCount = caseData->femParts()->partCount();
        std::tie( timeStepIdx, frameIdx ) = caseData->femPartResults()->stepListIndexToTimeStepAndDataFrameIndex( pMgrKey.viewerStepIndex() );
    }

    if ( partMgrToUpdate->initializedFemPartCount() != partCount )
    {
        partMgrToUpdate->clearAndSetReservoir( caseData );
    }

    partMgrToUpdate->updateDisplacements( m_geomechView->partsCollection(),
                                          m_geomechView->showDisplacements(),
                                          m_geomechView->displacementScaleFactor() );

    partMgrToUpdate->setTransform( m_geomechView->scaleTransform() );

    for ( int femPartIdx = 0; femPartIdx < partCount; ++femPartIdx )
    {
        cvf::ref<cvf::UByteArray> elmVisibility = partMgrToUpdate->cellVisibility( femPartIdx );

        if ( pMgrKey.geometryType() == RANGE_FILTERED )
        {
            cvf::CellRangeFilter cellRangeFilter;
            m_geomechView->cellFilterCollection()->compoundCellRangeFilter( &cellRangeFilter, femPartIdx );
            RivFemElmVisibilityCalculator::computeRangeVisibility( elmVisibility.p(), caseData->femParts()->part( femPartIdx ), cellRangeFilter );
        }
        else if ( pMgrKey.geometryType() == PROPERTY_FILTERED )
        {
            RivGeoMechPartMgr* cellfiltered = nullptr;
            if ( m_geomechView->cellFilterCollection()->hasActiveFilters() )
            {
                cellfiltered = getUpdatedPartMgr( RivGeoMechPartMgrCache::Key( RANGE_FILTERED, -1 ) );
            }
            else
            {
                cellfiltered = getUpdatedPartMgr( RivGeoMechPartMgrCache::Key( ALL_CELLS, -1 ) );
            }
            cvf::ref<cvf::UByteArray> rangeFiltVisibility = cellfiltered->cellVisibility( femPartIdx );

            RivFemElmVisibilityCalculator::computePropertyVisibility( elmVisibility.p(),
                                                                      caseData->femParts()->part( femPartIdx ),
                                                                      timeStepIdx,
                                                                      frameIdx,
                                                                      rangeFiltVisibility.p(),
                                                                      m_geomechView->geoMechPropertyFilterCollection() );
        }
        else if ( pMgrKey.geometryType() == OVERRIDDEN_CELL_VISIBILITY )
        {
            RivFemElmVisibilityCalculator::computeOverriddenCellVisibility( elmVisibility.p(),
                                                                            caseData->femParts()->part( femPartIdx ),
                                                                            m_geomechView->viewController() );
        }

        else if ( pMgrKey.geometryType() == ALL_CELLS )
        {
            RivFemElmVisibilityCalculator::computeAllVisible( elmVisibility.p(), caseData->femParts()->part( femPartIdx ) );
        }
        else
        {
            CVF_ASSERT( false ); // Unsupported CellSet Enum
        }

        partMgrToUpdate->setCellVisibility( femPartIdx, elmVisibility.p() );
    }

    m_partMgrCache->setGenerationFinished( pMgrKey );

    return partMgrToUpdate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int viewerStepIndex )
{
    if ( !m_geomechView->geoMechCase() ) return;

    int partCount = m_geomechView->femParts()->partCount();
    if ( partCount == 0 ) return;

    int elmCount = 0;
    for ( int i = 0; i < partCount; i++ )
    {
        RigFemPart* part = m_geomechView->femParts()->part( i );
        elmCount += part->elementCount();
    }
    totalVisibility->resize( elmCount );
    totalVisibility->setAll( false );

    std::vector<RivGeoMechPartMgrCache::Key> visiblePartMgrs = keysToVisiblePartMgrs( viewerStepIndex );
    for ( size_t pmIdx = 0; pmIdx < visiblePartMgrs.size(); ++pmIdx )
    {
        RivGeoMechPartMgr* partMgr = getUpdatedPartMgr( visiblePartMgrs[pmIdx] );
        CVF_ASSERT( partMgr );
        if ( partMgr )
        {
            int elmOffset = 0;
            for ( int i = 0; i < partCount; i++ )
            {
                RigFemPart* part = m_geomechView->femParts()->part( i );

                cvf::ref<cvf::UByteArray> visibility = partMgr->cellVisibility( i );
                for ( int elmIdx = 0; elmIdx < part->elementCount(); ++elmIdx )
                {
                    ( *totalVisibility )[elmOffset + elmIdx] |= ( *visibility )[elmIdx];
                }
                elmOffset += part->elementCount();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::resetPartMgrs()
{
    m_partMgrCache = new RivGeoMechPartMgrCache;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechVizLogic::calculateCellVisibility( cvf::UByteArray* totalVisibility, std::vector<RivCellSetEnum> geomTypes, int viewerStepIndex )
{
    if ( !m_geomechView->geoMechCase() ) return;

    int partCount = m_geomechView->femParts()->partCount();
    if ( partCount == 0 ) return;

    int elmCount = 0;
    for ( int i = 0; i < partCount; i++ )
    {
        RigFemPart* part = m_geomechView->femParts()->part( i );
        elmCount += part->elementCount();
    }
    totalVisibility->resize( elmCount );
    totalVisibility->setAll( false );

    std::vector<RivGeoMechPartMgrCache::Key> partMgrs;

    for ( auto geomType : geomTypes )
    {
        // skip types not support in geomech
        if ( geomType == PROPERTY_FILTERED_WELL_CELLS ) continue;
        if ( geomType == RANGE_FILTERED_WELL_CELLS ) continue;

        partMgrs.push_back( RivGeoMechPartMgrCache::Key( geomType, viewerStepIndex ) );
    }

    for ( size_t pmIdx = 0; pmIdx < partMgrs.size(); ++pmIdx )
    {
        RivGeoMechPartMgr* partMgr = getUpdatedPartMgr( partMgrs[pmIdx] );
        CVF_ASSERT( partMgr );
        if ( partMgr )
        {
            int elmOffset = 0;
            for ( int i = 0; i < partCount; i++ )
            {
                RigFemPart* part = m_geomechView->femParts()->part( i );

                cvf::ref<cvf::UByteArray> visibility = partMgr->cellVisibility( i );
                for ( int elmIdx = 0; elmIdx < part->elementCount(); ++elmIdx )
                {
                    ( *totalVisibility )[elmOffset + elmIdx] |= ( *visibility )[elmIdx];
                }
                elmOffset += part->elementCount();
            }
        }
    }
}
