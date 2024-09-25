/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
#include "RimVfpPlot_deprecated.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimCustomVfpPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimVfpPlotCollection : public caf::PdmObject, public RimTypedPlotCollection<RimVfpPlot_deprecated>
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpPlotCollection();

    RimCustomVfpPlot* createAndAppendPlots( RimVfpTable* mainDataSource, std::vector<RimVfpTable*> tableData );

    static void addImportItems( caf::CmdFeatureMenuBuilder& menuBuilder );

private:
    void                                addPlot( RimVfpPlot_deprecated* newPlot ) override;
    std::vector<RimVfpPlot_deprecated*> plots() const override;

    size_t plotCount() const final;
    void   insertPlot( RimVfpPlot_deprecated* vfpPlot, size_t index ) final;
    void   removePlot( RimVfpPlot_deprecated* vfpPlot ) final;
    void   deleteAllPlots() override;

    void loadDataAndUpdateAllPlots() override;
    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    caf::PdmChildArrayField<RimCustomVfpPlot*> m_customVfpPlots;

    // To be removed in a future release.
    // RimVfpPlotCollection can derive from RimTypedPlotCollection<RimCustomVfpPlot>
    // Consider renaming RimCustomVfpPlot to RimVfpPlot
    caf::PdmChildArrayField<RimVfpPlot_deprecated*> m_vfpPlots_deprecated;
};
