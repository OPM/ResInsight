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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"
#include "cafPdmChildField.h"

#include "RiaDefines.h"
#include "RimViewWindow.h"

#include <QPointer>

class RiuSummaryQwtPlot;
class RimSummaryCurve;
class RimSummaryCurveFilter;
class RimSummaryYAxisProperties;
class RimSummaryTimeAxisProperties;
class RimGridTimeHistoryCurve;
class PdmUiTreeOrdering;

class QwtPlotCurve;
class QwtInterval;

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

    void                                            addCurve(RimSummaryCurve* curve);
    void                                            addCurveFilter(RimSummaryCurveFilter* curveFilter);

    void                                            addGridTimeHistoryCurve(RimGridTimeHistoryCurve* curve);

    caf::PdmObject*                                 findRimCurveFromQwtCurve(const QwtPlotCurve* curve) const;
    size_t                                          curveCount() const;
    
    virtual void                                    loadDataAndUpdate() override;

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

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

    virtual QImage                                  snapshotWindowContent() override;

private:
    std::vector<RimSummaryCurve*>                   visibleSummaryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    std::vector<RimGridTimeHistoryCurve*>           visibleTimeHistoryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;
    bool                                            hasVisibleCurvesForAxis(RiaDefines::PlotAxis plotAxis) const;

    RimSummaryYAxisProperties*                      yAxisPropertiesForAxis(RiaDefines::PlotAxis plotAxis) const;
    void                                            updateAxis(RiaDefines::PlotAxis plotAxis);
    void                                            updateZoomForAxis(RiaDefines::PlotAxis plotAxis);

    void                                            updateTimeAxis();
    void                                            setZoomIntervalsInQwtPlot();

    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    void                                            updateMdiWindowTitle() override;
    virtual void                                    deleteViewWidget() override; 

private:
    caf::PdmField<bool>                                 m_showPlotTitle;
    caf::PdmField<bool>                                 m_showLegend;
    caf::PdmField<QString>                              m_userName;
    
    caf::PdmChildArrayField<RimGridTimeHistoryCurve*>   m_gridTimeHistoryCurves;
    caf::PdmChildArrayField<RimSummaryCurve*>           m_summaryCurves;
    caf::PdmChildArrayField<RimSummaryCurveFilter*>     m_curveFilters;

    caf::PdmField<bool>                                 m_isAutoZoom;
    caf::PdmChildField<RimSummaryYAxisProperties*>      m_leftYAxisProperties;
    caf::PdmChildField<RimSummaryYAxisProperties*>      m_rightYAxisProperties;
    caf::PdmChildField<RimSummaryTimeAxisProperties*>   m_timeAxisProperties;

    QPointer<RiuSummaryQwtPlot>                         m_qwtPlot;
};
