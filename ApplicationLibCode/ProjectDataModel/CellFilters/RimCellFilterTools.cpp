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

#include "RimCellFilterTools.h"

#include "RigCellFilterInterface.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray>
    RimCellFilterTools::computeReservoirCellVisibility( RigCellFilterInterface* filter, RimEclipseCase* eclipseCase, size_t timeStepIndex )
{
    if ( !filter ) return nullptr;

    return filter->computeReservoirCellVisibility( eclipseCase, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterTools::isDynamicFilter( const RigCellFilterInterface* filter )
{
    return filter && filter->isDynamic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigEclipseResultAddress> RimCellFilterTools::dynamicResultAddress( const RigCellFilterInterface* filter )
{
    if ( !filter ) return std::nullopt;

    return filter->dynamicResultAddress();
}
