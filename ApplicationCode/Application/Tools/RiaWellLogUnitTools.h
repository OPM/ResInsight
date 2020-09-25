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
#pragma once

#include "RiaDefines.h"

#include <QString>

#include <vector>

class RigWellPath;

template <typename FloatType>
class RiaWellLogUnitTools
{
public:
    static const FloatType gravityAcceleration();
    static const FloatType unitWeightOfWater();
    static bool            stringsMatch( const QString& lhs, const QString& rhs );

    static QString noUnitString();
    static QString sg_emwUnitString();
    static QString barUnitString();
    static QString barX100UnitString();
    static QString MPaUnitString();
    static QString gPerCm3UnitString();
    static QString kgPerM3UnitString();
    static QString pascalUnitString();
    static QString pascalUnitStringShort();

public:
    static std::vector<FloatType> convertDepths( const std::vector<FloatType>& depthsIn,
                                                 RiaDefines::DepthUnitType     unitsIn,
                                                 RiaDefines::DepthUnitType     unitsOut );

    static std::vector<std::pair<FloatType, FloatType>>
        convertDepths( const std::vector<std::pair<FloatType, FloatType>>& depthsIn,
                       RiaDefines::DepthUnitType                           unitsIn,
                       RiaDefines::DepthUnitType                           unitsOut );

    static FloatType
        convertDepth( FloatType depthIn, RiaDefines::DepthUnitType unitsIn, RiaDefines::DepthUnitType unitsOut );

    static bool convertValues( const std::vector<FloatType>& tvdRKBs,
                               const std::vector<FloatType>& valuesIn,
                               std::vector<FloatType>*       valuesOut,
                               const QString&                unitsIn,
                               const QString&                unitsOut );
    static bool convertValues( std::vector<std::pair<FloatType, FloatType>>* measuredDepthsAndValues,
                               const QString&                                unitsIn,
                               const QString&                                unitsOut,
                               const RigWellPath*                            wellPath );

    static std::vector<FloatType> tvdRKBs( const std::vector<FloatType>& measuredDepths, const RigWellPath* wellPath );

    // Supported conversions
    static std::vector<FloatType> convertGpcm3ToBar( const std::vector<FloatType>& tvdRKBs,
                                                     const std::vector<FloatType>& valuesInGpcm3 );
    static std::vector<FloatType> convertBarToGpcm3( const std::vector<FloatType>& tvdRKBs,
                                                     const std::vector<FloatType>& valuesInBar );

    static std::vector<FloatType> convertNormalizedByPPToBar( const std::vector<FloatType>& tvdRKBs,
                                                              const std::vector<FloatType>& valuesInBar );
    static std::vector<FloatType> convertBarToNormalizedByPP( const std::vector<FloatType>& tvdRKBs,
                                                              const std::vector<FloatType>& valuesInBar );
    static std::vector<FloatType> multiply( const std::vector<FloatType>& values, FloatType factor );
    static FloatType              pascalPerBar();
    static FloatType              MPaPerBar();
    static FloatType              hydrostaticPorePressureBar( FloatType tvdRKB );
};

#include "RiaWellLogUnitTools.inl"
