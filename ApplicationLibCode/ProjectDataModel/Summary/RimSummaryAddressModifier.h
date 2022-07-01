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

class RimSummaryCurve;
class RimEnsembleCurveSet;
class RimSummaryPlot;
class RifEclipseSummaryAddress;

#include <vector>

class RimSummaryAddressModifier
{
public:
    RimSummaryAddressModifier( RimSummaryCurve* curve );
    RimSummaryAddressModifier( RimEnsembleCurveSet* curveSet );

    static std::vector<RimSummaryAddressModifier> createAddressModifiersForPlot( RimSummaryPlot* summaryPlot );
    static std::vector<RifEclipseSummaryAddress>  createEclipseSummaryAddress( RimSummaryPlot* summaryPlot );

    RifEclipseSummaryAddress address() const;
    void                     setAddress( const RifEclipseSummaryAddress& address );

private:
    static std::vector<RifEclipseSummaryAddress>
        convertToEclipseSummaryAddress( const std::vector<RimSummaryAddressModifier>& modifiers );

private:
    RimSummaryCurve*     m_curve;
    RimEnsembleCurveSet* m_curveSet;
};
