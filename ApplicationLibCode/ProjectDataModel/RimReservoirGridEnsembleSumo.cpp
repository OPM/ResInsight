/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimReservoirGridEnsembleSumo.h"

#include "RiaLogging.h"

// The reference counted members of the base class need complete types here, so the destructor of this
// class can be generated.
#include "RigActiveCellInfo.h"
#include "RigMainGrid.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimRoffCaseSumo.h"
#include "Sumo/RimSumoDataSource.h"

#include <format>

CAF_PDM_SOURCE_INIT( RimReservoirGridEnsembleSumo, "RimReservoirGridEnsembleSumo" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsembleSumo::RimReservoirGridEnsembleSumo()
{
    CAF_PDM_InitObject( "Sumo Reservoir Grid Ensemble", ":/GridCaseGroup16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_sumoDataSource, "SumoDataSource", "Sumo Data Source" );
    m_sumoDataSource.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_gridName, "GridName", "Grid Name" );
    m_gridName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_realizations, "Realizations", "Realizations" );
    m_realizations.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_gridRealizations, "GridRealizations", "Grid Realizations" );
    m_gridRealizations.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_gridDimensionsAreIdentical, "GridDimensionsAreIdentical", false, "Grid Dimensions Are Identical" );
    m_gridDimensionsAreIdentical.uiCapability()->setUiHidden( true );

    // Set by the data source creating the ensemble, see RimSumoDataSource.
    CAF_PDM_InitField( &m_doComputeMobileVolumeWeightedMean, "DoComputeMobileVolumeWeightedMean", false, "Compute Mobile Volume Weighted Mean" );
    m_doComputeMobileVolumeWeightedMean.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleSumo::setSumoSource( RimSumoDataSource*      dataSource,
                                                  const QString&          gridName,
                                                  const std::vector<int>& realizations,
                                                  bool                    gridDimensionsAreIdentical )
{
    m_sumoDataSource             = dataSource;
    m_gridName                   = gridName;
    m_realizations               = realizations;
    m_gridDimensionsAreIdentical = gridDimensionsAreIdentical;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleSumo::setGridRealizations( const std::vector<int>& realizations )
{
    m_gridRealizations = realizations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimReservoirGridEnsembleSumo::gridRealizations() const
{
    return m_gridRealizations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimReservoirGridEnsembleSumo::sumoGridName() const
{
    return m_gridName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReservoirGridEnsembleSumo::doComputeMobileVolumeWeightedMean() const
{
    return m_doComputeMobileVolumeWeightedMean();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleSumo::setDoComputeMobileVolumeWeightedMean( bool enable )
{
    m_doComputeMobileVolumeWeightedMean = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleSumo::createGridCasesFromSumoSource()
{
    if ( !m_sumoDataSource ) return;

    recreateCaseObjects();
}

//--------------------------------------------------------------------------------------------------
/// Create one grid case per realization. Called both when the ensemble is created and when the project is
/// loaded, as the realization cases are not written to the project file.
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleSumo::createCaseObjects()
{
    if ( !m_sumoDataSource ) return;

    auto* project = RimProject::current();

    for ( int realization : m_realizations() )
    {
        auto* gridCase = RimRoffCaseSumo::createFromDataSource( m_sumoDataSource, m_gridName(), realization );
        if ( !gridCase ) continue;

        // The ensemble is put in the project tree before the cases are created, so the ids are unique.
        if ( project ) project->assignCaseIdToCase( gridCase );

        caseCollection()->reservoirs().push_back( gridCase );
    }
}

//--------------------------------------------------------------------------------------------------
/// Sumo reports the IJK dimensions of every realization, so this is decided when the ensemble is created,
/// without reading any grid. See RicCreateSumoReservoirGridEnsembleFeature.
//--------------------------------------------------------------------------------------------------
bool RimReservoirGridEnsembleSumo::detectGridDimensionEquality()
{
    return m_gridDimensionsAreIdentical();
}

//--------------------------------------------------------------------------------------------------
/// Open the first realization, whose grid becomes the grid shared by the whole ensemble. The other
/// realizations are opened lazily, each downloading and parsing its own blob to read its own active
/// cells before releasing the parsed geometry, see RimRoffCaseSumo::openEclipseGridFile. Opening a
/// realization means downloading a grid, so they are not opened up front.
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleSumo::loadGridsInSharedMode()
{
    auto allCases = cases();
    if ( allCases.empty() ) return;

    RiaLogging::info(
        std::format( "Grid ensemble '{}': loading the shared grid from the first of {} realizations.", name().toStdString(), allCases.size() ) );

    // openReservoirCase, not openEclipseGridFile, so the characteristic cell size and the face normals are
    // computed once, on the grid the other realizations share.
    allCases[0]->openReservoirCase();

    // computeUnionOfActiveCells is deliberately not called here: only one realization is open, and the
    // union caches its result. It is computed on demand, after every realization has been opened.
}

//--------------------------------------------------------------------------------------------------
/// The realization cases are not written to the project file, they are recreated by createCaseObjects.
/// Capture the realizations in the order the cases appear, so they come back in the same order: the
/// references from the views to the cases are positional.
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleSumo::setupBeforeSave()
{
    std::vector<int> realizations;

    for ( auto* eclipseCase : cases() )
    {
        if ( auto* sumoCase = dynamic_cast<RimRoffCaseSumo*>( eclipseCase ) )
        {
            realizations.push_back( sumoCase->realization() );
        }
    }

    m_realizations = realizations;
}
