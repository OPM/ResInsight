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

#include "RifEclipseSummaryAddress.h"

#include "RimPlot.h"
#include "RimSummaryDataSourceStepping.h"

#include "RiuQwtPlotWidget.h"
#include "RiuSummaryPlot.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrArrayField.h"
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
class RimSummaryCaseCollection;
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
class RimSummaryPlotFilterTextCurveSetEditor;
class RimSummaryPlotSourceStepping;
class RiaSummaryCurveDefinition;

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

public:
    RimSummaryPlot( bool isCrossPlot = false );
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

    void addTimeAnnotation( time_t time );
    void addTimeRangeAnnotation( time_t startTime, time_t endTime );
    void removeAllTimeAnnotations();

    void updateAxes() override;

    bool isLogarithmicScaleEnabled( RiuPlotAxis plotAxis ) const;

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
    QString                     generatedPlotTitleFromAllCurves() const;

    void copyAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot );
    void copyMatchingAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot );

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

    void           setAutoScaleXEnabled( bool enabled ) override;
    void           setAutoScaleYEnabled( bool enabled ) override;
    RiuPlotWidget* plotWidget() override;
    void           zoomAll() override;
    void           updateZoomInParentPlot() override;
    void           updateZoomFromParentPlot() override;

    caf::PdmObject* findPdmObjectFromPlotCurve( const RiuPlotCurve* curve ) const override;

    void onAxisSelected( RiuPlotAxis axis, bool toggle ) override;

    static constexpr int precision()
    {
        // Set precision to 8, as this is the precision used in summary data in libEcl
        return 8;
    }

    static void moveCurvesToPlot( RimSummaryPlot* plot, const std::vector<RimSummaryCurve*> curves, int insertAtPosition );

    std::vector<RimSummaryDataSourceStepping::Axis> availableAxes() const override;
    std::vector<RimSummaryCurve*>     curvesForStepping( RimSummaryDataSourceStepping::Axis axis ) const override;
    std::vector<RimEnsembleCurveSet*> curveSets() const override;
    std::vector<RimSummaryCurve*>     allCurves( RimSummaryDataSourceStepping::Axis axis ) const override;

    std::vector<RimPlotAxisPropertiesInterface*> plotAxes() const;

    RimPlotAxisPropertiesInterface* axisPropertiesForPlotAxis( RiuPlotAxis plotAxis ) const;

    RimPlotAxisProperties* addNewAxisProperties( RiaDefines::PlotAxis, const QString& name );

    std::vector<RimPlotCurve*> visibleCurvesForLegend() override;

public:
    // RimViewWindow overrides
    void deleteViewWidget() override;
    void initAfterRead() override;

    bool isDeletable() const override;

    void handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects ) override;

private:
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void updateNameHelperWithCurveData( RimSummaryPlotNameHelper* nameHelper ) const;

    void doUpdateLayout() override;

    void detachAllPlotItems();
    void deleteAllPlotCurves();

    void onCurveCollectionChanged( const SignalEmitter* emitter );

    void connectCurveToPlot( RimSummaryCurve* curve, bool update, bool autoAssignPlotAxis );

    RimPlotAxisProperties* addNewAxisProperties( RiuPlotAxis plotAxis, const QString& name );

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void                 onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;

    QImage snapshotWindowContent() override;

    bool handleGlobalKeyEvent( QKeyEvent* keyEvent ) override;

private slots:
    void onPlotZoomed();

private:
    std::vector<RimSummaryCurve*>         visibleSummaryCurvesForAxis( RiuPlotAxis plotAxis ) const;
    std::vector<RimGridTimeHistoryCurve*> visibleTimeHistoryCurvesForAxis( RiuPlotAxis plotAxis ) const;
    std::vector<RimAsciiDataCurve*>       visibleAsciiDataCurvesForAxis( RiuPlotAxis plotAxis ) const;
    bool                                  hasVisibleCurvesForAxis( RiuPlotAxis plotAxis ) const;
    std::vector<RimSummaryCurve*>         visibleStackedSummaryCurvesForAxis( RiuPlotAxis plotAxis );

    void updateAxis( RiaDefines::PlotAxis plotAxis );

    void updateZoomForAxis( RiuPlotAxis plotAxis );

    void updateTimeAxis( RimSummaryTimeAxisProperties* timeAxisProperties );

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

    void assignPlotAxis( RimSummaryCurve* curve );

    RimSummaryCurve* addNewCurveY( const RifEclipseSummaryAddress& address, RimSummaryCase* summaryCase );
    void addNewEnsembleCurveY( const RifEclipseSummaryAddress& address, RimSummaryCaseCollection* ensemble );

    void updateStackedCurveData();
    bool updateStackedCurveDataForAxis( RiuPlotAxis plotAxis );
    bool updateStackedCurveDataForRelevantAxes();

    std::pair<int, std::vector<RimSummaryCurve*>> handleSummaryCaseDrop( RimSummaryCase* summaryCase );
    std::pair<int, std::vector<RimSummaryCurve*>> handleAddressCollectionDrop( RimSummaryAddressCollection* addrColl );
    std::pair<int, std::vector<RimSummaryCurve*>> handleSummaryAddressDrop( RimSummaryAddress* summaryAddr );
    void applyDefaultCurveAppearances( std::vector<RimSummaryCurve*> curvesToUpdate );

    bool isOnlyWaterCutCurvesVisible( RiuPlotAxis plotAxis );

private:
#ifdef USE_QTCHARTS
    caf::PdmField<bool> m_useQtChartsPlot;
#endif
    caf::PdmField<bool> m_normalizeCurveYValues;

    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;
    caf::PdmField<QString> m_alternatePlotName;

    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>  m_gridTimeHistoryCurves;
    caf::PdmChildField<RimSummaryCurveCollection*>     m_summaryCurveCollection;
    caf::PdmChildField<RimEnsembleCurveSetCollection*> m_ensembleCurveSetCollection;

    caf::PdmChildArrayField<RimAsciiDataCurve*> m_asciiDataCurves;

    caf::PdmChildField<RimPlotAxisProperties*> m_leftYAxisProperties_OBSOLETE;
    caf::PdmChildField<RimPlotAxisProperties*> m_rightYAxisProperties_OBSOLETE;

    caf::PdmChildField<RimPlotAxisProperties*>        m_bottomAxisProperties_OBSOLETE;
    caf::PdmChildField<RimSummaryTimeAxisProperties*> m_timeAxisProperties_OBSOLETE;

    caf::PdmChildArrayField<RimPlotAxisPropertiesInterface*> m_axisProperties;

    caf::PdmChildField<RimSummaryPlotFilterTextCurveSetEditor*> m_textCurveSetEditor;

    std::unique_ptr<RiuSummaryPlot>   m_summaryPlot;
    std::unique_ptr<QwtPlotTextLabel> m_plotInfoLabel;

    bool m_isCrossPlot;

    std::unique_ptr<RimSummaryPlotNameHelper>         m_nameHelperAllCurves;
    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;
};
