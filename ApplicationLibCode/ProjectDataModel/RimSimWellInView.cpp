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

#include "RimSimWellInView.h"

#include "RicfCommandObject.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Well/RigMswCenterLineCalculator.h"
#include "Well/RigSimWellData.h"
#include "Well/RigSimulationWellCenterLineCalculator.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "Rim2dIntersectionView.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimPropertyFilterCollection.h"
#include "RimSimWellInViewCollection.h"
#include "RimSimWellInViewTools.h"
#include "RimSummaryCase.h"
#include "RimWellDiskConfig.h"

#include "RiuMainWindow.h"

#include "RivReservoirViewPartMgr.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionView* corresponding2dIntersectionView( RimSimWellInView* simWellInView );

CAF_PDM_SOURCE_INIT( RimSimWellInView, "Well" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInView::RimSimWellInView()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Simulation Well", ":/Well.svg", "", "", "SimulationWell", "An Eclipse Simulation Well" );

    CAF_PDM_InitScriptableFieldNoDefault( &name, "Name", "Name" );
    name.registerKeywordAlias( "WellName" );

    CAF_PDM_InitField( &showWell, "ShowWell", true, "Show well " );

    CAF_PDM_InitField( &showWellLabel, "ShowWellLabel", true, "Label" );
    CAF_PDM_InitField( &showWellHead, "ShowWellHead", true, "Well Head" );
    CAF_PDM_InitField( &showWellPipe, "ShowWellPipe", true, "Pipe" );
    CAF_PDM_InitField( &showWellSpheres, "ShowWellSpheres", false, "Spheres" );
    CAF_PDM_InitField( &showWellDisks, "ShowWellDisks", false, "Disks" );

    CAF_PDM_InitField( &wellHeadScaleFactor, "WellHeadScaleFactor", 1.0, "Well Head Scale" );
    CAF_PDM_InitField( &pipeScaleFactor, "WellPipeRadiusScale", 1.0, "Pipe Radius Scale" );
    CAF_PDM_InitField( &wellPipeColor, "WellPipeColor", cvf::Color3f( 0.588f, 0.588f, 0.804f ), "Pipe Color" );

    cvf::Color3f defaultWellDiskColor = cvf::Color3::OLIVE;
    CAF_PDM_InitField( &wellDiskColor, "WellDiskColor", defaultWellDiskColor, "Disk Color" );

    CAF_PDM_InitField( &showWellCells, "ShowWellCells", false, "Well Cells" );
    CAF_PDM_InitField( &showWellCellFence, "ShowWellCellFence", false, "Well Cell Fence" );

    name.uiCapability()->setUiHidden( true );
    name.uiCapability()->setUiReadOnly( true );

    m_resultWellIndex = cvf::UNDEFINED_SIZE_T;

    m_isInjector  = false;
    m_isValidDisk = false;
    m_diskScale   = 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSimWellInView::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( reservoirView )
    {
        if ( &showWellLabel == changedField || &showWellHead == changedField || &showWellPipe == changedField ||
             &showWellSpheres == changedField || &showWellDisks == changedField || &wellPipeColor == changedField ||
             &wellDiskColor == changedField )
        {
            reservoirView->scheduleCreateDisplayModelAndRedraw();
            schedule2dIntersectionViewUpdate();
        }
        else if ( &showWell == changedField || &showWellCells == changedField || &showWellCellFence == changedField )

        {
            reservoirView->scheduleGeometryRegen( VISIBLE_WELL_CELLS );
            reservoirView->scheduleCreateDisplayModelAndRedraw();
            schedule2dIntersectionViewUpdate();
        }
        else if ( &pipeScaleFactor == changedField || &wellHeadScaleFactor == changedField )
        {
            reservoirView->scheduleSimWellGeometryRegen();
            reservoirView->scheduleCreateDisplayModelAndRedraw();
            schedule2dIntersectionViewUpdate();
        }
    }

    RimSimWellInViewCollection* wellColl = firstAncestorOrThisOfType<RimSimWellInViewCollection>();
    if ( wellColl )
    {
        wellColl->updateStateForVisibilityCheckboxes();

        RiuMainWindow::instance()->refreshDrawStyleActions();
    }

    if ( changedField == &wellPipeColor )
    {
        RimSimWellInViewCollection::updateWellAllocationPlots();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSimWellInView::objectToggleField()
{
    return &showWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigWellPath*> RimSimWellInView::wellPipeBranches() const
{
    auto eclipseCase = firstAncestorOrThisOfType<RimEclipseCase>();
    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    {
        RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();

        bool includeCellCenters = isUsingCellCenterForPipe();
        auto simWellCollection  = firstAncestorOrThisOfTypeAsserted<RimSimWellInViewCollection>();
        bool detectBrances      = simWellCollection->isAutoDetectingBranches;

        return caseData->simulationWellBranches( name(), includeCellCenters, detectBrances );
    }
    return std::vector<const RigWellPath*>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SimulationWellCellBranch> RimSimWellInView::wellBranchesForVisualization() const
{
    const RigSimWellData* simWellData = this->simWellData();

    if ( simWellData && simWellData->isMultiSegmentWell() )
    {
        return RigMswCenterLineCalculator::calculateMswWellPipeGeometry( this );
    }

    return RigSimulationWellCenterLineCalculator::calculateWellPipeStaticCenterline( this );
}

//--------------------------------------------------------------------------------------------------
/// frameIndex = -1 will use the static well frame
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::wellHeadTopBottomPosition( int frameIndex, cvf::Vec3d* top, cvf::Vec3d* bottom )
{
    RimEclipseView* rimReservoirView = firstAncestorOrThisOfTypeAsserted<RimEclipseView>();

    if ( !rimReservoirView->eclipseCase() || !rimReservoirView->eclipseCase()->eclipseCaseData() )
    {
        return;
    }

    RigEclipseCaseData* rigReservoir = rimReservoirView->eclipseCase()->eclipseCaseData();

    const RigWellResultFrame* wellResultFramePtr = nullptr;
    const RigCell*            whCellPtr          = nullptr;

    if ( frameIndex >= 0 )
    {
        if ( !simWellData()->hasAnyValidCells( frameIndex ) ) return;

        wellResultFramePtr = simWellData()->wellResultFrame( frameIndex );
        whCellPtr          = &( rigReservoir->cellFromWellResultCell( wellResultFramePtr->wellHeadOrStartCell() ) );
    }
    else
    {
        wellResultFramePtr = simWellData()->staticWellResultFrame();
        whCellPtr          = &( rigReservoir->cellFromWellResultCell( wellResultFramePtr->wellHeadOrStartCell() ) );
    }

    const RigCell& whCell = *whCellPtr;

    // Match this position with pipe start position in RivWellPipesPartMgr::calculateWellPipeCenterline()

    ( *bottom ) = whCell.faceCenter( cvf::StructGridInterface::NEG_K );

    // Compute well head based on the z position of the top of the K column the well head is part of
    ( *top ) = ( *bottom );
    if ( rimReservoirView->wellCollection()->wellHeadPosition() == RimSimWellInViewCollection::WELLHEAD_POS_TOP_COLUMN )
    {
        // Position well head at top active cell of IJ-column

        size_t i, j, k;
        rigReservoir->mainGrid()->ijkFromCellIndex( whCell.mainGridCellIndex(), &i, &j, &k );

        size_t kIndexWellHeadCell = k;
        k                         = 0;

        size_t topActiveCellIndex = rigReservoir->mainGrid()->cellIndexFromIJK( i, j, k );
        while ( k < kIndexWellHeadCell && !rimReservoirView->currentActiveCellInfo()->isActive( topActiveCellIndex ) )
        {
            k++;
            topActiveCellIndex = rigReservoir->mainGrid()->cellIndexFromIJK( i, j, k );
        }

        const RigCell& topActiveCell = rigReservoir->mainGrid()->cell( topActiveCellIndex );
        cvf::Vec3d     topCellPos    = topActiveCell.faceCenter( cvf::StructGridInterface::NEG_K );

        // Modify position if top active cell is closer to sea than well head
        if ( kIndexWellHeadCell > k )
        {
            top->z() = topCellPos.z();
        }
    }
    else
    {
        // Position well head above top of active cells bounding box

        const auto boundingBox = rimReservoirView->currentActiveCellInfo()->geometryBoundingBox();
        const auto maxZ        = boundingBox.max().z();
        const auto extentZ     = boundingBox.extent().z();

        top->z() = maxZ + extentZ * rimReservoirView->wellCollection()->wellHeadPositionScaleFactor();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSimWellInView::pipeRadius()
{
    auto reservoirView = firstAncestorOrThisOfTypeAsserted<RimEclipseView>();

    RigEclipseCaseData* rigReservoir = reservoirView->eclipseCase()->eclipseCaseData();

    double characteristicCellSize = rigReservoir->mainGrid()->characteristicIJCellSize();

    double pipeRadius = reservoirView->wellCollection()->pipeScaleFactor() * pipeScaleFactor() * characteristicCellSize;

    return pipeRadius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSimWellInView::pipeCrossSectionVertexCount()
{
    auto simWellCollection = firstAncestorOrThisOfTypeAsserted<RimSimWellInViewCollection>();
    return simWellCollection->pipeCrossSectionVertexCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::intersectsDynamicWellCellsFilteredCells( size_t frameIndex ) const
{
    if ( simWellData() == nullptr ) return false;

    if ( !simWellData()->hasWellResult( frameIndex ) ) return false;

    const RigWellResultFrame* wrsf = simWellData()->wellResultFrame( frameIndex );

    return intersectsWellCellsFilteredCells( wrsf, frameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::intersectsWellCellsFilteredCells( const RigWellResultFrame* wrsf, size_t frameIndex ) const
{
    auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( !reservoirView ) return false;

    const std::vector<RivCellSetEnum>& visGridParts = reservoirView->visibleGridParts();
    RivReservoirViewPartMgr*           rvMan        = reservoirView->reservoirGridPartManager();

    for ( const RivCellSetEnum& visGridPart : visGridParts )
    {
        if ( visGridPart == ALL_WELL_CELLS || visGridPart == VISIBLE_WELL_CELLS || visGridPart == VISIBLE_WELL_FENCE_CELLS ||
             visGridPart == VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER || visGridPart == VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER )
        {
            // Exclude all cells related to well cells
            continue;
        }

        // First check the wellhead:

        size_t gridIndex     = wrsf->wellHead().gridIndex();
        size_t gridCellIndex = wrsf->wellHead().cellIndex();

        if ( gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T )
        {
            const cvf::UByteArray* cellVisibility = rvMan->cellVisibility( visGridPart, gridIndex, frameIndex );
            if ( gridCellIndex < cellVisibility->size() && ( *cellVisibility )[gridCellIndex] )
            {
                return true;
            }
        }

        // Then check the rest of the well, with all the branches

        const std::vector<RigWellResultBranch> wellResSegments = wrsf->wellResultBranches();
        for ( const RigWellResultBranch& branchSegment : wellResSegments )
        {
            for ( const RigWellResultPoint& wellResultPoint : branchSegment.branchResultPoints() )
            {
                if ( wellResultPoint.isCell() )
                {
                    gridIndex     = wellResultPoint.gridIndex();
                    gridCellIndex = wellResultPoint.cellIndex();

                    const cvf::UByteArray* cellVisibility = rvMan->cellVisibility( visGridPart, gridIndex, frameIndex );
                    if ( gridCellIndex < cellVisibility->size() && ( *cellVisibility )[gridCellIndex] )
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::schedule2dIntersectionViewUpdate()
{
    Rim2dIntersectionView* intersectionView = corresponding2dIntersectionView( this );
    if ( intersectionView )
    {
        intersectionView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::intersectsStaticWellCellsFilteredCells() const
{
    if ( simWellData() == nullptr ) return false;

    // NOTE: Read out static well cells, union of well cells across all time steps
    const RigWellResultFrame* wrsf = simWellData()->staticWellResultFrame();

    // NOTE: Use first time step for visibility evaluation
    size_t frameIndex = 0;

    return intersectsWellCellsFilteredCells( wrsf, frameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Visibility" );
    appearanceGroup->add( &showWellLabel );
    appearanceGroup->add( &showWellHead );
    appearanceGroup->add( &showWellPipe );
    appearanceGroup->add( &showWellSpheres );
    appearanceGroup->add( &showWellDisks );

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup( "Well Cells and Fence" );
    filterGroup->add( &showWellCells );
    filterGroup->add( &showWellCellFence );

    showWellCellFence.uiCapability()->setUiReadOnly( !showWellCells() );
    caf::PdmUiGroup* sizeScalingGroup = uiOrdering.addNewGroup( "Size Scaling" );
    sizeScalingGroup->add( &wellHeadScaleFactor );
    sizeScalingGroup->add( &pipeScaleFactor );

    caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup( "Colors" );
    colorGroup->add( &wellPipeColor );
    colorGroup->add( &wellDiskColor );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );

    const auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( !reservoirView ) return;

    if ( reservoirView->cellFilterCollection() && !reservoirView->cellFilterCollection()->hasActiveFilters() )
    {
        uiCapability()->setUiReadOnly( false );

        return;
    }

    const RimSimWellInViewCollection* wellColl = firstAncestorOrThisOfType<RimSimWellInViewCollection>();
    if ( !wellColl ) return;

    if ( wellColl->showWellsIntersectingVisibleCells() &&
         !intersectsDynamicWellCellsFilteredCells( static_cast<size_t>( reservoirView->currentTimeStep() ) ) )
    {
        // Mark well as read only if well is not intersecting visible cells

        uiCapability()->setUiReadOnly( true );
    }
    else
    {
        uiCapability()->setUiReadOnly( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellCellsVisible() const
{
    const auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();

    if ( reservoirView == nullptr ) return false;
    if ( simWellData() == nullptr ) return false;

    if ( !reservoirView->wellCollection()->isActive() ) return false;

    if ( !showWell() ) return false;

    if ( !showWellCells() ) return false;

    if ( reservoirView->intersectionCollection()->hasActiveIntersectionForSimulationWell( this ) ) return true;

    if ( reservoirView->wellCollection()->showWellsIntersectingVisibleCells() && reservoirView->cellFilterCollection()->hasActiveFilters() )
    {
        return intersectsStaticWellCellsFilteredCells();
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellPipeVisible( size_t frameIndex ) const
{
    const auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();

    if ( reservoirView == nullptr ) return false;
    if ( simWellData() == nullptr ) return false;

    if ( frameIndex >= simWellData()->m_resultTimeStepIndexToWellTimeStepIndex.size() )
    {
        return false;
    }

    size_t wellTimeStepIndex = simWellData()->m_resultTimeStepIndexToWellTimeStepIndex[frameIndex];
    if ( wellTimeStepIndex == cvf::UNDEFINED_SIZE_T )
    {
        return false;
    }

    if ( !reservoirView->wellCollection()->isActive() ) return false;

    if ( !showWell() ) return false;

    if ( !showWellPipe() ) return false;

    if ( reservoirView->intersectionCollection()->hasActiveIntersectionForSimulationWell( this ) ) return true;

    if ( reservoirView->wellCollection()->showWellsIntersectingVisibleCells() &&
         ( reservoirView->cellFilterCollection()->hasActiveFilters() || reservoirView->propertyFilterCollection()->hasActiveFilters() ) )
    {
        return intersectsDynamicWellCellsFilteredCells( frameIndex );
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellSpheresVisible( size_t frameIndex ) const
{
    const auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();

    if ( reservoirView == nullptr ) return false;
    if ( simWellData() == nullptr ) return false;

    if ( frameIndex >= simWellData()->m_resultTimeStepIndexToWellTimeStepIndex.size() )
    {
        return false;
    }

    size_t wellTimeStepIndex = simWellData()->m_resultTimeStepIndexToWellTimeStepIndex[frameIndex];
    if ( wellTimeStepIndex == cvf::UNDEFINED_SIZE_T )
    {
        return false;
    }

    if ( !reservoirView->wellCollection()->isActive() ) return false;

    if ( !showWell() ) return false;

    if ( !showWellSpheres() ) return false;

    if ( reservoirView->intersectionCollection()->hasActiveIntersectionForSimulationWell( this ) ) return true;

    if ( reservoirView->wellCollection()->showWellsIntersectingVisibleCells() && reservoirView->cellFilterCollection()->hasActiveFilters() )
    {
        return intersectsDynamicWellCellsFilteredCells( frameIndex );
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellValvesVisible( size_t frameIndex ) const
{
    const auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();

    if ( reservoirView == nullptr ) return false;
    if ( simWellData() == nullptr ) return false;

    if ( frameIndex >= simWellData()->m_resultTimeStepIndexToWellTimeStepIndex.size() )
    {
        return false;
    }

    size_t wellTimeStepIndex = simWellData()->m_resultTimeStepIndexToWellTimeStepIndex[frameIndex];
    if ( wellTimeStepIndex == cvf::UNDEFINED_SIZE_T )
    {
        return false;
    }

    if ( !showWell() ) return false;

    if ( !reservoirView->wellCollection()->isActive() ) return false;
    if ( !reservoirView->wellCollection()->showValves() ) return false;

    if ( reservoirView->intersectionCollection()->hasActiveIntersectionForSimulationWell( this ) ) return true;

    if ( reservoirView->wellCollection()->showWellsIntersectingVisibleCells() && reservoirView->cellFilterCollection()->hasActiveFilters() )
    {
        return intersectsDynamicWellCellsFilteredCells( frameIndex );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isUsingCellCenterForPipe() const
{
    const RimSimWellInViewCollection* wellColl = firstAncestorOrThisOfType<RimSimWellInViewCollection>();

    return ( wellColl && wellColl->wellPipeCoordType() == RimSimWellInViewCollection::WELLPIPE_CELLCENTER );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellDiskData RimSimWellInView::wellDiskData() const
{
    return m_wellDiskData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::setSimWellData( RigSimWellData* simWellData, size_t resultWellIndex )
{
    m_simWellData     = simWellData;
    m_resultWellIndex = resultWellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSimWellData* RimSimWellInView::simWellData()
{
    return m_simWellData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigSimWellData* RimSimWellInView::simWellData() const
{
    return m_simWellData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSimWellInView::resultWellIndex() const
{
    return m_resultWellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isWellDiskVisible() const
{
    const auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();

    if ( reservoirView == nullptr ) return false;
    if ( simWellData() == nullptr ) return false;

    if ( !reservoirView->wellCollection()->isActive() ) return false;
    if ( !reservoirView->wellCollection()->isWellDisksVisible() ) return false;

    if ( !showWell() ) return false;

    if ( !showWellDisks() ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSimWellInView::calculateInjectionProductionFractions( const RimWellDiskConfig& wellDiskConfig, bool* isOk )
{
    const auto reservoirView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( !reservoirView ) return false;
    if ( !reservoirView->eclipseCase() ) return false;

    size_t                 timeStep      = static_cast<size_t>( reservoirView->currentTimeStep() );
    std::vector<QDateTime> caseTimeSteps = reservoirView->eclipseCase()->timeStepDates();
    QDateTime              currentDate;
    if ( timeStep < caseTimeSteps.size() )
    {
        currentDate = caseTimeSteps[timeStep];
    }
    else
    {
        currentDate = caseTimeSteps.back();
    }

    RifSummaryReaderInterface* summaryReader = nullptr;
    {
        if ( wellDiskConfig.sourceCase() )
        {
            summaryReader = wellDiskConfig.sourceCase()->summaryReader();
        }
    }

    if ( !summaryReader )
    {
        m_isValidDisk = false;
        return -1.0;
    }

    if ( wellDiskConfig.isSingleProperty() )
    {
        double singleProperty =
            RimSimWellInViewTools::extractValueForTimeStep( summaryReader, name(), wellDiskConfig.getSingleProperty(), currentDate, isOk );
        if ( !( *isOk ) )
        {
            m_isValidDisk = false;
            return -1.0;
        }

        m_wellDiskData.setSinglePropertyValue( singleProperty );
    }
    else
    {
        m_isInjector = RimSimWellInViewTools::isInjector( this );

        double oil = RimSimWellInViewTools::extractValueForTimeStep( summaryReader,
                                                                     name(),
                                                                     wellDiskConfig.getOilProperty( m_isInjector ),
                                                                     currentDate,
                                                                     isOk );
        if ( !( *isOk ) )
        {
            m_isValidDisk = false;
            return -1.0;
        }

        double gas = RimSimWellInViewTools::extractValueForTimeStep( summaryReader,
                                                                     name(),
                                                                     wellDiskConfig.getGasProperty( m_isInjector ),
                                                                     currentDate,
                                                                     isOk ) /
                     1000.0;
        if ( !( *isOk ) )
        {
            m_isValidDisk = false;
            return -1.0;
        }

        double water = RimSimWellInViewTools::extractValueForTimeStep( summaryReader,
                                                                       name(),
                                                                       wellDiskConfig.getWaterProperty( m_isInjector ),
                                                                       currentDate,
                                                                       isOk );
        if ( !( *isOk ) )
        {
            m_isValidDisk = false;
            return -1.0;
        }

        m_wellDiskData.setOilGasWater( oil, gas, water );
    }

    m_isValidDisk = true;

    return m_wellDiskData.total();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::scaleDisk( double minValue, double maxValue )
{
    if ( m_isValidDisk )
    {
        m_diskScale = 1.0 + ( m_wellDiskData.total() - minValue ) / ( maxValue - minValue );
    }
    else
    {
        m_diskScale = 1.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimSimWellInView::boundingBoxInDomainCoords() const
{
    auto noConst         = const_cast<RimSimWellInView*>( this );
    auto simWellBranches = RigSimulationWellCenterLineCalculator::calculateWellPipeStaticCenterline( noConst );

    cvf::BoundingBox bb;
    for ( const auto& [coords, wellCells] : simWellBranches )
    {
        if ( !coords.empty() )
        {
            // Estimate the bounding box based on first, middle and last coordinate of branches
            bb.add( coords.front() );

            size_t mid = coords.size() / 2;
            bb.add( coords[mid] );

            bb.add( coords.back() );
        }
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInView::isValidDisk() const
{
    return m_isValidDisk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSimWellInView::diskScale() const
{
    if ( m_isValidDisk )
    {
        return m_diskScale;
    }
    else
    {
        return 1.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionView* corresponding2dIntersectionView( RimSimWellInView* simWellInView )
{
    Rim3dView* tdView = simWellInView->firstAncestorOrThisOfType<Rim3dView>();

    if ( tdView )
    {
        std::vector<RimIntersectionCollection*> intersectionColls = tdView->descendantsIncludingThisOfType<RimIntersectionCollection>();
        if ( intersectionColls.size() == 1 )
        {
            for ( const auto intersection : intersectionColls[0]->intersections() )
            {
                if ( intersection->simulationWell() == simWellInView )
                {
                    return intersection->correspondingIntersectionView();
                }
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSimWellInView::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimEclipseView* view = firstAncestorOrThisOfType<RimEclipseView>();
    if ( view )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}
