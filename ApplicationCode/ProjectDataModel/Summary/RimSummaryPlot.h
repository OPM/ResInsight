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

#include "RiaDefines.h"
#include "RiaQDateTimeTools.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifEclipseSummaryAddress.h"

#include "RimPlotInterface.h"
#include "RimPlotWindow.h"
#include "RimRiuQwtPlotOwnerInterface.h"

#include "qwt_plot_textlabel.h"

#include "cafPdmChildArrayField.h"
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

class QwtInterval;
class QwtPlotCurve;

class QKeyEvent;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryPlot : public RimPlotWindow, public RimPlotInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlot();
    ~RimSummaryPlot() override;

    void    setDescription( const QString& description );
    QString description() const;
    bool    isChecked() const override;

    void enableShowPlotTitle( bool enable );
    void enableAutoPlotTitle( bool enable );
    bool autoPlotTitle() const;

    void addCurveAndUpdate( RimSummaryCurve* curve );
    void addCurveNoUpdate( RimSummaryCurve* curve );

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

    void detachAllCurves();
    void reattachAllCurves();
    void updateCaseNameHasChanged();

    void updateAxes();

    bool isLogarithmicScaleEnabled( RiaDefines::PlotAxis plotAxis ) const;

    RimSummaryTimeAxisProperties* timeAxisProperties();
    time_t                        firstTimeStepOfFirstCurve();

    QWidget*          viewWidget() override;
    RiuQwtPlotWidget* viewer() override;

    QString asciiDataForPlotExport( DateTimePeriod resamplingPeriod, bool showTimeAsLongString ) const;

    std::vector<RimSummaryCurve*>       summaryAndEnsembleCurves() const;
    std::set<RiaSummaryCurveDefinition> summaryAndEnsembleCurveDefinitions() const;
    std::vector<RimSummaryCurve*>       summaryCurves() const;
    void                                deleteAllSummaryCurves();
    RimSummaryCurveCollection*          summaryCurveCollection() const;

    std::vector<RimEnsembleCurveSet*> curveSets() const;

    void updatePlotTitle() override;

    const RimSummaryPlotNameHelper* activePlotTitleHelperAllCurves() const;
    void                            updateCurveNames();
    QString                         generatedPlotTitleFromAllCurves() const;

    void copyAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot );

    void updateLayout() override;

    void updateAll();
    void updateAllLegendItems();

    void setPlotInfoLabel( const QString& label );
    void showPlotInfoLabel( bool show );
    void updatePlotInfoLabel();

    bool containsResamplableCurves() const;

    size_t singleColorCurveCount() const;
    void   applyDefaultCurveAppearances();

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

    void setNormalizationEnabled( bool enable );
    bool isNormalizationEnabled();
    void showLegend( bool enable );

    void                     setPlotTemplate( RimPlotTemplateFileItem* plotTemplate );
    RimPlotTemplateFileItem* plotTemplate() const;

    void                                      handleKeyPressEvent( QKeyEvent* keyEvent );
    virtual RimSummaryPlotSourceStepping*     sourceSteppingObjectForKeyEventHandling() const;
    virtual std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

    void setAutoScaleXEnabled( bool enabled ) override;
    void setAutoScaleYEnabled( bool enabled ) override;

    void zoomAll() override;
    void updateZoomInQwt() override;
    void updateZoomFromQwt() override;

    void            createPlotWidget() override;
    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;

    void onAxisSelected( int axis, bool toggle ) override;
    void loadDataAndUpdate();

    void addOrUpdateEnsembleCurveSetLegend( RimEnsembleCurveSet* curveSet );
    void removeEnsembleCurveSetLegend( RimEnsembleCurveSet* curveSet );

public:
    // RimViewWindow overrides
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;
    void     initAfterRead() override;

private:
    void updateMdiWindowTitle() override;
    void updateNameHelperWithCurveData( RimSummaryPlotNameHelper* nameHelper ) const;

protected:
    // Overridden PDM methods
    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;

    QImage snapshotWindowContent() override;

    void setAsCrossPlot();

private:
    std::vector<RimSummaryCurve*>         visibleSummaryCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;
    std::vector<RimGridTimeHistoryCurve*> visibleTimeHistoryCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;
    std::vector<RimAsciiDataCurve*>       visibleAsciiDataCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;
    bool                                  hasVisibleCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const;

    RimPlotAxisProperties* yAxisPropertiesLeftOrRight( RiaDefines::PlotAxis leftOrRightPlotAxis ) const;
    void                   updateYAxis( RiaDefines::PlotAxis plotAxis );

    void updateZoomForAxis( RiaDefines::PlotAxis plotAxis );

    void updateTimeAxis();
    void updateBottomXAxis();

    std::set<RimPlotAxisPropertiesInterface*> allPlotAxes() const;

private:
    caf::PdmField<bool> m_showPlotTitle;
    caf::PdmField<bool> m_showLegend;
    caf::PdmField<bool> m_normalizeCurveYValues;

    caf::PdmField<int> m_legendFontSize;

    caf::PdmField<bool> m_useAutoPlotTitle;

    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>  m_gridTimeHistoryCurves;
    caf::PdmChildField<RimSummaryCurveCollection*>     m_summaryCurveCollection;
    caf::PdmChildField<RimEnsembleCurveSetCollection*> m_ensembleCurveSetCollection;

    caf::PdmChildArrayField<RimAsciiDataCurve*> m_asciiDataCurves;

    caf::PdmChildField<RimPlotAxisProperties*> m_leftYAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*> m_rightYAxisProperties;

    caf::PdmChildField<RimPlotAxisProperties*>        m_bottomAxisProperties;
    caf::PdmChildField<RimSummaryTimeAxisProperties*> m_timeAxisProperties;

    caf::PdmPtrField<RimPlotTemplateFileItem*> m_plotTemplate;

    caf::PdmChildField<RimSummaryPlotFilterTextCurveSetEditor*> m_textCurveSetEditor;

    QPointer<RiuSummaryQwtPlot>       m_plotWidget;
    std::unique_ptr<QwtPlotTextLabel> m_plotInfoLabel;

    bool m_isCrossPlot;

    std::unique_ptr<RimSummaryPlotNameHelper> m_nameHelperAllCurves;

    // Obsolete fields
    caf::PdmChildArrayField<RimSummaryCurve*>                m_summaryCurves_OBSOLETE;
    caf::PdmChildArrayField<RimSummaryCurveFilter_OBSOLETE*> m_curveFilters_OBSOLETE;
    caf::PdmField<bool>                                      m_isAutoZoom_OBSOLETE;
};
