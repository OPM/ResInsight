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

#include "RifEclipseSummaryAddress.h"

#include "RimViewWindow.h"

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
class RimSummaryAxisProperties;
class RiuSummaryQwtPlot;
class RimSummaryPlotNameHelper;

class QwtInterval;
class QwtPlotCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlot();
    virtual ~RimSummaryPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    void                                            enableShowPlotTitle(bool enable);
    void                                            enableAutoPlotTitle(bool enable);
    bool                                            autoPlotTitle() const;

    void                                            addCurveAndUpdate(RimSummaryCurve* curve);
    void                                            addCurveNoUpdate(RimSummaryCurve* curve);

    //void                                            addEnsembleCurve(const RimEnsembleCurveSet* curveSet, RimSummaryCurve* curve);

    void                                            deleteCurve(RimSummaryCurve* curve);
    void                                            setCurveCollection(RimSummaryCurveCollection* curveCollection);
    void                                            deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase);

    RimEnsembleCurveSetCollection*                  ensembleCurveSets() const;

    void                                            addGridTimeHistoryCurve(RimGridTimeHistoryCurve* curve);

    void                                            addAsciiDataCruve(RimAsciiDataCurve* curve);

    caf::PdmObject*                                 findRimCurveFromQwtCurve(const QwtPlotCurve* curve) const;
    size_t                                          curveCount() const;
    
    void                                            detachAllCurves();
    void                                            updateCaseNameHasChanged();

    void                                            updateAxes();
    virtual void                                    zoomAll() override;
    void                                            setZoomWindow(const QwtInterval& leftAxis,
                                                                  const QwtInterval& rightAxis,
                                                                  const QwtInterval& timeAxis);

    void                                            updateZoomInQwt();
    void                                            updateZoomWindowFromQwt();
    void                                            disableAutoZoom();
    
    bool                                            isLogarithmicScaleEnabled(RiaDefines::PlotAxis plotAxis) const;

    RimSummaryTimeAxisProperties*                   timeAxisProperties();
    time_t                                          firstTimeStepOfFirstCurve();

    void                                            selectAxisInPropertyEditor(int axis);

    virtual QWidget*                                viewWidget() override;

    QString                                         asciiDataForPlotExport() const;

    std::vector<RimSummaryCurve*>                   summaryCurves() const;
    void                                            deleteAllSummaryCurves();
    RimSummaryCurveCollection*                      summaryCurveCollection() const;
    RiuSummaryQwtPlot*                              qwtPlot() const;

    void                                            updatePlotTitle();

    const RimSummaryPlotNameHelper*                 activePlotTitleHelper() const;
    void                                            updateCurveNames();
    QString                                         generatedPlotTitleFromVisibleCurves() const;

    void                                            copyAxisPropertiesFromOther(const RimSummaryPlot& sourceSummaryPlot);

    // RimViewWindow overrides
public:
    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 
    virtual void                                    initAfterRead() override;

private:
    void                                            updateMdiWindowTitle() override;
    QString                                         generatePlotTitle(RimSummaryPlotNameHelper* nameHelper) const;

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() override;
    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    onLoadDataAndUpdate() override;

    virtual QImage                                  snapshotWindowContent() override;

    void                                            setAsCrossPlot();

private:
    std::vector<RimSummaryCurve*>                   visibleSummaryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    std::vector<RimGridTimeHistoryCurve*>           visibleTimeHistoryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    std::vector<RimAsciiDataCurve*>                 visibleAsciiDataCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    bool                                            hasVisibleCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;

    RimSummaryAxisProperties*                       yAxisPropertiesLeftOrRight(RiaDefines::PlotAxis leftOrRightPlotAxis) const;
    void                                            updateAxis(RiaDefines::PlotAxis plotAxis);
    void                                            updateZoomForAxis(RiaDefines::PlotAxis plotAxis);

    void                                            updateTimeAxis();
    void                                            updateBottomXAxis();
    void                                            setZoomIntervalsInQwtPlot();

private:
    caf::PdmField<bool>                                 m_showPlotTitle;
    caf::PdmField<bool>                                 m_showLegend;
    caf::PdmField<int>                                  m_legendFontSize;

    caf::PdmField<bool>                                 m_useAutoPlotTitle;
    caf::PdmField<QString>                              m_userDefinedPlotTitle;
    
    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>   m_gridTimeHistoryCurves;
    caf::PdmChildField<RimSummaryCurveCollection*>        m_summaryCurveCollection;
    caf::PdmChildField<RimEnsembleCurveSetCollection*>  m_ensembleCurveSetCollection;

    caf::PdmChildArrayField<RimAsciiDataCurve*>         m_asciiDataCurves;

    caf::PdmField<bool>                                 m_isAutoZoom;
    caf::PdmChildField<RimSummaryAxisProperties*>       m_leftYAxisProperties;
    caf::PdmChildField<RimSummaryAxisProperties*>       m_rightYAxisProperties;

    caf::PdmChildField<RimSummaryAxisProperties*>       m_bottomAxisProperties;
    caf::PdmChildField<RimSummaryTimeAxisProperties*>   m_timeAxisProperties;

    QPointer<RiuSummaryQwtPlot>                         m_qwtPlot;


    bool                                                m_isCrossPlot;

    std::unique_ptr<RimSummaryPlotNameHelper>           m_nameHelper;

    // Obsolete fields
    caf::PdmChildArrayField<RimSummaryCurve*>                m_summaryCurves_OBSOLETE;
    caf::PdmChildArrayField<RimSummaryCurveFilter_OBSOLETE*> m_curveFilters_OBSOLETE;
};
