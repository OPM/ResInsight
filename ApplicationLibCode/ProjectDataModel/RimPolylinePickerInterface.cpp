/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024  Equinor ASA
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

#include "RimPolylinePickerInterface.h"

#include "RimPolylineTarget.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPolylinePickerInterface::scalingFactorForTarget() const
{
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimPolylineTarget*, RimPolylineTarget*>
    RimPolylinePickerInterface::findActiveTargetsAroundInsertionPoint( const RimPolylineTarget* targetToInsertBefore )
{
    auto targets = activeTargets();

    RimPolylineTarget* before = nullptr;
    RimPolylineTarget* after  = nullptr;

    bool foundTarget = false;
    for ( const auto& wt : targets )
    {
        if ( wt == targetToInsertBefore )
        {
            foundTarget = true;
        }

        if ( !after && foundTarget ) after = wt;

        if ( !foundTarget ) before = wt;
    }

    return { before, after };
}
