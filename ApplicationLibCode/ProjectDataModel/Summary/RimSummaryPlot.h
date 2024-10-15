/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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
#include "RimSummaryDataSourceStepping.h"

#include "RiuQwtPlotWidget.h"
#include "RiuSummaryPlot.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <memory>
#include <set>
#include <vector>

class PdmUiTreeOrdering;
class RimAsciiDataCurve;
class RimGridTimeHistoryCurve;
class RimSummaryAddress;
class RimSummaryAddressCollection;
class RimSummaryCase;
class RimSummaryEnsemble;
class RimSummaryCurve;
class RimSummaryCurveCollection;
class RimEnsembleCurveSet;
class RimEnsembleCurveSetCollection;
class RimSummaryCurveFilter_OBSOLETE;
class RimSummaryTimeAxisProperties;
class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RiuSummaryQwtPlot;
class RimSummaryNameHelper;
class RimSummaryPlotNameHelper;
class RimPlotTemplateFileItem;
class RimSummaryPlotSourceStepping;
class RimTimeAxisAnnotation;
class RiaSummaryCurveDefinition;
class RifEclipseSummaryAddress;
class RiaSummaryCurveAddress;

class QwtInterval;
class QwtPlotCurve;
class QwtPlotTextLabel;

class QKeyEvent;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryPlot : public RimPlot, public RimSummaryDataSourceStepping
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<>                curvesChanged;
    caf::Signal<RimSummaryPlot*> axisChanged;
    caf::Signal<>                plotZoomedByUser;
    caf::Signal<>                titleChanged;
    caf::Signal<RimSummaryPlot*> axisChangedReloadRequired;
    caf::Signal<bool>            autoTitleChanged;

