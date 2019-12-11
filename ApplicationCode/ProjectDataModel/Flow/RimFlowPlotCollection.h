/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"

class RimWellAllocationPlot;
class RimFlowCharacteristicsPlot;
class RimWellDistributionPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimFlowPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFlowPlotCollection();
    ~RimFlowPlotCollection() override;

    void   closeDefaultPlotWindowAndDeletePlots();
    void   loadDataAndUpdate();
    size_t plotCount() const;

    void                        addWellAllocPlotToStoredPlots( RimWellAllocationPlot* plot );
    void                        addFlowCharacteristicsPlotToStoredPlots( RimFlowCharacteristicsPlot* plot );
    RimWellAllocationPlot*      defaultWellAllocPlot();
    RimFlowCharacteristicsPlot* defaultFlowCharacteristicsPlot();
    void                        ensureDefaultFlowPlotsAreCreated();

private:
    caf::PdmChildField<RimFlowCharacteristicsPlot*>      m_flowCharacteristicsPlot;
    caf::PdmChildField<RimWellAllocationPlot*>           m_defaultWellAllocPlot;
    caf::PdmChildField<RimWellDistributionPlot*>         m_wellDistributionPlot;
    caf::PdmChildArrayField<RimWellAllocationPlot*>      m_storedWellAllocPlots;
    caf::PdmChildArrayField<RimFlowCharacteristicsPlot*> m_storedFlowCharacteristicsPlots;
};
