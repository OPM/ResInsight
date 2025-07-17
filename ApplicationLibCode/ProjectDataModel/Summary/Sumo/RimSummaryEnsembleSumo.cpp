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
#include "RiaTimeTTools.h"
#include "Summary/RiaSummaryDefines.h"
#include "Summary/RiaSummaryTools.h"

#include "RifArrowTools.h"
#include "RifByteArrayArrowRandomAccessFile.h"
#include "RifEclipseSummaryAddress.h"

#include "Cloud/RimCloudDataSourceCollection.h"
#include "RimSummaryCaseSumo.h"
#include "RimSummarySumoDataSource.h"

#include <arrow/type_fwd.h>

CAF_PDM_SOURCE_INIT( RimSummaryEnsembleSumo, "RimSummaryEnsembleSumo" );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::setSumoDataSource( RimSummarySumoDataSource* sumoDataSource )
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
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::loadSummaryData( const RifEclipseSummaryAddress& resultAddress )
{
    if ( resultAddress.isStatistics() ) return;

    if ( !m_sumoDataSource() ) return;

    auto resultText = QString::fromStdString( resultAddress.toEclipseTextAddress() );

    auto sumoCaseId       = m_sumoDataSource->caseId();
    auto sumoEnsembleName = m_sumoDataSource->ensembleName();

    auto key = ParquetKey{ sumoCaseId, sumoEnsembleName, resultText, false };
    if ( m_parquetTable.find( key ) == m_parquetTable.end() )
    {
        auto contents = loadParquetData( key );
        RiaLogging::debug( QString( "Load Summary Data. Contents size: %1" ).arg( contents.size() ) );

        std::shared_ptr<arrow::Table> table = readParquetTable( contents, QString::fromStdString( resultAddress.uiText() ) );
        m_parquetTable[key]                 = table;

        distributeDataToRealizations( resultAddress, table );
    }

    auto parametersKey = ParquetKey{ sumoCaseId, sumoEnsembleName, "", true };
    if ( m_parquetTable.find( parametersKey ) == m_parquetTable.end() )
    {
        auto contents = m_sumoConnector->requestParametersParquetDataBlocking( sumoCaseId, sumoEnsembleName );
        RiaLogging::debug( QString( "Load ensemble parameter sensitivities. Contents size: %1" ).arg( contents.size() ) );

        std::shared_ptr<arrow::Table> table = readParquetTable( contents, QString( "%1 parameter sensitivities" ).arg( sumoEnsembleName ) );
        m_parquetTable[parametersKey]       = table;

        distributeParametersDataToRealizations( table );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RimSummaryEnsembleSumo::loadParquetData( const ParquetKey& parquetKey )
{
    if ( !m_sumoConnector ) return {};

    return m_sumoConnector->requestParquetDataBlocking( SumoCaseId( parquetKey.caseId ), parquetKey.ensembleId, parquetKey.vectorName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<arrow::Table> RimSummaryEnsembleSumo::readParquetTable( const QByteArray& contents, const QString& messageTag )
{
    arrow::MemoryPool* pool = arrow::default_memory_pool();

    std::shared_ptr<arrow::io::RandomAccessFile> input = std::make_shared<RifByteArrayArrowRandomAccessFile>( contents );

    std::shared_ptr<arrow::Table>               table;
    std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
    if ( auto openResult = parquet::arrow::OpenFile( input, pool, &arrow_reader ); openResult.ok() )
    {
        if ( auto readResult = arrow_reader->ReadTable( &table ); readResult.ok() )
        {
            RiaLogging::info( QString( "Parquet: Read table successfully for %1" ).arg( messageTag ) );
        }
        else
        {
            RiaLogging::warning(
                QString( "Parquet: Error detected during parsing of table. Message: %1" ).arg( QString::fromStdString( readResult.ToString() ) ) );
        }
    }
    else
    {
        RiaLogging::warning(
            QString( "Parquet: Not able to open data stream. Message: %1" ).arg( QString::fromStdString( openResult.ToString() ) ) );
    }

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

        RiaLogging::info( txt );
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

        RiaLogging::debug( txt );
    }

    std::vector<int64_t> realizations;

    {
        const std::string                    columnName = "REAL";
        std::shared_ptr<arrow::ChunkedArray> column     = table->GetColumnByName( columnName );
        if ( column && column->type()->id() == arrow::Type::INT64 )
        {
            realizations = RifArrowTools::chunkedArrayToVector<arrow::Int64Array, int64_t>( column );
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
                if ( column->type()->id() == arrow::Type::DOUBLE )
                {
                    std::vector<double> values              = RifArrowTools::chunkedArrayToVector<arrow::DoubleArray, double>( column );
                    doubleValuesForRealizations[columnName] = values;
                }
                else if ( column->type()->id() == arrow::Type::INT64 )
                {
                    std::vector<int64_t> values          = RifArrowTools::chunkedArrayToVector<arrow::Int64Array, int64_t>( column );
                    intValuesForRealizations[columnName] = values;
                }
                else if ( column->type()->id() == arrow::Type::STRING )
                {
                    std::vector<std::string> values         = RifArrowTools::chunkedArrayToStringVector( column );
                    stringValuesForRealizations[columnName] = values;
                }
            }
            else
            {
                RiaLogging::warning( QString( "Failed to find values column for %1" ).arg( QString::fromStdString( columnName ) ) );
                return;
            }
        }
    }

    // find unique realizations
    std::set<int16_t> uniqueRealizations;
    for ( auto realizationNumber : realizations )
    {
        uniqueRealizations.insert( realizationNumber );
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
                        double value = it->second[realizationNumber];
                        parameters->addParameter( QString::fromStdString( columnName ), value );
                    }
                    else if ( auto it = intValuesForRealizations.find( columnName ); it != intValuesForRealizations.end() )
                    {
                        double value = it->second[realizationNumber];
                        parameters->addParameter( QString::fromStdString( columnName ), value );
                    }
                    else if ( auto it = stringValuesForRealizations.find( columnName ); it != stringValuesForRealizations.end() )
                    {
                        QString value = QString::fromStdString( it->second[realizationNumber] );
                        parameters->addParameter( QString::fromStdString( columnName ), value );
                    }
                }
            }

            summaryCase->setCaseRealizationParameters( parameters );
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
        summaryCase->summaryReader()->createAndSetAddresses();
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

    RiaLogging::info( QString( "Case: %1, ens: %2,  vector count: %3" ).arg( caseName ).arg( ensName ).arg( m_resultAddresses.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::clearCachedData()
{
    m_resultAddresses.clear();
    m_parquetTable.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::onLoadDataAndUpdate()
{
    if ( m_sumoDataSource() )
    {
        auto realizationIds = m_sumoDataSource->realizationIds();
        if ( realizationIds.size() != m_cases.size() )
        {
            m_cases.deleteChildren();

            for ( auto realId : realizationIds )
            {
                auto realization = new RimSummaryCaseSumo();
                realization->setEnsemble( this );
                realization->setRealizationName( QString( "real-%1" ).arg( realId ) );
                realization->setRealizationNumber( realId.toInt() );
                realization->updateAutoShortName();

                realization->setShowVectorItemsInProjectTree( m_cases.empty() );

                m_cases.push_back( realization );
            }
        }
    }

    RiaSummaryTools::updateSummaryEnsembleNames();
    updateResultAddresses();

    buildMetaData();

    // call the base class method after data has been loaded
    RimSummaryEnsemble::onLoadDataAndUpdate();
}
