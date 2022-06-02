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

public:
    enum class AxisRangeAggregation
    {
        NONE,
        SUB_PLOTS,
        REGIONS,
        WELLS,
        REALIZATIONS
    };

    RimSummaryMultiPlot();
    ~RimSummaryMultiPlot() override;

    const RimSummaryNameHelper* nameHelper() const;

    void setLayoutInfo( RimSummaryPlot* summaryPlot, int row, int col );
    void clearLayoutInfo();

    void setAutoTitlePlot( bool enable );
    void setAutoTitleGraphs( bool enable );

    std::vector<RimSummaryDataSourceStepping::Axis> availableAxes() const override;
    std::vector<RimSummaryCurve*>     curvesForStepping( RimSummaryDataSourceStepping::Axis axis ) const override;
    std::vector<RimEnsembleCurveSet*> curveSets() const override;
    std::vector<RimSummaryCurve*>     allCurves( RimSummaryDataSourceStepping::Axis axis ) const override;

    void addPlot( RimPlot* plot ) override;
    void insertPlot( RimPlot* plot, size_t index ) override;
    void removePlot( RimPlot* plot ) override;

    void removePlotNoUpdate( RimPlot* plot ) override;
    void updateAfterPlotRemove() override;
    void updatePlotWindowTitle() override;

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar() override;

    void syncAxisRanges();

    void handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects );

    void summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const;

    std::vector<RimSummaryPlot*> summaryPlots() const;
    std::vector<RimSummaryPlot*> visibleSummaryPlots() const;

    void makeSureIsVisible( RimSummaryPlot* plot );

    void setSubPlotAxesLinked( bool enable );
    bool isSubPlotAxesLinked() const;

    std::pair<int, int> gridLayoutInfoForSubPlot( RimSummaryPlot* summaryPlot ) const;

    void zoomAll() override;

    void setDefaultRangeAggregationSteppingDimension();
    void checkAndApplyAutoAppearance();

    void keepVisiblePageAfterUpdate( bool keepPage );

protected:
    bool handleGlobalKeyEvent( QKeyEvent* keyEvent ) override;
    bool handleGlobalWheelEvent( QWheelEvent* wheelEvent ) override;

    void initAfterRead() override;
    void onLoadDataAndUpdate() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void populateNameHelper( RimSummaryPlotNameHelper* nameHelper );

    void computeAggregatedAxisRange();
    void updateSourceStepper();

    void updatePlotVisibility();

    void duplicate();

    void appendSubPlotByStepping( int direction );
    void appendCurveByStepping( int direction );

    void analyzePlotsAndAdjustAppearanceSettings();

    void onSubPlotChanged( const caf::SignalEmitter* emitter );
    void onSubPlotAxisChanged( const caf::SignalEmitter* emitter, RimSummaryPlot* summaryPlot );

private:
    caf::PdmField<bool> m_autoPlotTitles;
    caf::PdmField<bool> m_autoPlotTitlesOnSubPlots;
    caf::PdmField<bool> m_disableWheelZoom;
    caf::PdmField<bool> m_createPlotDuplicate;
    caf::PdmField<bool> m_linkSubPlotAxes;
    caf::PdmField<bool> m_autoAdjustAppearance;

    caf::PdmField<bool>   m_hidePlotsWithValuesBelow;
    caf::PdmField<double> m_plotFilterYAxisThreshold;

    caf::PdmField<bool> m_appendNextPlot;
    caf::PdmField<bool> m_appendPrevPlot;

    caf::PdmField<bool> m_appendNextCurve;
    caf::PdmField<bool> m_appendPrevCurve;

    caf::PdmField<caf::AppEnum<AxisRangeAggregation>> m_axisRangeAggregation;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;

    std::unique_ptr<RimSummaryPlotNameHelper> m_nameHelper;

    std::map<RimSummaryPlot*, std::pair<int, int>> m_gridLayoutInfo;
};
