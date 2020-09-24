/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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
#include "RimSummaryPlot.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

class RimSummaryPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryPlotCollection : public caf::PdmObject, public RimTypedPlotCollection<RimSummaryPlot>
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlotCollection();
    ~RimSummaryPlotCollection() override;

    RimSummaryPlot* createSummaryPlotWithAutoTitle();
    RimSummaryPlot* createNamedSummaryPlot( const QString& name );

    void updateSummaryNameHasChanged();
    void summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const;
    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    std::vector<RimSummaryPlot*> plots() const final;
    size_t                       plotCount() const final;
    void                         insertPlot( RimSummaryPlot* summaryPlot, size_t index ) final;
    void                         removePlot( RimSummaryPlot* summaryPlot ) final;

private:
    caf::PdmChildArrayField<RimSummaryPlot*> m_summaryPlots;
};