public:
    RimSummaryPlot();
    ~RimSummaryPlot() override;

    void    setDescription( const QString& description );
    QString description() const override;

    void enableAutoPlotTitle( bool enable );
    bool autoPlotTitle() const;

    void addCurveAndUpdate( RimSummaryCurve* curve, bool autoAssignPlotAxis = true );
    void addCurveNoUpdate( RimSummaryCurve* curve, bool autoAssignPlotAxis = true );

    void insertCurve( RimSummaryCurve* curve, size_t insertAtPosition );

    void removeCurve( RimSummaryCurve* curve );

    void deleteCurve( RimSummaryCurve* curve );
    void deleteCurves( const std::vector<RimSummaryCurve*>& curves );

    void deleteCurvesAssosiatedWithCase( RimSummaryCase* summaryCase );
    void deleteAllGridTimeHistoryCurves();

    RimEnsembleCurveSetCollection* ensembleCurveSetCollection() const;

    void addGridTimeHistoryCurve( RimGridTimeHistoryCurve* curve );
    void addGridTimeHistoryCurveNoUpdate( RimGridTimeHistoryCurve* curve );

    std::vector<RimGridTimeHistoryCurve*> gridTimeHistoryCurves() const;

    void addAsciiDataCruve( RimAsciiDataCurve* curve );

    size_t curveCount() const;

    void detachAllCurves() override;
    void reattachAllCurves() override;
    void updateCaseNameHasChanged();

    RimTimeAxisAnnotation* addTimeAnnotation( time_t time );
    RimTimeAxisAnnotation* addTimeRangeAnnotation( time_t startTime, time_t endTime );
    void                   removeTimeAnnotation( RimTimeAxisAnnotation* annotation );
    void                   removeAllTimeAnnotations();

    void updateAxes() override;

    bool isLogarithmicScaleEnabled( RiuPlotAxis plotAxis ) const;

    bool isCurveHighlightSupported() const override;

    RimSummaryTimeAxisProperties* timeAxisProperties();
    time_t                        firstTimeStepOfFirstCurve();

    QWidget* viewWidget() override;

    QString asciiDataForPlotExport() const override;
    QString asciiDataForSummaryPlotExport( RiaDefines::DateTimePeriod resamplingPeriod, bool showTimeAsLongString ) const;

    std::vector<RimSummaryCurve*>       summaryAndEnsembleCurves() const;
    std::set<RiaSummaryCurveDefinition> summaryAndEnsembleCurveDefinitions() const;
    std::vector<RimSummaryCurve*>       summaryCurves() const;
    void                                deleteAllSummaryCurves();
    RimSummaryCurveCollection*          summaryCurveCollection() const;

    void updatePlotTitle();

    const RimSummaryNameHelper* activePlotTitleHelperAllCurves() const;
    const RimSummaryNameHelper* plotTitleHelper() const;
    void                        updateCurveNames();

    void copyAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot );
    void copyAxisPropertiesFromOther( RiaDefines::PlotAxis plotAxisType, const RimSummaryPlot& sourceSummaryPlot );
    void copyMatchingAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot );

    void updateAll();
    void updateLegend() override;
    void setLegendPosition( RiuPlotWidget::Legend position );

    void setPlotInfoLabel( const QString& label );
    void showPlotInfoLabel( bool show );
    void updatePlotInfoLabel();

    size_t singleColorCurveCount() const;
    void   applyDefaultCurveAppearances();
    void   applyDefaultCurveAppearances( std::vector<RimSummaryCurve*> curvesToUpdate );
    void   applyDefaultCurveAppearances( std::vector<RimEnsembleCurveSet*> ensembleCurvesToUpdate );

    void setNormalizationEnabled( bool enable );
    bool isNormalizationEnabled();

    RimSummaryPlotSourceStepping* sourceSteppingObjectForKeyEventHandling() const;

    void           setAutoScaleXEnabled( bool enabled ) override;
    void           setAutoScaleYEnabled( bool enabled ) override;
    RiuPlotWidget* plotWidget() override;
    void           zoomAll() override;
    void           updatePlotWidgetFromAxisRanges() override;
    void           updateAxisRangesFromPlotWidget() override;

    void onAxisSelected( RiuPlotAxis axis, bool toggle ) override;

    static constexpr int precision()
    {
        // Set precision to 8, as this is the precision used in summary data in libEcl
        return 8;
    }

    std::vector<RimSummaryCurve*>     curvesForStepping() const override;
    std::vector<RimEnsembleCurveSet*> curveSets() const override;
    std::vector<RimSummaryCurve*>     allCurves() const override;

    std::vector<RimPlotAxisProperties*> plotAxes( RimPlotAxisProperties::Orientation orientation ) const;

    RimPlotAxisPropertiesInterface* axisPropertiesForPlotAxis( RiuPlotAxis plotAxis ) const;
    RimPlotAxisProperties*          addNewAxisProperties( RiaDefines::PlotAxis, const QString& name );
    RimPlotAxisProperties*          addNewAxisProperties( RiuPlotAxis plotAxis, const QString& name );
    void                            findOrAssignPlotAxisX( RimSummaryCurve* curve );

    std::vector<RimPlotCurve*> visibleCurvesForLegend() override;

    RimSummaryPlotSourceStepping* sourceStepper();
    void                          scheduleReplotIfVisible();

public:
    // RimViewWindow overrides
    void deleteViewWidget() override;
    void initAfterRead() override;

    bool isDeletable() const override;

    void handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

private:
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void updateNameHelperWithCurveData( RimSummaryPlotNameHelper* nameHelper ) const;

    void doUpdateLayout() override;

    void detachAllPlotItems();
    void deleteAllPlotCurves();

    void onCurveCollectionChanged( const SignalEmitter* emitter );
    void onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex ) override;

    void connectCurveToPlot( RimSummaryCurve* curve, bool update, bool autoAssignPlotAxis );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;

    QImage snapshotWindowContent() override;

    bool handleGlobalKeyEvent( QKeyEvent* keyEvent ) override;

private slots:
    void onPlotZoomed();
    void onUpdateCurveOrder();

