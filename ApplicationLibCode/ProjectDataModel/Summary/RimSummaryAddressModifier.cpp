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

#include "RimEnsembleCurveSet.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

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

        auto curves = summaryPlot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS );
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
RifEclipseSummaryAddress RimSummaryAddressModifier::address() const
{
    if ( m_curve ) return m_curve->summaryAddressY();
    if ( m_curveSet ) return m_curveSet->summaryAddress();

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressModifier::setAddress( const RifEclipseSummaryAddress& address )
{
    if ( m_curve ) m_curve->setSummaryAddressY( address );
    if ( m_curveSet ) m_curveSet->setSummaryAddressAndStatisticsFlag( address );
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
