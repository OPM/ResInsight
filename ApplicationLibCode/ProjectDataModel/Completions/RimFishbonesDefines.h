/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

namespace RimFishbonesDefines
{
enum class LocationType_OBSOLETE
{
    FB_SUB_COUNT_END,
    FB_SUB_SPACING_END,
    FB_SUB_USER_DEFINED,
    FB_SUB_UNDEFINED
};

enum class LateralsOrientationType
{
    FIXED,
    RANDOM
};

enum class DrillingType
{
    STANDARD,
    EXTENDED,
    ACID_JETTING
};

enum class ValueSource
{
    AUTOMATIC,
    FIXED
};

struct RicFishbonesSystemParameters
{
    int    lateralsPerSub;
    double lateralLength;
    double holeDiameter;
    double buildAngle;
    int    icdsPerSub;
    double spacing;
};

RicFishbonesSystemParameters drillingStandardParameters();
RicFishbonesSystemParameters drillingExtendedParameters();
RicFishbonesSystemParameters acidJettingParameters();

} // namespace RimFishbonesDefines
