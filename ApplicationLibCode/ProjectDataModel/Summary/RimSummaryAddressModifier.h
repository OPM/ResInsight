////|/////////////////////////////////////////////////////////////////////////////
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

#pragma once

class RimSummaryCurve;
class RimEnsembleCurveSet;
class RimSummaryPlot;
class RifEclipseSummaryAddress;
class RiaSummaryCurveAddress;

#include "RimSummaryAddressCollection.h"
#include <variant>
#include <vector>

class RimSummaryAddressModifier
{
public:
    using CurveDefs = std::variant<RimSummaryCurve*, RimEnsembleCurveSet*>;

    RimSummaryAddressModifier( RimSummaryCurve* curve );
    RimSummaryAddressModifier( RimEnsembleCurveSet* curveSet );

    static std::vector<RiaSummaryCurveAddress> curveAddresses( const std::vector<CurveDefs>& curveDefs );
    static void applyCurveAddresses( const std::vector<CurveDefs>& curveDefs, const std::vector<RiaSummaryCurveAddress>& addresses );

    static void
        modifyAddresses( RimSummaryPlot* summaryPlot, std::string objectName, RimSummaryAddressCollection::CollectionContentType contentType );

    static std::vector<CurveDefs>                 createVariantAddressModifiersForPlot( RimSummaryPlot* summaryPlot );
    static std::vector<RimSummaryAddressModifier> createAddressModifiersForPlot( RimSummaryPlot* summaryPlot );
    static std::vector<RifEclipseSummaryAddress>  createEclipseSummaryAddress( RimSummaryPlot* summaryPlot );

    static void
        modify( std::vector<CurveDefs> curveDefs, std::string objectName, RimSummaryAddressCollection::CollectionContentType contentType );

    RifEclipseSummaryAddress address() const;
    void                     setAddress( const RifEclipseSummaryAddress& address );

private:
    static std::vector<RifEclipseSummaryAddress> convertToEclipseSummaryAddress( const std::vector<RimSummaryAddressModifier>& modifiers );

private:
    RimSummaryCurve*     m_curve;
    RimEnsembleCurveSet* m_curveSet;
};