private:
    std::vector<RimSummaryCurve*>         visibleSummaryCurvesForAxis( RiuPlotAxis plotAxis ) const;
    std::vector<RimGridTimeHistoryCurve*> visibleTimeHistoryCurvesForAxis( RiuPlotAxis plotAxis ) const;
    std::vector<RimAsciiDataCurve*>       visibleAsciiDataCurvesForAxis( RiuPlotAxis plotAxis ) const;
    bool                                  hasVisibleCurvesForAxis( RiuPlotAxis plotAxis ) const;
    std::vector<RimSummaryCurve*>         visibleStackedSummaryCurvesForAxis( RiuPlotAxis plotAxis );

    void updateNumericalAxis( RiaDefines::PlotAxis plotAxis );
    void updateZoomForAxis( RimPlotAxisPropertiesInterface* axisProperties );
    void updateZoomForNumericalAxis( RimPlotAxisProperties* axisProperties );
    void updateTimeAxis( RimSummaryTimeAxisProperties* timeAxisProperties );
    void updateZoomForTimeAxis( RimSummaryTimeAxisProperties* timeAxisProperties );

    void createAndSetCustomTimeAxisTickmarks( RimSummaryTimeAxisProperties* timeAxisProperties );
    void overrideTimeAxisSettingsIfTooManyCustomTickmarks( RimSummaryTimeAxisProperties* timeAxisProperties, bool showMessageBox );

    void deletePlotCurvesAndPlotWidget();

    void connectCurveSignals( RimSummaryCurve* curve );
    void disconnectCurveSignals( RimSummaryCurve* curve );

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

    void timeAxisSettingsChanged( const caf::SignalEmitter* emitter );
    void timeAxisSettingsChangedReloadRequired( const caf::SignalEmitter* emitter );

    void ensureRequiredAxisObjectsForCurves();
    void assignPlotAxis( RimSummaryCurve* curve );
    void assignYPlotAxis( RimSummaryCurve* curve );
    void assignXPlotAxis( RimSummaryCurve* curve );

    RimSummaryCurve* addNewCurve( const RifEclipseSummaryAddress& address,
                                  RimSummaryCase*                 summaryCase,
                                  const RifEclipseSummaryAddress& addressX,
                                  RimSummaryCase*                 summaryCaseX );

    void updateStackedCurveData();
    bool updateStackedCurveDataForAxis( RiuPlotAxis plotAxis );
    bool updateStackedCurveDataForRelevantAxes();

    struct CurveInfo;

    CurveInfo handleSummaryCaseDrop( RimSummaryCase* summaryCase );
    CurveInfo handleEnsembleDrop( RimSummaryEnsemble* ensemble );
    CurveInfo handleAddressCollectionDrop( RimSummaryAddressCollection* addrColl );
    CurveInfo handleSummaryAddressDrop( RimSummaryAddress* summaryAddr );

    bool isOnlyWaterCutCurvesVisible( RiuPlotAxis plotAxis );

    static RiuPlotAxis plotAxisForTime();

private:
#ifdef USE_QTCHARTS
    caf::PdmField<bool> m_useQtChartsPlot;
#endif
    caf::PdmField<bool> m_normalizeCurveYValues;

    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;
    caf::PdmField<QString> m_fallbackPlotName;

    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>  m_gridTimeHistoryCurves;
    caf::PdmChildField<RimSummaryCurveCollection*>     m_summaryCurveCollection;
    caf::PdmChildField<RimEnsembleCurveSetCollection*> m_ensembleCurveSetCollection;

    caf::PdmChildArrayField<RimAsciiDataCurve*> m_asciiDataCurves;

    caf::PdmChildField<RimPlotAxisProperties*> m_leftYAxisProperties_OBSOLETE;
    caf::PdmChildField<RimPlotAxisProperties*> m_rightYAxisProperties_OBSOLETE;

    caf::PdmChildField<RimPlotAxisProperties*>        m_bottomAxisProperties_OBSOLETE;
    caf::PdmChildField<RimSummaryTimeAxisProperties*> m_timeAxisProperties_OBSOLETE;

    caf::PdmChildArrayField<RimPlotAxisPropertiesInterface*> m_axisPropertiesArray;

    std::unique_ptr<RiuSummaryPlot>   m_summaryPlot;
    std::unique_ptr<QwtPlotTextLabel> m_plotInfoLabel;

    std::unique_ptr<RimSummaryPlotNameHelper>         m_nameHelperAllCurves;
    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;

    bool                  m_isValid;
    RiuPlotWidget::Legend m_legendPosition;
};
