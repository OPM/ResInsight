/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "cvfArray.h"
#include "cvfObject.h"

#include <optional>

class RigCellFilterInterface;
class RigEclipseResultAddress;
class RimEclipseCase;

//==================================================================================================
/// Null-safe helpers working on the cell filter interface
//==================================================================================================
class RimCellFilterTools
{
public:
    static cvf::ref<cvf::UByteArray>
        computeReservoirCellVisibility( RigCellFilterInterface* filter, RimEclipseCase* eclipseCase, size_t timeStepIndex );

    // True if the filter's visible cells can change from one time step to the next, i.e. it is (or
    // contains) a property filter on a dynamic (time-varying) result such as SOIL or SWAT.
    static bool isDynamicFilter( const RigCellFilterInterface* filter );

    // The dynamic result address the filter (or one of its descendants) is based on, if any. Used to make sure
    // that result is loaded before its time step count is queried.
    static std::optional<RigEclipseResultAddress> dynamicResultAddress( const RigCellFilterInterface* filter );
};
