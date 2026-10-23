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
        HYPERBOLIC,
        AUTO_ARPS // Arps model where the hyperbolic constant is fitted to the data
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
        double hyperbolicDeclineConstantB = 0.5; // fitted for AUTO_ARPS, otherwise copied from the model
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

    struct ArpsFit
    {
        double initialProductionRate;
        double initialDeclineRate; // [1/second]
        double hyperbolicDeclineConstantB;
    };

    // Automatic Arps fit inspired by AutoDCA (https://github.com/equinor/decline-curve-analysis), q(t) = qi / (1 + b * Di * t)^(1 / b),
    // where b = 0 is exponential and b = 1 is harmonic.
    //
    // Algorithm:
    //  1. Keep positive, finite rates and discard everything before the peak rate (ramp-up/plateau build-up). At least three points
    //     must remain.
    //  2. For a grid of b values in [0, 1] (step 0.05), minimize the sum of squared relative errors ((q_model - q) / q)^2 over (ln qi, Di)
    //     using Nelder-Mead. The start guess is the linearized hyperbolic fit for that b (the exponential fit for b = 0), and the
    //     minimization is restarted a few times from its own result to avoid a collapsed simplex. Parameter sets where the model is
    //     undefined in the fitted time range are rejected.
    //  3. Refine b with a finer grid (step 0.01) around the best coarse value, and return the parameters with the lowest loss. Fits with
    //     a negative decline rate are rejected.
    //
    // Time values are seconds relative to the reference time (<= 0).
    static std::optional<ArpsFit> fitAutoArps( const std::vector<double>& times, const std::vector<double>& flowRates );

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
