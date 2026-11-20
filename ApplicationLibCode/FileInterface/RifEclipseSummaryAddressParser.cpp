/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Equinor ASA
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

#include "RifEclipseSummaryAddressParser.h"

#include "RiaStdStringTools.h"

#include "RiuSummaryQuantityNameInfoProvider.h"

namespace RifEclipseSummaryAddressParser
{
namespace
{
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    RifEclipseSummaryAddress fromTokens( const std::vector<std::string>& tokens, bool enforceKeywordLength = true )
    {
        if ( tokens.empty() ) return {};

        std::string vectorName;
        std::string token1;
        std::string token2;

        int intValue0 = -1;
        int intValue1 = -1;
        int intValue2 = -1;

        vectorName = tokens[0];

        if ( tokens.size() > 1 ) token1 = tokens[1];
        if ( tokens.size() > 2 ) token2 = tokens[2];

        SummaryCategory category = SummaryCategory::SUMMARY_INVALID;
        if ( ( tokens.size() >= 3 ) && ( vectorName.starts_with( 'W' ) ) )
        {
            // Check if token2 is a valid integer (completion number) before classifying as SUMMARY_WELL_COMPLETION
            // If token2 is not an integer, it could be a well name containing a colon
            int tempInt = -1;
            if ( RiaStdStringTools::toInt( token2, tempInt ) )
            {
                category = SummaryCategory::SUMMARY_WELL_COMPLETION;
            }
            else
            {
                category = RiuSummaryQuantityNameInfoProvider::instance()->identifyCategory( vectorName, enforceKeywordLength );
            }
        }
        else
        {
            category = RiuSummaryQuantityNameInfoProvider::instance()->identifyCategory( vectorName, enforceKeywordLength );
        }

        switch ( category )
        {
            case SummaryCategory::SUMMARY_FIELD:
                return RifEclipseSummaryAddress::fieldAddress( vectorName );

            case SummaryCategory::SUMMARY_AQUIFER:
                if ( !token1.empty() )
                {
                    RiaStdStringTools::toInt( token1, intValue0 );
                    return RifEclipseSummaryAddress::aquiferAddress( vectorName, intValue0 );
                }
                break;

            case SummaryCategory::SUMMARY_NETWORK:
            {
                auto aggregated = token1;
                if ( !token2.empty() )
                {
                    // Network name can contain more than one token. Concatenate tokens using : as separator
                    // https://github.com/OPM/ResInsight/issues/11785
                    aggregated += ":";
                    aggregated += token2;
                }

                return RifEclipseSummaryAddress::networkAddress( vectorName, aggregated );
            }
            break;

            case SummaryCategory::SUMMARY_MISC:
                return RifEclipseSummaryAddress::miscAddress( vectorName );
                break;

            case SummaryCategory::SUMMARY_REGION:
                if ( !token1.empty() )
                {
                    RiaStdStringTools::toInt( token1, intValue0 );
                    return RifEclipseSummaryAddress::regionAddress( vectorName, intValue0 );
                }
                break;

            case SummaryCategory::SUMMARY_REGION_2_REGION:
                if ( !token1.empty() )
                {
                    auto regions = RiaStdStringTools::splitString( token1, '-' );
                    if ( regions.size() == 2 )
                    {
                        RiaStdStringTools::toInt( regions[0], intValue0 );
                        RiaStdStringTools::toInt( regions[1], intValue1 );

                        return RifEclipseSummaryAddress::regionToRegionAddress( vectorName, intValue0, intValue1 );
                    }
                }
                break;

            case SummaryCategory::SUMMARY_GROUP:
                if ( !token1.empty() ) return RifEclipseSummaryAddress::groupAddress( vectorName, token1 );
                break;

            case SummaryCategory::SUMMARY_WELL:
            {
                // Well name can contain colons. Concatenate all tokens after the vector name.
                // https://github.com/OPM/ResInsight/issues/13402
                std::string wellName;
                for ( size_t i = 1; i < tokens.size(); i++ )
                {
                    if ( !wellName.empty() ) wellName += ":";
                    wellName += tokens[i];
                }
                if ( !wellName.empty() ) return RifEclipseSummaryAddress::wellAddress( vectorName, wellName );
                break;
            }

            case SummaryCategory::SUMMARY_WELL_COMPLETION:
            {
                RiaStdStringTools::toInt( token2, intValue0 );

                if ( !token1.empty() ) return RifEclipseSummaryAddress::wellCompletionAddress( vectorName, token1, intValue0 );
                break;
            }

            case SummaryCategory::SUMMARY_WELL_CONNECTION:
                if ( !token2.empty() )
                {
                    auto ijk = RiaStdStringTools::splitString( token2, ',' );
                    if ( ijk.size() == 3 )
                    {
                        RiaStdStringTools::toInt( ijk[0], intValue0 );
                        RiaStdStringTools::toInt( ijk[1], intValue1 );
                        RiaStdStringTools::toInt( ijk[2], intValue2 );

                        return RifEclipseSummaryAddress::wellConnectionAddress( vectorName, token1, intValue0, intValue1, intValue2 );
                    }
                }
                break;

            case SummaryCategory::SUMMARY_WELL_LGR:
                if ( !token1.empty() && !token2.empty() ) return RifEclipseSummaryAddress::wellLgrAddress( vectorName, token1, token2 );
                break;

            case SummaryCategory::SUMMARY_WELL_CONNECTION_LGR:
                if ( tokens.size() > 3 )
                {
                    const auto& token3 = tokens[3];
                    const auto  ijk    = RiaStdStringTools::splitString( token3, ',' );
                    if ( ijk.size() == 3 )
                    {
                        RiaStdStringTools::toInt( ijk[0], intValue0 );
                        RiaStdStringTools::toInt( ijk[1], intValue1 );
                        RiaStdStringTools::toInt( ijk[2], intValue2 );

                        return RifEclipseSummaryAddress::wellCompletionLgrAddress( vectorName, token1, token2, intValue0, intValue1, intValue2 );
                    }
                }
                break;

            case SummaryCategory::SUMMARY_WELL_SEGMENT:

                if ( !token2.empty() )
                {
                    RiaStdStringTools::toInt( token2, intValue0 );

                    return RifEclipseSummaryAddress::wellSegmentAddress( vectorName, token1, intValue0 );
                }
                break;

            case SummaryCategory::SUMMARY_BLOCK:
                if ( !token1.empty() )
                {
                    auto ijk = RiaStdStringTools::splitString( token1, ',' );
                    if ( ijk.size() == 3 )
                    {
                        RiaStdStringTools::toInt( ijk[0], intValue0 );
                        RiaStdStringTools::toInt( ijk[1], intValue1 );
                        RiaStdStringTools::toInt( ijk[2], intValue2 );

                        return RifEclipseSummaryAddress::blockAddress( vectorName, intValue0, intValue1, intValue2 );
                    }
                }
                break;

            case SummaryCategory::SUMMARY_BLOCK_LGR:
                if ( !token2.empty() )
                {
                    auto ijk = RiaStdStringTools::splitString( token2, ',' );
                    if ( ijk.size() == 3 )
                    {
                        RiaStdStringTools::toInt( ijk[0], intValue0 );
                        RiaStdStringTools::toInt( ijk[1], intValue1 );
                        RiaStdStringTools::toInt( ijk[2], intValue2 );

                        return RifEclipseSummaryAddress::blockLgrAddress( vectorName, token1, intValue0, intValue1, intValue2 );
                    }
                }
                break;

            case SummaryCategory::SUMMARY_IMPORTED:
            case SummaryCategory::SUMMARY_INVALID:
            default:
                break;
        }

        return {};
    }
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress fromEclipseTextAddress( const std::string& textAddress )
{
    auto tokens = RiaStdStringTools::splitString( textAddress, ':' );

    return fromTokens( tokens );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress fromEclipseTextAddressParseErrorTokens( const std::string& textAddress )
{
    auto tokens = RiaStdStringTools::splitString( textAddress, ':' );

    bool isErrorResult = false;

    if ( tokens.size() > 1 )
    {
        auto firstToken = RiaStdStringTools::toUpper( RiaStdStringTools::trimString( tokens[0] ) );

        if ( ( firstToken == "ER" ) || ( firstToken == "ERR" ) || ( firstToken == "ERROR" ) )
        {
            isErrorResult = true;
            tokens.erase( tokens.begin() );
        }
    }

    // Column headers are free text and not limited to the Eclipse keyword length
    auto address = fromTokens( tokens, false );

    if ( address.category() == SummaryCategory::SUMMARY_INVALID || address.category() == SummaryCategory::SUMMARY_IMPORTED )
    {
        // Address category not recognized, use incoming text string without error prefix as vector name
        auto text = RiaStdStringTools::joinStrings( tokens, ':' );
        address   = RifEclipseSummaryAddress::importedAddress( text );
    }

    if ( isErrorResult ) address.setAsErrorResult();
    return address;
}
} // namespace RifEclipseSummaryAddressParser
