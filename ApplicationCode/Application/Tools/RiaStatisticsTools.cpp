/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaStatisticsTools.h"

#include "RifEclipseSummaryAddress.h"
#include "RigStatisticsMath.h"

#include "cafAssert.h"

#ifdef USE_GSL
#include "gsl/statistics/gsl_statistics_double.h"
#endif

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RiaStatisticsTools::replacePercentileByPValueText( const QString& percentile )
{
    QString result = percentile;

    if ( result == ENSEMBLE_STAT_P10_QUANTITY_NAME )
    {
        result = ENSEMBLE_STAT_P90_QUANTITY_NAME;
    }
    else if ( result == ENSEMBLE_STAT_P90_QUANTITY_NAME )
    {
        result = ENSEMBLE_STAT_P10_QUANTITY_NAME;
    }
    else if ( percentile.contains( QString( "%1:" ).arg( ENSEMBLE_STAT_P10_QUANTITY_NAME ) ) )
    {
        result.replace( ENSEMBLE_STAT_P10_QUANTITY_NAME, ENSEMBLE_STAT_P90_QUANTITY_NAME );
    }
    else if ( percentile.contains( QString( "%1:" ).arg( ENSEMBLE_STAT_P90_QUANTITY_NAME ) ) )
    {
        result.replace( ENSEMBLE_STAT_P90_QUANTITY_NAME, ENSEMBLE_STAT_P10_QUANTITY_NAME );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaStatisticsTools::pearsonCorrelation( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    const double eps    = 1.0e-8;
    double       rangeX = 0.0, rangeY = 0.0;
    RigStatisticsMath::calculateBasicStatistics( xValues, nullptr, nullptr, nullptr, &rangeX, nullptr, nullptr );
    RigStatisticsMath::calculateBasicStatistics( yValues, nullptr, nullptr, nullptr, &rangeY, nullptr, nullptr );
    if ( rangeX < eps || rangeY < eps ) return 0.0;

#ifdef USE_GSL
    return pearsonCorrelationGSL( xValues, yValues );
#else
    return pearsonCorrelationOwn( xValues, yValues );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaStatisticsTools::pearsonCorrelationGSL( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
#ifdef USE_GSL
    return gsl_stats_correlation( xValues.data(), 1, yValues.data(), 1, xValues.size() );
#else
    CAF_ASSERT( false );
    return std::numeric_limits<double>::infinity();
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaStatisticsTools::pearsonCorrelationOwn( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    const double eps = 1.0e-8;
    if ( xValues.size() != yValues.size() ) return 0.0;
    if ( xValues.empty() ) return 0.0;

    size_t sampleSize = xValues.size();

    double meanX = 0.0, meanY = 0.0;
    for ( size_t i = 0; i < sampleSize; ++i )
    {
        meanX += xValues[i];
        meanY += yValues[i];
    }
    meanX /= sampleSize;
    meanY /= sampleSize;

    double sumNumerator    = 0.0;
    double sumxDiffSquared = 0.0, sumyDiffSquared = 0.0;
    for ( size_t i = 0; i < sampleSize; ++i )
    {
        double xDiff = xValues[i] - meanX;
        double yDiff = yValues[i] - meanY;
        sumNumerator += xDiff * yDiff;
        sumxDiffSquared += xDiff * xDiff;
        sumyDiffSquared += yDiff * yDiff;
    }

    if ( sumxDiffSquared < eps && sumyDiffSquared < eps ) return 1.0;
    if ( sumxDiffSquared < eps || sumyDiffSquared < eps ) return 0.0;

    return sumNumerator / ( std::sqrt( sumxDiffSquared ) * std::sqrt( sumyDiffSquared ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaStatisticsTools::spearmanCorrelation( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    const double eps    = 1.0e-8;
    double       rangeX = 0.0, rangeY = 0.0;
    RigStatisticsMath::calculateBasicStatistics( xValues, nullptr, nullptr, nullptr, &rangeX, nullptr, nullptr );
    RigStatisticsMath::calculateBasicStatistics( yValues, nullptr, nullptr, nullptr, &rangeY, nullptr, nullptr );
    if ( rangeX < eps || rangeY < eps ) return 0.0;

#ifdef USE_GSL
    std::vector<double> work( 2 * xValues.size() );
    return gsl_stats_spearman( xValues.data(), 1, yValues.data(), 1, xValues.size(), work.data() );
#else
    return 0.0;
#endif
}
