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
    // Define a variant used to hold either a summary curve or ensemble curve set. This way we can treat them as similar object without a
    // common base class
    using CurveDefs = std::variant<RimSummaryCurve*, RimEnsembleCurveSet*>;

    static std::vector<CurveDefs>              createCurveDefinitions( RimSummaryPlot* summaryPlot );
    static std::vector<RiaSummaryCurveAddress> curveAddresses( const std::vector<CurveDefs>& curveDefs );
    static void                                applyCurveAddressesToCurveDefinitions( const std::vector<CurveDefs>&              curveDefs,
                                                                                      const std::vector<RiaSummaryCurveAddress>& addresses );

    static std::vector<RifEclipseSummaryAddress> createEclipseSummaryAddressesY( RimSummaryPlot* summaryPlot );

    static void modifyByObjectName( std::vector<CurveDefs>                             curveDefs,
                                    std::string                                        objectName,
                                    RimSummaryAddressCollection::CollectionContentType contentType );
};
