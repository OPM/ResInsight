/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RiuSummaryQuantityNameInfoProvider.h"

#include "RiaStdStringTools.h"
#include "RifEclipseSummaryAddress.h"
#include "RifOpmSummaryTools.h"

#include "cafAppEnum.h"

#include <regex>
#include <sstream>

// The region_to_region helper functions are taken from
// https://github.com/OPM/opm-common/blob/e1e0edba7da2d3b30f1f009511a62be073c27eb0/src/opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.cpp#L317-L342

namespace ParseHelpers
{
bool is_supported_region_to_region( const std::string& keyword )
{
    static const auto supported_kw = std::regex{ R"~~(R[OGW]F[RT][-+GL_]?([A-Z0-9_]{3})?)~~" };

    // R[OGW]F[RT][-+GL]? (e.g., "ROFTG", "RGFR+", or "RWFT")
    return std::regex_match( keyword, supported_kw );
}

bool is_unsupported_region_to_region( const std::string& keyword )
{
    static const auto unsupported_kw = std::regex{ R"~~(R([EK]|NL)F[RT][-+_]?([A-Z0-9_]{3})?)~~" };

    // R[EK]F[RT][-+]? (e.g., "REFT" or "RKFR+")
    // RNLF[RT][-+]? (e.g., "RNLFR-" or "RNLFT")
    return std::regex_match( keyword, unsupported_kw );
}

bool is_region_to_region( const std::string& keyword )
{
    return is_supported_region_to_region( keyword ) || is_unsupported_region_to_region( keyword );
}
} // namespace ParseHelpers

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQuantityNameInfoProvider* RiuSummaryQuantityNameInfoProvider::instance()
{
    static RiuSummaryQuantityNameInfoProvider theInstance;

    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddressDefines::SummaryCategory RiuSummaryQuantityNameInfoProvider::identifyCategory( const std::string& vectorName )
{
    // Try to an exact match on the vector name first in the vector table.
    bool exactMatch    = true;
    auto exactCategory = categoryFromVectorName( vectorName, exactMatch );
    if ( exactCategory != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID ) return exactCategory;

    if ( ( vectorName.size() < 3 || vectorName.size() > 8 ) && !vectorName.ends_with( RifEclipseSummaryAddressDefines::differenceIdentifier() ) )
        return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID;

    auto tokens = RiaStdStringTools::splitString( vectorName, ':' );
    if ( tokens.size() == 3 && tokens[0].starts_with( "W" ) )
    {
        return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_COMPLETION;
    }

    if ( auto category = RifOpmSummaryTools::categoryFromKeyword( vectorName );
         category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID )
    {
        return category;
    }

    // Try to match the base vector name with more heuristics
    auto strippedQuantityName = RifEclipseSummaryAddress::baseVectorName( vectorName );

    // First, try to lookup vector in vector table
    auto category = categoryFromVectorName( strippedQuantityName );
    if ( category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID ) return category;

    switch ( strippedQuantityName[0] )
    {
        case 'A':
            return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER;
        case 'B':
            return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK;
        case 'F':
            return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_FIELD;
        case 'N':
            return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK;
        case 'S':
            return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_SEGMENT;
        case 'W':
            return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL;
        default:
            break;
    }

    if ( strippedQuantityName[0] == 'R' )
    {
        if ( ParseHelpers::is_region_to_region( strippedQuantityName ) )
            return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION;

        return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION;
    }

    // Then check LGR categories
    std::string firstTwoLetters = strippedQuantityName.substr( 0, 2 );

    if ( firstTwoLetters == "LB" ) return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK_LGR;
    if ( firstTwoLetters == "LC" ) return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION_LGR;
    if ( firstTwoLetters == "LW" ) return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_LGR;

    return RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddressDefines::SummaryCategory RiuSummaryQuantityNameInfoProvider::categoryFromVectorName( const std::string& vectorName,
                                                                                                             bool exactMatch ) const
{
    auto info = quantityInfo( vectorName, exactMatch );

    return info.category;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQuantityNameInfoProvider::RiuSummaryQuantityInfo RiuSummaryQuantityNameInfoProvider::quantityInfo( const std::string& vectorName,
                                                                                                             bool exactMatch ) const
{
    auto it = m_summaryToDescMap.find( vectorName );
    if ( it != m_summaryToDescMap.end() )
    {
        return it->second;
    }

    // Stop searching if not found in lookup table and exact match was requested.
    if ( exactMatch ) return RiuSummaryQuantityInfo();

    if ( vectorName.size() > 1 && vectorName[1] == 'U' )
    {
        // User defined vector name
        // The summary type is given by the first letter, and U defines user-defined
        // FU : field user defined name
        // WU : well user defined name
        // .....

        return RiuSummaryQuantityInfo();
    }

    if ( vectorName.size() > 5 )
    {
        // Check for custom vector naming

        std::string postfix  = vectorName.substr( vectorName.size() - 5, 5 );
        std::string baseName = vectorName.substr( 0, 5 );
        while ( baseName.back() == '_' )
            baseName.pop_back();

        bool isDifference = ( postfix == "_DIFF" );

        it = m_summaryToDescMap.find( baseName );

        if ( it != m_summaryToDescMap.end() )
        {
            if ( isDifference )
            {
                return RiuSummaryQuantityInfo( it->second.category, it->second.longName + " Difference" );
            }
            return it->second;
        }
    }

    return RiuSummaryQuantityInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiuSummaryQuantityNameInfoProvider::longNameFromVectorName( const std::string& vectorName, bool returnVectorNameIfNotFound ) const
{
    auto info = quantityInfo( vectorName );
    return info.category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID || !returnVectorNameIfNotFound ? info.longName
                                                                                                                             : vectorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQuantityNameInfoProvider::setQuantityInfos( const std::unordered_map<std::string, std::pair<std::string, std::string>>& infos )
{
    for ( const auto& [key, content] : infos )
    {
        m_summaryToDescMap.insert( { key, { enumFromString( content.first ), content.second } } );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQuantityNameInfoProvider::RiuSummaryQuantityNameInfoProvider()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiuSummaryQuantityNameInfoProvider::stringFromEnum( RifEclipseSummaryAddressDefines::SummaryCategory category )
{
    return caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>::text( category ).toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddressDefines::SummaryCategory RiuSummaryQuantityNameInfoProvider::enumFromString( const std::string& category )
{
    auto qstring = QString::fromStdString( category );

    auto valid = caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>::isValid( qstring );
    if ( !valid )
    {
        // The category strings in keywords*.json must be mapped to the enum values in the enum definition
        // Ensure that the strings in the json file are correct in /ApplicationLibCode/Application/Resources/keyword-description
        throw std::runtime_error( "Invalid category string: " + category );
    }

    return caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>::fromText( qstring );
}
