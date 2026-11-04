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

#include "RimAbstractPlotCollection.h"

#include "cafPdmObjectCollection.h"

#include <vector>

class RimGroupNetworkData;
class RimGroupNetworkPlot;

//==================================================================================================
/// The production network plots, shown in the plot tree
//==================================================================================================
class RimGroupNetworkPlotCollection : public caf::PdmObjectCollection<RimGroupNetworkPlot>, public RimPlotCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimGroupNetworkPlotCollection();

    RimGroupNetworkPlot*              createPlot( RimGroupNetworkData* dataSource );
    std::vector<RimGroupNetworkPlot*> plots() const;

    size_t plotCount() const override;
    void   deleteAllPlots() override;
    void   loadDataAndUpdateAllPlots() override;

private:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
};
