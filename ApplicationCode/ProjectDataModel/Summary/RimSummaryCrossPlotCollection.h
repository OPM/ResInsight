/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryCrossPlotCollection : public caf::PdmObject, public RimTypedPlotCollection<RimSummaryPlot>
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCrossPlotCollection();
    ~RimSummaryCrossPlotCollection() override;

    void deleteAllPlots() final;

    std::vector<RimSummaryPlot*> plots() const final;
    size_t                       plotCount() const final;
    void                         insertPlot( RimSummaryPlot* plot, size_t index ) final;
    void                         removePlot( RimSummaryPlot* plot ) final;

    RimSummaryPlot* createSummaryPlot();

    void updateSummaryNameHasChanged();
    void summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const;

private:
    caf::PdmChildArrayField<RimSummaryPlot*> m_summaryCrossPlots;

public:
};
