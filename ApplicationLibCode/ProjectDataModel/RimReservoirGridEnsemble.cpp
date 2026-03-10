/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimReservoirGridEnsemble.h"

#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RifReaderOpmCommon.h"
#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"

#include "ContourMap/RimStatisticsContourMap.h"
#include "ContourMap/RimStatisticsContourMapView.h"
#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "Formations/RimFormationNames.h"
#include "Formations/RimFormationNamesCollection.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellTargetMapping.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafProgressInfo.h"

#include "RigCaseCellResultsData.h"

CAF_PDM_SOURCE_INIT( RimReservoirGridEnsemble, "RimReservoirGridEnsemble" );

namespace caf
{
template <>
void caf::AppEnum<RimReservoirGridEnsembleBase::GridModeType>::setUp()
{
    addItem( RimReservoirGridEnsembleBase::GridModeType::SHARED_GRID, "SharedGrid", "Shared Grid" );
    addItem( RimReservoirGridEnsembleBase::GridModeType::INDIVIDUAL_GRIDS, "IndividualGrids", "Individual Grids" );
    setDefault( RimReservoirGridEnsembleBase::GridModeType::SHARED_GRID );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsemble::RimReservoirGridEnsemble()
    : m_mainGrid( nullptr )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Reservoir Grid Ensemble",
                                                    ":/GridCaseGroup16x16.png",
                                                    "",
                                                    "",
                                                    "ReservoirGridEnsemble",
                                                    "Grid Ensemble from File Set" );

