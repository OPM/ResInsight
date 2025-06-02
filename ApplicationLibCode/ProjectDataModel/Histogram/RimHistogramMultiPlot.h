/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

// #include "RimHistogramDataSourceStepping.h"
#include "RimMultiPlot.h"

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmUiItem.h"
#include "cafSignal.h"

#include <QList>

#include <vector>

class RimHistogramPlot;
// class RimHistogramPlotSourceStepping;
// class RimHistogramPlotNameHelper;
// class RimHistogramNameHelper;
class RimPlotAxisProperties;
// class RimHistogramPlotReadOut;

//==================================================================================================
///
///
//==================================================================================================
class RimHistogramMultiPlot : public RimMultiPlot
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<RimHistogramMultiPlot*> duplicatePlot;

public:
    enum class AxisRangeAggregation
    {
        NONE,
        SUB_PLOTS,
        // REGIONS,
        // WELLS,
        REALIZATIONS
    };

    RimHistogramMultiPlot();
    ~RimHistogramMultiPlot() override;

    // const RimHistogramNameHelper* nameHelper() const;

    void setLayoutInfo( RimHistogramPlot* histogramPlot, int row, int col );
    void clearLayoutInfo();

    void setAutoPlotTitle( bool enable );
    void setAutoSubPlotTitle( bool enable );

    // std::vector<RimHistogramCurve*>     curvesForStepping() const override;
    // std::vector<RimEnsembleCurveSet*> curveSets() const override;
    // std::vector<RimHistogramCurve*>     allCurves() const override;

    void addPlot( RimPlot* plot ) override;
    void insertPlot( RimPlot* plot, size_t index ) override;
    void removePlot( RimPlot* plot ) override;

    void removePlotNoUpdate( RimPlot* plot ) override;
    void updateAfterPlotRemove() override;
    void updatePlotTitles() override;

    // std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar() override;

    void syncAxisRanges();
    // void syncTimeAxisRanges( RimHistogramPlot* sourceHistogramPlot );

    // void handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects );

    void histogramPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const;

    std::vector<RimHistogramPlot*> histogramPlots() const;
    std::vector<RimHistogramPlot*> visibleHistogramPlots() const;

    void makeSureIsVisible( RimHistogramPlot* plot );

    void setSubPlotAxesLinked( bool enable );
    bool isSubPlotAxesLinked() const;
    // void setTimeAxisLinked( bool enable );
    // bool isTimeAxisLinked() const;

    std::pair<int, int> gridLayoutInfoForSubPlot( RimHistogramPlot* histogramPlot ) const;

    void zoomAll() override;

    void setDefaultRangeAggregationSteppingDimension();
    void analyzePlotsAndAdjustAppearanceSettings();

    void keepVisiblePageAfterUpdate( bool keepPage );

    // void storeStepDimensionFromToolbar();
    // void updateStepDimensionFromDefault();

    // void selectWell( QString wellName );

    // void updateReadOutLines( double qwtTimeValue, double yValue );

protected:
    bool handleGlobalKeyEvent( QKeyEvent* keyEvent ) override;
    bool handleGlobalWheelEvent( QWheelEvent* wheelEvent ) override;

    void initAfterRead() override;
    void onLoadDataAndUpdate() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;

    void onPlotAdditionOrRemoval() override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    // void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

    // void populateNameHelper( RimHistogramPlotNameHelper* nameHelper );

    void computeAggregatedAxisRange();
    void updateSourceStepper();

    void        updatePlotVisibility();
    void        setAutoValueStates();
    static void setAutoValueStatesForPlot( RimHistogramPlot* histogramPlot, bool isMinMaxOverridden, bool isAppearanceOverridden );

    void duplicate();

    void appendSubPlotByStepping( int direction );
    void appendCurveByStepping( int direction );

    void onSubPlotChanged( const caf::SignalEmitter* emitter );
    // void onSubPlotAxisChanged( const caf::SignalEmitter* emitter, RimHistogramPlot* histogramPlot );
    // void onSubPlotAxisReloadRequired( const caf::SignalEmitter* emitter, RimHistogramPlot* histogramPlot );
    void onSubPlotAutoTitleChanged( const caf::SignalEmitter* emitter, bool isEnabled );

    // void updateTimeAxisRangesFromFirstTimePlot();

    void updateReadOnlyState();

    void updateReadOutSettings();

    std::pair<double, double> adjustedMinMax( const RimPlotAxisProperties* axis, double min, double max ) const;

private:
    caf::PdmField<bool> m_autoPlotTitle;
    caf::PdmField<bool> m_autoSubPlotTitle;
    caf::PdmField<bool> m_disableWheelZoom;
    caf::PdmField<bool> m_createPlotDuplicate;
    caf::PdmField<bool> m_linkSubPlotAxes;
    caf::PdmField<bool> m_autoAdjustAppearance;
    caf::PdmField<bool> m_allow3DSelectionLink;

    // caf::PdmChildField<RimHistogramPlotReadOut*> m_readOutSettings;

    caf::PdmField<bool>   m_hidePlotsWithValuesBelow;
    caf::PdmField<double> m_plotFilterYAxisThreshold;

    caf::PdmField<bool> m_appendNextPlot;
    caf::PdmField<bool> m_appendPrevPlot;

    caf::PdmField<bool> m_appendNextCurve;
    caf::PdmField<bool> m_appendPrevCurve;

    // caf::PdmField<caf::AppEnum<RimHistogramDataSourceStepping::SourceSteppingDimension>> m_defaultStepDimension;

    caf::PdmField<caf::AppEnum<AxisRangeAggregation>> m_axisRangeAggregation;

    // caf::PdmChildField<RimHistogramPlotSourceStepping*> m_sourceStepping;

    // std::unique_ptr<RimHistogramPlotNameHelper> m_nameHelper;

    std::map<RimHistogramPlot*, std::pair<int, int>> m_gridLayoutInfo;
};
