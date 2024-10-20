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

#pragma once

#include "RimSummaryAddressCollection.h"

#include <variant>
#include <vector>

class RimSummaryCurve;
class RimEnsembleCurveSet;
class RimSummaryPlot;
class RifEclipseSummaryAddress;
class RiaSummaryCurveAddress;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimSummaryAddressModifier
{
public:
    // Define a variant for summary curves and ensemble curve set. This way we can treat them as similar object without a
    // common base class
    using CurveAddressProvider = std::variant<RimSummaryCurve*, RimEnsembleCurveSet*>;

    static std::vector<CurveAddressProvider>   createAddressProviders( RimSummaryPlot* summaryPlot );
    static std::vector<RiaSummaryCurveAddress> curveAddresses( const std::vector<CurveAddressProvider>& curveAddressProviders );
    static void applyAddressesToCurveAddressProviders( const std::vector<CurveAddressProvider>&   curveAddressProviders,
                                                       const std::vector<RiaSummaryCurveAddress>& addresses );

    static std::vector<RifEclipseSummaryAddress> allSummaryAddressesY( RimSummaryPlot* summaryPlot );

    static void updateAddressesByObjectName( const std::vector<CurveAddressProvider>&           curveAddressProviders,
                                             const std::string&                                 objectName,
                                             RimSummaryAddressCollection::CollectionContentType contentType );

    static RifEclipseSummaryAddress replaceTokenForCategory( const RifEclipseSummaryAddress&                  sourceAdr,
                                                             std::string                                      token,
                                                             RifEclipseSummaryAddressDefines::SummaryCategory contentType );

private:
    static RiaSummaryCurveAddress curveAddress( RimSummaryCurve* curve );
    static RiaSummaryCurveAddress curveAddress( RimEnsembleCurveSet* curveSet );
    static void                   setCurveAddress( RimEnsembleCurveSet* curveSet, const RiaSummaryCurveAddress& curveAdr );
    static void                   setCurveAddress( RimSummaryCurve* curve, const RiaSummaryCurveAddress& curveAdr );
};
