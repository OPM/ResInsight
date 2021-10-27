/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaCurveDataTools.h"

#include <ctime>

//==================================================================================================
///
//==================================================================================================
class RiaWellLogCurveMerger
{
public:
    RiaWellLogCurveMerger();

    void   addCurveData( const std::vector<double>& xValues, const std::vector<double>& yValues );
    size_t curveCount() const;

    void computeLookupValues( bool includeValuesFromPartialCurves = true );

    RiaCurveDataTools::CurveIntervals validIntervalsForAllXValues() const;
    const std::vector<double>&        allXValues() const;
    const std::vector<double>&        lookupYValuesForAllXValues( size_t curveIdx ) const;

private:
    void computeUnionOfXValues( bool includeValuesFromPartialCurves );

    static double lookupYValue( const double&              xValue,
                                const std::vector<double>& curveXValues,
                                const std::vector<double>& curveYValues );

    std::vector<std::pair<std::vector<double>, std::vector<double>>> m_originalValues;

    RiaCurveDataTools::CurveIntervals m_validIntervalsForAllXValues;

    std::vector<double>              m_allXValues;
    std::vector<std::vector<double>> m_lookupValuesForAllCurves;
};
