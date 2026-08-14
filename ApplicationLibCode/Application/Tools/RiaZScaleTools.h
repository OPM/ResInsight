/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include <vector>

//==================================================================================================
//
// Keeps track of the Z-scale factors available in the user interface: the predefined factors from
// RiaDefines::viewScaleOptions() merged with any custom factors entered by the user during the
// session. All consumers of the scale factor list (toolbar combo box, property editor combo box
// and the increase/decrease keyboard shortcuts) use scaleFactorOptions() so they present and step
// through the same sorted list.
//
//==================================================================================================
class RiaZScaleTools
{
public:
    static void                registerScaleFactor( double scaleFactor );
    static std::vector<double> scaleFactorOptions();

    static double nextScaleFactor( double currentScaleFactor, const std::vector<double>& sortedOptions );
    static double previousScaleFactor( double currentScaleFactor, const std::vector<double>& sortedOptions );
};
