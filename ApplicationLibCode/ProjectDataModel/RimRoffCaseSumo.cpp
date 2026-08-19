/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimRoffCaseSumo.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferencesGrid.h"
#include "RiaResultNames.h"

#include "Cloud/RiaSumoConnector.h"
#include "Cloud/RifReaderSumoGridProperty.h"

#include "RifRoffFileTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"

#include "RimReservoirCellResultsStorage.h"
#include "RimReservoirGridEnsembleBase.h"
#include "Sumo/RimSumoDataSource.h"

#include "cafPdmObjectScriptingCapability.h"

#include <QDate>
#include <QDateTime>

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <vector>

CAF_PDM_SOURCE_INIT( RimRoffCaseSumo, "RimRoffCaseSumo" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRoffCaseSumo::RimRoffCaseSumo()
{
    CAF_PDM_InitScriptableObject( "Sumo Grid Case", ":/Case48x48.png" );

    CAF_PDM_InitFieldNoDefault( &m_sumoDataSource, "SumoDataSource", "Sumo Data Source" );
    m_sumoDataSource.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_sumoCaseId, "SumoCaseId", "Sumo Case Id" );
    m_sumoCaseId.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_ensembleName, "EnsembleName", "Ensemble Name" );
    m_ensembleName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_gridName, "GridName", "Grid Name" );
    m_gridName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_realization, "Realization", -1, "Realization" );
    m_realization.uiCapability()->setUiReadOnly( true );

    m_sumoConnector = RiaApplication::instance()->makeSumoConnector();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRoffCaseSumo::~RimRoffCaseSumo()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRoffCaseSumo* RimRoffCaseSumo::createFromDataSource( RimSumoDataSource* dataSource, const QString& gridName, int realization )
{
    if ( !dataSource ) return nullptr;

    auto* gridCase = new RimRoffCaseSumo();
    gridCase->setSumoDataSource( dataSource );
    gridCase->setSumoCaseId( dataSource->caseId().get() );
    gridCase->setEnsembleName( dataSource->ensembleName() );
    gridCase->setGridName( gridName );
    gridCase->setRealization( realization );

    // The grid is stored on Sumo, not on disk, so there is no real grid file name. Still assign a unique
    // synthetic one: RimReservoirGridEnsemble identifies its realizations by grid file name, see contains()
    // and findByFileName().
    gridCase->setGridFileName(
        QString( "sumo/%1/%2/realization-%3/%4.roff" ).arg( dataSource->caseId().get() ).arg( gridName ).arg( realization ).arg( gridName ) );

    // Name the case using grid name, asset, ensemble and realization, e.g. "Geogrid_Drogon_iter-0_Real_0".
    QString caseDisplayName =
        QString( "%1_%2_%3_Real_%4" ).arg( gridName, dataSource->assetName(), dataSource->ensembleName() ).arg( realization );
    gridCase->setCustomCaseName( caseDisplayName );

    return gridCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::setSumoDataSource( RimSumoDataSource* dataSource )
{
    m_sumoDataSource = dataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::setSumoCaseId( const QString& sumoCaseId )
{
    m_sumoCaseId = sumoCaseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::setEnsembleName( const QString& ensembleName )
{
    m_ensembleName = ensembleName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::setGridName( const QString& gridName )
{
    m_gridName = gridName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::setRealization( int realization )
{
    m_realization = realization;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRoffCaseSumo::gridName() const
{
    return m_gridName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimRoffCaseSumo::realization() const
{
    return m_realization();
}

//--------------------------------------------------------------------------------------------------
/// Every realization downloads and parses its own grid blob, because a roff grid carries its geometry and
/// its active cells in one stream and there is no way to read the active cells alone. When the ensemble
/// already holds a grid of the same dimensions, the geometry just parsed is released and the shared grid
/// is used instead: shared grid mode keeps one RigMainGrid in memory for the whole ensemble rather than
/// one per realization. The active cells are always this realization's own, ACTNUM is never assumed to be
/// equal across realizations.
//--------------------------------------------------------------------------------------------------
bool RimRoffCaseSumo::openEclipseGridFile()
{
    if ( eclipseCaseData() )
    {
        // Early exit if reservoir data is created
        return true;
    }

    if ( !m_sumoConnector )
    {
        RiaLogging::error( "No Sumo connector available, unable to load grid from Sumo." );
        return false;
    }

    setReservoirData( new RigEclipseCaseData( this ) );

    // RifRoffFileTools::openGridFile appends into the grid it is given, so this must be a grid of our own.
    // The shared grid can only be adopted once the parsing is done.
    if ( !downloadAndParseGrid() )
    {
        // Leave no half built case behind, so opening this case can be retried after a failed transfer.
        setReservoirData( nullptr );
        return false;
    }

    RigMainGrid* parsedGrid    = eclipseCaseData()->mainGrid();
    RigMainGrid* gridToUse     = parsedGrid;
    QString      gridOwnership = "own grid (case is not in a reservoir grid ensemble)";

    if ( auto* gridEnsemble = parentGridEnsembleBase() )
    {
        gridToUse     = gridEnsemble->shareOrAdoptMainGrid( parsedGrid );
        gridOwnership = ( gridToUse != parsedGrid ) ? "shared grid"
                        : gridEnsemble->gridMode() == RimReservoirGridEnsembleBase::GridModeType::SHARED_GRID
                            ? "own grid (published as the ensemble shared grid)"
                            : "own grid (ensemble uses individual grids)";
    }

    if ( gridToUse != parsedGrid )
    {
        // Point at the shared grid and release the geometry just parsed. The active cell info produced by
        // the same parse belongs to this realization and is kept.
        eclipseCaseData()->setMainGrid( gridToUse );

        // The cell search tree, the bounding box and the faults of the shared grid were computed by the
        // realization that loaded it, so computeCachedData() is not repeated here. The active cell bounding
        // box is per realization and must be computed, as in RimEclipseResultCase::openAndReadActiveCellData.
        computeActiveCellsBoundingBox();
        eclipseCaseData()->setActiveFormationNames( effectiveFormationNames() );
    }
    else
    {
        parsedGrid->setFlipAxis( m_flipXAxis, m_flipYAxis );
        computeCachedData();
    }

    // The same grid address for every realization, with the reference count climbing, is what shows the
    // grid is shared. The active cell count is this realization's own, and is expected to vary between
    // realizations even when they share a grid.
    const auto* activeCells = eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RiaLogging::debug( std::format( "Sumo grid '{}' realization {}: {} {}, {}x{}x{}, {} active cells, grid ref count {}",
                                    m_gridName().toStdString(),
                                    m_realization(),
                                    gridOwnership.toStdString(),
                                    static_cast<const void*>( gridToUse ),
                                    gridToUse->cellCountI(),
                                    gridToUse->cellCountJ(),
                                    gridToUse->cellCountK(),
                                    activeCells ? activeCells->reservoirActiveCellCount() : 0,
                                    gridToUse->refCount() ) );

    finalizeCaseSetup();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Download the roff grid blob for this realization and parse it into the case data.
//--------------------------------------------------------------------------------------------------
bool RimRoffCaseSumo::downloadAndParseGrid()
{
    QByteArray contents = m_sumoConnector->grid().gridData( SumoCaseId( m_sumoCaseId() ), m_ensembleName(), m_gridName(), m_realization() );
    if ( contents.isEmpty() )
    {
        RiaLogging::error(
            std::format( "Failed to download grid '{}' (realization {}) from Sumo.", m_gridName().toStdString(), m_realization() ) );
        return false;
    }

    // The downloaded blob is a binary roff grid. Parse it directly from memory.
    std::string        buffer = contents.toStdString();
    std::istringstream stream( buffer, std::ios::binary );

    QString errorMessages;
    if ( !RifRoffFileTools::openGridFile( stream, eclipseCaseData(), &errorMessages ) )
    {
        RiaLogging::error( errorMessages.toStdString() );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Register the results and attach the property reader. Shared by the two ways the grid can end up on
/// this case: parsed and kept, or parsed and replaced by the grid shared with the other realizations.
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::finalizeCaseSetup()
{
    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createPlaceholderResultEntries();

    if ( RiaPreferencesGrid::current()->autoComputeDepthRelatedProperties() )
    {
        eclipseCaseData()->computeDepthRelatedResults();
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeCellVolumes();

    // Make the Sumo grid properties available as cell results (fetched on demand when displayed).
    registerSumoGridProperties();

    // Rebuild the Data Sources result folders now that the result meta data is available.
    updateResultAddressCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRoffCaseSumo::locationOnDisc() const
{
    // The grid is stored on Sumo, not on disk.
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_caseUserDescription );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_caseId );

    auto group = uiOrdering.addNewGroup( "Sumo" );
    group->add( &m_sumoCaseId );
    group->add( &m_ensembleName );
    group->add( &m_gridName );
    group->add( &m_realization );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRoffCaseSumo::registerSumoGridProperties()
{
    if ( !m_sumoConnector || !eclipseCaseData() ) return;

    const auto gridPropertyInfos =
        m_sumoConnector->grid().propertyInfo( SumoCaseId( m_sumoCaseId() ), m_ensembleName(), m_gridName(), m_realization() );

    // Properties without a timestamp are static. Properties with a single timestamp are dynamic (one time
    // step per timestamp). Time intervals (the iso string contains '/') are not supported and skipped.
    std::vector<QString>                 staticPropertyNames;
    std::map<QString, std::set<QString>> dynamicPropertyTimestamps; // property name -> the timestamps it has
    std::set<QString>                    allTimestamps; // union of timestamps across all properties
    for ( const auto& info : gridPropertyInfos )
    {
        if ( info.isoDateOrInterval.isEmpty() )
        {
            staticPropertyNames.push_back( info.name );
        }
        else if ( !info.isoDateOrInterval.contains( '/' ) )
        {
            dynamicPropertyTimestamps[info.name].insert( info.isoDateOrInterval );
            allTimestamps.insert( info.isoDateOrInterval );
        }
    }

    if ( staticPropertyNames.empty() && dynamicPropertyTimestamps.empty() ) return;

    // The Eclipse readers take the available phases from the INIT file. There is no INIT file here, so derive
    // the phases from the saturation properties the case actually has. Without this the phase set stays empty,
    // and RigCaseCellResultsData::defaultResult() skips its SOIL and SGAS branches - both are guarded by the
    // phase set, while the SWAT branch is not - so a new view would always open on SWAT.
    auto hasProperty = [&]( const QString& name )
    { return dynamicPropertyTimestamps.contains( name ) || std::ranges::find( staticPropertyNames, name ) != staticPropertyNames.end(); };

    std::set<RiaDefines::PhaseType> availablePhases;
    if ( hasProperty( RiaResultNames::soil() ) ) availablePhases.insert( RiaDefines::PhaseType::OIL_PHASE );
    if ( hasProperty( RiaResultNames::sgas() ) ) availablePhases.insert( RiaDefines::PhaseType::GAS_PHASE );
    if ( hasProperty( RiaResultNames::swat() ) ) availablePhases.insert( RiaDefines::PhaseType::WATER_PHASE );

    if ( !availablePhases.empty() ) eclipseCaseData()->setAvailablePhases( availablePhases );

    auto cellResults = results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !cellResults ) return;

    auto reader = new RifReaderSumoGridProperty( m_sumoConnector, m_sumoCaseId(), m_ensembleName(), m_gridName(), m_realization() );
    reader->open( "", eclipseCaseData() );

    // Register the property names as cell results so they are listed in the cell result editor. The values are
    // not loaded here; the reader fetches them on demand the first time a property is displayed.

    for ( const auto& name : staticPropertyNames )
    {
        RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::ResultDataType::FLOAT, name );
        cellResults->createResultEntry( resultAddress, false );
    }
    reader->setStaticProperties( staticPropertyNames );

    if ( !dynamicPropertyTimestamps.empty() )
    {
        auto parseTimestamp = []( const QString& isoString ) -> QDateTime
        {
            QDateTime dateTime = QDateTime::fromString( isoString, Qt::ISODate );
            if ( !dateTime.isValid() )
            {
                // Date-only string, e.g. "2018-01-01".
                QDate date = QDate::fromString( isoString, Qt::ISODate );
                if ( date.isValid() ) dateTime = QDateTime( date, QTime( 0, 0, 0 ) );
            }
            return dateTime;
        };

        // All dynamic results share one common, case-wide set of time steps so they use the same time step
        // index space as the 3D view time slider. std::set is sorted, and ISO date strings sort chronologically.
        std::vector<QString> commonTimestamps( allTimestamps.begin(), allTimestamps.end() );

        std::vector<QDateTime> dates;
        std::vector<int>       reportNumbers;
        std::vector<double>    daysSinceStart;
        for ( int i = 0; i < static_cast<int>( commonTimestamps.size() ); i++ )
        {
            QDateTime date = parseTimestamp( commonTimestamps[i] );
            dates.push_back( date );
            reportNumbers.push_back( i );
            daysSinceStart.push_back( ( dates.front().isValid() && date.isValid() ) ? dates.front().daysTo( date ) : static_cast<double>( i ) );
        }
        auto commonTimeStepInfos = RigEclipseTimeStepInfo::createTimeStepInfos( dates, reportNumbers, daysSinceStart );

        // For each property, build a list aligned with commonTimestamps. An empty entry marks a time step the
        // property has no data for, so the reader reports "no data" there instead of another step's values.
        std::map<QString, std::vector<QString>> readerDynamicTimestamps;
        for ( const auto& [name, propertyTimestamps] : dynamicPropertyTimestamps )
        {
            RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaDefines::ResultDataType::FLOAT, name );
            cellResults->createResultEntry( resultAddress, false );
            cellResults->setTimeStepInfos( resultAddress, commonTimeStepInfos );

            std::vector<QString> alignedTimestamps;
            alignedTimestamps.reserve( commonTimestamps.size() );
            for ( const auto& timestamp : commonTimestamps )
            {
                alignedTimestamps.push_back( propertyTimestamps.count( timestamp ) > 0 ? timestamp : QString() );
            }
            readerDynamicTimestamps[name] = alignedTimestamps;
        }

        reader->setDynamicProperties( readerDynamicTimestamps );
    }

    cellResults->setReaderInterface( reader );
}
