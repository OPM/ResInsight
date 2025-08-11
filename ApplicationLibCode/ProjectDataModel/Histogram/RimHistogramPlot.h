/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RiaDateTimeDefines.h"
#include "RiaPlotDefines.h"

#include "RimPlot.h"
#include "RimPlotAxisProperties.h"

#include "RiuPlotWidget.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObjectHandle.h"

#include <QPointer>

#include <memory>
#include <vector>

class RimHistogramCurve;
class RimHistogramCurveCollection;
class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RimPlotTemplateFileItem;
class RimStackablePlotCurve;

class PdmUiTreeOrdering;

class QwtInterval;
class QwtPlotCurve;
class QwtPlotTextLabel;

class QKeyEvent;

//==================================================================================================
///
///
//==================================================================================================
class RimHistogramPlot : public RimPlot
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<>                  curvesChanged;
    caf::Signal<RimHistogramPlot*> axisChanged;
    caf::Signal<>                  titleChanged;
    caf::Signal<RimHistogramPlot*> axisChangedReloadRequired;
    caf::Signal<bool>              autoTitleChanged;

public:
    enum class FrequencyType
    {
        ABSOLUTE_FREQUENCY,
        RELATIVE_FREQUENCY,
        RELATIVE_FREQUENCY_PERCENT
    };

    enum class GraphType
    {
        BAR_GRAPH,
        LINE_GRAPH
    };

    RimHistogramPlot();
    ~RimHistogramPlot() override;

    void    setDescription( const QString& description );
    QString description() const override;

    void enableAutoPlotTitle( bool enable );
    bool autoPlotTitle() const;

    void addCurveNoUpdate( RimHistogramCurve* curve, bool autoAssignPlotAxis = true );

    size_t curveCount() const;

    void detachAllCurves() override;
    void reattachAllCurves() override;

    void updateAxes() override;

    bool isLogarithmicScaleEnabled( RiuPlotAxis plotAxis ) const;

    bool isCurveHighlightSupported() const override;

    QWidget* viewWidget() override;

    QString asciiDataForPlotExport() const override;
    QString asciiDataForHistogramPlotExport( RiaDefines::DateTimePeriod resamplingPeriod, bool showTimeAsLongString ) const;

    FrequencyType frequencyType() const;
    GraphType     graphType() const;

    void updatePlotTitle();

    void updateCurveNames();

    void copyMatchingAxisPropertiesFromOther( const RimHistogramPlot& sourceHistogramPlot );

    void updateAll();
    void updateLegend() override;
    void setLegendPosition( RiuPlotWidget::Legend position );

    void showPlotInfoLabel( bool show );
    void updatePlotInfoLabel();

    void applyDefaultCurveAppearances();

    void setNormalizationEnabled( bool enable );
    bool isNormalizationEnabled();

    void           setAutoScaleXEnabled( bool enabled ) override;
    void           setAutoScaleYEnabled( bool enabled ) override;
    RiuPlotWidget* plotWidget() override;
    void           zoomAll() override;
    void           updatePlotWidgetFromAxisRanges() override;
    void           updateAxisRangesFromPlotWidget() override;

    void onAxisSelected( RiuPlotAxis axis, bool toggle ) override;

    std::vector<RimPlotAxisProperties*> plotAxes( RimPlotAxisProperties::Orientation orientation ) const;

    RimPlotAxisPropertiesInterface* axisPropertiesForPlotAxis( RiuPlotAxis plotAxis ) const;
    RimPlotAxisProperties*          addNewAxisProperties( RiaDefines::PlotAxis, const QString& name );
    RimPlotAxisProperties*          addNewAxisProperties( RiuPlotAxis plotAxis, const QString& name );

    std::vector<RimPlotCurve*> visibleCurvesForLegend() override;

    void scheduleReplotIfVisible();

public:
    // RimViewWindow overrides
    void deleteViewWidget() override;
    bool isDeletable() const override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void zoomAllForMultiPlot() override;

private:
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void doUpdateLayout() override;

    void detachAllPlotItems();
    void deleteAllPlotCurves();

    void onCurveCollectionChanged( const SignalEmitter* emitter );
    void onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex ) override;

    void connectCurveToPlot( RimHistogramCurve* curve, bool update, bool autoAssignPlotAxis );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;

    QImage snapshotWindowContent() override;

private slots:
    void onPlotZoomed();
    void onUpdateCurveOrder();

private:
    void updateNumericalAxis( RiaDefines::PlotAxis plotAxis );
    void updateZoomForAxis( RimPlotAxisPropertiesInterface* axisProperties );
    void updateZoomForNumericalAxis( RimPlotAxisProperties* axisProperties );

    void deletePlotCurvesAndPlotWidget();

    void connectCurveSignals( RimStackablePlotCurve* curve );
    void disconnectCurveSignals( RimStackablePlotCurve* curve );

    void curveDataChanged( const caf::SignalEmitter* emitter );
    void curveVisibilityChanged( const caf::SignalEmitter* emitter, bool visible );
    void curveAppearanceChanged( const caf::SignalEmitter* emitter );
    void curveStackingChanged( const caf::SignalEmitter* emitter, bool stacked );
    void curveStackingColorsChanged( const caf::SignalEmitter* emitter, bool stackWithPhaseColors );

    void connectAxisSignals( RimPlotAxisProperties* axis );
    void axisSettingsChanged( const caf::SignalEmitter* emitter );
    void axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic );
    void axisPositionChanged( const caf::SignalEmitter* emitter,
                              RimPlotAxisProperties*    axisProperties,
                              RiuPlotAxis               oldPlotAxis,
                              RiuPlotAxis               newPlotAxis );

    std::vector<RimPlotAxisPropertiesInterface*> allPlotAxes() const;

    std::vector<RimHistogramCurve*> histogramCurves() const;
    std::vector<RimHistogramCurve*> visibleHistogramCurvesForAxis( RiuPlotAxis plotAxis ) const;

    void assignPlotAxis( RimHistogramCurve* curve );

    void updateStackedCurveData();

private:
    caf::PdmField<bool>    m_normalizeCurveYValues;
    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;
    caf::PdmField<QString> m_fallbackPlotName;

    caf::PdmChildField<RimHistogramCurveCollection*> m_histogramCurveCollection;

    caf::PdmChildArrayField<RimPlotAxisPropertiesInterface*> m_axisPropertiesArray;

    caf::PdmField<caf::AppEnum<FrequencyType>> m_histogramFrequencyType;
    caf::PdmField<caf::AppEnum<GraphType>>     m_graphType;

    QPointer<RiuPlotWidget> m_histogramPlot;

    bool                  m_isValid;
    RiuPlotWidget::Legend m_legendPosition;
};
