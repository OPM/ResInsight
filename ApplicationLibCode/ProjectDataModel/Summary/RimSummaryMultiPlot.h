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
#include "cafSignal.h"

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

protected:
    bool handleGlobalKeyEvent( QKeyEvent* keyEvent ) override;
    bool handleGlobalWheelEvent( QWheelEvent* wheelEvent ) override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void populateNameHelper( RimSummaryPlotNameHelper* nameHelper );
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    std::vector<RimSummaryPlot*> summaryPlots() const;

    static void insertGraphsIntoPlot( RimSummaryMultiPlot* plot, const std::vector<RimSummaryPlot*>& graphs );

    void updatePlotWindowTitle() override;

    void onCreateNewPlot( const SignalEmitter* emitter, const std::vector<caf::PdmObjectHandle*>& objects );

private:
    caf::PdmField<bool> m_autoPlotTitles;
    caf::PdmField<bool> m_autoPlotTitlesOnSubPlots;
    caf::PdmField<bool> m_syncAxisRanges;
    caf::PdmField<bool> m_disableWheelZoom;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;

    std::unique_ptr<RimSummaryPlotNameHelper> m_nameHelper;
};
