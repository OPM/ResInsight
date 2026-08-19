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

#include "RimSummaryEnsembleSumo.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaQStringFormatter.h"
#include "RiaTimeTTools.h"
#include "Summary/RiaSummaryDefines.h"
#include "Summary/RiaSummaryTools.h"

#include "RifArrowTools.h"
#include "RifByteArrayArrowRandomAccessFile.h"
#include "RifEclipseSummaryAddress.h"

#include "Cloud/RimCloudDataSourceCollection.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCaseSumo.h"
#include "RimSummaryPlot.h"
#include "RimSumoDataSource.h"

#include <arrow/type_fwd.h>

#include <algorithm>
#include <map>
#include <memory>
#include <optional>

CAF_PDM_SOURCE_INIT( RimSummaryEnsembleSumo, "RimSummaryEnsembleSumo" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// Read an integer column of any width as int64. The bit width of a column is decided by the producer
/// of the parquet file, and can not be assumed to be a specific type.
//--------------------------------------------------------------------------------------------------
std::optional<std::vector<int64_t>> readIntegerColumn( const std::shared_ptr<arrow::ChunkedArray>& column )
{
    if ( !column ) return {};

    const auto typeId = column->type()->id();

    if ( typeId == arrow::Type::INT8 ) return RifArrowTools::chunkedArrayToVector<arrow::Int8Array, int64_t>( column );
    if ( typeId == arrow::Type::INT16 ) return RifArrowTools::chunkedArrayToVector<arrow::Int16Array, int64_t>( column );
    if ( typeId == arrow::Type::INT32 ) return RifArrowTools::chunkedArrayToVector<arrow::Int32Array, int64_t>( column );
    if ( typeId == arrow::Type::INT64 ) return RifArrowTools::chunkedArrayToVector<arrow::Int64Array, int64_t>( column );

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Read a floating point column of any precision as double
//--------------------------------------------------------------------------------------------------
std::optional<std::vector<double>> readFloatingPointColumn( const std::shared_ptr<arrow::ChunkedArray>& column )
{
    if ( !column ) return {};

    const auto typeId = column->type()->id();

    if ( typeId == arrow::Type::FLOAT ) return RifArrowTools::chunkedArrayToVector<arrow::FloatArray, double>( column );
    if ( typeId == arrow::Type::DOUBLE ) return RifArrowTools::chunkedArrayToVector<arrow::DoubleArray, double>( column );

    return {};
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsembleSumo::RimSummaryEnsembleSumo()
{
    CAF_PDM_InitObject( "Sumo Ensemble", ":/SummaryCase.svg", "", "The Base Class for all Summary Cases" );

    CAF_PDM_InitFieldNoDefault( &m_sumoDataSource, "SumoDataSource", "Sumo Data Source" + RiaDefines::betaFeaturePostfix() );

    // Disable IO for cases, as the reconstruction is done by loading data from Sumo
    // Will also reduce the amount of data stored in the project file
    m_cases.xmlCapability()->disableIO();

    setAsEnsemble( true );

    m_sumoConnector = RiaApplication::instance()->makeSumoConnector();

    m_lifetimeToken = std::make_shared<bool>( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::setSumoDataSource( RimSumoDataSource* sumoDataSource )
{
    m_sumoDataSource = sumoDataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryEnsembleSumo::unitName( const RifEclipseSummaryAddress& resultAddress )
{
    // TODO: Not implemented yet. Need to get the unit name from the Sumo data source
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimSummaryEnsembleSumo::unitSystem() const
{
    // TODO: Not implemented yet. Need to get the unit name from the Sumo data source
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryEnsembleSumo::allResultAddresses() const
{
    return m_resultAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::string> RimSummaryEnsembleSumo::nameKeys() const
{
    if ( m_sumoDataSource() )
    {
        return { m_sumoDataSource()->name().toStdString(), "" };
    }

    return { "Sumo Data Source", "" };
}

//--------------------------------------------------------------------------------------------------
/// Name the ensemble after the data source, like RimSummaryFileSetEnsemble does with its file set. A
/// lone ensemble needs no disambiguation and uses the short name ("iter-0"), otherwise the data source
/// name, which also carries the case name ("iter-0 (drogon_ahm)").
///
/// The base class KEY1/KEY2 template can not be used: KEY1 is the data source name and KEY2 is unused,
/// so a single Sumo ensemble falls back to the empty KEY2.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::updateName( const std::set<QString>& existingEnsembleNames )
{
    // A user defined name template is still resolved by the base class.
    if ( !isAutoNameChecked() )
    {
        RimSummaryEnsemble::updateName( existingEnsembleNames );
        return;
    }

    QString candidateName = "Sumo Data Source";

    if ( m_sumoDataSource() )
    {
        auto mainCollection = firstAncestorOrThisOfType<RimSummaryCaseMainCollection>();
        bool isOnlyEnsemble = mainCollection && mainCollection->summaryEnsembles().size() == 1;

        candidateName = isOnlyEnsemble ? m_sumoDataSource()->ensembleName() : m_sumoDataSource()->name();
    }

    if ( m_name == candidateName ) return;

    m_name = candidateName;
    caseNameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::loadSummaryData( const RifEclipseSummaryAddress& resultAddress )
{
    loadSummaryData( std::vector<RifEclipseSummaryAddress>{ resultAddress } );
}

//--------------------------------------------------------------------------------------------------
/// Load several vectors at once. Each vector is one parquet blob covering every realization, so the
/// addresses that are not cached yet are fetched as one concurrent batch rather than one after another.
/// That matters because a vector the service has not aggregated yet is produced on demand by the request
/// asking for it, so fetching serially costs the sum of those aggregations.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::loadSummaryData( const std::vector<RifEclipseSummaryAddress>& resultAddresses )
{
    // Nothing is fetched while the caller waits. A curve asking for values it does not have yet gets none,
    // the request is put on its way, and the curve is drawn again once it arrives. Waiting here instead
    // stopped the application for as long as the service took, which for a vector it has not aggregated yet
    // is a good while.
    prefetchSummaryData( resultAddresses );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::loadEnsembleParameters()
{
    if ( !m_sumoDataSource() || !m_sumoConnector ) return;

    auto sumoCaseId       = m_sumoDataSource->caseId();
    auto sumoEnsembleName = m_sumoDataSource->ensembleName();

    auto parametersKey = ParquetKey{ sumoCaseId, sumoEnsembleName, "", true };
    if ( m_parquetTable.find( parametersKey ) != m_parquetTable.end() ) return;
    if ( m_pendingVectors.find( parametersKey ) != m_pendingVectors.end() ) return;

    // Asked for without waiting, like the vectors. The service aggregates the parameters on demand too, so
    // the first request for them can take a while, and it used to be made from inside the read of a curve
    // value: dropping a vector into a plot stopped the application until the parameters had been fetched.
    m_pendingVectors[parametersKey] = RifEclipseSummaryAddress();

    std::weak_ptr<bool> isAlive = m_lifetimeToken;

    m_sumoConnector->summary().parameterDataAsync( sumoCaseId,
                                                   sumoEnsembleName,
                                                   [this, isAlive, parametersKey]( const QByteArray& contents )
                                                   {
                                                       if ( isAlive.expired() ) return;

                                                       onParameterDataReceived( parametersKey, contents );
                                                   } );
}

//--------------------------------------------------------------------------------------------------
/// The ensemble parameters have arrived. Called on the thread owning the user interface.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::onParameterDataReceived( const ParquetKey& parquetKey, const QByteArray& contents )
{
    auto it = m_pendingVectors.find( parquetKey );

    // No longer wanted: the data source changed, or the cache was cleared, while this was on its way.
    if ( it == m_pendingVectors.end() ) return;

    m_pendingVectors.erase( it );

    RiaLogging::debug( std::format( "Load ensemble parameter sensitivities. Contents size: {}", contents.size() ) );

    std::shared_ptr<arrow::Table> table = readParquetTable( contents, QString( "%1 parameter sensitivities" ).arg( parquetKey.ensembleId ) );

    m_parquetTable[parquetKey] = table;

    distributeParametersDataToRealizations( table );

    updatePlotsUsingThisEnsemble();
}

//--------------------------------------------------------------------------------------------------
/// Ask for everything the plots are about to read, and return without waiting for any of it. Each vector is
/// taken in as it arrives, so a plot shows the vectors that are ready while the rest are still on their way
/// rather than staying blank until the slowest one is done.
///
/// A vector still on its way is reported as having no data, and the curves using it are drawn empty. They are
/// redrawn when it arrives, see onVectorDataReceived.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::prefetchSummaryData( const std::vector<RifEclipseSummaryAddress>& resultAddresses )
{
    if ( !m_sumoDataSource() || !m_sumoConnector ) return;

    auto sumoCaseId       = m_sumoDataSource->caseId();
    auto sumoEnsembleName = m_sumoDataSource->ensembleName();

    std::vector<QString> vectorNamesToFetch;
    for ( const auto& resultAddress : resultAddresses )
    {
        if ( resultAddress.isStatistics() ) continue;
        if ( resultAddress.vectorName().empty() ) continue;

        auto resultText = QString::fromStdString( resultAddress.toEclipseTextAddress() );
        auto key        = ParquetKey{ sumoCaseId, sumoEnsembleName, resultText, false };

        if ( m_parquetTable.find( key ) != m_parquetTable.end() ) continue;
        if ( m_pendingVectors.find( key ) != m_pendingVectors.end() ) continue;

        m_pendingVectors[key] = resultAddress;
        vectorNamesToFetch.push_back( resultText );
    }

    // The parameters belong to the ensemble rather than to any one vector, and are wanted as soon as
    // anything of it is read. Asked for here so they travel alongside the vectors.
    loadEnsembleParameters();

    if ( vectorNamesToFetch.empty() ) return;

    std::weak_ptr<bool> isAlive = m_lifetimeToken;

    m_sumoConnector->summary().vectorDataAsync( sumoCaseId,
                                                sumoEnsembleName,
                                                vectorNamesToFetch,
                                                [this, isAlive, sumoCaseId, sumoEnsembleName]( const QString&    vectorName,
                                                                                               const QByteArray& contents )
                                                {
                                                    // The request outlived the ensemble that asked for it.
                                                    if ( isAlive.expired() ) return;

                                                    onVectorDataReceived( ParquetKey{ sumoCaseId, sumoEnsembleName, vectorName, false },
                                                                          contents );
                                                } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsembleSumo::isSummaryDataPending( const std::vector<RifEclipseSummaryAddress>& resultAddresses ) const
{
    if ( m_pendingVectors.empty() || !m_sumoDataSource() ) return false;

    auto sumoCaseId       = m_sumoDataSource()->caseId();
    auto sumoEnsembleName = m_sumoDataSource()->ensembleName();

    for ( const auto& resultAddress : resultAddresses )
    {
        auto resultText = QString::fromStdString( resultAddress.toEclipseTextAddress() );

        if ( m_pendingVectors.contains( ParquetKey{ sumoCaseId, sumoEnsembleName, resultText, false } ) ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// One requested vector has arrived. Called on the thread owning the user interface, once per vector.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::onVectorDataReceived( const ParquetKey& parquetKey, const QByteArray& contents )
{
    auto it = m_pendingVectors.find( parquetKey );

    // No longer wanted: the data source changed, or the cache was cleared, while this was on its way.
    if ( it == m_pendingVectors.end() ) return;

    const auto resultAddress = it->second;
    m_pendingVectors.erase( it );

    RiaLogging::debug( std::format( "Load Summary Data. Contents size: {}", contents.size() ) );

    // Empty contents mean the request failed. The empty result is stored like any other, so a failure is not
    // retried on every redraw, matching what a failed blocking load does.
    std::shared_ptr<arrow::Table> table = readParquetTable( contents, QString::fromStdString( resultAddress.uiText() ) );

    m_parquetTable[parquetKey] = table;
    distributeDataToRealizations( resultAddress, table );

    loadEnsembleParameters();

    updatePlotsUsingThisEnsemble();
}

//--------------------------------------------------------------------------------------------------
/// Redraw with what has arrived so far. The curves read their values again, those still waiting for data come
/// back empty, and the replot itself is coalesced by the redraw scheduler.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::updatePlotsUsingThisEnsemble()
{
    // Loading a plot can bring in the next vector, which asks for this update again. Finish the pass that is
    // running and repeat it afterwards, rather than reloading plots from inside their own load.
    if ( m_isUpdatingPlots )
    {
        m_hasMissedPlotUpdate = true;
        return;
    }

    m_isUpdatingPlots = true;

    do
    {
        m_hasMissedPlotUpdate = false;

        for ( RimSummaryPlot* summaryPlot : RimProject::current()->descendantsOfType<RimSummaryPlot>() )
        {
            if ( !summaryPlot->summaryAddressesByEnsemble().contains( this ) ) continue;

            summaryPlot->loadDataAndUpdate();
            summaryPlot->scheduleReplotIfVisible();
        }
    } while ( m_hasMissedPlotUpdate );

    m_isUpdatingPlots = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<arrow::Table> RimSummaryEnsembleSumo::readParquetTable( const QByteArray& contents, const QString& messageTag )
{
    arrow::MemoryPool* pool = arrow::default_memory_pool();

    std::shared_ptr<arrow::io::RandomAccessFile> input = std::make_shared<RifByteArrayArrowRandomAccessFile>( contents );

    std::shared_ptr<arrow::Table> table;
#if ARROW_VERSION_MAJOR >= 20
    // New API: OpenFile returns arrow::Result
    auto openResult = parquet::arrow::OpenFile( input, pool );
    if ( openResult.ok() )
    {
        std::unique_ptr<parquet::arrow::FileReader> arrow_reader = std::move( openResult ).ValueOrDie();
        if ( auto readResult = arrow_reader->ReadTable( &table ); readResult.ok() )
        {
            RiaLogging::info( std::format( "Parquet: Read table successfully for {}", messageTag ) );
        }
        else
        {
            RiaLogging::warning( std::format( "Parquet: Error detected during parsing of table. Message: {}", readResult.ToString() ) );
        }
    }
    else
    {
        RiaLogging::warning( std::format( "Parquet: Not able to open data stream. Message: {}", openResult.status().ToString() ) );
    }
#else
    // Old API: OpenFile takes output parameter
    std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
    if ( auto openResult = parquet::arrow::OpenFile( input, pool, &arrow_reader ); openResult.ok() )
    {
        if ( auto readResult = arrow_reader->ReadTable( &table ); readResult.ok() )
        {
            RiaLogging::info( std::format( "Parquet: Read table successfully for {}", messageTag ) );
        }
        else
        {
            RiaLogging::warning( std::format( "Parquet: Error detected during parsing of table. Message: {}", readResult.ToString() ) );
        }
    }
    else
    {
        RiaLogging::warning( std::format( "Parquet: Not able to open data stream. Message: {}", openResult.ToString() ) );
    }
#endif

    return table;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::distributeDataToRealizations( const RifEclipseSummaryAddress& resultAddress, std::shared_ptr<arrow::Table> table )
{
    if ( !table )
    {
        RiaLogging::warning( "Failed to load table" );
        return;
    }

    {
        // print header information
        QString txt = "Column Names: ";

        for ( std::string columnName : table->ColumnNames() )
        {
            txt += QString::fromStdString( columnName ) + " ";
        }

        RiaLogging::info( txt.toStdString() );
    }

    std::vector<time_t>  timeSteps;
    std::vector<int16_t> realizations;
    std::vector<float>   values;

    {
        const std::string                    columnName = "DATE";
        std::shared_ptr<arrow::ChunkedArray> column     = table->GetColumnByName( columnName );
        if ( column && column->type()->id() == arrow::Type::TIMESTAMP )
        {
            auto timeColumn = RifArrowTools::chunkedArrayToVector<arrow::Int64Array, int64_t>( column );
            timeSteps       = std::vector<time_t>( timeColumn.size() );

            for ( size_t i = 0; i < timeColumn.size(); ++i )
            {
                timeSteps[i] = RiaTimeTTools::fromDouble( timeColumn[i] );
            }
        }
        else
        {
            RiaLogging::warning( "Failed to find DATE column" );
            return;
        }
    }

    {
        const std::string                    columnName = "REAL";
        std::shared_ptr<arrow::ChunkedArray> column     = table->GetColumnByName( columnName );
        if ( column && column->type()->id() == arrow::Type::INT16 )
        {
            realizations = RifArrowTools::chunkedArrayToVector<arrow::Int16Array, int16_t>( column );
        }
        else
        {
            RiaLogging::warning( "Failed to find realization column" );
            return;
        }
    }

    {
        const std::string                    columnName = resultAddress.toEclipseTextAddress();
        std::shared_ptr<arrow::ChunkedArray> column     = table->GetColumnByName( columnName );
        if ( column && column->type()->id() == arrow::Type::FLOAT )
        {
            values = RifArrowTools::chunkedArrayToVector<arrow::FloatArray, float>( column );
        }
        else
        {
            RiaLogging::warning( "Failed to find values column" );
            return;
        }
    }

    // find unique realizations
    std::set<int16_t> uniqueRealizations;
    for ( auto realizationNumber : realizations )
    {
        uniqueRealizations.insert( realizationNumber );
    }

    // find start and end index for a given realization number
    std::map<int16_t, std::pair<size_t, size_t>> realizationIndex;
    for ( size_t i = 0; i < realizations.size(); ++i )
    {
        auto realizationNumber = realizations[i];
        uniqueRealizations.insert( realizationNumber );

        if ( realizationIndex.find( realizationNumber ) == realizationIndex.end() )
        {
            realizationIndex[realizationNumber] = { i, i };
        }
        else
        {
            realizationIndex[realizationNumber].second = i;
        }
    }

    auto findSummaryCase = [this]( int16_t realizationNumber ) -> RimSummaryCaseSumo*
    {
        for ( auto sumCase : allSummaryCases() )
        {
            auto sumCaseSumo = dynamic_cast<RimSummaryCaseSumo*>( sumCase );
            if ( sumCaseSumo->realizationNumber() == realizationNumber ) return sumCaseSumo;
        }

        return nullptr;
    };

    for ( auto realizationNumber : uniqueRealizations )
    {
        auto summaryCase = findSummaryCase( realizationNumber );
        if ( !summaryCase ) continue;

        auto start = realizationIndex[realizationNumber].first;
        auto end   = realizationIndex[realizationNumber].second;

        std::vector<time_t> realizationTimeSteps( timeSteps.begin() + start, timeSteps.begin() + end );
        std::vector<float>  realizationValues( values.begin() + start, values.begin() + end );

        summaryCase->setValues( realizationTimeSteps, resultAddress, realizationValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::distributeParametersDataToRealizations( std::shared_ptr<arrow::Table> table )
{
    if ( !table )
    {
        RiaLogging::warning( "Failed to load table" );
        return;
    }

    {
        // print header information
        QString txt = "Column Names: ";
        for ( std::string columnName : table->ColumnNames() )
        {
            txt += QString::fromStdString( columnName ) + " (" +
                   QString::fromStdString( table->GetColumnByName( columnName )->type()->ToString() + ") " );
        }

        RiaLogging::debug( txt.toStdString() );
    }

    std::vector<int64_t> realizations;

    {
        const std::string columnName = "REAL";
        if ( auto values = readIntegerColumn( table->GetColumnByName( columnName ) ) )
        {
            realizations = *values;
        }
        else
        {
            RiaLogging::warning( "Failed to find realization column for parameter sensitivities." );
            return;
        }
    }

    std::map<std::string, std::vector<double>>      doubleValuesForRealizations;
    std::map<std::string, std::vector<int64_t>>     intValuesForRealizations;
    std::map<std::string, std::vector<std::string>> stringValuesForRealizations;
    for ( std::string columnName : table->ColumnNames() )
    {
        if ( columnName != "REAL" )
        {
            std::shared_ptr<arrow::ChunkedArray> column = table->GetColumnByName( columnName );

            if ( column )
            {
                if ( auto values = readFloatingPointColumn( column ) )
                {
                    doubleValuesForRealizations[columnName] = *values;
                }
                else if ( auto values = readIntegerColumn( column ) )
                {
                    intValuesForRealizations[columnName] = *values;
                }
                else if ( column->type()->id() == arrow::Type::STRING )
                {
                    stringValuesForRealizations[columnName] = RifArrowTools::chunkedArrayToStringVector( column );
                }
            }
            else
            {
                RiaLogging::warning( std::format( "Failed to find values column for {}", columnName ) );
                return;
            }
        }
    }

    // find the row index in the table for each unique realization. The realization number can not be used as row index, as
    // the realization numbers are not guaranteed to be continuous and starting at zero.
    std::map<int32_t, size_t> rowIndexForRealization;
    for ( size_t i = 0; i < realizations.size(); ++i )
    {
        rowIndexForRealization.try_emplace( static_cast<int32_t>( realizations[i] ), i );
    }

    auto findSummaryCase = [this]( int32_t realizationNumber ) -> RimSummaryCaseSumo*
    {
        for ( auto sumCase : allSummaryCases() )
        {
            auto sumCaseSumo = dynamic_cast<RimSummaryCaseSumo*>( sumCase );
            if ( sumCaseSumo->realizationNumber() == realizationNumber ) return sumCaseSumo;
        }

        return nullptr;
    };

    for ( const auto& [realizationNumber, rowIndex] : rowIndexForRealization )
    {
        if ( auto summaryCase = findSummaryCase( realizationNumber ) )
        {
            auto parameters = std::make_shared<RigCaseRealizationParameters>();
            parameters->setRealizationNumber( realizationNumber );
            parameters->addParameter( RiaDefines::summaryRealizationNumber(), realizationNumber );

            for ( std::string columnName : table->ColumnNames() )
            {
                if ( columnName != "REAL" )
                {
                    if ( auto it = doubleValuesForRealizations.find( columnName ); it != doubleValuesForRealizations.end() )
                    {
                        if ( rowIndex >= it->second.size() ) continue;

                        double value = it->second[rowIndex];
                        parameters->addParameter( QString::fromStdString( columnName ), value );
                    }
                    else if ( auto it = intValuesForRealizations.find( columnName ); it != intValuesForRealizations.end() )
                    {
                        if ( rowIndex >= it->second.size() ) continue;

                        double value = static_cast<double>( it->second[rowIndex] );
                        parameters->addParameter( QString::fromStdString( columnName ), value );
                    }
                    else if ( auto it = stringValuesForRealizations.find( columnName ); it != stringValuesForRealizations.end() )
                    {
                        if ( rowIndex >= it->second.size() ) continue;

                        QString value = QString::fromStdString( it->second[rowIndex] );
                        parameters->addParameter( QString::fromStdString( columnName ), value );
                    }
                }
            }

            summaryCase->setCaseRealizationParameters( parameters );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Re-distribute already cached parquet tables to the current set of realization cases. Used after
/// the realization cases have been rebuilt (e.g. when the realization filter changes), as the cached
/// tables hold the data for all realizations and a re-download is not needed.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::redistributeCachedDataToRealizations()
{
    for ( const auto& [key, table] : m_parquetTable )
    {
        if ( !table ) continue;

        if ( key.isSensitivityParameters )
        {
            distributeParametersDataToRealizations( table );
        }
        else
        {
            auto resultAddress = RifEclipseSummaryAddress::fromEclipseTextAddress( key.vectorName.toStdString() );
            distributeDataToRealizations( resultAddress, table );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::buildMetaData()
{
    for ( auto summaryCase : allSummaryCases() )
    {
        if ( auto reader = summaryCase->summaryReader() )
        {
            reader->createAndSetAddresses();
        }
    }

    RimSummaryEnsemble::buildMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_sumoDataSource );

    auto nameGroup = uiOrdering.addNewGroup( "Name" );
    RimSummaryEnsemble::defineUiOrdering( uiConfigName, *nameGroup );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryEnsembleSumo::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_sumoDataSource )
    {
        for ( const auto& sumoDataSource : RimCloudDataSourceCollection::instance()->sumoDataSources() )
        {
            options.push_back( { sumoDataSource->name(), sumoDataSource } );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSummaryEnsemble::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_sumoDataSource )
    {
        clearCachedData();
        updateResultAddresses();
        RiaSummaryTools::updateSummaryEnsembleNames();
        buildMetaData();

        updateConnectedEditors();

        RiaSummaryTools::updateConnectedPlots( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::updateResultAddresses()
{
    m_resultAddresses.clear();

    if ( !m_sumoDataSource() ) return;

    auto vectorNames = m_sumoDataSource->vectorNames();
    for ( auto vectorName : vectorNames )
    {
        auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( vectorName.toStdString() );
        m_resultAddresses.insert( adr );
    }

    auto caseName = m_sumoDataSource->caseId().get();
    auto ensName  = m_sumoDataSource->ensembleName();

    RiaLogging::info( std::format( "Case: {}, ens: {},  vector count: {}", caseName, ensName, m_resultAddresses.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::clearCachedData()
{
    m_resultAddresses.clear();
    m_parquetTable.clear();

    // Anything still on its way belongs to the data just thrown away. Forgetting it here makes those replies
    // drop their contents on arrival, and lets the vectors be requested again if they are still wanted.
    m_pendingVectors.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::onLoadDataAndUpdate()
{
    if ( m_sumoDataSource() )
    {
        std::set<int> selectedRealizations;
        for ( const auto& realizationId : m_sumoDataSource->selectedRealizationIds() )
        {
            bool ok    = false;
            int  value = realizationId.toInt( &ok );
            if ( ok ) selectedRealizations.insert( value );
        }

        std::set<int> currentRealizations;
        for ( auto summaryCase : allSummaryCases() )
        {
            if ( auto sumoCase = dynamic_cast<RimSummaryCaseSumo*>( summaryCase ) )
            {
                currentRealizations.insert( sumoCase->realizationNumber() );
            }
        }

        // Update the realization cases whenever the selected set changes (added, removed or swapped),
        // so editing the realizations on the data source updates the summary plot.
        if ( selectedRealizations != currentRealizations )
        {
            // Update incrementally: a case whose realization is still selected is kept alive and reused.
            // Widening the selection therefore deletes nothing, which keeps the curves referring to those
            // cases valid and avoids a large tree structure change while the realization filter is being
            // edited. Only the deselected realizations are deleted.
            std::map<int, RimSummaryCaseSumo*> reusableCases;
            std::vector<RimSummaryCase*>       obsoleteCases;
            for ( auto summaryCase : m_cases.childrenByType() )
            {
                auto sumoCase = dynamic_cast<RimSummaryCaseSumo*>( summaryCase );
                if ( sumoCase && selectedRealizations.contains( sumoCase->realizationNumber() ) )
                {
                    reusableCases[sumoCase->realizationNumber()] = sumoCase;
                }
                else
                {
                    obsoleteCases.push_back( summaryCase );
                }
            }

            // Detach everything without deleting, then re-attach in realization order. The reused cases
            // survive this, so only the obsolete ones are destroyed.
            m_cases.clearWithoutDelete();

            for ( int realization : selectedRealizations )
            {
                auto it = reusableCases.find( realization );
                if ( it != reusableCases.end() )
                {
                    m_cases.push_back( it->second );
                    continue;
                }

                auto realizationCase = new RimSummaryCaseSumo();
                realizationCase->setEnsemble( this );
                realizationCase->setRealizationName( QString( "real-%1" ).arg( realization ) );
                realizationCase->setRealizationNumber( realization );
                realizationCase->updateAutoShortName();

                m_cases.push_back( realizationCase );
            }

            // Only the first case shows its tree nodes, also after cases have been reused.
            bool isFirstCase = true;
            for ( auto summaryCase : m_cases.childrenByType() )
            {
                if ( auto sumoCase = dynamic_cast<RimSummaryCaseSumo*>( summaryCase ) )
                {
                    sumoCase->setShowTreeNodes( isFirstCase );
                    isFirstCase = false;
                }
            }

            // Refresh the connected editors while the obsolete cases are still alive, so no tree item
            // editor ends up referencing freed objects during the update. Deleting them first caused a
            // use-after-free crash. caf::CmdDeleteItemExec and RicDeleteSummaryCaseCollectionFeature use
            // this same ordering.
            m_cases.uiCapability()->updateConnectedEditors();

            for ( auto obsoleteCase : obsoleteCases )
            {
                delete obsoleteCase;
            }

            // Newly created cases hold no values, while the parquet tables are still cached. Push the
            // cached data into the cases, otherwise loadSummaryData() would skip the distribution (the
            // cache lookup hits) and the new cases would have no values, making the plot empty.
            redistributeCachedDataToRealizations();
        }
    }

    RiaSummaryTools::updateSummaryEnsembleNames();
    updateResultAddresses();

    // The base class clears the child (address) nodes. Build the metadata afterwards so the available
    // vectors remain present in the tree after the cases have been rebuilt (e.g. when editing the
    // realization selection).
    RimSummaryEnsemble::onLoadDataAndUpdate();

    buildMetaData();
}

//--------------------------------------------------------------------------------------------------
/// Rebuild the realization cases from the data source selection and refresh dependent plots. This
/// mirrors RimSummaryFileSetEnsemble::onFileSetChanged, which updates the realization cases (the
/// RimSummaryCase collection) and the connected plots when the underlying file set changes.
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::onRealizationSelectionChanged()
{
    // Rebuild the realization cases (done in onLoadDataAndUpdate) and refresh the connected plots,
    // mirroring RimSummaryFileSetEnsemble::onFileSetChanged.
    //
    // Note: we intentionally do not use addCase()/replaceCases() here. Those validate ensemble
    // parameters, which Sumo realizations do not carry, and would raise a "case has no ensemble
    // parameters" warning dialog.
    loadDataAndUpdate();

    RiaSummaryTools::updateConnectedPlots( this );
    updateAllRequiredEditors();
}
