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
#include "RiaQDateTimeTools.h"

#include <QDateTime>

using QDT = RiaQDateTimeTools;

//==================================================================================================
///
//==================================================================================================
class RiaTimeHistoryCurveResampler
{
public:
    RiaTimeHistoryCurveResampler();

    void setCurveData( const std::vector<double>& values, const std::vector<time_t>& timeSteps );

    void resampleAndComputePeriodEndValues( RiaQDateTimeTools::DateTimePeriod period );
    void resampleAndComputeWeightedMeanValues( RiaQDateTimeTools::DateTimePeriod period );

    const std::vector<time_t>& resampledTimeSteps() const;
    const std::vector<double>& resampledValues() const;

    static std::vector<time_t>
        timeStepsFromTimeRange( RiaQDateTimeTools::DateTimePeriod period, time_t minTime, time_t maxTime );

private:
    void computeWeightedMeanValues( RiaQDateTimeTools::DateTimePeriod period );
    void computePeriodEndValues( RiaQDateTimeTools::DateTimePeriod period );

    void             clearData();
    void             computeResampledTimeSteps( RiaQDateTimeTools::DateTimePeriod period );
    static QDateTime firstResampledTimeStep( const QDateTime& firstTimestep, RiaQDateTimeTools::DateTimePeriod period );
    inline double    interpolatedValue( time_t t, time_t t1, double v1, time_t t2, double v2 );

private:
    std::pair<std::vector<double>, std::vector<time_t>> m_originalValues;

    std::vector<time_t> m_timeSteps;
    std::vector<double> m_values;
};
