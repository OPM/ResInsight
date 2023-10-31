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

#include "RifEclipseSummaryAddress.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RimEnsembleCurveSet.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress curveAdr( RimEnsembleCurveSet* curveSet )
{
    if ( curveSet == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curveSet->curveAddress();
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void setCurveAdr( RimEnsembleCurveSet* curveSet, const RiaSummaryCurveAddress& curveAdr )
{
    if ( curveSet )
    {
        curveSet->setCurveAddress( curveAdr );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress curveAdr( RimSummaryCurve* curve )
{
    if ( curve == nullptr ) return RiaSummaryCurveAddress( RifEclipseSummaryAddress() );
    return curve->curveAddress();
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void setCurveAdr( RimSummaryCurve* curve, const RiaSummaryCurveAddress& curveAdr )
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
RimSummaryAddressModifier::RimSummaryAddressModifier( RimSummaryCurve* curve )
    : m_curve( curve )
    , m_curveSet( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressModifier::RimSummaryAddressModifier( RimEnsembleCurveSet* curveSet )
    : m_curve( nullptr )
    , m_curveSet( curveSet )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::modifyAddresses( RimSummaryPlot*                                    summaryPlot,
                                                 std::string                                        objectName,
                                                 RimSummaryAddressCollection::CollectionContentType contentType )
{
    auto curveDefs = createVariantAddressModifiersForPlot( summaryPlot );

    RimSummaryAddressModifier::modify( curveDefs, objectName, contentType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressModifier::CurveDefs> RimSummaryAddressModifier::createVariantAddressModifiersForPlot( RimSummaryPlot* summaryPlot )
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
std::vector<RimSummaryAddressModifier> RimSummaryAddressModifier::createAddressModifiersForPlot( RimSummaryPlot* summaryPlot )
{
    std::vector<RimSummaryAddressModifier> mods;
    if ( summaryPlot )
    {
        auto curveSets = summaryPlot->curveSets();
        for ( auto curveSet : curveSets )
        {
            mods.emplace_back( RimSummaryAddressModifier( curveSet ) );
        }

        auto curves = summaryPlot->allCurves();
        for ( auto c : curves )
        {
            mods.emplace_back( RimSummaryAddressModifier( c ) );
        }
    }

    return mods;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress> RimSummaryAddressModifier::createEclipseSummaryAddress( RimSummaryPlot* summaryPlot )
{
    auto mods = createAddressModifiersForPlot( summaryPlot );
    return convertToEclipseSummaryAddress( mods );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::modify( std::vector<CurveDefs>                             curveDefs,
                                        std::string                                        objectName,
                                        RimSummaryAddressCollection::CollectionContentType contentType )
{
    for ( auto& adr : curveDefs )
    {
        std::visit(
            [objectName, contentType]( auto&& arg )
            {
                auto myAdr = curveAdr( arg );

                auto xAdr = myAdr.summaryAddressX();
                auto yAdr = myAdr.summaryAddressY();

                auto newAdrX = modifyAddress( xAdr, objectName, contentType );
                auto newAdrY = modifyAddress( yAdr, objectName, contentType );

                setCurveAdr( arg, RiaSummaryCurveAddress( newAdrX, newAdrY ) );
            },
            adr );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryAddressModifier::address() const
{
    if ( m_curve ) return m_curve->summaryAddressY();
    if ( m_curveSet ) return m_curveSet->summaryAddressY();

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::setAddress( const RifEclipseSummaryAddress& address )
{
    if ( m_curve ) m_curve->setSummaryAddressY( address );
    if ( m_curveSet ) m_curveSet->setSummaryAddressYAndStatisticsFlag( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress>
    RimSummaryAddressModifier::convertToEclipseSummaryAddress( const std::vector<RimSummaryAddressModifier>& modifiers )
{
    std::vector<RifEclipseSummaryAddress> tmp;
    tmp.reserve( modifiers.size() );
    for ( const auto& m : modifiers )
    {
        tmp.emplace_back( m.address() );
    }
    return tmp;
}
