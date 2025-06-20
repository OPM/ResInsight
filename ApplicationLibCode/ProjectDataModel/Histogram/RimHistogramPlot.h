/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Statoil ASA
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
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

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
    // void updateAnnotationsInPlotWidget();

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

    // void copyAxisPropertiesFromOther( const RimHistogramPlot& sourceHistogramPlot );
    // void copyAxisPropertiesFromOther( RiaDefines::PlotAxis plotAxisType, const RimHistogramPlot& sourceHistogramPlot );
    void copyMatchingAxisPropertiesFromOther( const RimHistogramPlot& sourceHistogramPlot );

    void updateAll();
    void updateLegend() override;
    void setLegendPosition( RiuPlotWidget::Legend position );

    // void setPlotInfoLabel( const QString& label );
    void showPlotInfoLabel( bool show );
    void updatePlotInfoLabel();

    // size_t singleColorCurveCount() const;
    void applyDefaultCurveAppearances();
    // void   applyDefaultCurveAppearances( std::vector<RimHistogramCurve*> curvesToUpdate );
    // void applyDefaultCurveAppearances( std::vector<RimEnsembleCurveSet*> ensembleCurvesToUpdate );

    void setNormalizationEnabled( bool enable );
    bool isNormalizationEnabled();

    // RimHistogramPlotSourceStepping* sourceSteppingObjectForKeyEventHandling() const;

    void           setAutoScaleXEnabled( bool enabled ) override;
    void           setAutoScaleYEnabled( bool enabled ) override;
    RiuPlotWidget* plotWidget() override;
    void           zoomAll() override;
    void           updatePlotWidgetFromAxisRanges() override;
    void           updateAxisRangesFromPlotWidget() override;

    void onAxisSelected( RiuPlotAxis axis, bool toggle ) override;

    // std::vector<RimHistogramCurve*>   curvesForStepping() const override;

    std::vector<RimPlotAxisProperties*> plotAxes( RimPlotAxisProperties::Orientation orientation ) const;

    RimPlotAxisPropertiesInterface* axisPropertiesForPlotAxis( RiuPlotAxis plotAxis ) const;
    RimPlotAxisProperties*          addNewAxisProperties( RiaDefines::PlotAxis, const QString& name );
    RimPlotAxisProperties*          addNewAxisProperties( RiuPlotAxis plotAxis, const QString& name );

    std::vector<RimPlotCurve*> visibleCurvesForLegend() override;

    void scheduleReplotIfVisible();

    void enableCurvePointTracking( bool enable );

public:
    // RimViewWindow overrides
    void deleteViewWidget() override;
    bool isDeletable() const override;

    // void handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void zoomAllForMultiPlot() override;

private:
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    // void updateNameHelperWithCurveData( RimHistogramPlotNameHelper* nameHelper ) const;

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

    // void createAndSetCustomTimeAxisTickmarks( RimHistogramTimeAxisProperties* timeAxisProperties );
    // void overrideTimeAxisSettingsIfTooManyCustomTickmarks( RimHistogramTimeAxisProperties* timeAxisProperties, bool showMessageBox );

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

    // void timeAxisSettingsChanged( const caf::SignalEmitter* emitter );
    // void timeAxisSettingsChangedReloadRequired( const caf::SignalEmitter* emitter );

    // void ensureRequiredAxisObjectsForCurves();
    void assignPlotAxis( RimHistogramCurve* curve );
    // void assignYPlotAxis( RimHistogramCurve* curve );
    // void assignXPlotAxis( RimHistogramCurve* curve );

    // RimHistogramCurve* addNewCurve( const RifEclipseHistogramAddress& address,
    //                               RimHistogramCase*                 histogramCase,
    //                               const RifEclipseHistogramAddress& addressX,
    //                               RimHistogramCase*                 histogramCaseX );

    void updateStackedCurveData();
    // bool updateStackedCurveDataForAxis( RiuPlotAxis plotAxis );
    // bool updateStackedCurveDataForRelevantAxes();

    // struct CurveInfo;

    // CurveInfo handleHistogramCaseDrop( RimHistogramCase* histogramCase );
    // CurveInfo handleEnsembleDrop( RimHistogramEnsemble* ensemble );
    // CurveInfo handleAddressCollectionDrop( RimHistogramAddressCollection* addrColl );
    // CurveInfo handleHistogramAddressDrop( RimHistogramAddress* histogramAddr );

    // bool isOnlyWaterCutCurvesVisible( RiuPlotAxis plotAxis );

    // static RiuPlotAxis plotAxisForTime();

private:
    caf::PdmField<bool>    m_normalizeCurveYValues;
    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;
    caf::PdmField<QString> m_fallbackPlotName;

    caf::PdmChildField<RimHistogramCurveCollection*> m_histogramCurveCollection;

    caf::PdmChildArrayField<RimPlotAxisPropertiesInterface*> m_axisPropertiesArray;

    caf::PdmField<caf::AppEnum<FrequencyType>> m_histogramFrequencyType;
    caf::PdmField<caf::AppEnum<GraphType>>     m_graphType;

    std::unique_ptr<RiuPlotWidget> m_histogramPlot;

    // caf::PdmChildField<RimHistogramPlotSourceStepping*> m_sourceStepping;

    bool                  m_isValid;
    RiuPlotWidget::Legend m_legendPosition;
};
