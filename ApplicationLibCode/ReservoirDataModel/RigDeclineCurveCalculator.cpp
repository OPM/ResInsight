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
#include <array>
#include <cmath>
#include <limits>
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

namespace
{
double autoArpsLoss( double b, double lnQi, double diPerYear, const std::vector<double>& tYears, const std::vector<double>& rates )
{
    const double qi   = std::exp( lnQi );
    double       loss = 0.0;
    for ( size_t i = 0; i < tYears.size(); i++ )
    {
        double q = 0.0;
        if ( b < 1.0e-3 )
        {
            q = qi * std::exp( -diPerYear * tYears[i] );
        }
        else
        {
            const double base = 1.0 + b * diPerYear * tYears[i];
            if ( base <= 0.0 ) return std::numeric_limits<double>::max();
            q = qi / std::pow( base, 1.0 / b );
        }
        const double rel = ( q - rates[i] ) / rates[i];
        loss += rel * rel;
    }
    return std::isfinite( loss ) ? loss : std::numeric_limits<double>::max();
}

// Nelder-Mead minimization of the loss in (ln qi, Di) for a fixed b
std::pair<double, std::array<double, 2>>
    minimizeAutoArps( double b, std::array<double, 2> start, const std::vector<double>& tYears, const std::vector<double>& rates )
{
    using Point = std::array<double, 2>;
    auto f      = [&]( const Point& p ) { return autoArpsLoss( b, p[0], p[1], tYears, rates ); };

    std::array<Point, 3>  s = { start,
                                Point{ start[0] + 0.1, start[1] },
                                Point{ start[0], start[1] + 0.1 * std::max( 1.0, std::abs( start[1] ) ) } };
    std::array<double, 3> v = { f( s[0] ), f( s[1] ), f( s[2] ) };

    for ( int iter = 0; iter < 500; iter++ )
    {
        std::array<int, 3> idx = { 0, 1, 2 };
        std::sort( idx.begin(), idx.end(), [&]( int a, int c ) { return v[a] < v[c]; } );
        const Point  best = s[idx[0]], mid = s[idx[1]], worst = s[idx[2]];
        const double vb = v[idx[0]], vm = v[idx[1]], vw = v[idx[2]];
        s = { best, mid, worst };
        v = { vb, vm, vw };

        if ( std::abs( vw - vb ) < 1.0e-18 ) break;

        const Point centroid = { 0.5 * ( best[0] + mid[0] ), 0.5 * ( best[1] + mid[1] ) };
        auto        along    = [&]( double k )
        { return Point{ centroid[0] + k * ( worst[0] - centroid[0] ), centroid[1] + k * ( worst[1] - centroid[1] ) }; };

        const Point  refl  = along( -1.0 );
        const double vRefl = f( refl );
        if ( vRefl < vb )
        {
            const Point  exp  = along( -2.0 );
            const double vExp = f( exp );
            if ( vExp < vRefl )
            {
                s[2] = exp;
                v[2] = vExp;
            }
            else
            {
                s[2] = refl;
                v[2] = vRefl;
            }
        }
        else if ( vRefl < vm )
        {
            s[2] = refl;
            v[2] = vRefl;
        }
        else
        {
            const Point  contr  = along( vRefl < vw ? -0.5 : 0.5 );
            const double vContr = f( contr );
            if ( vContr < std::min( vw, vRefl ) )
            {
                s[2] = contr;
                v[2] = vContr;
            }
            else
            {
                for ( int k = 1; k < 3; k++ )
                {
                    s[k] = { best[0] + 0.5 * ( s[k][0] - best[0] ), best[1] + 0.5 * ( s[k][1] - best[1] ) };
                    v[k] = f( s[k] );
                }
            }
        }
    }

    const int bestIdx = static_cast<int>( std::min_element( v.begin(), v.end() ) - v.begin() );
    return { v[bestIdx], s[bestIdx] };
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigDeclineCurveCalculator::ArpsFit> RigDeclineCurveCalculator::fitAutoArps( const std::vector<double>& times,
                                                                                          const std::vector<double>& flowRates )
{
    // Only use positive rates, starting at the peak rate to skip the ramp-up phase
    std::vector<double> t, q;
    for ( size_t i = 0; i < std::min( times.size(), flowRates.size() ); i++ )
    {
        if ( flowRates[i] > 0.0 && std::isfinite( flowRates[i] ) )
        {
            t.push_back( times[i] / secondsPerYear );
            q.push_back( flowRates[i] );
        }
    }

    if ( q.empty() ) return std::nullopt;
    const size_t peak = static_cast<size_t>( std::max_element( q.begin(), q.end() ) - q.begin() );
    t.erase( t.begin(), t.begin() + peak );
    q.erase( q.begin(), q.begin() + peak );
    if ( q.size() < 3 ) return std::nullopt;

    // Start from the exponential fit
    std::array<double, 2> start = { std::log( q.back() ), 0.0 };
    {
        std::vector<double> lnq;
        for ( double v : q )
            lnq.push_back( std::log( v ) );
        std::vector<double> tSecondsStart;
        for ( double v : t )
            tSecondsStart.push_back( v * secondsPerYear );
        auto line = fitLine( tSecondsStart, lnq );
        if ( !line ) return std::nullopt;
        start = { line->first, -line->second * secondsPerYear };
        if ( !std::isfinite( start[0] ) || !std::isfinite( start[1] ) ) return std::nullopt;
    }

    double                bestLoss  = std::numeric_limits<double>::max();
    double                bestB     = 0.0;
    std::array<double, 2> bestPoint = start;

    std::vector<double> tSeconds;
    for ( double v : t )
        tSeconds.push_back( v * secondsPerYear );

    auto evaluate = [&]( double b )
    {
        // Use the linearized fit for this b as starting point when available
        std::array<double, 2> bStart = start;
        if ( b >= 1.0e-3 )
        {
            auto linear = fitHyperbolicDecline( tSeconds, q, b );
            if ( linear && linear->first > 0.0 && std::isfinite( linear->second ) )
                bStart = { std::log( linear->first ), linear->second * secondsPerYear };
        }
        // Restart from the previous result to escape a collapsed simplex
        auto [loss, point] = minimizeAutoArps( b, bStart, t, q );
        for ( int restart = 0; restart < 3; restart++ )
            std::tie( loss, point ) = minimizeAutoArps( b, point, t, q );
        if ( loss < bestLoss )
        {
            bestLoss  = loss;
            bestB     = b;
            bestPoint = point;
        }
    };

    for ( int i = 0; i <= 20; i++ )
        evaluate( 0.05 * i );

    // Refine around the best grid value
    const double coarseB = bestB;
    for ( int i = -4; i <= 4; i++ )
    {
        const double b = coarseB + 0.01 * i;
        if ( b >= 0.0 && b <= 1.0 ) evaluate( b );
    }

    if ( bestLoss == std::numeric_limits<double>::max() ) return std::nullopt;

    const double qi = std::exp( bestPoint[0] );
    const double di = bestPoint[1] / secondsPerYear;
    if ( !std::isfinite( qi ) || !std::isfinite( di ) || di < 0.0 ) return std::nullopt;

    return ArpsFit{ qi, di, bestB };
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
    double hyperbolicConstantB   = model.hyperbolicDeclineConstantB;

    if ( model.fitMethod == FitMethod::START_END_POINT && model.type != DeclineCurveType::AUTO_ARPS )
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
        if ( model.type == DeclineCurveType::AUTO_ARPS )
        {
            auto arps = fitAutoArps( times, rates );
            if ( !arps ) return std::nullopt;
            fit                 = std::make_pair( arps->initialProductionRate, arps->initialDeclineRate );
            hyperbolicConstantB = arps->hyperbolicDeclineConstantB;
        }
        else if ( model.type == DeclineCurveType::EXPONENTIAL )
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

    return DeclineParameters{ initialProductionRate, initialDeclineRate, referenceTime, hyperbolicConstantB };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigDeclineCurveCalculator::computePredictedValue( const DeclineModel& model, const DeclineParameters& parameters, double timeSinceStart )
{
    const double qi = parameters.initialProductionRate;
    const double di = parameters.initialDeclineRate;
    const double b  = parameters.hyperbolicDeclineConstantB;

    switch ( model.type )
    {
        case DeclineCurveType::EXPONENTIAL:
            return model.isAccumulated ? computeCumulativeProductionExponentialDecline( qi, di, timeSinceStart )
                                       : computeFlowRateExponentialDecline( qi, di, timeSinceStart );
        case DeclineCurveType::HARMONIC:
            return model.isAccumulated ? computeCumulativeProductionHarmonicDecline( qi, di, timeSinceStart )
                                       : computeFlowRateHarmonicDecline( qi, di, timeSinceStart );
        case DeclineCurveType::AUTO_ARPS:
            if ( b < 1.0e-3 )
                return model.isAccumulated ? computeCumulativeProductionExponentialDecline( qi, di, timeSinceStart )
                                           : computeFlowRateExponentialDecline( qi, di, timeSinceStart );
            if ( std::abs( b - 1.0 ) < 1.0e-3 )
                return model.isAccumulated ? computeCumulativeProductionHarmonicDecline( qi, di, timeSinceStart )
                                           : computeFlowRateHarmonicDecline( qi, di, timeSinceStart );
            [[fallthrough]];
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
        case DeclineCurveType::AUTO_ARPS:
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
    if ( model.type == DeclineCurveType::HYPERBOLIC || model.type == DeclineCurveType::AUTO_ARPS )
        parameterText += QString( ", b = %1" ).arg( parameters.hyperbolicDeclineConstantB, 0, 'g', 4 );
    lines << parameterText;

    const QString referenceDate = QDateTime::fromSecsSinceEpoch( parameters.referenceTime ).date().toString( Qt::ISODate );
    lines << QString( "t is time in years since %1" ).arg( referenceDate );
    if ( model.isAccumulated ) lines << "Np0 is the accumulated value at t = 0";

    return lines.join( "\n" );
}
