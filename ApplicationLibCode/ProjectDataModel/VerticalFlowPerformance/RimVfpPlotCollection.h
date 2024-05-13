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
#include "RimVfpPlot.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimVfpDeck;

//==================================================================================================
///
///
//==================================================================================================
class RimVfpPlotCollection : public caf::PdmObject, public RimTypedPlotCollection<RimVfpPlot>
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpPlotCollection();

    RimVfpPlot* createAndAppendPlots( RimVfpTableData* tableData );
    RimVfpPlot* plotForTableNumber( int tableNumber ) const;

    void                     addPlot( RimVfpPlot* newPlot ) override;
    std::vector<RimVfpPlot*> plots() const override;

    size_t plotCount() const final;
    void   insertPlot( RimVfpPlot* vfpPlot, size_t index ) final;
    void   removePlot( RimVfpPlot* vfpPlot ) final;
    void   deleteAllPlots() override;

    static void addImportItems( caf::CmdFeatureMenuBuilder& menuBuilder );

private:
    void loadDataAndUpdateAllPlots() override;
    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void addDeck( RimVfpDeck* deck );

private:
    caf::PdmChildArrayField<RimVfpPlot*> m_vfpPlots;
    caf::PdmChildArrayField<RimVfpDeck*> m_vfpDecks;
};
