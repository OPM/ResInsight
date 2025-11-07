/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimFishbonesDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void AppEnum<RimFishbonesDefines::LocationType_OBSOLETE>::setUp()
{
    addItem( RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_COUNT_END, "FB_SUB_COUNT", "Start/End/Number of Subs" );
    addItem( RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_SPACING_END, "FB_SUB_SPACING", "Start/End/Spacing" );
    addItem( RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_USER_DEFINED, "FB_SUB_CUSTOM", "User Specification" );
    setDefault( RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_COUNT_END );
}

template <>
void AppEnum<RimFishbonesDefines::LateralsOrientationType>::setUp()
{
    addItem( RimFishbonesDefines::LateralsOrientationType::FIXED, "FIXED", "Fixed Angle", { "FB_LATERAL_ORIENTATION_FIXED" } );
    addItem( RimFishbonesDefines::LateralsOrientationType::RANDOM, "RANDOM", "Random Angle", { "FB_LATERAL_ORIENTATION_RANDOM" } );
    setDefault( RimFishbonesDefines::LateralsOrientationType::RANDOM );
}

template <>
void AppEnum<RimFishbonesDefines::DrillingType>::setUp()
{
    addItem( RimFishbonesDefines::DrillingType::STANDARD, "STANDARD", "Standard Drilling" );
    addItem( RimFishbonesDefines::DrillingType::EXTENDED, "EXTENDED", "Extended Drilling" );
    addItem( RimFishbonesDefines::DrillingType::ACID_JETTING, "ACID_JETTING", "Acid Jetting" );
    setDefault( RimFishbonesDefines::DrillingType::STANDARD );
}

template <>
void caf::AppEnum<RimFishbonesDefines::ValueSource>::setUp()
{
    addItem( RimFishbonesDefines::ValueSource::AUTOMATIC, "AUTOMATIC", "Automatic" );
    addItem( RimFishbonesDefines::ValueSource::FIXED, "FIXED", "Fixed" );

    setDefault( RimFishbonesDefines::ValueSource::AUTOMATIC );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesDefines::RicFishbonesSystemParameters RimFishbonesDefines::drillingStandardParameters()
{
    return { .lateralsPerSub = 3, .lateralLength = 11.0, .holeDiameter = 12.5, .buildAngle = 6.0, .icdsPerSub = 3, .spacing = 12.5 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesDefines::RicFishbonesSystemParameters RimFishbonesDefines::drillingExtendedParameters()
{
    return { .lateralsPerSub = 3, .lateralLength = 18.0, .holeDiameter = 12.5, .buildAngle = 4.0, .icdsPerSub = 3, .spacing = 18.5 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesDefines::RicFishbonesSystemParameters RimFishbonesDefines::acidJettingParameters()
{
    return { .lateralsPerSub = 4, .lateralLength = 12.0, .holeDiameter = 15.0, .buildAngle = 6.0, .icdsPerSub = 4, .spacing = 12.5 };
}
