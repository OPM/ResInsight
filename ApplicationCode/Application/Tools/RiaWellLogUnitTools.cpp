/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor AS
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
#include "RiaWellLogUnitTools.h"

#include "RiaEclipseUnitTools.h"
#include "RigWellPath.h"

#include "cafAssert.h"

#include <limits>

const double RiaWellLogUnitTools::GRAVITY_ACCEL        = 9.81;
const double RiaWellLogUnitTools::UNIT_WEIGHT_OF_WATER = RiaWellLogUnitTools::GRAVITY_ACCEL * 1000.0; // N / m^3

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellLogUnitTools::noUnitString()
{
    return "NO_UNIT";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellLogUnitTools::barUnitString()
{
    return "Bar";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellLogUnitTools::barX100UnitString()
{
    return "Bar x100";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellLogUnitTools::MPaUnitString()
{
    return "MPa";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaWellLogUnitTools::gPerCm3UnitString()
{
    return "g/cm3";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaWellLogUnitTools::convertDepths( const std::vector<double>& depthsIn,
                                                        RiaDefines::DepthUnitType  unitsIn,
                                                        RiaDefines::DepthUnitType  unitsOut )
{
    if ( unitsOut == RiaDefines::UNIT_METER && unitsIn == RiaDefines::UNIT_FEET )
    {
        return multiply( depthsIn, RiaEclipseUnitTools::feetPerMeter() );
    }
    else if ( unitsOut == RiaDefines::UNIT_FEET && unitsIn == RiaDefines::UNIT_METER )
    {
        return multiply( depthsIn, RiaEclipseUnitTools::meterPerFeet() );
    }
    return depthsIn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaWellLogUnitTools::convertValues( const std::vector<double>& tvdRKBs,
                                         const std::vector<double>& valuesIn,
                                         std::vector<double>*       valuesOut,
                                         const QString&             unitsIn,
                                         const QString&             unitsOut )
{
    CAF_ASSERT( valuesOut );

    if ( unitsIn == unitsOut )
    {
        *valuesOut = valuesIn;
        return true;
    }

    if ( unitsIn == gPerCm3UnitString() && unitsOut == barUnitString() )
    {
        *valuesOut = convertGpcm3ToBar( tvdRKBs, valuesIn );
        return true;
    }
    else if ( unitsIn == barUnitString() && unitsOut == gPerCm3UnitString() )
    {
        *valuesOut = convertBarToGpcm3( tvdRKBs, valuesIn );
        return true;
    }
    else if ( unitsIn == RiaWellLogUnitTools::noUnitString() && unitsOut == gPerCm3UnitString() )
    {
        *valuesOut = convertNormalizedByPPToBar( tvdRKBs, valuesIn );
        *valuesOut = convertBarToGpcm3( tvdRKBs, *valuesOut );
    }
    else if ( unitsIn == gPerCm3UnitString() && unitsOut == RiaWellLogUnitTools::noUnitString() )
    {
        *valuesOut = convertGpcm3ToBar( tvdRKBs, valuesIn );
        *valuesOut = convertBarToNormalizedByPP( tvdRKBs, *valuesOut );
    }
    else if ( unitsIn == MPaUnitString() && unitsOut == barUnitString() )

    {
        *valuesOut = multiply( valuesIn, 1.0 / MPaPerBar() );
        return true;
    }
    else if ( unitsIn == barX100UnitString() && unitsOut == MPaUnitString() )
    {
        *valuesOut = multiply( valuesIn, 100.0 );
        *valuesOut = multiply( *valuesOut, MPaPerBar() );
        return true;
    }
    else if ( unitsIn == barX100UnitString() && unitsOut == barUnitString() )
    {
        *valuesOut = multiply( valuesIn, 100 );
        return true;
    }
    else if ( unitsIn == barUnitString() && unitsOut == MPaUnitString() )
    {
        *valuesOut = multiply( valuesIn, MPaPerBar() );
        return true;
    }
    else if ( unitsIn == barUnitString() && unitsOut == barX100UnitString() )
    {
        *valuesOut = multiply( valuesIn, 1.0 / 100.0 );
        return true;
    }
    else if ( unitsIn == RiaWellLogUnitTools::noUnitString() && unitsOut == barUnitString() )
    {
        *valuesOut = convertNormalizedByPPToBar( tvdRKBs, valuesIn );
        return true;
    }
    else if ( unitsIn == barUnitString() && unitsOut == RiaWellLogUnitTools::noUnitString() )
    {
        *valuesOut = convertBarToNormalizedByPP( tvdRKBs, valuesIn );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaWellLogUnitTools::convertValues( std::vector<std::pair<double, double>>* measuredDepthsAndValues,
                                         const QString&                          unitsIn,
                                         const QString&                          unitsOut,
                                         const RigWellPath*                      wellPath )
{
    CAF_ASSERT( measuredDepthsAndValues );
    if ( unitsIn == unitsOut ) return true;

    std::vector<double> tvdRKBs( measuredDepthsAndValues->size(), 0.0 );
    std::vector<double> values( measuredDepthsAndValues->size(), 0.0 );
    for ( size_t i = 0; i < measuredDepthsAndValues->size(); ++i )
    {
        auto       depthValuePair = ( *measuredDepthsAndValues )[i];
        cvf::Vec3d point          = wellPath->interpolatedPointAlongWellPath( depthValuePair.first );

        tvdRKBs[i] = -point.z() + wellPath->rkbDiff();
        values[i]  = depthValuePair.second;
    }
    std::vector<double> valuesOut;
    if ( convertValues( tvdRKBs, values, &valuesOut, unitsIn, unitsOut ) )
    {
        CAF_ASSERT( measuredDepthsAndValues->size() == valuesOut.size() );
        for ( size_t i = 0; i < measuredDepthsAndValues->size(); ++i )
        {
            ( *measuredDepthsAndValues )[i].second = valuesOut[i];
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaWellLogUnitTools::tvdRKBs( const std::vector<double>& measuredDepths, const RigWellPath* wellPath )
{
    std::vector<double> tvdRKBs( measuredDepths.size(), 0.0 );
    for ( size_t i = 0; i < measuredDepths.size(); ++i )
    {
        cvf::Vec3d point = wellPath->interpolatedPointAlongWellPath( measuredDepths[i] );
        tvdRKBs[i]       = -point.z() + wellPath->rkbDiff();
    }
    return tvdRKBs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaWellLogUnitTools::convertGpcm3ToBar( const std::vector<double>& tvdRKBs,
                                                            const std::vector<double>& valuesInGpcm3 )
{
    CAF_ASSERT( tvdRKBs.size() == valuesInGpcm3.size() );

    std::vector<double> valuesInBar( valuesInGpcm3.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        // We need SI as input (kg / m^3), so multiply by 1000:
        double mudWeightsSI = valuesInGpcm3[i] * 1000;

        // To get specific mudWeight (N / m^3):
        double specificMudWeight = mudWeightsSI * GRAVITY_ACCEL;

        // Pore pressure in pascal
        double valuePascal = specificMudWeight * tvdRKBs[i];

        // Pore pressure in bar
        valuesInBar[i] = 1.0 / pascalPerBar() * valuePascal;
    }
    return valuesInBar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaWellLogUnitTools::convertBarToGpcm3( const std::vector<double>& tvdRKBs,
                                                            const std::vector<double>& valuesInBar )
{
    CAF_ASSERT( tvdRKBs.size() == valuesInBar.size() );

    std::vector<double> valuesInGpcm3( valuesInBar.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        // Pore pressure in pascal (N / m^2)
        double valuePascal = pascalPerBar() * valuesInBar[i];

        // N / m^3:
        double specificMudWeight = valuePascal / tvdRKBs[i];

        // Mud weight in SI (kg / m^3):
        double mudWeightsSI = specificMudWeight / GRAVITY_ACCEL;

        // Mud weights g/cm^3:
        valuesInGpcm3[i] = mudWeightsSI / 1000;
    }
    return valuesInGpcm3;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaWellLogUnitTools::convertNormalizedByPPToBar( const std::vector<double>& tvdRKBs,
                                                                     const std::vector<double>& normalizedValues )
{
    CAF_ASSERT( tvdRKBs.size() == normalizedValues.size() );

    std::vector<double> valuesInBar( tvdRKBs.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        valuesInBar[i] = normalizedValues[i] * hydrostaticPorePressureBar( tvdRKBs[i] );
    }
    return valuesInBar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaWellLogUnitTools::convertBarToNormalizedByPP( const std::vector<double>& tvdRKBs,
                                                                     const std::vector<double>& valuesInBar )
{
    CAF_ASSERT( tvdRKBs.size() == valuesInBar.size() );

    std::vector<double> normalizedValues( tvdRKBs.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        normalizedValues[i] = valuesInBar[i] / hydrostaticPorePressureBar( tvdRKBs[i] );
    }
    return normalizedValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaWellLogUnitTools::multiply( const std::vector<double>& valuesIn, double factor )
{
    std::vector<double> valuesOut( valuesIn.size(), std::numeric_limits<double>::infinity() );
    for ( size_t i = 0; i < 100; ++i )
    {
        valuesOut[i] = valuesIn[i] * factor;
    }
    return valuesOut;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWellLogUnitTools::pascalPerBar()
{
    return 1.0e5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWellLogUnitTools::MPaPerBar()
{
    return 1.0e-1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWellLogUnitTools::hydrostaticPorePressureBar( double depth )
{
    return 1.0 / pascalPerBar() * depth * UNIT_WEIGHT_OF_WATER;
}
