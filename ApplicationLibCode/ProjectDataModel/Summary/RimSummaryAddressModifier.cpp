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
RiaSummaryCurveAddress curveAddress( RimEnsembleCurveSet* curveSet )
{
    if ( curveSet == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curveSet->curveAddress();
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void setCurveAddress( RimEnsembleCurveSet* curveSet, const RiaSummaryCurveAddress& curveAdr )
{
    if ( curveSet )
    {
        curveSet->setCurveAddress( curveAdr );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress curveAddress( RimSummaryCurve* curve )
{
    if ( curve == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curve->curveAddress();
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void setCurveAddress( RimSummaryCurve* curve, const RiaSummaryCurveAddress& curveAdr )
{
    if ( curve )
    {
        curve->setSummaryAddressX( curveAdr.summaryAddressX() );
        curve->setSummaryAddressY( curveAdr.summaryAddressY() );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress modifyAddress( const RifEclipseSummaryAddress&                    sourceAdr,
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
                                         "for region text : " );

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
std::vector<RiaSummaryCurveAddress> RimSummaryAddressModifier::curveAddresses( const std::vector<CurveDefs>& curveDefs )
{
    std::vector<RiaSummaryCurveAddress> addresses;

    for ( auto& curveDef : curveDefs )
    {
        std::visit(
            [&addresses]( auto&& arg )
            {
                auto curveAdr = curveAddress( arg );
                addresses.push_back( curveAdr );
            },
            curveDef );
    };

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::applyCurveAddressesToCurveDefinitions( const std::vector<CurveDefs>&              curveDefs,
                                                                       const std::vector<RiaSummaryCurveAddress>& addresses )
{
    if ( curveDefs.size() != addresses.size() ) return;

    for ( size_t i = 0; i < curveDefs.size(); i++ )
    {
        auto        curveDef = curveDefs[i];
        const auto& address  = addresses[i];

        std::visit( [address]( auto&& arg ) { setCurveAddress( arg, address ); }, curveDef );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressModifier::CurveDefs> RimSummaryAddressModifier::createCurveDefinitions( RimSummaryPlot* summaryPlot )
{
    std::vector<CurveDefs> curvesOrCurveSets;

    for ( auto c : summaryPlot->allCurves() )
    {
        curvesOrCurveSets.push_back( c );
    }

    for ( auto cs : summaryPlot->curveSets() )
    {
        curvesOrCurveSets.push_back( cs );
    }

    return curvesOrCurveSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress> RimSummaryAddressModifier::createEclipseSummaryAddressesY( RimSummaryPlot* summaryPlot )
{
    std::vector<RifEclipseSummaryAddress> addresses;

    auto curveDefs = createCurveDefinitions( summaryPlot );
    for ( auto& curveDef : curveDefs )
    {
        std::visit(
            [&addresses]( auto&& arg )
            {
                auto curveAdr = curveAddress( arg );
                addresses.push_back( curveAdr.summaryAddressY() );
            },
            curveDef );
    };

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::modifyByObjectName( std::vector<CurveDefs>                             curveDefs,
                                                    std::string                                        objectName,
                                                    RimSummaryAddressCollection::CollectionContentType contentType )
{
    for ( auto& adr : curveDefs )
    {
        std::visit(
            [objectName, contentType]( auto&& arg )
            {
                auto myAdr = curveAddress( arg );

                auto xAdr = myAdr.summaryAddressX();
                auto yAdr = myAdr.summaryAddressY();

                auto newAdrX = modifyAddress( xAdr, objectName, contentType );
                auto newAdrY = modifyAddress( yAdr, objectName, contentType );

                setCurveAddress( arg, RiaSummaryCurveAddress( newAdrX, newAdrY ) );
            },
            adr );
    };
}
