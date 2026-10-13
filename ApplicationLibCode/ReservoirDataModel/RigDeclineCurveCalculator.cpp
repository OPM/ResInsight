/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RigDeclineCurveCalculator.h"

#include <QDateTime>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <tuple>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computeDeclineRate( double time0, double value0, double time1, double value1 )
{
    return ( 1.0 / ( time1 - time0 ) ) * std::log( value0 / value1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computeFlowRateExponentialDecline( double initialProductionRateQi,
                                                                     double initialDeclineRateDi,
                                                                     double timeSinceStart )
{
    return initialProductionRateQi * std::exp( -initialDeclineRateDi * timeSinceStart );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computeFlowRateHarmonicDecline( double initialProductionRateQi, double initialDeclineRateDi, double timeSinceStart )
{
    return initialProductionRateQi / ( 1.0 + initialDeclineRateDi * timeSinceStart );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computeFlowRateHyperbolicDecline( double initialProductionRateQi,
                                                                    double initialDeclineRateDi,
                                                                    double timeSinceStart,
                                                                    double hyperbolicDeclineConstantB )
{
    return initialProductionRateQi /
           std::pow( 1.0 + hyperbolicDeclineConstantB * initialDeclineRateDi * timeSinceStart, 1.0 / hyperbolicDeclineConstantB );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computeCumulativeProductionExponentialDecline( double initialProductionRateQi,
                                                                                 double initialDeclineRateDi,
                                                                                 double timeSinceStart )
{
    double productionRate = computeFlowRateExponentialDecline( initialProductionRateQi, initialDeclineRateDi, timeSinceStart );
    return ( 1 / initialDeclineRateDi ) * ( initialProductionRateQi - productionRate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computeCumulativeProductionHarmonicDecline( double initialProductionRateQi,
                                                                              double initialDeclineRateDi,
                                                                              double timeSinceStart )
{
    double productionRate = computeFlowRateHarmonicDecline( initialProductionRateQi, initialDeclineRateDi, timeSinceStart );
    return ( initialProductionRateQi / initialDeclineRateDi ) * std::log( initialProductionRateQi / productionRate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computeCumulativeProductionHyperbolicDecline( double initialProductionRateQi,
                                                                                double initialDeclineRateDi,
                                                                                double timeSinceStart,
                                                                                double hyperbolicDeclineConstantB )
{
    double productionRate =
        computeFlowRateHyperbolicDecline( initialProductionRateQi, initialDeclineRateDi, timeSinceStart, hyperbolicDeclineConstantB );

    return ( std::pow( initialProductionRateQi, hyperbolicDeclineConstantB ) / ( ( 1.0 - hyperbolicDeclineConstantB ) * initialDeclineRateDi ) ) *
           ( std::pow( initialProductionRateQi, 1.0 - hyperbolicDeclineConstantB ) -
             std::pow( productionRate, 1.0 - hyperbolicDeclineConstantB ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>> RigDeclineCurveCalculator::fitLine( const std::vector<double>& t, const std::vector<double>& y )
{
    if ( t.size() != y.size() || t.size() < 2 ) return std::nullopt;

    // Scale time to avoid numerical problems when time is given in seconds
    const double timeScale = 365.25 * 24.0 * 3600.0;

    const double n    = static_cast<double>( t.size() );
    double       sumT = 0.0, sumY = 0.0, sumTT = 0.0, sumTY = 0.0;
    for ( size_t i = 0; i < t.size(); i++ )
    {
        const double ts = t[i] / timeScale;
        sumT += ts;
        sumY += y[i];
        sumTT += ts * ts;
        sumTY += ts * y[i];
    }

    const double denominator = n * sumTT - sumT * sumT;
    if ( std::abs( denominator ) < 1.0e-12 ) return std::nullopt;

    const double slopePerYear = ( n * sumTY - sumT * sumY ) / denominator;
    const double intercept    = ( sumY - slopePerYear * sumT ) / n;
    if ( !std::isfinite( slopePerYear ) || !std::isfinite( intercept ) ) return std::nullopt;

    return std::make_pair( intercept, slopePerYear / timeScale );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigDeclineCurveCalculator::FitResult RigDeclineCurveCalculator::fitExponentialDecline( const std::vector<double>& times,
                                                                                       const std::vector<double>& flowRates )
{
    // ln(q) = ln(qi) - Di * t
    std::vector<double> t, y;
    for ( size_t i = 0; i < std::min( times.size(), flowRates.size() ); i++ )
    {
        if ( flowRates[i] > 0.0 )
        {
            t.push_back( times[i] );
            y.push_back( std::log( flowRates[i] ) );
        }
    }

    auto line = fitLine( t, y );
    if ( !line ) return std::nullopt;

    return std::make_pair( std::exp( line->first ), -line->second );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigDeclineCurveCalculator::FitResult RigDeclineCurveCalculator::fitHarmonicDecline( const std::vector<double>& times,
                                                                                    const std::vector<double>& flowRates )
{
    // 1/q = 1/qi + (Di/qi) * t
    std::vector<double> t, y;
    for ( size_t i = 0; i < std::min( times.size(), flowRates.size() ); i++ )
    {
        if ( flowRates[i] > 0.0 )
        {
            t.push_back( times[i] );
            y.push_back( 1.0 / flowRates[i] );
        }
    }

    auto line = fitLine( t, y );
    if ( !line || line->first <= 0.0 ) return std::nullopt;

    const double qi = 1.0 / line->first;
    return std::make_pair( qi, line->second * qi );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigDeclineCurveCalculator::FitResult RigDeclineCurveCalculator::fitHyperbolicDecline( const std::vector<double>& times,
                                                                                      const std::vector<double>& flowRates,
                                                                                      double hyperbolicDeclineConstantB )
{
    if ( hyperbolicDeclineConstantB <= 0.0 ) return std::nullopt;

    // q^-b = qi^-b * (1 + b * Di * t)
    std::vector<double> t, y;
    for ( size_t i = 0; i < std::min( times.size(), flowRates.size() ); i++ )
    {
        if ( flowRates[i] > 0.0 )
        {
            t.push_back( times[i] );
            y.push_back( std::pow( flowRates[i], -hyperbolicDeclineConstantB ) );
        }
    }

    auto line = fitLine( t, y );
    if ( !line || line->first <= 0.0 ) return std::nullopt;

    const double qi = std::pow( line->first, -1.0 / hyperbolicDeclineConstantB );
    const double di = line->second / ( line->first * hyperbolicDeclineConstantB );
    return std::make_pair( qi, di );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RigDeclineCurveCalculator::computeStartEndPointParameters( const std::vector<double>& values,
                                                                                     const std::vector<time_t>& timeSteps,
                                                                                     bool                       isAccumulated )
{
    auto computeProductionRate = []( double t0, double v0, double t1, double v1 ) { return ( v1 - v0 ) / ( t1 - t0 ); };

    const double endTime = static_cast<double>( timeSteps.back() );

    if ( !isAccumulated )
    {
        // Use the first (time step, value) pair as t0. The last point (within the selected range) is the initial production rate.
        const double initialProductionRate = values.back();
        const double initialDeclineRate =
            computeDeclineRate( static_cast<double>( timeSteps.front() ), values.front(), endTime, initialProductionRate );
        return { initialProductionRate, initialDeclineRate };
    }

    // Select a point (t0) 1/4 into the user-specified range
    const double historyStep = 0.25;
    const size_t idx0        = static_cast<size_t>( timeSteps.size() * historyStep );
    const double t0          = static_cast<double>( timeSteps[idx0] );
    const double v0          = values[idx0];

    // For accumulated result: compute the initial production rate from the two points.
    const double initialProductionRate = computeProductionRate( t0, v0, endTime, values.back() );

    // Compute the production rate at time t0 by using the first point in the existing curve.
    const double productionRate0 = computeProductionRate( static_cast<double>( timeSteps.front() ), values.front(), t0, v0 );

    const double initialDeclineRate = computeDeclineRate( t0, productionRate0, endTime, initialProductionRate );
    return { initialProductionRate, initialDeclineRate };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigDeclineCurveCalculator::DeclineParameters>
    RigDeclineCurveCalculator::computeDeclineParameters( const DeclineModel&        model,
                                                         const std::vector<double>& values,
                                                         const std::vector<time_t>& timeSteps )
{
    if ( values.empty() || values.size() != timeSteps.size() ) return std::nullopt;

    const time_t referenceTime = timeSteps.back();

    double initialProductionRate = 0.0;
    double initialDeclineRate    = 0.0;

    if ( model.fitMethod == FitMethod::START_END_POINT )
    {
        std::tie( initialProductionRate, initialDeclineRate ) = computeStartEndPointParameters( values, timeSteps, model.isAccumulated );
    }
    else
    {
        // Regression is performed on flow rates. Derive rates from the differences if the curve is accumulated.
        std::vector<double> times;
        std::vector<double> rates;
        for ( size_t i = 0; i < values.size(); i++ )
        {
            if ( model.isAccumulated )
            {
                if ( i == 0 || timeSteps[i] <= timeSteps[i - 1] ) continue;
                rates.push_back( ( values[i] - values[i - 1] ) / static_cast<double>( timeSteps[i] - timeSteps[i - 1] ) );

                // The average rate over the interval is best represented at the interval midpoint
                times.push_back( 0.5 * static_cast<double>( timeSteps[i - 1] - referenceTime ) +
                                 0.5 * static_cast<double>( timeSteps[i] - referenceTime ) );
            }
            else
            {
                rates.push_back( values[i] );
                times.push_back( static_cast<double>( timeSteps[i] - referenceTime ) );
            }
        }

        FitResult fit;
        if ( model.type == DeclineCurveType::EXPONENTIAL )
        {
            fit = fitExponentialDecline( times, rates );
        }
        else if ( model.type == DeclineCurveType::HARMONIC )
        {
            fit = fitHarmonicDecline( times, rates );
        }
        else
        {
            fit = fitHyperbolicDecline( times, rates, model.hyperbolicDeclineConstantB );
        }

        if ( !fit ) return std::nullopt;
        std::tie( initialProductionRate, initialDeclineRate ) = *fit;
    }

    if ( !std::isfinite( initialProductionRate ) || !std::isfinite( initialDeclineRate ) ) return std::nullopt;

    return DeclineParameters{ initialProductionRate, initialDeclineRate, referenceTime };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computePredictedValue( const DeclineModel& model, const DeclineParameters& parameters, double timeSinceStart )
{
    const double qi = parameters.initialProductionRate;
    const double di = parameters.initialDeclineRate;
    const double b  = model.hyperbolicDeclineConstantB;

    switch ( model.type )
    {
        case DeclineCurveType::EXPONENTIAL:
            return model.isAccumulated ? computeCumulativeProductionExponentialDecline( qi, di, timeSinceStart )
                                       : computeFlowRateExponentialDecline( qi, di, timeSinceStart );
        case DeclineCurveType::HARMONIC:
            return model.isAccumulated ? computeCumulativeProductionHarmonicDecline( qi, di, timeSinceStart )
                                       : computeFlowRateHarmonicDecline( qi, di, timeSinceStart );
        case DeclineCurveType::HYPERBOLIC:
            return model.isAccumulated ? computeCumulativeProductionHyperbolicDecline( qi, di, timeSinceStart, b )
                                       : computeFlowRateHyperbolicDecline( qi, di, timeSinceStart, b );
    }

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::vector<double>> RigDeclineCurveCalculator::computeDeclineCurveValues( const DeclineModel&        model,
                                                                                         const std::vector<double>& values,
                                                                                         const std::vector<time_t>& timeSteps,
                                                                                         const std::vector<time_t>& futureTimeSteps )
{
    auto parameters = computeDeclineParameters( model, values, timeSteps );
    if ( !parameters ) return std::nullopt;

    auto computeValue = [&]( time_t time )
    {
        double timeSinceStart = static_cast<double>( time - parameters->referenceTime );
        double predictedValue = computePredictedValue( model, *parameters, timeSinceStart );
        if ( model.isAccumulated ) predictedValue += values.back();
        return predictedValue;
    };

    // The history part is a backward extrapolation of the decline curve. Keep the original value if the equation is not valid in the
    // history time range.
    std::vector<double> outValues;
    for ( size_t i = 0; i < values.size(); i++ )
    {
        double value = computeValue( timeSteps[i] );
        outValues.push_back( std::isfinite( value ) ? value : values[i] );
    }

    for ( time_t futureTime : futureTimeSteps )
    {
        outValues.push_back( computeValue( futureTime ) );
    }

    return outValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigDeclineCurveCalculator::equationText( const DeclineModel& model, const DeclineParameters& parameters )
{
    QString rateEquation;
    QString cumulativeEquation;
    switch ( model.type )
    {
        case DeclineCurveType::EXPONENTIAL:
            rateEquation       = "q(t) = qi * exp(-Di * t)";
            cumulativeEquation = "Np(t) = Np0 + (qi - q(t)) / Di";
            break;
        case DeclineCurveType::HARMONIC:
            rateEquation       = "q(t) = qi / (1 + Di * t)";
            cumulativeEquation = "Np(t) = Np0 + (qi / Di) * ln(qi / q(t))";
            break;
        case DeclineCurveType::HYPERBOLIC:
            rateEquation       = "q(t) = qi / (1 + b * Di * t)^(1 / b)";
            cumulativeEquation = "Np(t) = Np0 + qi^b / ((1 - b) * Di) * (qi^(1 - b) - q(t)^(1 - b))";
            break;
    }

    QStringList lines;
    lines << rateEquation;
    if ( model.isAccumulated ) lines << cumulativeEquation;

    // qi is derived per second for accumulated curves, scale to the time unit of t
    const double  qi     = model.isAccumulated ? parameters.initialProductionRate * secondsPerYear : parameters.initialProductionRate;
    const QString qiUnit = model.isAccumulated ? " [per year]" : "";
    QString       parameterText =
        QString( "qi = %1%2, Di = %3 [1/year]" ).arg( qi, 0, 'g', 6 ).arg( qiUnit ).arg( parameters.initialDeclineRate * secondsPerYear, 0, 'g', 6 );
    if ( model.type == DeclineCurveType::HYPERBOLIC )
        parameterText += QString( ", b = %1" ).arg( model.hyperbolicDeclineConstantB, 0, 'g', 4 );
    lines << parameterText;

    const QString referenceDate = QDateTime::fromSecsSinceEpoch( parameters.referenceTime ).date().toString( Qt::ISODate );
    lines << QString( "t is time in years since %1" ).arg( referenceDate );
    if ( model.isAccumulated ) lines << "Np0 is the accumulated value at t = 0";

    return lines.join( "\n" );
}
