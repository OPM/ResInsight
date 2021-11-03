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

#include "RiaPlotDefines.h"
#include "RiaQDateTimeTools.h"

#include "RifEclipseSummaryAddress.h"

#include "RimQwtPlot.h"

#include "qwt_plot_textlabel.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <memory>
#include <set>

class PdmUiTreeOrdering;
class RimAsciiDataCurve;
class RimGridTimeHistoryCurve;
class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryCurveCollection;
class RimEnsembleCurveSet;
class RimEnsembleCurveSetCollection;
class RimSummaryCurveFilter_OBSOLETE;
class RimSummaryTimeAxisProperties;
class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RiuSummaryQwtPlot;
class RimSummaryPlotNameHelper;
class RimPlotTemplateFileItem;
class RimSummaryPlotFilterTextCurveSetEditor;
class RimSummaryPlotSourceStepping;
class RiaSummaryCurveDefinition;

class QwtInterval;
class QwtPlotCurve;

class QKeyEvent;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryPlot : public RimQwtPlot
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlot();
    ~RimSummaryPlot() override;

    void    setDescription( const QString& description );
    QString description() const override;

    void enableAutoPlotTitle( bool enable );
    bool autoPlotTitle() const;

    void addCurveAndUpdate( RimSummaryCurve* curve );
    void addCurveNoUpdate( RimSummaryCurve* curve );

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

    void addTimeAnnotation( time_t time );
    void addTimeRangeAnnotation( time_t startTime, time_t endTime );
    void removeAllTimeAnnotations();

    void updateAxes() override;

    bool isLogarithmicScaleEnabled( RiaDefines::PlotAxis plotAxis ) const;

    RimSummaryTimeAxisProperties* timeAxisProperties();
    time_t                        firstTimeStepOfFirstCurve();

    QWidget*          viewWidget() override;
    RiuQwtPlotWidget* viewer() override;

    QString asciiDataForPlotExport() const override;
    QString asciiDataForSummaryPlotExport( RiaQDateTimeTools::DateTimePeriod resamplingPeriod,
                                           bool                              showTimeAsLongString ) const;

    std::vector<RimSummaryCurve*>       summaryAndEnsembleCurves() const;
    std::set<RiaSummaryCurveDefinition> summaryAndEnsembleCurveDefinitions() const;
    std::vector<RimSummaryCurve*>       summaryCurves() const;
    void                                deleteAllSummaryCurves();
    RimSummaryCurveCollection*          summaryCurveCollection() const;

    std::vector<RimEnsembleCurveSet*> curveSets() const;

    void updatePlotTitle();

    const RimSummaryPlotNameHelper* activePlotTitleHelperAllCurves() const;
    void                            updateCurveNames();
    QString                         generatedPlotTitleFromAllCurves() const;

    void copyAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot );

    void updateAll();
    void updateLegend() override;

    void setPlotInfoLabel( const QString& label );
    void showPlotInfoLabel( bool show );
    void updatePlotInfoLabel();

    bool containsResamplableCurves() const;

    size_t singleColorCurveCount() const;
    void   applyDefaultCurveAppearances();

    void setNormalizationEnabled( bool enable );
    bool isNormalizationEnabled();

    virtual RimSummaryPlotSourceStepping*     sourceSteppingObjectForKeyEventHandling() const;
    virtual std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

    void setAutoScaleXEnabled( bool enabled ) override;
    void setAutoScaleYEnabled( bool enabled ) override;

    void zoomAll() override;
    void updateZoomInQwt() override;
    void updateZoomFromQwt() override;

    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;

    void onAxisSelected( int axis, bool toggle ) override;

    static constexpr int precision()
    {
        // Set precision to 8, as this is the precision used in summary data in libEcl
        return 8;
    }

    static void moveCurvesToPlot( RimSummaryPlot* plot, const std::vector<RimSummaryCurve*> curves, int insertAtPosition );

public:
    // RimViewWindow overrides
    void deleteViewWidget() override;
    void initAfterRead() override;

    bool isDeletable() const override;

private:
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void updateNameHelperWithCurveData( RimSummaryPlotNameHelper* nameHelper ) const;

    void doUpdateLayout() override;

    void detachAllPlotItems();

    void handleKeyPressEvent( QKeyEvent* keyEvent ) override;

    void onCurveCollectionChanged( const SignalEmitter* emitter );

protected:
    // Overridden PDM methods
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void                 updateStackedCurveData();
    void                 updateStackedCurveDataForAxis( RiaDefines::PlotAxis plotAxis );

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;

    QImage snapshotWindowContent() override;

    void setAsCrossPlot();

private slots:
    void onPlotZoomed();

private:
    std::vector<RimSummaryCurve*>         visibleSummaryCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;
    std::vector<RimGridTimeHistoryCurve*> visibleTimeHistoryCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;
    std::vector<RimAsciiDataCurve*>       visibleAsciiDataCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;
    bool                                  hasVisibleCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;
    std::vector<RimSummaryCurve*>         visibleStackedSummaryCurvesForAxis( RiaDefines::PlotAxis plotAxis );

    RimPlotAxisProperties* yAxisPropertiesLeftOrRight( RiaDefines::PlotAxis leftOrRightPlotAxis ) const;
    void                   updateYAxis( RiaDefines::PlotAxis plotAxis );

    void updateZoomForAxis( RiaDefines::PlotAxis plotAxis );

    void updateTimeAxis();
    void updateBottomXAxis();

    std::set<RimPlotAxisPropertiesInterface*> allPlotAxes() const;

    void cleanupBeforeClose();

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

private:
    caf::PdmField<bool> m_normalizeCurveYValues;

    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;

    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>  m_gridTimeHistoryCurves;
    caf::PdmChildField<RimSummaryCurveCollection*>     m_summaryCurveCollection;
    caf::PdmChildField<RimEnsembleCurveSetCollection*> m_ensembleCurveSetCollection;

    caf::PdmChildArrayField<RimAsciiDataCurve*> m_asciiDataCurves;

    caf::PdmChildField<RimPlotAxisProperties*> m_leftYAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*> m_rightYAxisProperties;

    caf::PdmChildField<RimPlotAxisProperties*>        m_bottomAxisProperties;
    caf::PdmChildField<RimSummaryTimeAxisProperties*> m_timeAxisProperties;

    caf::PdmChildField<RimSummaryPlotFilterTextCurveSetEditor*> m_textCurveSetEditor;

    QPointer<RiuSummaryQwtPlot>       m_plotWidget;
    std::unique_ptr<QwtPlotTextLabel> m_plotInfoLabel;

    bool m_isCrossPlot;

    std::unique_ptr<RimSummaryPlotNameHelper> m_nameHelperAllCurves;
};
