/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RiaSummaryAddressModifier.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "RifEclipseSummaryAddress.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RiaSummaryAddressModifier::replaceTokenForCategory( const RifEclipseSummaryAddress&                  sourceAdr,
                                                                             const std::string&                               token,
                                                                             RifEclipseSummaryAddressDefines::SummaryCategory contentType )
{
    auto adr = sourceAdr;

    if ( contentType == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL )
    {
        adr.setWellName( token );
    }
    else if ( contentType == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP )
    {
        adr.setGroupName( token );
    }
    else if ( contentType == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK )
    {
        adr.setNetworkName( token );
    }
    else if ( contentType == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION )
    {
        int intValue = -1;
        if ( !RiaStdStringTools::toInt( token, intValue ) )
        {
            QString errorText = QString( "Failed to convert region text to region integer value "
                                         "for region text : %1" )
                                    .arg( QString::fromStdString( token ) );

            RiaLogging::error( errorText );
        }
        else
        {
            adr.setRegion( intValue );
        }
    }
    else if ( contentType == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION )
    {
        int intValue = -1;
        if ( !RiaStdStringTools::toInt( token, intValue ) )
        {
            QString errorText = QString( "Failed to convert region text to region integer value "
                                         "for region text : %1" )
                                    .arg( QString::fromStdString( token ) );

            RiaLogging::error( errorText );
        }
        else
        {
            adr.setRegion2( intValue );
        }
    }
    else if ( contentType == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER )
    {
        int intValue = -1;
        if ( !RiaStdStringTools::toInt( token, intValue ) )
        {
            QString errorText = QString( "Failed to convert aquifer text to aquifer integer value "
                                         "for aquifer text : %1" )
                                    .arg( QString::fromStdString( token ) );

            RiaLogging::error( errorText );
        }
        else
        {
            adr.setAquiferNumber( intValue );
        }
    }

    return adr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveAddress> RiaSummaryAddressModifier::curveAddresses( const std::vector<CurveAddressProvider>& curveAddressProviders )
{
    std::vector<RiaSummaryCurveAddress> addresses;

    for ( auto& provider : curveAddressProviders )
    {
        std::visit(
            [&addresses]( auto&& arg )
            {
                auto curveAdr = RiaSummaryAddressModifier::curveAddress( arg );
                addresses.push_back( curveAdr );
            },
            provider );
    };

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressModifier::applyAddressesToCurveAddressProviders( const std::vector<CurveAddressProvider>&   curveAddressProviders,
                                                                       const std::vector<RiaSummaryCurveAddress>& addresses )
{
    if ( curveAddressProviders.size() != addresses.size() ) return;

    for ( size_t i = 0; i < curveAddressProviders.size(); i++ )
    {
        auto        provider = curveAddressProviders[i];
        const auto& address  = addresses[i];

        std::visit( [address]( auto&& arg ) { RiaSummaryAddressModifier::setCurveAddress( arg, address ); }, provider );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryAddressModifier::CurveAddressProvider> RiaSummaryAddressModifier::createAddressProviders( RimSummaryPlot* summaryPlot )
{
    std::vector<CurveAddressProvider> providers;

    for ( auto c : summaryPlot->allCurves() )
    {
        providers.push_back( c );
    }

    for ( auto cs : summaryPlot->curveSets() )
    {
        providers.push_back( cs );
    }

    return providers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress> RiaSummaryAddressModifier::allSummaryAddressesY( RimSummaryPlot* summaryPlot )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    auto curveAddressProviders = createAddressProviders( summaryPlot );
    for ( auto& provider : curveAddressProviders )
    {
        std::visit(
            [&addresses]( auto&& arg )
            {
                auto curveAdr = RiaSummaryAddressModifier::curveAddress( arg );
                addresses.push_back( curveAdr.summaryAddressY() );
            },
            provider );
    };

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressModifier::updateAddressesByObjectName( const std::vector<CurveAddressProvider>&           curveAddressProviders,
                                                             const std::string&                                 objectName,
                                                             RimSummaryAddressCollection::CollectionContentType contentType )
{
    auto category = RimSummaryAddressCollection::contentTypeToSummaryCategory( contentType );

    for ( auto& provider : curveAddressProviders )
    {
        std::visit(
            [objectName, category]( auto&& arg )
            {
                const auto sourceAdr = RiaSummaryAddressModifier::curveAddress( arg );

                const auto sourceX = sourceAdr.summaryAddressX();
                const auto sourceY = sourceAdr.summaryAddressY();

                const auto newAdrX = RiaSummaryAddressModifier::replaceTokenForCategory( sourceX, objectName, category );
                const auto newAdrY = RiaSummaryAddressModifier::replaceTokenForCategory( sourceY, objectName, category );

                RiaSummaryAddressModifier::setCurveAddress( arg, RiaSummaryCurveAddress( newAdrX, newAdrY ) );
            },
            provider );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress RiaSummaryAddressModifier::curveAddress( RimEnsembleCurveSet* curveSet )
{
    if ( curveSet == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curveSet->curveAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress RiaSummaryAddressModifier::curveAddress( RimSummaryCurve* curve )
{
    if ( curve == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curve->curveAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressModifier::setCurveAddress( RimEnsembleCurveSet* curveSet, const RiaSummaryCurveAddress& curveAdr )
{
    if ( curveSet )
    {
        curveSet->setCurveAddress( curveAdr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryAddressModifier::setCurveAddress( RimSummaryCurve* curve, const RiaSummaryCurveAddress& curveAdr )
{
    if ( curve )
    {
        curve->setSummaryAddressX( curveAdr.summaryAddressX() );
        curve->setSummaryAddressY( curveAdr.summaryAddressY() );
    }
}
