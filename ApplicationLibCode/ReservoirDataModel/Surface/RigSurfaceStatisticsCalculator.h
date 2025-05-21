/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RigSurface.h"

#include "cvfObject.h"

#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RigSurfaceStatisticsCalculator
{
public:
    enum class StatisticsType
    {
        MIN,
        MAX,
        P10,
        P50,
        P90,
        MEAN
    };

    static cvf::ref<RigSurface> computeStatistics( const std::vector<cvf::ref<RigSurface>>& surfaces );
    static bool                 areSurfacesSameSize( const std::vector<cvf::ref<RigSurface>>& surfaces );
};
