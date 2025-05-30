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

#include "RimContourMapResolutionTools.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RimContourMapResolutionTools::SamplingResolution>::setUp()
{
    addItem( RimContourMapResolutionTools::SamplingResolution::EXTRA_FINE, "Extra Fine", "Extra Fine" );
    addItem( RimContourMapResolutionTools::SamplingResolution::FINE, "Fine", "Fine" );
    addItem( RimContourMapResolutionTools::SamplingResolution::NORMAL, "Normal", "Normal" );
    addItem( RimContourMapResolutionTools::SamplingResolution::COARSE, "Coarse", "Coarse" );
    addItem( RimContourMapResolutionTools::SamplingResolution::EXTRA_COARSE, "Extra Coarse", "Extra Coarse" );
    setDefault( RimContourMapResolutionTools::SamplingResolution::NORMAL );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapResolutionTools::resolutionFromEnumValue( SamplingResolution resEnumVal )
{
    switch ( resEnumVal )
    {
        case RimContourMapResolutionTools::SamplingResolution::EXTRA_FINE:
            return 0.5;
        case RimContourMapResolutionTools::SamplingResolution::FINE:
            return 0.7;
        case RimContourMapResolutionTools::SamplingResolution::NORMAL:
        default:
            return 0.9;
        case RimContourMapResolutionTools::SamplingResolution::COARSE:
            return 2.0;
        case RimContourMapResolutionTools::SamplingResolution::EXTRA_COARSE:
            return 5.0;
    }
}
