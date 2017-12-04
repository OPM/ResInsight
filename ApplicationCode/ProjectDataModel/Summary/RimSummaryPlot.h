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
#include "RimViewWindow.h"

#include <QPointer>

class PdmUiTreeOrdering;
class RimAsciiDataCurve;
class RimGridTimeHistoryCurve;
class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryCurveCollection;
class RimSummaryCurveFilter_OBSOLETE;
class RimSummaryTimeAxisProperties;
class RimSummaryAxisProperties;
class RiuSummaryQwtPlot;

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
    void                                            setShowDescription(bool showDescription);
    void                                            enableAutoName(bool enable);

    void                                            addCurveAndUpdate(RimSummaryCurve* curve);
    void                                            addCurveNoUpdate(RimSummaryCurve* curve);

    void                                            deleteCurve(RimSummaryCurve* curve);
    void                                            setCurveCollection(RimSummaryCurveCollection* curveCollection);
    void                                            deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase);
    //void                                            deleteAllTopLevelCurves();

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

    // RimViewWindow overrides
public:
    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 
    virtual void                                    initAfterRead() override;

private:
    void                                            updateMdiWindowTitle() override;

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField();
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);
    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    onLoadDataAndUpdate() override;

    virtual QImage                                  snapshotWindowContent() override;

    void                                            setAsCrossPlot();



private:
    std::vector<RimSummaryCurve*>                   visibleSummaryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    std::vector<RimGridTimeHistoryCurve*>           visibleTimeHistoryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    std::vector<RimAsciiDataCurve*>                 visibleAsciiDataCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    bool                                            hasVisibleCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;

    RimSummaryAxisProperties*                       yAxisPropertiesForAxis(RiaDefines::PlotAxis plotAxis) const;
    void                                            updateAxis(RiaDefines::PlotAxis plotAxis);
    void                                            updateZoomForAxis(RiaDefines::PlotAxis plotAxis);

    void                                            updateTimeAxis();
    void                                            updateBottomXAxis();
    void                                            setZoomIntervalsInQwtPlot();

private:
    caf::PdmField<bool>                                 m_showPlotTitle;
    caf::PdmField<bool>                                 m_showLegend;

    caf::PdmField<bool>                                 m_isUsingAutoName;
    caf::PdmField<QString>                              m_userDefinedPlotTitle;
    
    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>   m_gridTimeHistoryCurves;
	caf::PdmChildField<RimSummaryCurveCollection*>		m_summaryCurveCollection;
    caf::PdmChildArrayField<RimAsciiDataCurve*>         m_asciiDataCurves;

    caf::PdmField<bool>                                 m_isAutoZoom;
    caf::PdmChildField<RimSummaryAxisProperties*>       m_leftYAxisProperties;
    caf::PdmChildField<RimSummaryAxisProperties*>       m_rightYAxisProperties;

    caf::PdmChildField<RimSummaryAxisProperties*>       m_bottomAxisProperties;
    caf::PdmChildField<RimSummaryTimeAxisProperties*>   m_timeAxisProperties;

    QPointer<RiuSummaryQwtPlot>                         m_qwtPlot;

    caf::PdmChildArrayField<RimSummaryCurve*>           m_summaryCurves_OBSOLETE;
    caf::PdmChildArrayField<RimSummaryCurveFilter_OBSOLETE*> m_curveFilters_OBSOLETE;

    bool                                                m_isCrossPlot;
};