    CAF_PDM_InitScriptableField( &m_groupId, "GroupId", -1, "Ensemble ID" );
    m_groupId.uiCapability()->setUiReadOnly( true );
    m_groupId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitFieldNoDefault( &m_ensembleFileSet, "EnsembleFileSet", "Ensemble File Set" );
    m_ensembleFileSet.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_autoDetectGridType, "AutoDetectGridType", true, "Auto Detect Grid Type" );
    CAF_PDM_InitField( &m_gridMode, "GridMode", GridModeType::SHARED_GRID, "Grid Mode" );
    CAF_PDM_InitField( &m_detectedGridMode, "DetectedGridMode", GridModeType::SHARED_GRID, "Detected Grid Mode" );
    m_detectedGridMode.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_activeFormationNames, "DefaultFormationNames", "Formation Names" );

    CAF_PDM_InitFieldNoDefault( &m_caseCollection, "CaseCollection", "Source Cases" );
    m_caseCollection = new RimCaseCollection;
    m_caseCollection->uiCapability()->setUiName( "Realizations" );
    m_caseCollection->uiCapability()->setUiIconFromResourceString( ":/Cases16x16.png" );
    m_caseCollection.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_statisticsCaseCollection, "StatisticsCaseCollection", "Statistics Cases" );
    m_statisticsCaseCollection = new RimCaseCollection;
    m_statisticsCaseCollection->uiCapability()->setUiName( "Derived Statistics" );
    m_statisticsCaseCollection->uiCapability()->setUiIconFromResourceString( ":/Histograms16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_viewCollection, "ViewCollection", "Views" );
    m_viewCollection = new RimEclipseViewCollection;
    m_viewCollection->setEclipseCaseProvider( [this]() { return this->cases(); } );

    CAF_PDM_InitFieldNoDefault( &m_wellTargetMappings, "WellTargetMappings", "Well Target Mappings" );
    CAF_PDM_InitFieldNoDefault( &m_statisticsContourMaps, "StatisticsContourMaps", "Statistics Contour Maps" );

    m_unionOfMatrixActiveCells   = new RigActiveCellInfo;
    m_unionOfFractureActiveCells = new RigActiveCellInfo;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimReservoirGridEnsemble::groupId() const
{
    return m_groupId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::setGroupId( int id )
{
    m_groupId = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::setEnsembleFileSet( RimEnsembleFileSet* ensembleFileSet )
{
    if ( m_ensembleFileSet )
    {
        m_ensembleFileSet->fileSetChanged.disconnect( this );
        m_ensembleFileSet->nameChanged.disconnect( this );
    }

    m_ensembleFileSet = ensembleFileSet;

    if ( m_ensembleFileSet )
    {
        m_ensembleFileSet->fileSetChanged.connect( this, &RimReservoirGridEnsemble::onFileSetChanged );
        m_ensembleFileSet->nameChanged.connect( this,
                                                [this]( const caf::SignalEmitter* )
                                                {
                                                    if ( m_ensembleFileSet ) setName( m_ensembleFileSet->name() );
                                                } );
        setName( m_ensembleFileSet->name() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFileSet* RimReservoirGridEnsemble::ensembleFileSet() const
{
    return m_ensembleFileSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::addCase( RimEclipseCase* reservoir )
{
    CVF_ASSERT( reservoir );

    if ( !m_mainGrid && reservoir->eclipseCaseData() )
    {
        m_mainGrid = reservoir->eclipseCaseData()->mainGrid();
    }
    else if ( hasSharedGrid() && reservoir->eclipseCaseData() )
    {
        // Share the main grid for identical grids
        reservoir->eclipseCaseData()->setMainGrid( m_mainGrid );
    }

    m_caseCollection()->reservoirs().push_back( reservoir );

    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::removeCase( RimEclipseCase* reservoir )
{
    if ( m_caseCollection()->reservoirs().count( reservoir ) == 0 ) return;

    m_caseCollection()->reservoirs().removeChild( reservoir );

    if ( m_caseCollection()->reservoirs().empty() )
    {
        m_mainGrid = nullptr;
    }

    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReservoirGridEnsemble::contains( RimEclipseCase* reservoir ) const
{
    CVF_ASSERT( reservoir );

    for ( RimEclipseCase* rimReservoir : cases() )
    {
        if ( reservoir->gridFileName() == rimReservoir->gridFileName() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimReservoirGridEnsemble::cases() const
{
    if ( !m_caseCollection ) return {};

    return m_caseCollection->reservoirs.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimReservoirGridEnsemble::mainCase()
{
    if ( !m_caseCollection()->reservoirs().empty() )
    {
        return m_caseCollection()->reservoirs()[0];
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimReservoirGridEnsemble::sourceCases() const
{
    return cases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimReservoirGridEnsemble::findByFileName( const QString& gridFileName ) const
{
    for ( RimEclipseCase* rimReservoir : cases() )
    {
        if ( gridFileName == rimReservoir->gridFileName() ) return rimReservoir;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReservoirGridEnsemble::hasSharedGrid() const
{
    return gridMode() == GridModeType::SHARED_GRID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReservoirGridEnsemble::isGridDataLoaded() const
{
    return m_mainGrid != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsembleBase::GridModeType RimReservoirGridEnsemble::gridMode() const
{
    if ( m_autoDetectGridType ) return m_detectedGridMode.v();
    return m_gridMode.v();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimReservoirGridEnsemble::ensembleName() const
{
    return RimNamedObject::name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimReservoirGridEnsemble::mainGrid()
{
    // Trigger deferred loading if needed
    if ( !m_mainGrid && !cases().empty() )
    {
        const_cast<RimReservoirGridEnsemble*>( this )->loadGridDataFromFiles();
    }

    return m_mainGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::setupSharedGrid()
{
    if ( !hasSharedGrid() ) return;

    auto allCases = cases();
    if ( allCases.empty() ) return;

    // Use the first case's grid as the shared grid
    RimEclipseCase* firstCase = allCases[0];
    if ( !firstCase || !firstCase->eclipseCaseData() ) return;

    m_mainGrid = firstCase->eclipseCaseData()->mainGrid();

    // Set the shared grid on all other cases
    for ( size_t i = 1; i < allCases.size(); i++ )
    {
        if ( allCases[i] && allCases[i]->eclipseCaseData() )
        {
            allCases[i]->eclipseCaseData()->setMainGrid( m_mainGrid );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimReservoirGridEnsemble::unionOfActiveCells( RiaDefines::PorosityModelType porosityType )
{
    if ( porosityType == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_unionOfMatrixActiveCells.p();
    }
    else
    {
        return m_unionOfFractureActiveCells.p();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::computeUnionOfActiveCells()
{
    if ( !hasSharedGrid() ) return;

    if ( m_unionOfMatrixActiveCells->reservoirActiveCellCount() > 0 )
    {
        return;
    }

    if ( m_caseCollection->reservoirs.empty() || !m_mainGrid )
    {
        clearActiveCellUnions();
        return;
    }

    m_unionOfMatrixActiveCells->setReservoirCellCount( m_mainGrid->totalCellCount() );
    m_unionOfFractureActiveCells->setReservoirCellCount( m_mainGrid->totalCellCount() );
    m_unionOfMatrixActiveCells->setGridCount( m_mainGrid->gridCount() );
    m_unionOfFractureActiveCells->setGridCount( m_mainGrid->gridCount() );

    size_t globalActiveMatrixIndex   = 0;
    size_t globalActiveFractureIndex = 0;

    for ( size_t gridIdx = 0; gridIdx < m_mainGrid->gridCount(); gridIdx++ )
    {
        RigGridBase* grid = m_mainGrid->gridByIndex( gridIdx );

        std::vector<char> activeM( grid->cellCount(), 0 );
        std::vector<char> activeF( grid->cellCount(), 0 );

        for ( size_t gridLocalCellIndex = 0; gridLocalCellIndex < grid->cellCount(); gridLocalCellIndex++ )
        {
            for ( size_t caseIdx = 0; caseIdx < m_caseCollection->reservoirs.size(); caseIdx++ )
            {
                size_t reservoirCellIndex = grid->reservoirCellIndex( gridLocalCellIndex );

                if ( activeM[gridLocalCellIndex] == 0 )
                {
                    if ( m_caseCollection->reservoirs[caseIdx]
                             ->eclipseCaseData()
                             ->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )
                             ->isActive( reservoirCellIndex ) )
                    {
                        activeM[gridLocalCellIndex] = 1;
                    }
                }

                if ( activeF[gridLocalCellIndex] == 0 )
                {
                    if ( m_caseCollection->reservoirs[caseIdx]
                             ->eclipseCaseData()
                             ->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL )
                             ->isActive( reservoirCellIndex ) )
                    {
                        activeF[gridLocalCellIndex] = 1;
                    }
                }
            }
        }

        size_t activeMatrixIndex   = 0;
        size_t activeFractureIndex = 0;

        for ( size_t gridLocalCellIndex = 0; gridLocalCellIndex < grid->cellCount(); gridLocalCellIndex++ )
        {
            size_t reservoirCellIndex = grid->reservoirCellIndex( gridLocalCellIndex );

            if ( activeM[gridLocalCellIndex] != 0 )
            {
                m_unionOfMatrixActiveCells->setCellResultIndex( reservoirCellIndex, globalActiveMatrixIndex++ );
                activeMatrixIndex++;
            }

            if ( activeF[gridLocalCellIndex] != 0 )
            {
                m_unionOfFractureActiveCells->setCellResultIndex( reservoirCellIndex, globalActiveFractureIndex++ );
                activeFractureIndex++;
            }
        }

        m_unionOfMatrixActiveCells->setGridActiveCellCounts( gridIdx, activeMatrixIndex );
        m_unionOfFractureActiveCells->setGridActiveCellCounts( gridIdx, activeFractureIndex );
    }

    m_unionOfMatrixActiveCells->computeDerivedData();
    m_unionOfFractureActiveCells->computeDerivedData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimReservoirGridEnsemble::statisticsCaseCollection() const
{
    return m_statisticsCaseCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RimReservoirGridEnsemble::createAndAppendStatisticsCase()
{
    if ( !hasSharedGrid() )
    {
        RiaLogging::warning( QString( "Cannot create statistics case for ensemble '%1': grids are not identical." ).arg( name() ) );
        return nullptr;
    }

    return RimReservoirGridEnsembleBase::createAndAppendStatisticsCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::addView( RimEclipseView* view )
{
    m_viewCollection->addView( view );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimReservoirGridEnsemble::addViewForCase( RimEclipseCase* eclipseCase )
{
    return m_viewCollection->addView( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection* RimReservoirGridEnsemble::viewCollection() const
{
    return m_viewCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseView*> RimReservoirGridEnsemble::allViews() const
{
    std::vector<RimEclipseView*> views;

    for ( auto view : m_viewCollection->views() )
    {
        views.push_back( view );
    }

    for ( auto statCase : m_statisticsCaseCollection->reservoirs() )
    {
        for ( auto view : statCase->reservoirViews() )
        {
            views.push_back( view );
        }
    }

    for ( auto cmap : m_statisticsContourMaps )
    {
        for ( auto view : cmap->views() )
        {
            views.push_back( view );
        }
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimEclipseCase*> RimReservoirGridEnsemble::casesInViews() const
{
    if ( !m_caseCollection ) return {};
    if ( !m_viewCollection || m_viewCollection->isEmpty() ) return {};

    std::set<RimEclipseCase*> retCases;

    for ( auto view : m_viewCollection->views() )
    {
        if ( view->eclipseCase() != nullptr ) retCases.insert( view->eclipseCase() );
    }

    return retCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::addWellTargetMapping( RimWellTargetMapping* wellTargetMapping )
{
    m_wellTargetMappings.push_back( wellTargetMapping );
    wellTargetMapping->updateResultDefinition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellTargetMapping*> RimReservoirGridEnsemble::wellTargetMappings() const
{
    return m_wellTargetMappings.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::addStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap )
{
    m_statisticsContourMaps.push_back( statisticsContourMap );
    statisticsContourMap->setName( QString( "Ensemble Contour Map #%1" ).arg( m_statisticsContourMaps.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::loadDataAndUpdate()
{
    if ( !isGridDataLoaded() )
    {
        loadGridDataFromFiles();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::createGridCasesFromEnsembleFileSet()
{
    if ( !m_ensembleFileSet ) return;

    // Clear existing cases and statistics
    m_caseCollection->reservoirs.deleteChildren();
    m_statisticsCaseCollection->reservoirs.deleteChildren();
    m_mainGrid = nullptr;
    clearActiveCellUnions();

    // Create case objects without loading grids
    createCaseObjectsFromEnsembleFileSet();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewViewForGridEnsembleFeature";
    menuBuilder << "RicNewStatisticsContourMapFeature";
    menuBuilder << "RicNewWellTargetMappingFeature";

    if ( hasSharedGrid() )
    {
        menuBuilder << "RicNewStatisticsCaseFeature";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RimReservoirGridEnsemble::activeFormationNames() const
{
    if ( !hasSharedGrid() ) return nullptr;
    return m_activeFormationNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimReservoirGridEnsemble::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_activeFormationNames )
    {
        RimProject* proj = RimProject::current();
        if ( proj && proj->activeOilField() && proj->activeOilField()->formationNamesCollection() )
        {
            for ( RimFormationNames* fnames : proj->activeOilField()->formationNamesCollection()->formationNamesList() )
            {
                options.push_back( caf::PdmOptionItemInfo( fnames->shortName(), fnames, false, fnames->uiCapability()->uiIconProvider() ) );
            }
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_groupId );
    uiOrdering.add( &m_ensembleFileSet );
    uiOrdering.add( &m_autoDetectGridType );

    if ( m_autoDetectGridType )
    {
        uiOrdering.add( &m_detectedGridMode );
    }
    else
    {
        uiOrdering.add( &m_gridMode );
    }

    if ( hasSharedGrid() )
    {
        uiOrdering.add( &m_activeFormationNames );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_autoDetectGridType || changedField == &m_gridMode )
    {
        updateStatisticsVisibility();
        updateConnectedEditors();
    }
    else if ( changedField == &m_activeFormationNames )
    {
        for ( auto rimCase : cases() )
        {
            rimCase->updateFormationNamesData();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::initAfterRead()
{
    if ( m_ensembleFileSet )
    {
        m_ensembleFileSet->fileSetChanged.connect( this, &RimReservoirGridEnsemble::onFileSetChanged );
        m_ensembleFileSet->nameChanged.connect( this,
                                                [this]( const caf::SignalEmitter* )
                                                {
                                                    if ( m_ensembleFileSet ) setName( m_ensembleFileSet->name() );
                                                } );

        // Create case objects WITHOUT loading grid data (deferred loading)
        if ( m_caseCollection && m_caseCollection->reservoirs().empty() )
        {
            createCaseObjectsFromEnsembleFileSet();
        }

        // NB! This code must be run AFTER the grid case objects are created.
        for ( auto view : m_viewCollection->views() )
        {
            if ( view )
            {
                // Resolve the grid case reference for the view after grids are loaded
                view->resolveReferencesRecursively();

                // Propagate the eclipse case to child objects to ensure all references are updated. setEclipseCase() calls
                // propagateEclipseCaseToChildObjects() internally, but we need to call it here to ensure propagation after loading and
                // reference resolution.
                auto eclipseCase = view->eclipseCase();
                view->setEclipseCase( eclipseCase );
            }
        }
    }

    // Set the case provider for views in the view collection
    if ( m_viewCollection )
    {
        m_viewCollection->setEclipseCaseProvider( [this]() { return this->cases(); } );
    }

    updateStatisticsVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::onFileSetChanged( const caf::SignalEmitter* emitter )
{
    createGridCasesFromEnsembleFileSet();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::clearActiveCellUnions()
{
    m_unionOfMatrixActiveCells->clear();
    m_unionOfFractureActiveCells->clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::clearStatisticsResults()
{
    for ( size_t i = 0; i < m_statisticsCaseCollection->reservoirs().size(); i++ )
    {
        RimEclipseCase* rimStaticsCase = m_statisticsCaseCollection->reservoirs[i];
        if ( !rimStaticsCase ) continue;

        if ( rimStaticsCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
        {
            rimStaticsCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->clearAllResults();
        }
        if ( rimStaticsCase->results( RiaDefines::PorosityModelType::FRACTURE_MODEL ) )
        {
            rimStaticsCase->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->clearAllResults();
        }

        auto views = rimStaticsCase->reservoirViews();
        for ( size_t j = 0; j < views.size(); j++ )
        {
            RimEclipseView* rimReservoirView = views[j];
            rimReservoirView->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
            rimReservoirView->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::updateStatisticsVisibility()
{
    bool hidden = !hasSharedGrid();
    m_statisticsCaseCollection->uiCapability()->setUiTreeHidden( hidden );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::updateMainGridAndActiveCellsForStatisticsCases()
{
    for ( size_t i = 0; i < m_statisticsCaseCollection->reservoirs().size(); i++ )
    {
        RimEclipseCase* rimStaticsCase = m_statisticsCaseCollection->reservoirs[i];

        if ( rimStaticsCase->eclipseCaseData() )
        {
            rimStaticsCase->eclipseCaseData()->setMainGrid( mainGrid() );

            if ( i == 0 )
            {
                rimStaticsCase->computeActiveCellsBoundingBox();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::createCaseObjectsFromEnsembleFileSet()
{
    if ( !m_ensembleFileSet ) return;

    // Get grid file paths from the file set
    QStringList gridFiles = m_ensembleFileSet->createPaths( ".EGRID" );
    if ( gridFiles.empty() )
    {
        // Try .GRID extension
        gridFiles = m_ensembleFileSet->createPaths( ".GRID" );
    }

    if ( gridFiles.empty() )
    {
        RiaLogging::warning( QString( "No grid files found for ensemble '%1'" ).arg( name() ) );
        return;
    }

    // Create case objects without loading grids
    for ( const QString& gridFile : gridFiles )
    {
        RimEclipseResultCase* resultCase = new RimEclipseResultCase();
        resultCase->setGridFileName( gridFile );
        // DO NOT call openEclipseGridFile() here - deferred loading

        RimProject::current()->assignCaseIdToCase( resultCase );
        m_caseCollection->reservoirs().push_back( resultCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::loadGridDataFromFiles()
{
    // Guard: Only load once
    if ( m_mainGrid != nullptr ) return;

    auto allCases = cases();
    if ( allCases.empty() ) return;

    // Determine effective grid mode
    if ( m_autoDetectGridType )
    {
        bool identical     = detectGridDimensionEquality();
        m_detectedGridMode = identical ? GridModeType::SHARED_GRID : GridModeType::INDIVIDUAL_GRIDS;
    }

    GridModeType effectiveMode = gridMode();

    // Load grids based on effective mode
    if ( effectiveMode == GridModeType::SHARED_GRID )
    {
        loadGridsInSharedMode();
    }
    else
    {
        // TODO: Probably not needed load grids here, they are loaded on demand in individual mode
        // loadGridsInIndividualMode();
    }

    updateStatisticsVisibility();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReservoirGridEnsemble::detectGridDimensionEquality()
{
    auto allCases = cases();
    if ( allCases.size() < 2 ) return true;

    RifReaderOpmCommon::GridDimensions firstDim;

    for ( int i = 0; i < static_cast<int>( allCases.size() ); i++ )
    {
        auto dim = RifReaderOpmCommon::readGridDimensions( allCases[i]->gridFileName() );

        if ( i == 0 )
        {
            firstDim = dim;
        }
        else if ( dim.i != firstDim.i || dim.j != firstDim.j || dim.k != firstDim.k )
        {
            return false; // Different dimensions
        }
    }

    return true; // All dimensions identical
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::loadGridsInSharedMode()
{
    auto allCases = cases();

    RiaLogging::info( QString( "Grid ensemble '%1': Loading grid in shared mode for %2 cases." ).arg( name() ).arg( allCases.size() ) );

    // Load first case fully
    RimEclipseCase* firstCase = allCases[0];
    if ( firstCase->openEclipseGridFile() )
    {
        m_mainGrid = firstCase->eclipseCaseData()->mainGrid();

        // Load remaining cases and share grid
        for ( size_t i = 1; i < allCases.size(); i++ )
        {
            RimEclipseCase* eclipseCase = allCases[i];
            if ( auto resultCase = dynamic_cast<RimEclipseResultCase*>( eclipseCase ) )
            {
                resultCase->openAndReadActiveCellData( firstCase->eclipseCaseData() );
            }
            eclipseCase->eclipseCaseData()->setMainGrid( m_mainGrid );
        }

        RigCaseCellResultsData::copyResultsMetaDataFromMainCase( firstCase->eclipseCaseData(),
                                                                 RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                 allCases );

        computeUnionOfActiveCells();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsemble::loadGridsInIndividualMode()
{
    auto allCases = cases();

    RiaLogging::info( QString( "Grid ensemble '%1': Loading grids in individual mode for %2 cases." ).arg( name() ).arg( allCases.size() ) );

    for ( auto eclipseCase : allCases )
    {
        eclipseCase->openEclipseGridFile();
    }

    // Store first grid as reference
    if ( !allCases.empty() && allCases[0]->eclipseCaseData() )
    {
        m_mainGrid = allCases[0]->eclipseCaseData()->mainGrid();
    }
}
