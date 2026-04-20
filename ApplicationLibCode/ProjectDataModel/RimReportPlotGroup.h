/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

class RimPlot;

//==================================================================================================
/// Capability mixin: a composite plot window that exposes a set of child RimPlot
/// instances to be exported as individual text dialogs by the "Show Plot Data" feature.
//==================================================================================================
class RimReportPlotGroup
{
public:
    virtual ~RimReportPlotGroup() = default;

    virtual std::vector<RimPlot*> childPlotsForTextExport() const = 0;
};
