/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

namespace RiaCurveDefines
{
enum class InterpolationMethod
{
    STEP_LEFT,
    STEP_RIGHT,
    LINEAR
};
}

template <typename XValueType>
class XValueComparator
{
public:
    bool          operator()( const XValueType& lhs, const XValueType& rhs ) const;
    static bool   equals( const XValueType& lhs, const XValueType& rhs );
    static double diff( const XValueType& lhs, const XValueType& rhs );
};

//==================================================================================================
///
//==================================================================================================
template <typename XValueType>
class RiaCurveMerger
{
public:
    using XComparator = XValueComparator<XValueType>;

    RiaCurveMerger( RiaCurveDefines::InterpolationMethod method );

    void   addCurveData( const std::vector<XValueType>& xValues, const std::vector<double>& yValues );
    size_t curveCount() const;

    void computeInterpolatedValues( bool includeValuesFromPartialCurves );

    RiaCurveDataTools::CurveIntervals validIntervalsForAllXValues() const;
    const std::vector<XValueType>&    allXValues() const;
    const std::vector<double>&        interpolatedYValuesForAllXValues( size_t curveIdx ) const;

    // Non-const access is not required by any clients, but the expression parser has no available const interface
    // for specifying a data source for an expression variable. Allow non-const access to avoid copy of the contained
    // values, interpolated for all time steps
    //
    // See ExpressionParserImpl::assignVector()
    std::vector<double>& interpolatedYValuesForAllXValues( size_t curveIdx );

    static void removeValuesForPartialCurves( std::set<XValueType, XComparator>&                    unionOfXValues,
                                              const std::vector<std::pair<XValueType, XValueType>>& originalXBounds );

public:
    // Helper methods, available as public to be able to access from unit tests

    static double interpolatedYValue( const XValueType&                    xValue,
                                      const std::vector<XValueType>&       curveXValues,
                                      const std::vector<double>&           curveYValues,
                                      RiaCurveDefines::InterpolationMethod interpolationMethod );

private:
    void        computeUnionOfXValues( bool includeValuesFromPartialCurves );
    static bool isMonotonicallyIncreasing( const std::vector<XValueType>& curveXValues );

private:
    std::vector<std::pair<std::vector<XValueType>, std::vector<double>>> m_originalValues;

    RiaCurveDataTools::CurveIntervals m_validIntervalsForAllXValues;

    std::vector<XValueType>          m_allXValues;
    std::vector<std::vector<double>> m_interpolatedValuesForAllCurves;

    bool                                 m_isXValuesSharedBetweenCurves;
    bool                                 m_isXValuesMonotonicallyIncreasing;
    RiaCurveDefines::InterpolationMethod m_interpolationMethod;
};

using RiaTimeHistoryCurveMerger = RiaCurveMerger<time_t>;

template <>
bool XValueComparator<double>::equals( const double& lhs, const double& rhs );
template <>
double XValueComparator<time_t>::diff( const time_t& lhs, const time_t& rhs );

#include "RiaCurveMerger.inl"
