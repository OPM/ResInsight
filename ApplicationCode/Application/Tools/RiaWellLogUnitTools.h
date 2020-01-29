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

class RiaWellLogUnitTools
{
public:
    static const double GRAVITY_ACCEL;
    static const double UNIT_WEIGHT_OF_WATER;

    static QString noUnitString();
    static QString barUnitString();
    static QString barX100UnitString();
    static QString MPaUnitString();
    static QString gPerCm3UnitString();

public:
    static std::vector<double> convertDepths( const std::vector<double>& depthsIn,
                                              RiaDefines::DepthUnitType  unitsIn,
                                              RiaDefines::DepthUnitType  unitsOut );
    static bool                convertValues( const std::vector<double>& tvdRKBs,
                                              const std::vector<double>& valuesIn,
                                              std::vector<double>*       valuesOut,
                                              const QString&             unitsIn,
                                              const QString&             unitsOut );
    static bool                convertValues( std::vector<std::pair<double, double>>* measuredDepthsAndValues,
                                              const QString&                          unitsIn,
                                              const QString&                          unitsOut,
                                              const RigWellPath*                      wellPath );

    static std::vector<double> tvdRKBs( const std::vector<double>& measuredDepths, const RigWellPath* wellPath );

    // Supported conversions
    static std::vector<double> convertGpcm3ToBar( const std::vector<double>& tvdRKBs,
                                                  const std::vector<double>& valuesInGpcm3 );
    static std::vector<double> convertBarToGpcm3( const std::vector<double>& tvdRKBs,
                                                  const std::vector<double>& valuesInBar );

    static std::vector<double> convertNormalizedByPPToBar( const std::vector<double>& tvdRKBs,
                                                           const std::vector<double>& valuesInBar );
    static std::vector<double> convertBarToNormalizedByPP( const std::vector<double>& tvdRKBs,
                                                           const std::vector<double>& valuesInBar );
    static std::vector<double> multiply( const std::vector<double>& values, double factor );
    static double              pascalPerBar();
    static double              MPaPerBar();
    static double              hydrostaticPorePressureBar( double tvdRKB );
};
