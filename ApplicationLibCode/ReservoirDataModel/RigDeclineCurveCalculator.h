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

//==================================================================================================
//
//
//
//==================================================================================================
class RigDeclineCurveCalculator
{
public:
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
};
