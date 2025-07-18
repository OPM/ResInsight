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

#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#ifdef _MSC_VER
// Disable warning from external library to make sure treat warnings as error works
#pragma warning( disable : 4267 )
#endif
#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

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
void RifOpmCommonEclipseSummary::resetEnhancedSummaryFileCount()
{
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
bool RifOpmCommonEclipseSummary::open( const QString& fileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger )
{
    // If an ESMRY file is present, and the SMSPEC file is newer than the ESMRY file, the ESMRY file will be recreated.
    // If no ESMRY file is present, the ESMRY file will be created if the flag m_createEsmryFiles is set to true.
    //
    // NB! Always make sure the logic is consistent with the logic in RifHdf5SummaryExporter::ensureHdf5FileIsCreated

    bool writeDataToEsmry = false;

    auto candidateEsmryFileName = enhancedSummaryFilename( fileName );

    // Make sure to check the smspec file name, as it is supported to import ESMRY files without any SMSPEC data
    auto smspecFileName = smspecSummaryFilename( fileName );

    if ( !QFile::exists( candidateEsmryFileName ) && QFile::exists( smspecFileName ) && m_createEsmryFiles )
    {
        writeDataToEsmry = true;
    }

    if ( RiaFilePathTools::isFirstOlderThanSecond( candidateEsmryFileName.toStdString(), smspecFileName.toStdString() ) )
    {
        QString root = QFileInfo( smspecFileName ).canonicalPath();

        const QString smspecFileNameShort = QFileInfo( smspecFileName ).fileName();
        const QString esmryFileNameShort  = QFileInfo( candidateEsmryFileName ).fileName();

        RiaLogging::debug(
            QString( " %3 : %1 is older than %2, recreating %1." ).arg( esmryFileNameShort ).arg( smspecFileNameShort ).arg( root ) );

        // Check if we have write permission in the folder
        QFileInfo info( smspecFileName );

        if ( !info.isWritable() )
        {
            QString txt =
                QString( "ESMRY is older than SMSPEC, but export to file %1 failed due to missing write permissions. Aborting operation." )
                    .arg( candidateEsmryFileName );
            RiaLogging::error( txt );

            return false;
        }

        std::filesystem::remove( candidateEsmryFileName.toStdString() );

        writeDataToEsmry = true;
    }

    if ( writeDataToEsmry )
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
    }

    if ( !openFileReader( fileName, includeRestartFiles, threadSafeLogger ) ) return false;

    if ( !m_standardReader && !m_enhancedReader ) return false;

    populateTimeSteps();

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

    auto [addresses, addressMap] = RifOpmCommonSummaryTools::buildAddressesAndKeywordMap( keywords );
    m_allResultAddresses         = addresses;
    m_summaryAddressToKeywordMap = addressMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::openFileReader( const QString& fileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger )
{
    // Make sure to check the SMSPEC file name, as it is supported to import ESMRY files without any SMSPEC data.
    auto smspecFileName = smspecSummaryFilename( fileName );

    if ( m_useEsmryFiles )
    {
        try
        {
            auto candidateEsmryFileName = enhancedSummaryFilename( fileName );

            if ( QFile::exists( candidateEsmryFileName ) )
            {
                bool isValidEsmryFile = false;

                if ( !QFile::exists( smspecFileName ) )
                {
                    // No SMSPEC file present, OK to import ESMRY file
                    isValidEsmryFile = true;
                }
                else if ( RiaFilePathTools::isFirstOlderThanSecond( smspecFileName.toStdString(), candidateEsmryFileName.toStdString() ) )
                {
                    isValidEsmryFile = true;
                }
                else
                {
                    QString root = QFileInfo( smspecFileName ).canonicalPath();

                    const QString smspecFileNameShort = QFileInfo( smspecFileName ).fileName();
                    const QString esmryFileNameShort  = QFileInfo( candidateEsmryFileName ).fileName();

                    RiaLogging::warning( QString( " %3 : %1 is older than %2, importing data from newest file %2." )
                                             .arg( esmryFileNameShort )
                                             .arg( smspecFileNameShort )
                                             .arg( root ) );
                }

                if ( isValidEsmryFile )
                {
                    m_enhancedReader = std::make_unique<Opm::EclIO::ExtESmry>( candidateEsmryFileName.toStdString(), includeRestartFiles );

                    return true;
                }
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

    populateTimeSteps();

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifOpmCommonEclipseSummary::enhancedSummaryFilename( const QString& fileName )
{
    QString s( fileName );
    return s.replace( ".SMSPEC", ".ESMRY" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifOpmCommonEclipseSummary::smspecSummaryFilename( const QString& fileName )
{
    QString s( fileName );
    return s.replace( ".ESMRY", ".SMSPEC" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<std::set<RifEclipseSummaryAddress>, std::map<RifEclipseSummaryAddress, size_t>, std::map<RifEclipseSummaryAddress, std::string>>
    RifOpmCommonSummaryTools::buildAddressesSmspecAndKeywordMap( const Opm::EclIO::ESmry* summaryFile )
{
    std::set<RifEclipseSummaryAddress>              addresses;
    std::map<RifEclipseSummaryAddress, size_t>      addressToSmspecIndexMap;
    std::map<RifEclipseSummaryAddress, std::string> addressToKeywordMap;

    if ( summaryFile )
    {
        auto keywords = summaryFile->keywordList();
        for ( const auto& keyword : keywords )
        {
            auto eclAdr = RifEclipseSummaryAddress::fromEclipseTextAddress( keyword );
            if ( !eclAdr.isValid() )
            {
                // If a category is not found, use the MISC category
                eclAdr = RifEclipseSummaryAddress::miscAddress( keyword );
            }

            if ( eclAdr.isValid() )
            {
                addresses.insert( eclAdr );
                size_t smspecIndex              = summaryFile->getSmspecIndexForKeyword( keyword );
                addressToSmspecIndexMap[eclAdr] = smspecIndex;
                addressToKeywordMap[eclAdr]     = keyword;
            }
        }
    }

    return { addresses, addressToSmspecIndexMap, addressToKeywordMap };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::set<RifEclipseSummaryAddress>, std::map<RifEclipseSummaryAddress, std::string>>
    RifOpmCommonSummaryTools::buildAddressesAndKeywordMap( const std::vector<std::string>& keywords )
{
    std::set<RifEclipseSummaryAddress>              addresses;
    std::map<RifEclipseSummaryAddress, std::string> addressToKeywordMap;

    std::vector<std::string> invalidKeywords;

#pragma omp parallel
    {
        std::vector<RifEclipseSummaryAddress>                         threadAddresses;
        std::vector<std::pair<RifEclipseSummaryAddress, std::string>> threadAddressToKeywordMap;
        std::vector<std::string>                                      threadInvalidKeywords;

#pragma omp for
        for ( int index = 0; index < (int)keywords.size(); index++ )
        {
            auto keyword = keywords[index];

            auto eclAdr = RifEclipseSummaryAddress::fromEclipseTextAddress( keyword );
            if ( !eclAdr.isValid() )
            {
                threadInvalidKeywords.push_back( keyword );

                // If a category is not found, use the MISC category
                eclAdr = RifEclipseSummaryAddress::miscAddress( keyword );
            }

            if ( eclAdr.isValid() )
            {
                threadAddresses.emplace_back( eclAdr );
                threadAddressToKeywordMap.emplace_back( std::make_pair( eclAdr, keyword ) );
            }
        }

#pragma omp critical
        {
            addresses.insert( threadAddresses.begin(), threadAddresses.end() );
            addressToKeywordMap.insert( threadAddressToKeywordMap.begin(), threadAddressToKeywordMap.end() );
            invalidKeywords.insert( invalidKeywords.end(), threadInvalidKeywords.begin(), threadInvalidKeywords.end() );
        }

        // DEBUG code
        // Used to print keywords not being categorized correctly
        /*
            for ( const auto& kw : invalidKeywords )
            {
                RiaLogging::warning( QString::fromStdString( kw ) );
            }
        */
    }

    return { addresses, addressToKeywordMap };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SummaryCategory RifOpmCommonSummaryTools::categoryFromKeyword( const std::string& keyword )
{
    auto opmCategory = Opm::EclIO::SummaryNode::category_from_keyword( keyword );
    switch ( opmCategory )
    {
        case Opm::EclIO::SummaryNode::Category::Aquifer:
            return SummaryCategory::SUMMARY_AQUIFER;
        case Opm::EclIO::SummaryNode::Category::Block:
            return SummaryCategory::SUMMARY_BLOCK;
        case Opm::EclIO::SummaryNode::Category::Connection:
            return SummaryCategory::SUMMARY_WELL_CONNECTION;
        case Opm::EclIO::SummaryNode::Category::Completion:
            return SummaryCategory::SUMMARY_WELL_COMPLETION;
        case Opm::EclIO::SummaryNode::Category::Field:
            return SummaryCategory::SUMMARY_FIELD;
        case Opm::EclIO::SummaryNode::Category::Group:
            return SummaryCategory::SUMMARY_GROUP;
    }

    return SummaryCategory::SUMMARY_INVALID;
}
