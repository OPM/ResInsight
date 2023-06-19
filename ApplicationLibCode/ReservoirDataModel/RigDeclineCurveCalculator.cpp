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

#include <cmath>

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
