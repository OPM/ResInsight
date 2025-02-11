////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include <vector>

class RimSummaryCurve;
class RimEnsembleCurveSet;

class RimSummaryDataSourceStepping
{
public:
    enum class SourceSteppingDimension
    {
        SUMMARY_CASE,
        ENSEMBLE,
        WELL,
        WELL_COMPLETION_NUMBER,
        GROUP,
        NETWORK,
        REGION,
        VECTOR,
        BLOCK,
        AQUIFER,
        WELL_SEGMENT,
        WELL_CONNECTION
    };

public:
    virtual std::vector<RimSummaryCurve*>     curvesForStepping() const = 0;
    virtual std::vector<RimSummaryCurve*>     allCurves() const         = 0;
    virtual std::vector<RimEnsembleCurveSet*> curveSets() const         = 0;
};
