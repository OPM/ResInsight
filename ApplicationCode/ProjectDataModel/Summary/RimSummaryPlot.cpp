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

#include "RimSummaryPlot.h"

#include "RiaApplication.h"
#include "RiaSummaryCurveAnalyzer.h"

#include "SummaryPlotCommands/RicSummaryCurveCreator.h"

#include "RimAsciiDataCurve.h"
#include "RimEnsembleCurveSet.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimProject.h"
#include "RimSummaryAxisProperties.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryCurvesCalculator.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryQwtPlot.h"

#include "cvfBase.h"
#include "cvfColor3.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiCheckBoxEditor.h"

#include "qwt_abstract_legend.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_renderer.h"

#include <QDateTime>
#include <QString>
#include <QRectF>

#include <set>


CAF_PDM_SOURCE_INIT(RimSummaryPlot, "SummaryPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot()
{
    CAF_PDM_InitObject("Summary Plot", ":/SummaryPlotLight16x16.png", "", "");

    CAF_PDM_InitField(&m_userDefinedPlotTitle, "PlotDescription", QString("Summary Plot"), "Name", "", "", "");
    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Plot Title", "", "", "");
    m_showPlotTitle.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Legend", "", "", "");
    m_showLegend.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&m_legendFontSize, "LegendFontSize", 11, "Legend Font Size", "", "", "");
    m_showLegend.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Name", "", "", "");
    m_useAutoPlotTitle.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_curveFilters_OBSOLETE, "SummaryCurveFilters", "", "", "", "");
    m_curveFilters_OBSOLETE.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_summaryCurveCollection, "SummaryCurveCollection", "", "", "", "");
    m_summaryCurveCollection.uiCapability()->setUiTreeHidden(true);
    m_summaryCurveCollection = new RimSummaryCurveCollection;

    CAF_PDM_InitFieldNoDefault(&m_ensembleCurveSetCollection, "EnsembleCurveSetCollection", "", "", "", "");
    m_ensembleCurveSetCollection.uiCapability()->setUiTreeHidden(true);
    m_ensembleCurveSetCollection.uiCapability()->setUiHidden(true);
    m_ensembleCurveSetCollection = new RimEnsembleCurveSetCollection();

    CAF_PDM_InitFieldNoDefault(&m_summaryCurves_OBSOLETE, "SummaryCurves", "", "", "", "");
    m_summaryCurves_OBSOLETE.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_gridTimeHistoryCurves, "GridTimeHistoryCurves", "", "", "", "");
    m_gridTimeHistoryCurves.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_asciiDataCurves, "AsciiDataCurves", "", "", "", "");
    m_asciiDataCurves.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_leftYAxisProperties, "LeftYAxisProperties", "Left Y Axis", "", "", "");
    m_leftYAxisProperties.uiCapability()->setUiTreeHidden(true);
    m_leftYAxisProperties = new RimSummaryAxisProperties;
    m_leftYAxisProperties->setNameAndAxis("Left Y-Axis", QwtPlot::yLeft);

    CAF_PDM_InitFieldNoDefault(&m_rightYAxisProperties, "RightYAxisProperties", "Right Y Axis", "", "", "");
    m_rightYAxisProperties.uiCapability()->setUiTreeHidden(true);
    m_rightYAxisProperties = new RimSummaryAxisProperties;
    m_rightYAxisProperties->setNameAndAxis("Right Y-Axis", QwtPlot::yRight);

    CAF_PDM_InitFieldNoDefault(&m_bottomAxisProperties, "BottomAxisProperties", "Bottom X Axis", "", "", "");
    m_bottomAxisProperties.uiCapability()->setUiTreeHidden(true);
    m_bottomAxisProperties = new RimSummaryAxisProperties;
    m_bottomAxisProperties->setNameAndAxis("Bottom X-Axis", QwtPlot::xBottom);

    CAF_PDM_InitFieldNoDefault(&m_timeAxisProperties, "TimeAxisProperties", "Time Axis", "", "", "");
    m_timeAxisProperties.uiCapability()->setUiTreeHidden(true);
    m_timeAxisProperties = new RimSummaryTimeAxisProperties;

    CAF_PDM_InitField(&m_isAutoZoom_OBSOLETE, "AutoZoom", true, "Auto Zoom", "", "", "");
    m_isAutoZoom_OBSOLETE.uiCapability()->setUiHidden(true);
    m_isAutoZoom_OBSOLETE.xmlCapability()->setIOWritable(false);

    m_isCrossPlot = false;

    m_nameHelperAllCurves.reset(new RimSummaryPlotNameHelper);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::~RimSummaryPlot()
{
    removeMdiWindowFromMdiArea();

    deleteViewWidget();

    m_summaryCurves_OBSOLETE.deleteAllChildObjects();
    m_curveFilters_OBSOLETE.deleteAllChildObjects();
    delete m_summaryCurveCollection;
    delete m_ensembleCurveSetCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxes()
{
    updateAxis(RiaDefines::PLOT_AXIS_LEFT);
    updateAxis(RiaDefines::PLOT_AXIS_RIGHT);

    if (m_isCrossPlot)
    {
        updateBottomXAxis();
    }
    else
    {
        updateTimeAxis();
    }

    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isLogarithmicScaleEnabled(RiaDefines::PlotAxis plotAxis) const
{
    return yAxisPropertiesLeftOrRight(plotAxis)->isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties* RimSummaryPlot::timeAxisProperties()
{
    return m_timeAxisProperties();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::selectAxisInPropertyEditor(int axis)
{
    RiuPlotMainWindowTools::showPlotMainWindow();
    if (axis == QwtPlot::yLeft)
    {
        RiuPlotMainWindowTools::selectAsCurrentItem(m_leftYAxisProperties);
    }
    else if (axis == QwtPlot::yRight)
    {
        RiuPlotMainWindowTools::selectAsCurrentItem(m_rightYAxisProperties);
    }
    else if (axis == QwtPlot::xBottom)
    {
        if (m_isCrossPlot)
        {
            RiuPlotMainWindowTools::selectAsCurrentItem(m_bottomAxisProperties);
        }
        else
        {
            RiuPlotMainWindowTools::selectAsCurrentItem(m_timeAxisProperties);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
time_t RimSummaryPlot::firstTimeStepOfFirstCurve()
{
    RimSummaryCurve * firstCurve = nullptr;

    if (m_summaryCurveCollection)
    {
        std::vector<RimSummaryCurve*> curves = m_summaryCurveCollection->curves();
        size_t i = 0;
        while (firstCurve == nullptr && i < curves.size())
        {
            firstCurve = curves[i];
            ++i;
        }
    }

    if (firstCurve && firstCurve->timeStepsY().size() > 0)
    {
        return firstCurve->timeStepsY()[0];
    }
    else return time_t(0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryPlot::viewWidget()
{
    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::asciiDataForPlotExport() const
{
    QString out;

    out += description();

    {
        std::vector<RimSummaryCurve*> curves;
        this->descendantsIncludingThisOfType(curves);

        std::vector<QString> caseNames;
        std::vector<std::vector<time_t> > timeSteps;

        std::vector<std::vector<std::vector<double> > > allCurveData;
        std::vector<std::vector<QString > > allCurveNames;
        //Vectors containing cases - curves - data points/curve name

        for (RimSummaryCurve* curve : curves)
        {
            if (!curve->isCurveVisible()) continue;
            QString curveCaseName = curve->summaryCaseY()->caseName();

            size_t casePosInList = cvf::UNDEFINED_SIZE_T;
            for (size_t i = 0; i < caseNames.size(); i++)
            {
                if (curveCaseName == caseNames[i]) casePosInList = i;
            }

            if (casePosInList == cvf::UNDEFINED_SIZE_T)
            {
                caseNames.push_back(curveCaseName);
            
                std::vector<time_t> curveTimeSteps = curve->timeStepsY();
                timeSteps.push_back(curveTimeSteps);

                std::vector<std::vector<double> > curveDataForCase;
                std::vector<double> curveYData = curve->valuesY();
                curveDataForCase.push_back(curveYData);
                allCurveData.push_back(curveDataForCase);

                std::vector<QString> curveNamesForCase;
                curveNamesForCase.push_back(curve->curveName());
                allCurveNames.push_back(curveNamesForCase);
            }
            else
            {
                std::vector<double> curveYData = curve->valuesY();
                allCurveData[casePosInList].push_back(curveYData);

                QString curveName = curve->curveName();
                allCurveNames[casePosInList].push_back(curveName);
            }
        }

        for (size_t i = 0; i < timeSteps.size(); i++) //cases
        {
            out += "\n\n";
            out += "Case: " + caseNames[i];
            out += "\n";

            for (size_t j = 0; j < timeSteps[i].size(); j++) //time steps & data points
            {
                if (j == 0)
                {
                    out += "Date and time";
                    for (size_t k = 0; k < allCurveNames[i].size(); k++) // curves
                    {
                        out += "\t" + (allCurveNames[i][k]);
                    }
                }
                out += "\n";
                out += QDateTime::fromTime_t(timeSteps[i][j]).toUTC().toString("yyyy-MM-dd hh:mm:ss ");

                for (size_t k = 0; k < allCurveData[i].size(); k++) // curves
                {
                    QString valueText;
                    if (j < allCurveData[i][k].size())
                    {
                        valueText = QString::number(allCurveData[i][k][j], 'g', 6);
                    }
                    out += "\t" + valueText;
                }
            }
        }
    }


    {
        std::vector<QString> caseNames;
        std::vector<std::vector<time_t> > timeSteps;

        std::vector<std::vector<std::vector<double> > > allCurveData;
        std::vector<std::vector<QString > > allCurveNames;
        //Vectors containing cases - curves - data points/curve name

        for (RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves)
        {
            if (!curve->isCurveVisible()) continue;
            QString curveCaseName = curve->caseName();

            size_t casePosInList = cvf::UNDEFINED_SIZE_T;
            for (size_t i = 0; i < caseNames.size(); i++)
            {
                if (curveCaseName == caseNames[i]) casePosInList = i;
            }

            if (casePosInList == cvf::UNDEFINED_SIZE_T)
            {
                caseNames.push_back(curveCaseName);

                std::vector<time_t> curveTimeSteps = curve->timeStepValues();
                timeSteps.push_back(curveTimeSteps);

                std::vector<std::vector<double> > curveDataForCase;
                std::vector<double> curveYData = curve->yValues();
                curveDataForCase.push_back(curveYData);
                allCurveData.push_back(curveDataForCase);

                std::vector<QString> curveNamesForCase;
                curveNamesForCase.push_back(curve->curveName());
                allCurveNames.push_back(curveNamesForCase);
            }
            else
            {
                std::vector<double> curveYData = curve->yValues();
                allCurveData[casePosInList].push_back(curveYData);

                QString curveName = curve->curveName();
                allCurveNames[casePosInList].push_back(curveName);
            }
        }

        for (size_t i = 0; i < timeSteps.size(); i++) //cases
        {
            out += "\n\n";
            out += "Case: " + caseNames[i];
            out += "\n";

            for (size_t j = 0; j < timeSteps[i].size(); j++) //time steps & data points
            {
                if (j == 0)
                {
                    out += "Date and time";
                    for (size_t k = 0; k < allCurveNames[i].size(); k++) // curves
                    {
                        out += "\t" + (allCurveNames[i][k]);
                    }
                }
                out += "\n";
                out += QDateTime::fromTime_t(timeSteps[i][j]).toUTC().toString("yyyy-MM-dd hh:mm:ss ");

                for (size_t k = 0; k < allCurveData[i].size(); k++) // curves
                {
                    out += "\t" + QString::number(allCurveData[i][k][j], 'g', 6);
                }
            }
        }
    }

    {
        std::vector<std::vector<time_t> > timeSteps;

        std::vector<std::vector<std::vector<double> > > allCurveData;
        std::vector<std::vector<QString > > allCurveNames;
        //Vectors containing cases - curves - data points/curve name

        for (RimAsciiDataCurve* curve : m_asciiDataCurves)
        {
            if (!curve->isCurveVisible()) continue;

            size_t casePosInList = cvf::UNDEFINED_SIZE_T;

            if (casePosInList == cvf::UNDEFINED_SIZE_T)
            {
                std::vector<time_t> curveTimeSteps = curve->timeSteps();
                timeSteps.push_back(curveTimeSteps);

                std::vector<std::vector<double> > curveDataForCase;
                std::vector<double> curveYData = curve->yValues();
                curveDataForCase.push_back(curveYData);
                allCurveData.push_back(curveDataForCase);

                std::vector<QString> curveNamesForCase;
                curveNamesForCase.push_back(curve->curveName());
                allCurveNames.push_back(curveNamesForCase);
            }
            else
            {
                std::vector<double> curveYData = curve->yValues();
                allCurveData[casePosInList].push_back(curveYData);

                QString curveName = curve->curveName();
                allCurveNames[casePosInList].push_back(curveName);
            }
        }

        for (size_t i = 0; i < timeSteps.size(); i++) //cases
        {
            out += "\n\n";

            for (size_t j = 0; j < timeSteps[i].size(); j++) //time steps & data points
            {
                if (j == 0)
                {
                    out += "Date and time";
                    for (size_t k = 0; k < allCurveNames[i].size(); k++) // curves
                    {
                        out += "\t" + (allCurveNames[i][k]);
                    }
                }
                out += "\n";
                out += QDateTime::fromTime_t(timeSteps[i][j]).toUTC().toString("yyyy-MM-dd hh:mm:ss ");

                for (size_t k = 0; k < allCurveData[i].size(); k++) // curves
                {
                    out += "\t" + QString::number(allCurveData[i][k][j], 'g', 6);
                }
            }
        }
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::summaryAndEnsembleCurves() const
{
    std::vector<RimSummaryCurve*> curves = summaryCurves();
    
    for (const auto& curveSet : ensembleCurveSetCollection()->curveSets())
    {
        for (const auto& curve : curveSet->curves())
        {
            curves.push_back(curve);
        }
    }
    return curves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::summaryCurves() const
{
    return m_summaryCurveCollection->curves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteAllSummaryCurves()
{
    m_summaryCurveCollection->deleteAllCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection* RimSummaryPlot::summaryCurveCollection() const
{
    return m_summaryCurveCollection(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot* RimSummaryPlot::qwtPlot() const
{
    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updatePlotTitle()
{
    updateNameHelperWithCurveData(m_nameHelperAllCurves.get());

    if (m_useAutoPlotTitle)
    {
        m_userDefinedPlotTitle = m_nameHelperAllCurves->plotTitle();
    }
    
    updateCurveNames();
    updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimSummaryPlotNameHelper* RimSummaryPlot::activePlotTitleHelperAllCurves() const
{
    if (m_useAutoPlotTitle())
    {
        return m_nameHelperAllCurves.get();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::generatedPlotTitleFromAllCurves() const
{
    RimSummaryPlotNameHelper nameHelper;
    updateNameHelperWithCurveData(&nameHelper);
    return nameHelper.plotTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::copyAxisPropertiesFromOther(const RimSummaryPlot& sourceSummaryPlot)
{
    {
        QString data = sourceSummaryPlot.yAxisPropertiesLeftOrRight(RiaDefines::PLOT_AXIS_LEFT)->writeObjectToXmlString();
        yAxisPropertiesLeftOrRight(RiaDefines::PLOT_AXIS_LEFT)
            ->readObjectFromXmlString(data, caf::PdmDefaultObjectFactory::instance());
    }

    {
        QString data = sourceSummaryPlot.yAxisPropertiesLeftOrRight(RiaDefines::PLOT_AXIS_RIGHT)->writeObjectToXmlString();
        yAxisPropertiesLeftOrRight(RiaDefines::PLOT_AXIS_RIGHT)
            ->readObjectFromXmlString(data, caf::PdmDefaultObjectFactory::instance());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAll()
{
    if (qwtPlot())
    {
        updatePlotTitle();
        qwtPlot()->updateLegend();
        updateAxes();
        updateZoomInQwt();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxis(RiaDefines::PlotAxis plotAxis)
{
    if (!m_qwtPlot) return;

    QwtPlot::Axis qwtAxis = QwtPlot::yLeft;
    if (plotAxis == RiaDefines::PLOT_AXIS_LEFT)
    {
        qwtAxis = QwtPlot::yLeft;
    }
    else
    {
        qwtAxis = QwtPlot::yRight;
    }

    RimSummaryAxisProperties* yAxisProperties = yAxisPropertiesLeftOrRight(plotAxis);
    if (yAxisProperties->isActive() && hasVisibleCurvesForAxis(plotAxis))
    {
        m_qwtPlot->enableAxis(qwtAxis, true);

        std::set<QString> timeHistoryQuantities;

        for (auto c : visibleTimeHistoryCurvesForAxis(plotAxis))
        {
            timeHistoryQuantities.insert(c->quantityName());
        }

        RimSummaryPlotYAxisFormatter calc(yAxisProperties,
                                          visibleSummaryCurvesForAxis(plotAxis),
                                          visibleAsciiDataCurvesForAxis(plotAxis),
                                          timeHistoryQuantities);
        calc.applyYAxisPropertiesToPlot(m_qwtPlot);
    }
    else
    {
        m_qwtPlot->enableAxis(qwtAxis, false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomForAxis(RiaDefines::PlotAxis plotAxis)
{
    if (plotAxis == RiaDefines::PLOT_AXIS_BOTTOM)
    {
        if (m_isCrossPlot)
        {
            if (m_bottomAxisProperties->isAutoZoom())
            {
                m_qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
            }
            else
            {
                m_qwtPlot->setAxisScale(QwtPlot::xBottom, m_bottomAxisProperties->visibleRangeMin(), m_bottomAxisProperties->visibleRangeMax());
            }
        }
        else
        {
            if (m_timeAxisProperties->isAutoZoom())
            {
                m_qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
            }
            else
            {
                m_qwtPlot->setAxisScale(QwtPlot::xBottom, m_timeAxisProperties->visibleRangeMin(), m_timeAxisProperties->visibleRangeMax());
            }

        }
    }
    else
    {
        RimSummaryAxisProperties* yAxisProps = yAxisPropertiesLeftOrRight(plotAxis);

        if (yAxisProps->isAutoZoom())
        {
            if (yAxisProps->isLogarithmicScaleEnabled)
            {
                std::vector<double> yValues;
                std::vector<QwtPlotCurve*> plotCurves;

                for (RimSummaryCurve* c : visibleSummaryCurvesForAxis(plotAxis))
                {
                    std::vector<double> curveValues = c->valuesY();
                    yValues.insert(yValues.end(), curveValues.begin(), curveValues.end());
                    plotCurves.push_back(c->qwtPlotCurve());
                }

                for (RimGridTimeHistoryCurve* c : visibleTimeHistoryCurvesForAxis(plotAxis))
                {
                    std::vector<double> curveValues = c->yValues();
                    yValues.insert(yValues.end(), curveValues.begin(), curveValues.end());
                    plotCurves.push_back(c->qwtPlotCurve());
                }

                for (RimAsciiDataCurve* c : visibleAsciiDataCurvesForAxis(plotAxis))
                {
                    std::vector<double> curveValues = c->yValues();
                    yValues.insert(yValues.end(), curveValues.begin(), curveValues.end());
                    plotCurves.push_back(c->qwtPlotCurve());
                }

                double min, max;
                RimSummaryPlotYAxisRangeCalculator calc(plotCurves, yValues);
                calc.computeYRange(&min, &max);

                m_qwtPlot->setAxisScale(yAxisProps->qwtPlotAxisType(), min, max);
            }
            else
            {
                m_qwtPlot->setAxisAutoScale(yAxisProps->qwtPlotAxisType(), true);
            }
        }
        else
        {
            m_qwtPlot->setAxisScale(yAxisProps->qwtPlotAxisType(), yAxisProps->visibleRangeMin(), yAxisProps->visibleRangeMax());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::visibleSummaryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const
{
    std::vector<RimSummaryCurve*> curves;

    if (plotAxis == RiaDefines::PLOT_AXIS_BOTTOM)
    {
        if (m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible())
        {
            for (RimSummaryCurve* curve : m_summaryCurveCollection->curves())
            {
                if (curve->isCurveVisible())
                {
                    curves.push_back(curve);
                }
            }
        }
    }
    else
    {
        if (m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible())
        {
            for (RimSummaryCurve* curve : m_summaryCurveCollection->curves())
            {
                if (curve->isCurveVisible() && curve->axisY() == plotAxis)
                {
                    curves.push_back(curve);
                }
            }
        }

        if (m_ensembleCurveSetCollection && m_ensembleCurveSetCollection->isCurveSetsVisible())
        {
            for (RimEnsembleCurveSet* curveSet : m_ensembleCurveSetCollection->curveSets())
            {
                for (RimSummaryCurve* curve : curveSet->curves())
                {
                    if (curve->isCurveVisible() && curve->axisY() == plotAxis)
                    {
                        curves.push_back(curve);
                    }
                }
            }
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::hasVisibleCurvesForAxis(RiaDefines::PlotAxis plotAxis) const
{
    if (visibleSummaryCurvesForAxis(plotAxis).size() > 0)
    {
        return true;
    }

    if (visibleTimeHistoryCurvesForAxis(plotAxis).size() > 0)
    {
        return true;
    }

    if (visibleAsciiDataCurvesForAxis(plotAxis).size() > 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryAxisProperties* RimSummaryPlot::yAxisPropertiesLeftOrRight(RiaDefines::PlotAxis leftOrRightPlotAxis) const
{
    RimSummaryAxisProperties* yAxisProps = nullptr;

    if (leftOrRightPlotAxis == RiaDefines::PLOT_AXIS_LEFT)
    {
        yAxisProps = m_leftYAxisProperties();
    }
    else
    {
        yAxisProps = m_rightYAxisProperties();
    }

    CVF_ASSERT(yAxisProps);

    return yAxisProps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimGridTimeHistoryCurve*> RimSummaryPlot::visibleTimeHistoryCurvesForAxis(RiaDefines::PlotAxis plotAxis) const
{
    std::vector<RimGridTimeHistoryCurve*> curves;

    for (auto c : m_gridTimeHistoryCurves)
    {
        if (c->isCurveVisible())
        {
            if (c->yAxis() == plotAxis || plotAxis == RiaDefines::PLOT_AXIS_BOTTOM)
            {
                curves.push_back(c);
            }
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimAsciiDataCurve*> RimSummaryPlot::visibleAsciiDataCurvesForAxis(RiaDefines::PlotAxis plotAxis) const
{
    std::vector<RimAsciiDataCurve*> curves;

    for (auto c : m_asciiDataCurves)
    {
        if (c->isCurveVisible())
        {
            if (c->yAxis() == plotAxis || plotAxis == RiaDefines::PLOT_AXIS_BOTTOM)
            {
                curves.push_back(c);
            }
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateTimeAxis()
{
    if (!m_qwtPlot) return;

    if (!m_timeAxisProperties->isActive())
    {
        m_qwtPlot->enableAxis(QwtPlot::xBottom, false);

        return;
    }

    if (m_timeAxisProperties->timeMode() == RimSummaryTimeAxisProperties::DATE)
    {
        m_qwtPlot->useDateBasedTimeAxis();
    }
    else 
    {
        m_qwtPlot->useTimeBasedTimeAxis();
    }   

    m_qwtPlot->enableAxis(QwtPlot::xBottom, true);

    {
        QString axisTitle;
        if (m_timeAxisProperties->showTitle) axisTitle = m_timeAxisProperties->title();

        QwtText timeAxisTitle = m_qwtPlot->axisTitle(QwtPlot::xBottom);

        QFont font = timeAxisTitle.font();
        font.setBold(true);
        font.setPixelSize(m_timeAxisProperties->titleFontSize);
        timeAxisTitle.setFont(font);

        timeAxisTitle.setText(axisTitle);

        switch ( m_timeAxisProperties->titlePositionEnum() )
        {
            case RimSummaryTimeAxisProperties::AXIS_TITLE_CENTER:
            timeAxisTitle.setRenderFlags(Qt::AlignCenter);
            break;
            case RimSummaryTimeAxisProperties::AXIS_TITLE_END:
            timeAxisTitle.setRenderFlags(Qt::AlignRight);
            break;
        }

        m_qwtPlot->setAxisTitle(QwtPlot::xBottom, timeAxisTitle);
    }

    {
        QFont timeAxisFont = m_qwtPlot->axisFont(QwtPlot::xBottom);
        timeAxisFont.setBold(false);
        timeAxisFont.setPixelSize(m_timeAxisProperties->valuesFontSize);
        m_qwtPlot->setAxisFont(QwtPlot::xBottom, timeAxisFont);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateBottomXAxis()
{
    if (!m_qwtPlot) return;

    QwtPlot::Axis qwtAxis = QwtPlot::xBottom;

    RimSummaryAxisProperties* bottomAxisProperties = m_bottomAxisProperties();

    if (bottomAxisProperties->isActive())
    {
        m_qwtPlot->enableAxis(qwtAxis, true);

        std::set<QString> timeHistoryQuantities;

        RimSummaryPlotYAxisFormatter calc(bottomAxisProperties,
                                          visibleSummaryCurvesForAxis(RiaDefines::PLOT_AXIS_BOTTOM),
                                          visibleAsciiDataCurvesForAxis(RiaDefines::PLOT_AXIS_BOTTOM),
                                          timeHistoryQuantities);
        calc.applyYAxisPropertiesToPlot(m_qwtPlot);
    }
    else
    {
        m_qwtPlot->enableAxis(qwtAxis, false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateCaseNameHasChanged()
{
    if (m_summaryCurveCollection)
    {
        m_summaryCurveCollection->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::zoomAll()
{
    setAutoZoomForAllAxes(true);
    updateZoomInQwt();
    updateAxisRangesFromQwt();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveAndUpdate(RimSummaryCurve* curve)
{
    if (curve)
    {
        m_summaryCurveCollection->addCurve(curve);

        if (m_qwtPlot)
        {
            curve->setParentQwtPlotAndReplot(m_qwtPlot);
            this->updateAxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveNoUpdate(RimSummaryCurve* curve)
{
    if (curve)
    {
        m_summaryCurveCollection->addCurve(curve);

        if (m_qwtPlot)
        {
            curve->setParentQwtPlotNoReplot(m_qwtPlot);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteCurve(RimSummaryCurve* curve)
{
    if (curve)
    {
        if (m_summaryCurveCollection)
        {
            for (auto& c : m_summaryCurveCollection->curves())
            {
                if (c == curve)
                {
                    m_summaryCurveCollection->deleteCurve(curve);
                    return;
                }
            }
        }
        if (m_ensembleCurveSetCollection)
        {
            for (auto& curveSet : m_ensembleCurveSetCollection->curveSets())
            {
                for (auto& c : curveSet->curves())
                {
                    if (c == curve)
                    {
                        curveSet->deleteCurve(curve);
                        if (curveSet->curves().empty())
                        {
                            if (curveSet->colorMode() == RimEnsembleCurveSet::BY_ENSEMBLE_PARAM)
                            {
                                qwtPlot()->removeEnsembleCurveSetLegend(curveSet);
                            }
                            m_ensembleCurveSetCollection->deleteCurveSet(curveSet);
                        }
                        return;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase)
{
    if (m_summaryCurveCollection)
    {
        m_summaryCurveCollection->deleteCurvesAssosiatedWithCase(summaryCase);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSetCollection* RimSummaryPlot::ensembleCurveSetCollection() const
{
    return m_ensembleCurveSetCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setCurveCollection(RimSummaryCurveCollection* curveCollection)
{
    if (curveCollection)
    {
        // Delete current curve coll ?


        m_summaryCurveCollection = curveCollection;
        if (m_qwtPlot)
        {
            m_summaryCurveCollection->setParentQwtPlotAndReplot(m_qwtPlot);
            this->updateAxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addGridTimeHistoryCurve(RimGridTimeHistoryCurve* curve)
{
    CVF_ASSERT(curve);

    m_gridTimeHistoryCurves.push_back(curve);
    if (m_qwtPlot)
    {
        curve->setParentQwtPlotAndReplot(m_qwtPlot);
        this->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addAsciiDataCruve(RimAsciiDataCurve* curve)
{
    CVF_ASSERT(curve);

    m_asciiDataCurves.push_back(curve);
    if (m_qwtPlot)
    {
        curve->setParentQwtPlotAndReplot(m_qwtPlot);
        this->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryPlot::userDescriptionField()
{
    return &m_userDefinedPlotTitle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_legendFontSize)
    {
        std::vector<int> fontSizes;
        fontSizes.push_back(8);
        fontSizes.push_back(9);
        fontSizes.push_back(10);
        fontSizes.push_back(11);
        fontSizes.push_back(12);
        fontSizes.push_back(14);
        fontSizes.push_back(16);
        fontSizes.push_back(18);
        fontSizes.push_back(24);

        for (int value : fontSizes)
        {
            QString text = QString("%1").arg(value);
            options.push_back(caf::PdmOptionItemInfo(text, value));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_userDefinedPlotTitle || 
        changedField == &m_showPlotTitle ||
        changedField == &m_showLegend ||
        changedField == &m_legendFontSize || 
        changedField == &m_useAutoPlotTitle)
    {
        updatePlotTitle();
        updateConnectedEditors();
    }

    if (changedField == &m_useAutoPlotTitle && !m_useAutoPlotTitle)
    {
        // When auto name of plot is turned off, update the auto name for all curves

        for (auto c : summaryCurves())
        {
            c->updateCurveNameNoLegendUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimSummaryPlot::snapshotWindowContent()
{
#if 0
// This does not work with the color legend widgets. Is there a reason for doing this, and not to grab the widget ?
    QImage image;

    if (m_qwtPlot)
    {
        image = QImage(m_qwtPlot->size(), QImage::Format_ARGB32);
        image.fill(QColor(Qt::white).rgb());

        QPainter painter(&image);
        QRectF rect(0, 0, m_qwtPlot->size().width(), m_qwtPlot->size().height());

        QwtPlotRenderer plotRenderer;
        plotRenderer.render(m_qwtPlot, &painter, rect);
    }

    return image;
    #endif
    QImage image;

    if (m_qwtPlot)
    {
        QPixmap pix = QPixmap::grabWidget(m_qwtPlot);
        image = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (uiConfigName == RicSummaryCurveCreator::CONFIGURATION_NAME)
    {
        uiTreeOrdering.add(&m_summaryCurveCollection);
        if (!m_isCrossPlot)
        {
            uiTreeOrdering.add(&m_ensembleCurveSetCollection);
        }
    }
    else
    {
        caf::PdmUiTreeOrdering* axisFolder = uiTreeOrdering.add("Axes", ":/Axes16x16.png");

        if (m_isCrossPlot)
        {
            axisFolder->add(&m_bottomAxisProperties);
        }
        else
        {
            axisFolder->add(&m_timeAxisProperties);
        }
        axisFolder->add(&m_leftYAxisProperties);
        axisFolder->add(&m_rightYAxisProperties);

        uiTreeOrdering.add(&m_summaryCurveCollection);
        if (!m_isCrossPlot)
        {
            uiTreeOrdering.add(&m_ensembleCurveSetCollection);
        }
        uiTreeOrdering.add(&m_gridTimeHistoryCurves);
        uiTreeOrdering.add(&m_asciiDataCurves);
    }

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onLoadDataAndUpdate()
{
    updatePlotTitle();

    updateMdiWindowVisibility();    

    if (m_summaryCurveCollection)
    {
        m_summaryCurveCollection->loadDataAndUpdate(false);
    }
 
    m_ensembleCurveSetCollection->loadDataAndUpdate(false);

    for (RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves)
    {
        curve->loadDataAndUpdate(false);
    }

    for (RimAsciiDataCurve* curve : m_asciiDataCurves)
    {
        curve->loadDataAndUpdate(false);
    }

    if (m_qwtPlot) m_qwtPlot->updateLegend();
    this->updateAxes();
    updateZoomInQwt();
    if(m_qwtPlot) m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomInQwt()
{
    if (m_qwtPlot)
    {

        updateZoomForAxis(RiaDefines::PLOT_AXIS_BOTTOM);
        updateZoomForAxis(RiaDefines::PLOT_AXIS_LEFT);
        updateZoomForAxis(RiaDefines::PLOT_AXIS_RIGHT);

        m_qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomWindowFromQwt()
{
    updateAxisRangesFromQwt();
    setAutoZoomForAllAxes(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxisRangesFromQwt()
{
    if (!m_qwtPlot) return;

    QwtInterval leftAxis, rightAxis, timeAxis;
    m_qwtPlot->currentVisibleWindow(&leftAxis, &rightAxis, &timeAxis);

    m_leftYAxisProperties->visibleRangeMax = leftAxis.maxValue();
    m_leftYAxisProperties->visibleRangeMin = leftAxis.minValue();
    m_leftYAxisProperties->updateConnectedEditors();

    m_rightYAxisProperties->visibleRangeMax = rightAxis.maxValue();
    m_rightYAxisProperties->visibleRangeMin = rightAxis.minValue();
    m_rightYAxisProperties->updateConnectedEditors();

    if (m_isCrossPlot)
    {
        m_bottomAxisProperties->visibleRangeMax = timeAxis.maxValue();
        m_bottomAxisProperties->visibleRangeMin = timeAxis.minValue();
        m_bottomAxisProperties->updateConnectedEditors();
    }
    else
    {
        m_timeAxisProperties->setVisibleRangeMin(timeAxis.minValue());
        m_timeAxisProperties->setVisibleRangeMax(timeAxis.maxValue());
        m_timeAxisProperties->updateConnectedEditors();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setAutoZoomForAllAxes(bool enableAutoZoom)
{
    m_leftYAxisProperties->setAutoZoom(enableAutoZoom);
    m_rightYAxisProperties->setAutoZoom(enableAutoZoom);

    if (m_isCrossPlot)
    {
        m_bottomAxisProperties->setAutoZoom(enableAutoZoom);
    }
    else
    {
        m_timeAxisProperties->setAutoZoom(enableAutoZoom);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setDescription(const QString& description)
{
    m_userDefinedPlotTitle = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::description() const
{
    return m_userDefinedPlotTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::enableShowPlotTitle(bool enable)
{
    m_showPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::enableAutoPlotTitle(bool enable)
{
    m_useAutoPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::autoPlotTitle() const
{
    return m_useAutoPlotTitle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setAsCrossPlot()
{
    m_isCrossPlot = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_showPlotTitle);
    uiOrdering.add(&m_useAutoPlotTitle);
    uiOrdering.add(&m_userDefinedPlotTitle);
    uiOrdering.add(&m_showLegend);

    if (m_showLegend())
    {
        uiOrdering.add(&m_legendFontSize);
    }

    m_userDefinedPlotTitle.uiCapability()->setUiReadOnly(m_useAutoPlotTitle);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryPlot::createViewWidget(QWidget* mainWindowParent)
{
    if (!m_qwtPlot)
    {
        m_qwtPlot = new RiuSummaryQwtPlot(this, mainWindowParent);

        for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
        {
            curve->setParentQwtPlotNoReplot(m_qwtPlot);
        }

        for (RimAsciiDataCurve* curve : m_asciiDataCurves)
        {
            curve->setParentQwtPlotNoReplot(m_qwtPlot);
        }

        if ( m_summaryCurveCollection )
        {
            m_summaryCurveCollection->setParentQwtPlotAndReplot(m_qwtPlot);
        }

        if (m_ensembleCurveSetCollection)
        {
            m_ensembleCurveSetCollection->setParentQwtPlotAndReplot(m_qwtPlot);
        }
   }

    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteViewWidget()
{
    detachAllCurves();

    if (m_qwtPlot)
    {
        m_qwtPlot->deleteLater();
        m_qwtPlot = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::initAfterRead()
{
    // Move summary curves from obsolete storage to the new curve collection
    std::vector<RimSummaryCurve*> curvesToMove;

    for (auto& curveFilter : m_curveFilters_OBSOLETE)
    {
        const auto& tmpCurves = curveFilter->curves();
        curvesToMove.insert(curvesToMove.end(), tmpCurves.begin(), tmpCurves.end());
        curveFilter->clearCurvesWithoutDelete();
    }
    m_curveFilters_OBSOLETE.clear();

    curvesToMove.insert(curvesToMove.end(), m_summaryCurves_OBSOLETE.begin(), m_summaryCurves_OBSOLETE.end());
    m_summaryCurves_OBSOLETE.clear();

    for (const auto& curve : curvesToMove)
    {
        m_summaryCurveCollection->addCurve(curve);
    }

    if (!m_isAutoZoom_OBSOLETE())
    {
        setAutoZoomForAllAxes(false);
    }

    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfType(proj);
    if (proj)
    {
        if (proj->isProjectFileVersionEqualOrOlderThan("2017.0.0"))
        {
            m_useAutoPlotTitle = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateMdiWindowTitle()
{
    if (m_qwtPlot)
    {
        QString plotTitle = description();

        m_qwtPlot->setWindowTitle(plotTitle);

        if (m_showPlotTitle)
        {
            m_qwtPlot->setTitle(plotTitle);
        }
        else
        {
            m_qwtPlot->setTitle("");
        }

        if (m_showLegend)
        {
            // Will be released in plot destructor or when a new legend is set
            QwtLegend* legend = new QwtLegend(m_qwtPlot);

            auto font = legend->font();
            font.setPixelSize(m_legendFontSize());
            legend->setFont(font);
            m_qwtPlot->insertLegend(legend, QwtPlot::BottomLegend);
        }
        else
        {
            m_qwtPlot->insertLegend(nullptr);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateNameHelperWithCurveData(RimSummaryPlotNameHelper* nameHelper) const
{
    if (!nameHelper) return;

    nameHelper->clear();
    std::vector<RifEclipseSummaryAddress> addresses;
    std::vector<RimSummaryCase*>          sumCases;
    std::vector<RimSummaryCaseCollection*>  ensembleCases;

    if (m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible())
    {
        for (RimSummaryCurve* curve : m_summaryCurveCollection->curves())
        {
            addresses.push_back(curve->summaryAddressY());
            sumCases.push_back(curve->summaryCaseY());

            if (curve->summaryCaseX())
            {
                sumCases.push_back(curve->summaryCaseX());

                if (curve->summaryAddressX().category() != RifEclipseSummaryAddress::SUMMARY_INVALID)
                {
                    addresses.push_back(curve->summaryAddressX());
                }
            }
        }
    }

    for (auto curveSet : m_ensembleCurveSetCollection->curveSets())
    {
        addresses.push_back(curveSet->summaryAddress());
        ensembleCases.push_back(curveSet->summaryCaseCollection());
    }

    nameHelper->clear();
    nameHelper->appendAddresses(addresses);
    nameHelper->appendSummaryCases(sumCases);
    nameHelper->appendEnsembleCases(ensembleCases);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateCurveNames()
{
    if (m_summaryCurveCollection->isCurvesVisible())
    {
        for (auto c : summaryCurves())
        {
            c->updateCurveNameNoLegendUpdate();            
        }
    }

    for (auto curveSet : m_ensembleCurveSetCollection->curveSets())
    {
        curveSet->updateEnsembleLegendItem();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::detachAllCurves()
{
    if (m_summaryCurveCollection)
    {
        m_summaryCurveCollection->detachQwtCurves();
    }

    m_ensembleCurveSetCollection->detachQwtCurves();

    for (RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves)
    {
        curve->detachQwtCurve();
    }

    for (RimAsciiDataCurve* curve : m_asciiDataCurves)
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimSummaryPlot::findRimPlotObjectFromQwtCurve(const QwtPlotCurve* qwtCurve) const
{
    for (RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves)
    {
        if (curve->qwtPlotCurve() == qwtCurve)
        {
            return curve;
        }
    }

    for (RimAsciiDataCurve* curve : m_asciiDataCurves)
    {
        if (curve->qwtPlotCurve() == qwtCurve)
        {
            return curve;
        }
    }

    if (m_summaryCurveCollection)
    {
        RimSummaryCurve* foundCurve = m_summaryCurveCollection->findRimCurveFromQwtCurve(qwtCurve);
        
        if (foundCurve)
        {
            m_summaryCurveCollection->setCurrentSummaryCurve(foundCurve);

            return foundCurve;
        }
    }

    if (m_ensembleCurveSetCollection)
    {
        RimEnsembleCurveSet* foundCurveSet = m_ensembleCurveSetCollection->findRimCurveSetFromQwtCurve(qwtCurve);

        if (foundCurveSet)
        {
            m_ensembleCurveSetCollection->setCurrentSummaryCurveSet(foundCurveSet);

            return foundCurveSet;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimSummaryPlot::curveCount() const
{
    return m_summaryCurveCollection->curves().size() + m_gridTimeHistoryCurves.size() + m_asciiDataCurves.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &m_showLegend || 
        field == &m_showPlotTitle ||
        field == &m_useAutoPlotTitle)
    {
        caf::PdmUiCheckBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_useNativeCheckBoxLabel = true;
        }
    }
}
