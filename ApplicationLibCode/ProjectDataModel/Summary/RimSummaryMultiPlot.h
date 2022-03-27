/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimMultiPlot.h"
#include "RimSummaryDataSourceStepping.h"

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmUiItem.h"
#include "cafSignal.h"

#include <QList>

#include <vector>

class RimSummaryPlot;
class RimSummaryPlotSourceStepping;
class RimSummaryPlotNameHelper;
class RimSummaryNameHelper;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryMultiPlot : public RimMultiPlot, public RimSummaryDataSourceStepping
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<RimSummaryMultiPlot*> duplicatePlot;
    caf::Signal<RimSummaryMultiPlot*> refreshTree;

public:
    RimSummaryMultiPlot();
    ~RimSummaryMultiPlot() override;

    const RimSummaryNameHelper* nameHelper() const;

    void setAutoTitlePlot( bool enable );
    void setAutoTitleGraphs( bool enable );

    std::vector<RimSummaryDataSourceStepping::Axis> availableAxes() const override;
    std::vector<RimSummaryCurve*>     curvesForStepping( RimSummaryDataSourceStepping::Axis axis ) const override;
    std::vector<RimEnsembleCurveSet*> curveSets() const override;
    std::vector<RimSummaryCurve*>     allCurves( RimSummaryDataSourceStepping::Axis axis ) const override;

    void addPlot( RimPlot* plot ) override;
    void insertPlot( RimPlot* plot, size_t index ) override;
    void removePlot( RimPlot* plot ) override;

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

    void syncAxisRanges();

    void addPlot( const std::vector<caf::PdmObjectHandle*>& objects );

    void summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const;

    std::vector<RimSummaryPlot*> summaryPlots() const;

protected:
    bool handleGlobalKeyEvent( QKeyEvent* keyEvent ) override;
    bool handleGlobalWheelEvent( QWheelEvent* wheelEvent ) override;

    void initAfterRead() override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void populateNameHelper( RimSummaryPlotNameHelper* nameHelper );
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void updatePlotWindowTitle() override;

    void duplicate();
    void signalRefresh();

    void onSubPlotChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<bool> m_autoPlotTitles;
    caf::PdmField<bool> m_autoPlotTitlesOnSubPlots;
    caf::PdmField<bool> m_syncAxisRanges;
    caf::PdmField<bool> m_disableWheelZoom;
    caf::PdmField<bool> m_createPlotDuplicate;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;

    std::unique_ptr<RimSummaryPlotNameHelper> m_nameHelper;
};
