/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RimGridCrossPlot.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

//==================================================================================================
///
///
//==================================================================================================
class RimGridCrossPlotCollection : public caf::PdmObject, public RimTypedPlotCollection<RimGridCrossPlot>
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotCollection();
    ~RimGridCrossPlotCollection() override;

    std::vector<RimGridCrossPlot*> plots() const final;
    size_t                         plotCount() const final;
    void                           insertPlot( RimGridCrossPlot* plot, size_t index ) final;
    void                           removePlot( RimGridCrossPlot* plot ) final;

    RimGridCrossPlot* createGridCrossPlot();

private:
    caf::PdmChildArrayField<RimGridCrossPlot*> m_gridCrossPlots;
};
