/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmUiItem.h"

#include <QList>

class RimSummaryMultiPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryMultiPlotCollection : public caf::PdmObject, public RimPlotCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryMultiPlotCollection();
    ~RimSummaryMultiPlotCollection() override;

    void   deleteAllPlots() override;
    void   loadDataAndUpdateAllPlots() override;
    size_t plotCount() const override;

    std::vector<RimSummaryMultiPlot*> multiPlots() const;

    void addSummaryMultiPlot( RimSummaryMultiPlot* plot );
    void summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const;
    void duplicatePlot( RimSummaryMultiPlot* plotToDuplicate );
    void removePlotNoUpdate( RimSummaryMultiPlot* plotToRemove );

    void updateSummaryNameHasChanged();

private:
    void initAfterRead() override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;

    void onDuplicatePlot( const caf::SignalEmitter* emitter, RimSummaryMultiPlot* plotToDuplicate );
    void onRefreshTree( const caf::SignalEmitter* emitter, RimSummaryMultiPlot* plotRequesting );

private:
    caf::PdmChildArrayField<RimSummaryMultiPlot*> m_summaryMultiPlots;
};
