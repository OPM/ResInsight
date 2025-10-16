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
#include "cafPdmUiItem.h"
#include "cafSignal.h"

#include <QList>

#include <vector>

class RimSummaryPlot;
class RimSummaryPlotSourceStepping;
class RimSummaryPlotNameHelper;
class RimSummaryNameHelper;
class RimPlotAxisProperties;
class RimSummaryPlotReadOut;

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

    void setAutoPlotTitle( bool enable );
    void setAutoSubPlotTitle( bool enable );

    std::vector<RimSummaryCurve*>     curvesForStepping() const override;
    std::vector<RimEnsembleCurveSet*> curveSets() const override;
    std::vector<RimSummaryCurve*>     allCurves() const override;

    void addPlot( RimPlot* plot ) override;
    void insertPlot( RimPlot* plot, size_t index ) override;
    void removePlot( RimPlot* plot ) override;

    void removePlotNoUpdate( RimPlot* plot ) override;
    void updateAfterPlotRemove() override;
    void updatePlotTitles() override;

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar() override;

    void syncAxisRanges();
    void syncTimeAxisRanges( RimSummaryPlot* sourceSummaryPlot );

    void handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects );

    void summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const;

    std::vector<RimSummaryPlot*> summaryPlots() const;
    std::vector<RimSummaryPlot*> visibleSummaryPlots() const;

    void makeSureIsVisible( RimSummaryPlot* plot );

    void setSubPlotAxesLinked( bool enable );
    bool isSubPlotAxesLinked() const;
    void setTimeAxisLinked( bool enable );
    bool isTimeAxisLinked() const;

    std::pair<int, int> gridLayoutInfoForSubPlot( RimSummaryPlot* summaryPlot ) const;

    void zoomAll() override;

    void setDefaultRangeAggregationSteppingDimension();
    void analyzePlotsAndAdjustAppearanceSettings();

    void keepVisiblePageAfterUpdate( bool keepPage );

    void storeStepDimensionFromToolbar();
    void updateStepDimensionFromDefault();

    void selectWell( QString wellName );

    void updateReadOutSettings();
    void updateReadOutLines( double qwtTimeValue, double yValue );

protected:
    bool handleGlobalKeyEvent( QKeyEvent* keyEvent ) override;
    bool handleGlobalWheelEvent( QWheelEvent* wheelEvent ) override;

    void initAfterRead() override;
    void onLoadDataAndUpdate() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;

    void onPlotAdditionOrRemoval() override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

    void populateNameHelper( RimSummaryPlotNameHelper* nameHelper );

    void computeAggregatedAxisRange();
    void updateSourceStepper();

    void        updatePlotVisibility();
    void        setAutoValueStates();
    static void setAutoValueStatesForPlot( RimSummaryPlot* summaryPlot, bool isMinMaxOverridden, bool isAppearanceOverridden );

    void duplicate();

    void appendSubPlotByStepping( int direction );
    void appendCurveByStepping( int direction );

    void onSubPlotChanged( const caf::SignalEmitter* emitter );
    void onSubPlotAxisChanged( const caf::SignalEmitter* emitter, RimSummaryPlot* summaryPlot );
    void onSubPlotAxisReloadRequired( const caf::SignalEmitter* emitter, RimSummaryPlot* summaryPlot );
    void onSubPlotAutoTitleChanged( const caf::SignalEmitter* emitter, bool isEnabled );

    void updateTimeAxisRangesFromFirstTimePlot();

    void updateReadOnlyState();

    std::pair<double, double> adjustedMinMax( const RimPlotAxisProperties* axis, double min, double max ) const;

    RimSummaryPlotReadOut* activeReadoutSettings() const;

private:
    caf::PdmField<bool> m_autoPlotTitle;
    caf::PdmField<bool> m_autoSubPlotTitle;
    caf::PdmField<bool> m_disableWheelZoom;
    caf::PdmField<bool> m_createPlotDuplicate;
    caf::PdmField<bool> m_linkSubPlotAxes;
    caf::PdmField<bool> m_linkTimeAxis;
    caf::PdmField<bool> m_autoAdjustAppearance;
    caf::PdmField<bool> m_allow3DSelectionLink;

    caf::PdmChildField<RimSummaryPlotReadOut*> m_readOutSettings;
    caf::PdmField<bool>                        m_goToCommonSettings;

    caf::PdmField<bool>   m_hidePlotsWithValuesBelow;
    caf::PdmField<double> m_plotFilterYAxisThreshold;

    caf::PdmField<bool> m_appendNextPlot;
    caf::PdmField<bool> m_appendPrevPlot;

    caf::PdmField<bool> m_appendNextCurve;
    caf::PdmField<bool> m_appendPrevCurve;

    caf::PdmField<caf::AppEnum<RimSummaryDataSourceStepping::SourceSteppingDimension>> m_defaultStepDimension;

    caf::PdmField<caf::AppEnum<AxisRangeAggregation>> m_axisRangeAggregation;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;

    std::unique_ptr<RimSummaryPlotNameHelper> m_nameHelper;

    std::map<RimSummaryPlot*, std::pair<int, int>> m_gridLayoutInfo;
};
