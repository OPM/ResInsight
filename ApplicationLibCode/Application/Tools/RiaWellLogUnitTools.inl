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
#include "RiaEclipseUnitTools.h"
#include "RigWellPath.h"

#include "cafAssert.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
const FloatType RiaWellLogUnitTools<FloatType>::gravityAcceleration()
{
    return (FloatType)9.81;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
const FloatType RiaWellLogUnitTools<FloatType>::unitWeightOfWater()
{
    return RiaWellLogUnitTools<FloatType>::gravityAcceleration() * 1000.0; // N / m^3
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
bool RiaWellLogUnitTools<FloatType>::stringsMatch( const QString& lhs, const QString& rhs )
{
    return QString::compare( lhs, rhs, Qt::CaseInsensitive ) == 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::noUnitString()
{
    return "NO_UNIT";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::sg_emwUnitString()
{
    return "sg_EMW";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::barUnitString()
{
    return "Bar";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::barX100UnitString()
{
    return "Bar x100";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::MPaUnitString()
{
    return "MPa";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::gPerCm3UnitString()
{
    return "g/cm3";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::kgPerM3UnitString()
{
    return "kg/m3";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::pascalUnitString()
{
    return "pascal";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
QString RiaWellLogUnitTools<FloatType>::pascalUnitStringShort()
{
    return "pa";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
std::vector<FloatType> RiaWellLogUnitTools<FloatType>::convertDepths( const std::vector<FloatType>& depthsIn,
                                                                      RiaDefines::DepthUnitType     unitsIn,
                                                                      RiaDefines::DepthUnitType     unitsOut )
{
    if ( unitsOut == RiaDefines::DepthUnitType::UNIT_METER && unitsIn == RiaDefines::DepthUnitType::UNIT_FEET )
    {
        return multiply( depthsIn, RiaEclipseUnitTools::meterPerFeet() );
    }
    else if ( unitsOut == RiaDefines::DepthUnitType::UNIT_FEET && unitsIn == RiaDefines::DepthUnitType::UNIT_METER )
    {
        return multiply( depthsIn, RiaEclipseUnitTools::feetPerMeter() );
    }
    return depthsIn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
FloatType RiaWellLogUnitTools<FloatType>::convertDepth( FloatType                 depthIn,
                                                        RiaDefines::DepthUnitType unitsIn,
                                                        RiaDefines::DepthUnitType unitsOut )
{
    FloatType factor = 1.0;
    if ( unitsOut == RiaDefines::DepthUnitType::UNIT_METER && unitsIn == RiaDefines::DepthUnitType::UNIT_FEET )
    {
        factor = RiaEclipseUnitTools::meterPerFeet();
    }
    else if ( unitsOut == RiaDefines::DepthUnitType::UNIT_FEET && unitsIn == RiaDefines::DepthUnitType::UNIT_METER )
    {
        factor = RiaEclipseUnitTools::feetPerMeter();
    }
    return depthIn * factor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
std::vector<std::pair<FloatType, FloatType>>
    RiaWellLogUnitTools<FloatType>::convertDepths( const std::vector<std::pair<FloatType, FloatType>>& depthsIn,
                                                   RiaDefines::DepthUnitType                           unitsIn,
                                                   RiaDefines::DepthUnitType                           unitsOut )
{
    std::vector<std::pair<FloatType, FloatType>> convertedDepths( depthsIn.size() );
    double                                       factor = 1.0;
    if ( unitsOut == RiaDefines::DepthUnitType::UNIT_METER && unitsIn == RiaDefines::DepthUnitType::UNIT_FEET )
    {
        factor = RiaEclipseUnitTools::meterPerFeet();
    }
    else if ( unitsOut == RiaDefines::DepthUnitType::UNIT_FEET && unitsIn == RiaDefines::DepthUnitType::UNIT_METER )
    {
        factor = RiaEclipseUnitTools::feetPerMeter();
    }

    int i = 0;
    for ( auto& p : depthsIn )
    {
        convertedDepths[i++] = std::make_pair( factor * p.first, factor * p.second );
    }

    return convertedDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
bool RiaWellLogUnitTools<FloatType>::convertValues( const std::vector<FloatType>& tvdRKBs,
                                                    const std::vector<FloatType>& valuesIn,
                                                    std::vector<FloatType>*       valuesOut,
                                                    const QString&                unitsIn,
                                                    const QString&                unitsOut )
{
    CAF_ASSERT( valuesOut );

    if ( stringsMatch( unitsIn, unitsOut ) )
    {
        *valuesOut = valuesIn;
        return true;
    }

    if ( stringsMatch( unitsIn, gPerCm3UnitString() ) && stringsMatch( unitsOut, barUnitString() ) )
    {
        *valuesOut = convertGpcm3ToBar( tvdRKBs, valuesIn );
        return true;
    }
    else if ( stringsMatch( unitsOut, barUnitString() ) && stringsMatch( unitsOut, gPerCm3UnitString() ) )
    {
        *valuesOut = convertBarToGpcm3( tvdRKBs, valuesIn );
        return true;
    }
    else if ( ( stringsMatch( unitsIn, noUnitString() ) || stringsMatch( unitsIn, sg_emwUnitString() ) ) &&
              stringsMatch( unitsOut, gPerCm3UnitString() ) )
    {
        *valuesOut = convertNormalizedByPPToBar( tvdRKBs, valuesIn );
        *valuesOut = convertBarToGpcm3( tvdRKBs, *valuesOut );
        return true;
    }
    else if ( stringsMatch( unitsIn, gPerCm3UnitString() ) &&
              ( stringsMatch( unitsOut, noUnitString() ) || stringsMatch( unitsOut, sg_emwUnitString() ) ) )
    {
        *valuesOut = convertGpcm3ToBar( tvdRKBs, valuesIn );
        *valuesOut = convertBarToNormalizedByPP( tvdRKBs, *valuesOut );
        return true;
    }
    else if ( stringsMatch( unitsIn, MPaUnitString() ) && stringsMatch( unitsOut, barUnitString() ) )
    {
        *valuesOut = multiply( valuesIn, 1.0 / MPaPerBar() );
        return true;
    }
    else if ( stringsMatch( unitsIn, barX100UnitString() ) && stringsMatch( unitsOut, MPaUnitString() ) )
    {
        *valuesOut = multiply( valuesIn, (FloatType)100.0 );
        *valuesOut = multiply( *valuesOut, MPaPerBar() );
        return true;
    }
    else if ( stringsMatch( unitsIn, barX100UnitString() ) && stringsMatch( unitsOut, barUnitString() ) )
    {
        *valuesOut = multiply( valuesIn, (FloatType)100 );
        return true;
    }
    else if ( stringsMatch( unitsIn, barUnitString() ) && stringsMatch( unitsOut, MPaUnitString() ) )
    {
        *valuesOut = multiply( valuesIn, MPaPerBar() );
        return true;
    }
    else if ( stringsMatch( unitsIn, barUnitString() ) && stringsMatch( unitsOut, barX100UnitString() ) )
    {
        *valuesOut = multiply( valuesIn, (FloatType)0.01 );
        return true;
    }
    else if ( ( stringsMatch( unitsIn, noUnitString() ) || stringsMatch( unitsIn, sg_emwUnitString() ) ) &&
              stringsMatch( unitsOut, barUnitString() ) )
    {
        *valuesOut = convertNormalizedByPPToBar( tvdRKBs, valuesIn );
        return true;
    }
    else if ( stringsMatch( unitsIn, barUnitString() ) &&
              ( stringsMatch( unitsOut, noUnitString() ) || stringsMatch( unitsOut, sg_emwUnitString() ) ) )
    {
        *valuesOut = convertBarToNormalizedByPP( tvdRKBs, valuesIn );
        return true;
    }
    else if ( ( stringsMatch( unitsIn, pascalUnitString() ) ||
                stringsMatch( unitsIn, pascalUnitString() ) && stringsMatch( unitsOut, barUnitString() ) ) )
    {
        *valuesOut = multiply( valuesIn, 1.0 / pascalPerBar() );
        return true;
    }
    else if ( stringsMatch( unitsIn, barUnitString() ) &&
              ( stringsMatch( unitsIn, pascalUnitString() ) || stringsMatch( unitsIn, pascalUnitString() ) ) )
    {
        *valuesOut = multiply( valuesIn, pascalPerBar() );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
bool RiaWellLogUnitTools<FloatType>::convertValues( std::vector<std::pair<FloatType, FloatType>>* measuredDepthsAndValues,
                                                    const QString&                                unitsIn,
                                                    const QString&                                unitsOut,
                                                    const RigWellPath*                            wellPath )
{
    CAF_ASSERT( measuredDepthsAndValues );
    if ( unitsIn == unitsOut ) return true;

    std::vector<FloatType> tvdRKBs( measuredDepthsAndValues->size(), 0.0 );
    std::vector<FloatType> values( measuredDepthsAndValues->size(), 0.0 );
    for ( size_t i = 0; i < measuredDepthsAndValues->size(); ++i )
    {
        auto       depthValuePair = ( *measuredDepthsAndValues )[i];
        cvf::Vec3d point          = wellPath->interpolatedPointAlongWellPath( depthValuePair.first );

        tvdRKBs[i] = -point.z() + wellPath->rkbDiff();
        values[i]  = depthValuePair.second;
    }
    std::vector<FloatType> valuesOut;
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
template <typename FloatType>
std::vector<FloatType>
    RiaWellLogUnitTools<FloatType>::tvdRKBs( const std::vector<FloatType>& measuredDepths, const RigWellPath* wellPath )
{
    std::vector<double> tvdRKBs( measuredDepths.size(), 0.0 );
    for ( size_t i = 0; i < measuredDepths.size(); ++i )
    {
        cvf::Vec3d point = wellPath->interpolatedPointAlongWellPath( measuredDepths[i] );
        tvdRKBs[i]       = -point.z() + wellPath->rkbDiff();
    }
    return std::vector<FloatType>( tvdRKBs.begin(), tvdRKBs.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
std::vector<FloatType> RiaWellLogUnitTools<FloatType>::convertGpcm3ToBar( const std::vector<FloatType>& tvdRKBs,
                                                                          const std::vector<FloatType>& valuesInGpcm3 )
{
    CAF_ASSERT( tvdRKBs.size() == valuesInGpcm3.size() );

    std::vector<FloatType> valuesInBar( valuesInGpcm3.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        // We need SI as input (kg / m^3), so multiply by 1000:
        FloatType mudWeightsSI = valuesInGpcm3[i] * 1000;

        // To get specific mudWeight (N / m^3):
        FloatType specificMudWeight = mudWeightsSI * gravityAcceleration();

        // Pore pressure in pascal
        FloatType valuePascal = specificMudWeight * tvdRKBs[i];

        // Pore pressure in bar
        valuesInBar[i] = 1.0 / pascalPerBar() * valuePascal;
    }
    return valuesInBar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
std::vector<FloatType> RiaWellLogUnitTools<FloatType>::convertBarToGpcm3( const std::vector<FloatType>& tvdRKBs,
                                                                          const std::vector<FloatType>& valuesInBar )
{
    CAF_ASSERT( tvdRKBs.size() == valuesInBar.size() );

    std::vector<FloatType> valuesInGpcm3( valuesInBar.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        // Pore pressure in pascal (N / m^2)
        FloatType valuePascal = pascalPerBar() * valuesInBar[i];

        // N / m^3:
        FloatType specificMudWeight = valuePascal / tvdRKBs[i];

        // Mud weight in SI (kg / m^3):
        FloatType mudWeightsSI = specificMudWeight / gravityAcceleration();

        // Mud weights g/cm^3:
        valuesInGpcm3[i] = mudWeightsSI / 1000;
    }
    return valuesInGpcm3;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
std::vector<FloatType>
    RiaWellLogUnitTools<FloatType>::convertNormalizedByPPToBar( const std::vector<FloatType>& tvdRKBs,
                                                                const std::vector<FloatType>& normalizedValues )
{
    CAF_ASSERT( tvdRKBs.size() == normalizedValues.size() );

    std::vector<FloatType> valuesInBar( tvdRKBs.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        valuesInBar[i] = normalizedValues[i] * hydrostaticPorePressureBar( tvdRKBs[i] );
    }
    return valuesInBar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
std::vector<FloatType>
    RiaWellLogUnitTools<FloatType>::convertBarToNormalizedByPP( const std::vector<FloatType>& tvdRKBs,
                                                                const std::vector<FloatType>& valuesInBar )
{
    CAF_ASSERT( tvdRKBs.size() == valuesInBar.size() );

    std::vector<FloatType> normalizedValues( tvdRKBs.size(), 0.0 );
    for ( size_t i = 0; i < tvdRKBs.size(); ++i )
    {
        normalizedValues[i] = valuesInBar[i] / hydrostaticPorePressureBar( tvdRKBs[i] );
    }
    return normalizedValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
std::vector<FloatType> RiaWellLogUnitTools<FloatType>::multiply( const std::vector<FloatType>& valuesIn, FloatType factor )
{
    std::vector<FloatType> valuesOut( valuesIn.size(), std::numeric_limits<FloatType>::infinity() );
    for ( size_t i = 0; i < valuesIn.size(); ++i )
    {
        valuesOut[i] = valuesIn[i] * factor;
    }
    return valuesOut;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
FloatType RiaWellLogUnitTools<FloatType>::pascalPerBar()
{
    return 1.0e5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
FloatType RiaWellLogUnitTools<FloatType>::MPaPerBar()
{
    return (FloatType)1.0e-1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FloatType>
FloatType RiaWellLogUnitTools<FloatType>::hydrostaticPorePressureBar( FloatType depth )
{
    return (FloatType)1.0 / pascalPerBar() * depth * unitWeightOfWater();
}
