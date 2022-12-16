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
bool RifOpmCommonEclipseSummary::open( const QString&       headerFileName,
                                       bool                 includeRestartFiles,
                                       RiaThreadSafeLogger* threadSafeLogger )
{
    if ( m_createEsmryFiles )
    {
        auto candidateFileName = enhancedSummaryFilename( headerFileName );
        if ( !QFileInfo::exists( candidateFileName ) )
        {
            try
            {
                auto temporarySummaryFile =
                    std::make_unique<Opm::EclIO::ESmry>( headerFileName.toStdString(), includeRestartFiles );

                temporarySummaryFile->make_esmry_file();

                RifOpmCommonEclipseSummary::increaseEsmryFileCount();
            }
            catch ( std::exception& e )
            {
                QString txt = QString( "Failed to create optimized summary file. Error text : %1" ).arg( e.what() );

                if ( threadSafeLogger ) threadSafeLogger->error( txt );

                return false;
            }
        }
    }

    if ( !openFileReader( headerFileName, includeRestartFiles, threadSafeLogger ) ) return false;

    if ( !m_standardReader && !m_enhancedReader ) return false;

    buildMetaData();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifOpmCommonEclipseSummary::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    auto it = m_summaryAddressToKeywordMap.find( resultAddress );
    if ( it != m_summaryAddressToKeywordMap.end() )
    {
        auto keyword = it->second;
        if ( m_enhancedReader )
        {
            auto fileValues = m_enhancedReader->get( keyword );
            values->insert( values->begin(), fileValues.begin(), fileValues.end() );
        }
        else if ( m_standardReader )
        {
            auto fileValues = m_standardReader->get( keyword );
            values->insert( values->begin(), fileValues.begin(), fileValues.end() );
        }
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifOpmCommonEclipseSummary::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    std::string nameString;
    auto        it = m_summaryAddressToKeywordMap.find( resultAddress );
    if ( it != m_summaryAddressToKeywordMap.end() )
    {
        auto keyword = it->second;
        if ( m_enhancedReader )
        {
            nameString = m_enhancedReader->get_unit( keyword );
        }
        else if ( m_standardReader )
        {
            nameString = m_standardReader->get_unit( keyword );
        }
    }

    return RiaStdStringTools::trimString( nameString );
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
void RifOpmCommonEclipseSummary::buildMetaData()
{
    std::vector<Opm::time_point> dates;
    std::vector<std::string>     keywords;

    if ( m_enhancedReader )
    {
        dates    = m_enhancedReader->dates();
        keywords = m_enhancedReader->keywordList();
    }
    else if ( m_standardReader )
    {
        dates    = m_standardReader->dates();
        keywords = m_standardReader->keywordList();
    }

    for ( const auto& d : dates )
    {
        auto timeAsTimeT = std::chrono::system_clock::to_time_t( d );
        m_timeSteps.push_back( timeAsTimeT );
    }

    auto [addresses, addressMap] = RifOpmCommonSummaryTools::buildAddressesAndKeywordMap( keywords );

    m_allResultAddresses         = addresses;
    m_summaryAddressToKeywordMap = addressMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::openFileReader( const QString&       headerFileName,
                                                 bool                 includeRestartFiles,
                                                 RiaThreadSafeLogger* threadSafeLogger )
{
    if ( m_useEsmryFiles )
    {
        try
        {
            auto candidateFileName = enhancedSummaryFilename( headerFileName );
            m_enhancedReader =
                std::make_unique<Opm::EclIO::ExtESmry>( candidateFileName.toStdString(), includeRestartFiles );

            return true;
        }
        catch ( ... )
        {
            // Do not do anything here, try to open the file using standard esmy reader
        }
    }

    try
    {
        m_standardReader = std::make_unique<Opm::EclIO::ESmry>( headerFileName.toStdString(), includeRestartFiles );
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
void RifOpmCommonEclipseSummary::increaseEsmryFileCount()
{
    // This function can be called from a parallel loop, make it thread safe
#pragma omp critical
    sm_createdEsmryFileCount++;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifOpmCommonEclipseSummary::enhancedSummaryFilename( const QString& headerFileName )
{
    QString s( headerFileName );
    return s.replace( ".SMSPEC", ".ESMRY" );
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
        std::set<RifEclipseSummaryAddress>              threadAddresses;
        std::map<RifEclipseSummaryAddress, std::string> threadAddressToKeywordMap;
        std::vector<std::string>                        threadInvalidKeywords;

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
                threadAddresses.insert( eclAdr );
                threadAddressToKeywordMap[eclAdr] = keyword;
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
