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
#include "RiaSummaryDefines.h"
#include "RiaSummaryTools.h"
#include "RiaTimeTTools.h"

#include "RifArrowTools.h"
#include "RifByteArrayArrowRandomAccessFile.h"
#include "RifEclipseSummaryAddress.h"

#include "Cloud/RimCloudDataSourceCollection.h"
#include "RimSummaryCaseSumo.h"
#include "RimSummarySumoDataSource.h"

CAF_PDM_SOURCE_INIT( RimSummaryEnsembleSumo, "RimSummaryEnsembleSumo" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsembleSumo::RimSummaryEnsembleSumo()
{
    CAF_PDM_InitObject( "Sumo Ensemble", ":/SummaryCase.svg", "", "The Base Class for all Summary Cases" );

    CAF_PDM_InitFieldNoDefault( &m_sumoDataSource, "SumoDataSource", "Sumo Data Source" );

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
void RimSummaryEnsembleSumo::updateName()
{
    if ( m_sumoDataSource() )
    {
        setName( m_sumoDataSource()->name() );
    }
    else
    {
        setName( "No Sumo Data Source" );
    }
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
void RimSummaryEnsembleSumo::loadSummaryData( const RifEclipseSummaryAddress& resultAddress )
{
    if ( resultAddress.category() == SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS ) return;

    if ( !m_sumoDataSource() ) return;

    auto resultText = QString::fromStdString( resultAddress.toEclipseTextAddress() );

    auto sumoCaseId       = m_sumoDataSource->caseId();
    auto sumoEnsembleName = m_sumoDataSource->ensembleName();

    auto key = ParquetKey{ sumoCaseId, sumoEnsembleName, resultText };

    if ( m_parquetTable.find( key ) == m_parquetTable.end() )
    {
        auto contents = loadParquetData( key );

        arrow::MemoryPool* pool = arrow::default_memory_pool();

        std::shared_ptr<arrow::io::RandomAccessFile> input = std::make_shared<RifByteArrayArrowRandomAccessFile>( contents );

        std::shared_ptr<arrow::Table>               table;
        std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
        if ( parquet::arrow::OpenFile( input, pool, &arrow_reader ).ok() )
        {
            if ( arrow_reader->ReadTable( &table ).ok() )
            {
                RiaLogging::info( "Parquet: Read table" );
            }
            else
            {
                RiaLogging::warning( "Parquet: Error detected during parsing of table" );
            }
        }
        else
        {
            RiaLogging::warning( "Parquet: Not able to open data stream" );
        }

        m_parquetTable[key] = table;

        distributeDataToRealizations( resultAddress, table );
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
void RimSummaryEnsembleSumo::buildMetaData()
{
    for ( auto summaryCase : allSummaryCases() )
    {
        summaryCase->summaryReader()->buildMetaData();
    }

    RimSummaryCaseCollection::buildMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleSumo::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_sumoDataSource );

    auto group = uiOrdering.addNewGroup( "General" );
    RimSummaryCaseCollection::defineUiOrdering( uiConfigName, *group );
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
    if ( changedField == &m_sumoDataSource )
    {
        clearCachedData();
        updateResultAddresses();
        updateName();
        buildMetaData();

        updateConnectedEditors();

        RiaSummaryTools::reloadSummaryEnsemble( this );
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

                // Create realization parameters, required to make derived ensemble cases work
                // See RimDerivedEnsembleCaseCollection::createDerivedEnsembleCases()
                auto parameters = std::shared_ptr<RigCaseRealizationParameters>( new RigCaseRealizationParameters() );

                int realizationNumber = realId.toInt();
                parameters->setRealizationNumber( realizationNumber );
                parameters->addParameter( RiaDefines::summaryRealizationNumber(), realizationNumber );

                realization->setCaseRealizationParameters( parameters );

                m_cases.push_back( realization );
            }
        }
    }

    updateName();
    updateResultAddresses();

    buildMetaData();

    // call the base class method after data has been loaded
    RimSummaryCaseCollection::onLoadDataAndUpdate();
}
