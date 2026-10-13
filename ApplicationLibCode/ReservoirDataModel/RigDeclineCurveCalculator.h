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

#pragma once

#include <QString>

#include <ctime>
#include <optional>
#include <utility>
#include <vector>

//==================================================================================================
//
//
//
//==================================================================================================
class RigDeclineCurveCalculator
{
public:
    enum class DeclineCurveType
    {
        EXPONENTIAL,
        HARMONIC,
        HYPERBOLIC
    };

    enum class FitMethod
    {
        START_END_POINT,
        REGRESSION
    };

    struct DeclineModel
    {
        DeclineCurveType type;
        FitMethod        fitMethod;
        double           hyperbolicDeclineConstantB;
        bool             isAccumulated;
    };

    struct DeclineParameters
    {
        double initialProductionRate; // [rate unit]
        double initialDeclineRate; // [1/second]
        time_t referenceTime; // time at which the initial production rate applies
    };

    static constexpr double secondsPerYear = 365.25 * 24.0 * 3600.0;

    // Estimate the decline parameters from the history values. The reference time is the last time step.
    static std::optional<DeclineParameters>
        computeDeclineParameters( const DeclineModel& model, const std::vector<double>& values, const std::vector<time_t>& timeSteps );

    // Flow rate, or cumulative production increment since the reference time if the model is accumulated
    static double computePredictedValue( const DeclineModel& model, const DeclineParameters& parameters, double timeSinceStart );

    // Returns the decline curve evaluated at the history time steps (backward extrapolation) followed by the future time steps.
    static std::optional<std::vector<double>> computeDeclineCurveValues( const DeclineModel&        model,
                                                                         const std::vector<double>& values,
                                                                         const std::vector<time_t>& timeSteps,
                                                                         const std::vector<time_t>& futureTimeSteps );

    static QString equationText( const DeclineModel& model, const DeclineParameters& parameters );

    static double computeDeclineRate( double time0, double value0, double time1, double value1 );

    static double computeFlowRateExponentialDecline( double initialProductionRateQi, double initialDeclineRateDi, double timeSinceStart );

    static double computeFlowRateHarmonicDecline( double initialProductionRateQi, double initialDeclineRateDi, double timeSinceStart );

    static double computeFlowRateHyperbolicDecline( double initialProductionRateQi,
                                                    double initialDeclineRateDi,
                                                    double timeSinceStart,
                                                    double hyperbolicDeclineConstantB );

    static double
        computeCumulativeProductionExponentialDecline( double initialProductionRateQi, double initialDeclineRateDi, double timeSinceStart );

    static double
        computeCumulativeProductionHarmonicDecline( double initialProductionRateQi, double initialDeclineRateDi, double timeSinceStart );

    static double computeCumulativeProductionHyperbolicDecline( double initialProductionRateQi,
                                                                double initialDeclineRateDi,
                                                                double timeSinceStart,
                                                                double hyperbolicDeclineConstantB );

    // Least squares fit of the decline equation to the given flow rates. Time values are seconds relative to the reference time (the
    // initial production rate is the rate at time zero). Returns {initialProductionRateQi, initialDeclineRateDi}, or nullopt if the
    // data does not allow a fit. Non-positive rates are ignored.
    using FitResult = std::optional<std::pair<double, double>>;

    static FitResult fitExponentialDecline( const std::vector<double>& times, const std::vector<double>& flowRates );
    static FitResult fitHarmonicDecline( const std::vector<double>& times, const std::vector<double>& flowRates );
    static FitResult
        fitHyperbolicDecline( const std::vector<double>& times, const std::vector<double>& flowRates, double hyperbolicDeclineConstantB );

private:
    static std::pair<double, double>
        computeStartEndPointParameters( const std::vector<double>& values, const std::vector<time_t>& timeSteps, bool isAccumulated );

    // Least squares fit of y = a + b * t. Returns {a, b}.
    static std::optional<std::pair<double, double>> fitLine( const std::vector<double>& t, const std::vector<double>& y );
};
