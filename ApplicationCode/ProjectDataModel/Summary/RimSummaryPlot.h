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

#include "cafPdmChildArrayField.h"

#include "RiaDefines.h"
#include "RiaQDateTimeTools.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifEclipseSummaryAddress.h"

#include "RimRiuQwtPlotOwnerInterface.h"
#include "RimViewWindow.h"

#include "qwt_plot_textlabel.h"
#include <QPointer>

#include <set>
#include <memory>

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

class QwtInterval;
class QwtPlotCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryPlot : public RimViewWindow, public RimRiuQwtPlotOwnerInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlot();
    ~RimSummaryPlot() override;

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    void                                            enableShowPlotTitle(bool enable);
    void                                            enableAutoPlotTitle(bool enable);
    bool                                            autoPlotTitle() const;

    void                                            addCurveAndUpdate(RimSummaryCurve* curve);
    void                                            addCurveNoUpdate(RimSummaryCurve* curve);

    void                                            deleteCurve(RimSummaryCurve* curve);
    void                                            deleteCurves(const std::vector<RimSummaryCurve*>& curves);

    void                                            deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase);

    RimEnsembleCurveSetCollection*                  ensembleCurveSetCollection() const;

    void                                            addGridTimeHistoryCurve(RimGridTimeHistoryCurve* curve);

    void                                            addAsciiDataCruve(RimAsciiDataCurve* curve);

    size_t                                          curveCount() const;
    
    void                                            detachAllCurves();
    void                                            reattachAllCurves();
    void                                            updateCaseNameHasChanged();

    void                                            updateAxes();
    void                                            zoomAll() override;

    void                                            updateZoomInQwt();
    
    bool                                            isLogarithmicScaleEnabled(RiaDefines::PlotAxis plotAxis) const;

    RimSummaryTimeAxisProperties*                   timeAxisProperties();
    time_t                                          firstTimeStepOfFirstCurve();

    QWidget*                                        viewWidget() override;

    QString                                         asciiDataForPlotExport(DateTimePeriod resamplingPeriod = DateTimePeriod::NONE) const;

    std::vector<RimSummaryCurve*>                   summaryAndEnsembleCurves() const;
    std::set<RiaSummaryCurveDefinition>             summaryAndEnsembleCurveDefinitions() const;
    std::vector<RimSummaryCurve*>                   summaryCurves() const;
    void                                            deleteAllSummaryCurves();
    RimSummaryCurveCollection*                      summaryCurveCollection() const;
    RiuSummaryQwtPlot*                              qwtPlot() const;

    std::vector<RimEnsembleCurveSet*>               curveSets() const;

    void                                            updatePlotTitle();

    const RimSummaryPlotNameHelper*                 activePlotTitleHelperAllCurves() const;
    void                                            updateCurveNames();
    QString                                         generatedPlotTitleFromAllCurves() const;

    void                                            copyAxisPropertiesFromOther(const RimSummaryPlot& sourceSummaryPlot);

    void                                            updateAll();
    void                                            updateAllLegendItems();

    void                                            setPlotInfoLabel(const QString& label);
    void                                            showPlotInfoLabel(bool show);
    void                                            updatePlotInfoLabel();

    bool                                            containsResamplableCurves() const;

    size_t                                          singleColorCurveCount() const;
    void                                            applyDefaultCurveAppearances();


    bool hasCustomFontSizes(RiaDefines::FontSettingType fontSettingType, int defaultFontSize) const override;
    bool applyFontSize(RiaDefines::FontSettingType fontSettingType, int oldFontSize, int fontSize, bool forceChange = false) override;

public:
    // Rim2dPlotInterface overrides
    void updateAxisScaling() override;
    void updateAxisDisplay() override;
    void updateZoomWindowFromQwt() override;
    void selectAxisInPropertyEditor(int axis) override;
    void setAutoZoomForAllAxes(bool enableAutoZoom) override;
    caf::PdmObject* findRimPlotObjectFromQwtCurve(const QwtPlotCurve* curve) const override;
    void showLegend(bool enable);

public:
    // RimViewWindow overrides
    QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    void                                    deleteViewWidget() override; 
    void                                    initAfterRead() override;

private:
    void                                            updateMdiWindowTitle() override;
    void                                            updateNameHelperWithCurveData(RimSummaryPlotNameHelper* nameHelper) const;

protected:
    // Overridden PDM methods
    caf::PdmFieldHandle*                    userDescriptionField() override;
    QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                                    onLoadDataAndUpdate() override;

    QImage                                  snapshotWindowContent() override;

    void                                            setAsCrossPlot();

private:
    std::vector<RimSummaryCurve*>                   visibleSummaryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    std::vector<RimGridTimeHistoryCurve*>           visibleTimeHistoryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    std::vector<RimAsciiDataCurve*>                 visibleAsciiDataCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    bool                                            hasVisibleCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;

    RimPlotAxisProperties*                          yAxisPropertiesLeftOrRight(RiaDefines::PlotAxis leftOrRightPlotAxis) const;
    void                                            updateAxis(RiaDefines::PlotAxis plotAxis);

    void                                            updateZoomForAxis(RiaDefines::PlotAxis plotAxis);

    void                                            updateTimeAxis();
    void                                            updateBottomXAxis();

    void                                            updateAxisRangesFromQwt();

    std::set<RimPlotAxisPropertiesInterface*>       allPlotAxes() const;

private:
    caf::PdmField<bool>                                 m_showPlotTitle;
    caf::PdmField<bool>                                 m_showLegend;
    caf::PdmField<int>                                  m_legendFontSize;

    caf::PdmField<bool>                                 m_useAutoPlotTitle;
    caf::PdmField<QString>                              m_userDefinedPlotTitle;
    
    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>   m_gridTimeHistoryCurves;
    caf::PdmChildField<RimSummaryCurveCollection*>      m_summaryCurveCollection;
    caf::PdmChildField<RimEnsembleCurveSetCollection*>  m_ensembleCurveSetCollection;

    caf::PdmChildArrayField<RimAsciiDataCurve*>         m_asciiDataCurves;

    caf::PdmChildField<RimPlotAxisProperties*>       m_leftYAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*>       m_rightYAxisProperties;

    caf::PdmChildField<RimPlotAxisProperties*>       m_bottomAxisProperties;
    caf::PdmChildField<RimSummaryTimeAxisProperties*>   m_timeAxisProperties;

    QPointer<RiuSummaryQwtPlot>                         m_qwtPlot;
    std::unique_ptr<QwtPlotTextLabel>                   m_plotInfoLabel;

    bool                                                m_isCrossPlot;

    std::unique_ptr<RimSummaryPlotNameHelper>           m_nameHelperAllCurves;

    // Obsolete fields
    caf::PdmChildArrayField<RimSummaryCurve*>                m_summaryCurves_OBSOLETE;
    caf::PdmChildArrayField<RimSummaryCurveFilter_OBSOLETE*> m_curveFilters_OBSOLETE;
    caf::PdmField<bool>                                      m_isAutoZoom_OBSOLETE;

};
