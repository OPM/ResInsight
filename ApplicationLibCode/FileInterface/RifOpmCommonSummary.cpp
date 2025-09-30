/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RifOpmCommonSummary.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RifOpmSummaryTools.h"

#ifdef _MSC_VER
// Disable warning from external library to make sure treat warnings as error works
#pragma warning( disable : 4267 )
#endif
#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

#include "RiaPreferencesSystem.h"
#include <QFileInfo>

size_t RifOpmCommonEclipseSummary::sm_createdEsmryFileCount = 0;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmCommonEclipseSummary::RifOpmCommonEclipseSummary()
    : m_useEsmryFiles( false )
    , m_createEsmryFiles( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmCommonEclipseSummary::~RifOpmCommonEclipseSummary() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::useEnhancedSummaryFiles( bool enable )
{
    m_useEsmryFiles = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::createEnhancedSummaryFiles( bool enable )
{
    m_createEsmryFiles = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::setEnsembleImportState( RifEnsembleImportConfig ensembleImportState )
{
    m_ensembleImportState = ensembleImportState;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::resetEnhancedSummaryFileCount()
{
    // This function can be called from a parallel loop, make it thread safe
#pragma omp critical
    sm_createdEsmryFileCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifOpmCommonEclipseSummary::numberOfEnhancedSummaryFileCreated()
{
    return sm_createdEsmryFileCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::writeEsmryFile( QString& smspecFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger )
{
    try
    {
        auto temporarySummaryFile = std::make_unique<Opm::EclIO::ESmry>( smspecFileName.toStdString(), includeRestartFiles );

        if ( temporarySummaryFile->numberOfTimeSteps() > 0 )
        {
            temporarySummaryFile->make_esmry_file();
        }

        RifOpmCommonEclipseSummary::increaseEsmryFileCount();
    }
    catch ( std::exception& )
    {
        if ( threadSafeLogger )
        {
            QString txt = QString( "Warning, could not open summary file : %1" ).arg( smspecFileName );
            threadSafeLogger->warning( txt );
        }

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::open( const QString& fileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger )
{
    // If an ESMRY file is present, and the SMSPEC file is newer than the ESMRY file, the ESMRY file will be recreated.
    // If no ESMRY file is present, the ESMRY file will be created if the flag m_createEsmryFiles is set to true.
    //
    // NB! Always make sure the logic is consistent with the logic in RifHdf5SummaryExporter::ensureHdf5FileIsCreated

    auto startTime = RiaLogging::currentTime();

    bool isEsmryConversionRequired = RifOpmSummaryTools::isEsmryConversionRequired( fileName );

    bool isLoggingEnabled = RiaPreferencesSystem::current()->isLoggingActivatedForKeyword( "OpmSummaryImport" );
    if ( isLoggingEnabled ) RiaLogging::logElapsedTime( "Check if conversion from SMSPEC to ESMRY is required", startTime );

    bool hasCreatedEsmry = false;
    if ( isEsmryConversionRequired && m_createEsmryFiles )
    {
        auto smspecFileName = RifOpmSummaryTools::smspecSummaryFilename( fileName );
        if ( writeEsmryFile( smspecFileName, includeRestartFiles, threadSafeLogger ) )
        {
            hasCreatedEsmry = true;
        }
    }

    bool importEsmryFile = m_useEsmryFiles;

    if ( isEsmryConversionRequired && !hasCreatedEsmry )
    {
        // Make sure to check the SMSPEC file name, as it is supported to import ESMRY files without any SMSPEC data.
        auto smspecFileName         = RifOpmSummaryTools::smspecSummaryFilename( fileName );
        auto candidateEsmryFileName = RifOpmSummaryTools::enhancedSummaryFilename( fileName );

        // If conversion is required, but we do not create ESMRY files, we cannot use the ESMRY file

        importEsmryFile = false;

        QString root = QFileInfo( smspecFileName ).canonicalPath();

        const QString smspecFileNameShort = QFileInfo( smspecFileName ).fileName();
        const QString esmryFileNameShort  = QFileInfo( candidateEsmryFileName ).fileName();

        RiaLogging::warning( QString( " %3 : %1 is older than %2, importing data from newest file %2." )
                                 .arg( esmryFileNameShort )
                                 .arg( smspecFileNameShort )
                                 .arg( root ) );
    }

    auto timeBeforeReader = RiaLogging::currentTime();
    if ( !openFileReader( fileName, includeRestartFiles, importEsmryFile, threadSafeLogger ) ) return false;
    if ( isLoggingEnabled ) RiaLogging::logElapsedTime( "Completed openFileReader", timeBeforeReader );

    if ( !m_standardReader && !m_enhancedReader ) return false;

    auto timeBeforeTimeSteps = RiaLogging::currentTime();
    populateTimeSteps();
    if ( isLoggingEnabled ) RiaLogging::logElapsedTime( "Completed populateTimeSteps", timeBeforeTimeSteps );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifOpmCommonEclipseSummary::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RifOpmCommonEclipseSummary::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    std::string keyword = keywordForAddress( resultAddress );
    if ( !keyword.empty() )
    {
        std::vector<double> values;
        if ( m_enhancedReader && m_enhancedReader->hasKey( keyword ) )
        {
            auto fileValues = m_enhancedReader->get( keyword );
            values.insert( values.begin(), fileValues.begin(), fileValues.end() );
        }
        else if ( m_standardReader && m_standardReader->hasKey( keyword ) )
        {
            auto fileValues = m_standardReader->get( keyword );
            values.insert( values.begin(), fileValues.begin(), fileValues.end() );
        }
        return { true, values };
    }

    return { false, {} };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifOpmCommonEclipseSummary::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    std::string nameString;

    std::string keyword = keywordForAddress( resultAddress );
    if ( !keyword.empty() )
    {
        if ( m_enhancedReader && m_enhancedReader->hasKey( keyword ) )
        {
            nameString = m_enhancedReader->get_unit( keyword );
        }
        else if ( m_standardReader && m_standardReader->hasKey( keyword ) )
        {
            nameString = m_standardReader->get_unit( keyword );
        }
    }

    return std::string( RiaStdStringTools::trimString( nameString ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifOpmCommonEclipseSummary::unitSystem() const
{
    // TODO: Not implemented
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifOpmCommonEclipseSummary::keywordCount() const
{
    if ( m_enhancedReader )
    {
        return m_enhancedReader->keywordList().size();
    }
    else if ( m_standardReader )
    {
        return m_standardReader->keywordList().size();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::createAndSetAddresses()
{
    std::vector<std::string> keywords;

    if ( m_enhancedReader )
    {
        keywords = m_enhancedReader->keywordList();
    }
    else if ( m_standardReader )
    {
        keywords = m_standardReader->keywordList();
    }

    auto [addresses, addressMap] = RifOpmSummaryTools::buildAddressesAndKeywordMap( keywords );
    m_allResultAddresses         = addresses;
    m_summaryAddressToKeywordMap = addressMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::openFileReader( const QString&       fileName,
                                                 bool                 includeRestartFiles,
                                                 bool                 importEsmryFile,
                                                 RiaThreadSafeLogger* threadSafeLogger )
{
    // Make sure to check the SMSPEC file name, as it is supported to import ESMRY files without any SMSPEC data.
    auto smspecFileName         = RifOpmSummaryTools::smspecSummaryFilename( fileName );
    auto candidateEsmryFileName = RifOpmSummaryTools::enhancedSummaryFilename( fileName );

    if ( importEsmryFile )
    {
        try
        {
            if ( QFile::exists( candidateEsmryFileName ) )
            {
                m_enhancedReader = std::make_unique<Opm::EclIO::ExtESmry>( candidateEsmryFileName.toStdString(), includeRestartFiles );
                return true;
            }
        }
        catch ( ... )
        {
            // Do not do anything here, try to open the file using standard esmy reader
        }
    }

    try
    {
        // The standard reader will import data from SMSPEC and UNSMRY files
        if ( QFile::exists( smspecFileName ) )
        {
            m_standardReader = std::make_unique<Opm::EclIO::ESmry>( smspecFileName.toStdString(), includeRestartFiles );
        }
    }
    catch ( std::exception& e )
    {
        QString txt = QString( "Optimized Summary Reader error : %1" ).arg( e.what() );

        if ( threadSafeLogger ) threadSafeLogger->error( txt );

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::populateTimeSteps()
{
    m_timeSteps.clear();

    Opm::time_point    startOfSimulation;
    std::vector<float> daysSinceStartOfSimulation;

    if ( m_enhancedReader )
    {
        startOfSimulation = m_enhancedReader->startdate();

        if ( m_enhancedReader->numberOfTimeSteps() > 0 )
        {
            daysSinceStartOfSimulation = m_enhancedReader->get( "TIME" );
        }
    }
    else if ( m_standardReader )
    {
        startOfSimulation = m_standardReader->startdate();
        if ( m_standardReader->numberOfTimeSteps() > 0 )
        {
            daysSinceStartOfSimulation = m_standardReader->get( "TIME" );
        }
    }

    const auto   startAsTimeT    = std::chrono::system_clock::to_time_t( startOfSimulation );
    const double secondsInOneDay = 24 * 3600;
    for ( const auto& days : daysSinceStartOfSimulation )
    {
        m_timeSteps.push_back( startAsTimeT + days * secondsInOneDay );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifOpmCommonEclipseSummary::keywordForAddress( const RifEclipseSummaryAddress& address ) const
{
    auto it = m_summaryAddressToKeywordMap.find( address );
    if ( it != m_summaryAddressToKeywordMap.end() )
    {
        return it->second;
    }

    // For ensembles, the address may not be in the map, so we try to convert it to a text address
    return address.toEclipseTextAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::increaseEsmryFileCount()
{
    // This function can be called from a parallel loop, make it thread safe
#pragma omp critical
    sm_createdEsmryFileCount++;
}
