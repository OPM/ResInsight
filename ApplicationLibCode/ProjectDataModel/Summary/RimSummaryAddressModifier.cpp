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

#include "RimSummaryAddressModifier.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "RifEclipseSummaryAddress.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryAddressModifier::replaceObjectName( const RifEclipseSummaryAddress&                    sourceAdr,
                                                                       std::string                                        objectName,
                                                                       RimSummaryAddressCollection::CollectionContentType contentType )
{
    auto adr = sourceAdr;

    if ( contentType == RimSummaryAddressCollection::CollectionContentType::WELL )
    {
        adr.setWellName( objectName );
    }
    else if ( contentType == RimSummaryAddressCollection::CollectionContentType::GROUP )
    {
        adr.setGroupName( objectName );
    }
    else if ( contentType == RimSummaryAddressCollection::CollectionContentType::REGION )
    {
        int intValue = RiaStdStringTools::toInt( objectName );
        if ( intValue == -1 )
        {
            QString errorText = QString( "Failed to convert region text to region integer value "
                                         "for region text : %1" )
                                    .arg( QString::fromStdString( objectName ) );

            RiaLogging::error( errorText );
        }
        else
        {
            adr.setRegion( intValue );
        }
    }

    return adr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveAddress> RimSummaryAddressModifier::curveAddresses( const std::vector<CurveAddressProvider>& curveAddressProviders )
{
    std::vector<RiaSummaryCurveAddress> addresses;

    for ( auto& provider : curveAddressProviders )
    {
        std::visit(
            [&addresses]( auto&& arg )
            {
                auto curveAdr = RimSummaryAddressModifier::curveAddress( arg );
                addresses.push_back( curveAdr );
            },
            provider );
    };

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::applyAddressesToCurveAddressProviders( const std::vector<CurveAddressProvider>&   curveAddressProviders,
                                                                       const std::vector<RiaSummaryCurveAddress>& addresses )
{
    if ( curveAddressProviders.size() != addresses.size() ) return;

    for ( size_t i = 0; i < curveAddressProviders.size(); i++ )
    {
        auto        provider = curveAddressProviders[i];
        const auto& address  = addresses[i];

        std::visit( [address]( auto&& arg ) { RimSummaryAddressModifier::setCurveAddress( arg, address ); }, provider );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressModifier::CurveAddressProvider> RimSummaryAddressModifier::createAddressProviders( RimSummaryPlot* summaryPlot )
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
std::vector<RifEclipseSummaryAddress> RimSummaryAddressModifier::allSummaryAddressesY( RimSummaryPlot* summaryPlot )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    auto curveAddressProviders = createAddressProviders( summaryPlot );
    for ( auto& provider : curveAddressProviders )
    {
        std::visit(
            [&addresses]( auto&& arg )
            {
                auto curveAdr = RimSummaryAddressModifier::curveAddress( arg );
                addresses.push_back( curveAdr.summaryAddressY() );
            },
            provider );
    };

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::updateAddressesByObjectName( const std::vector<CurveAddressProvider>&           curveAddressProviders,
                                                             const std::string&                                 objectName,
                                                             RimSummaryAddressCollection::CollectionContentType contentType )
{
    for ( auto& provider : curveAddressProviders )
    {
        std::visit(
            [objectName, contentType]( auto&& arg )
            {
                const auto sourceAdr = RimSummaryAddressModifier::curveAddress( arg );

                const auto sourceX = sourceAdr.summaryAddressX();
                const auto sourceY = sourceAdr.summaryAddressY();

                const auto newAdrX = RimSummaryAddressModifier::replaceObjectName( sourceX, objectName, contentType );
                const auto newAdrY = RimSummaryAddressModifier::replaceObjectName( sourceY, objectName, contentType );

                RimSummaryAddressModifier::setCurveAddress( arg, RiaSummaryCurveAddress( newAdrX, newAdrY ) );
            },
            provider );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress RimSummaryAddressModifier::curveAddress( RimEnsembleCurveSet* curveSet )
{
    if ( curveSet == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curveSet->curveAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress RimSummaryAddressModifier::curveAddress( RimSummaryCurve* curve )
{
    if ( curve == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curve->curveAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::setCurveAddress( RimEnsembleCurveSet* curveSet, const RiaSummaryCurveAddress& curveAdr )
{
    if ( curveSet )
    {
        curveSet->setCurveAddress( curveAdr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::setCurveAddress( RimSummaryCurve* curve, const RiaSummaryCurveAddress& curveAdr )
{
    if ( curve )
    {
        curve->setSummaryAddressX( curveAdr.summaryAddressX() );
        curve->setSummaryAddressY( curveAdr.summaryAddressY() );
    }
}
