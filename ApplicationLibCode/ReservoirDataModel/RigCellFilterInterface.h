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

#include "RigEclipseResultAddress.h"

#include "cvfArray.h"
#include "cvfObject.h"

#include <cstddef>
#include <optional>

class RigGridBase;
class RimEclipseCase;

//==================================================================================================
/// Interface for consumers of cell filters. Consumers should only depend on this interface, not on
/// the concrete filter classes.
//==================================================================================================
class RigCellFilterInterface
{
public:
    virtual ~RigCellFilterInterface() = default;

    // True if the filter currently filters cells. A filter that is checked in the project tree, but
    // only shows an outline in the view, is not active.
    virtual bool isActive() const = 0;

    // Take an incoming per-cell visibility mask and hide the cells that this filter rejects
    // (respecting its own INCLUDE/EXCLUDE mode). When sourceCaseOverride is set, property filters evaluate
    // against that case's results instead of the case the filter is bound to.
    virtual void updateCellVisibility( cvf::UByteArray*   cellVisibility,
                                       const RigGridBase* grid,
                                       size_t             timeStepIndex,
                                       RimEclipseCase*    sourceCaseOverride = nullptr ) = 0;

    // Evaluate the filter against the given case for all grids, and return the visibility indexed by
    // reservoir cell index.
    virtual cvf::ref<cvf::UByteArray> computeReservoirCellVisibility( RimEclipseCase* eclipseCase, size_t timeStepIndex ) = 0;

    // True if the visible cells can change from one time step to the next
    virtual bool isDynamic() const = 0;

    // The dynamic result address the filter is based on, if any
    virtual std::optional<RigEclipseResultAddress> dynamicResultAddress() const = 0;
};
