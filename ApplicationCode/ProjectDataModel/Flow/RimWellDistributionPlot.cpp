/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellDistributionPlot.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimFlowDiagSolution.h"

#include "RigEclipseCaseData.h"
#include "RigTofWellDistributionCalculator.h"

#include "RiaColorTools.h"

#include "RiuQwtPlotTools.h"

#include "qwt_plot.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"

#include <QWidget>
#include <QGridLayout>
#include <QTextBrowser>

#include "cvfBase.h"
#include "cvfTrace.h"
#include "cvfDebugTimer.h"


//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellDistributionPlot, "WellDistributionPlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlot::RimWellDistributionPlot()
{
    cvf::Trace::show("RimWellDistributionPlot::RimWellDistributionPlot()");

    CAF_PDM_InitObject("Well Distribution Plot", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "Case", "Case", "", "", "");
    CAF_PDM_InitField(&m_timeStepIndex, "TimeStepIndex", -1, "Time Step", "", "", "");
    CAF_PDM_InitField(&m_wellName, "WellName", QString("None"), "Well", "", "", "");

    m_showWindow = false;

    setAsPlotMdiWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlot::~RimWellDistributionPlot() 
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellDistributionPlot::viewWidget()
{
    cvf::Trace::show("RimWellDistributionPlot::viewWidget()");
    return m_myViewWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellDistributionPlot::snapshotWindowContent()
{
    cvf::Trace::show("RimWellDistributionPlot::snapshotWindowContent()");
    return QImage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::zoomAll()
{
    cvf::Trace::show("RimWellDistributionPlot::zoomAll()");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellDistributionPlot::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellDistributionPlot::createViewWidget(QWidget* mainWindowParent)
{
    cvf::Trace::show("RimWellDistributionPlot::createViewWidget()");

    m_myViewWidget = new QWidget(mainWindowParent);

    QGridLayout* gridLayout = new QGridLayout(m_myViewWidget);

    m_textBrowser = new QTextBrowser;
    m_textBrowser->setText("<center><h1>NotYet</h1></center>");
    gridLayout->addWidget(m_textBrowser, 0, 0);

    m_plotWidgets[0] = constructNewPlotWidget();
    m_plotWidgets[1] = constructNewPlotWidget();
    m_plotWidgets[2] = constructNewPlotWidget();

    gridLayout->addWidget(m_plotWidgets[0], 1, 0);
    gridLayout->addWidget(m_plotWidgets[1], 0, 1);
    gridLayout->addWidget(m_plotWidgets[2], 1, 1);

    onLoadDataAndUpdate();

    return m_myViewWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlot* RimWellDistributionPlot::constructNewPlotWidget()
{
    class MyQwtPlot : public QwtPlot
    {
    public:
        MyQwtPlot(QWidget* parent) : QwtPlot(parent) {}
        QSize sizeHint() const override              { return QSize(100, 100); }
        QSize minimumSizeHint() const override       { return QSize(0, 0); }
    };

    QwtPlot* plotWidget = new MyQwtPlot(nullptr);
    RiuQwtPlotTools::setCommonPlotBehaviour(plotWidget);

    QwtLegend* legend = new QwtLegend(plotWidget);
    plotWidget->insertLegend(legend, QwtPlot::BottomLegend);

    return plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::deleteViewWidget()
{
    cvf::Trace::show("RimWellDistributionPlot::deleteViewWidget()");

    if (m_myViewWidget)
    {
        m_myViewWidget->deleteLater();
        m_myViewWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::populatePlotWidgetWithCurveData(const RigTofWellDistributionCalculator& calculator, RiaDefines::PhaseType phase, const RimFlowDiagSolution& flowDiagSolution, QwtPlot* plotWidget)
{
    cvf::Trace::show("RimWellDistributionPlot::populatePlotWidgetWithCurves(phase=%d '%s')", phase, phase == RiaDefines::OIL_PHASE ? "oil" : (phase == RiaDefines::GAS_PHASE ? "gas" : "water"));

    // Currently select this value so that the grid appears on top of the curves
    const double baseCurveZValue = 9.5;

    plotWidget->detachItems(QwtPlotItem::Rtti_PlotCurve);
    plotWidget->setAxisScale(QwtPlot::xBottom, 0, 1);
    plotWidget->setAxisScale(QwtPlot::yLeft, 0, 1);
    plotWidget->setAxisAutoScale(QwtPlot::xBottom, true);
    plotWidget->setAxisAutoScale(QwtPlot::yLeft, true);

    const std::vector<double>& tofValuesDays = calculator.sortedUniqueTOFValues();
    if (tofValuesDays.size() == 0)
    {
        cvf::Trace::show("No TOF values!");
        return;
    }

    std::vector<double> tofValuesYears;
    for (double tofDays : tofValuesDays)
    {
        const double tofYears = tofDays/365.2425;
        tofValuesYears.push_back(tofYears);
    }

    cvf::Trace::show("numTofValues: %d  (min, max: %f, %f)", static_cast<int>(tofValuesYears.size()), tofValuesYears.front(), tofValuesYears.back());

    const size_t numWells = calculator.contributingWellCount();
    cvf::Trace::show("numContribWells: %d", static_cast<int>(numWells));

    std::vector<double> yVals(tofValuesYears.size(), 0);

    for (size_t i = 0; i < numWells; i++)
    {
        QString wellName = calculator.contributingWellName(i);
        const std::vector<double>&  volArr = calculator.accumulatedPhaseVolumeForContributingWell(phase, i);

        cvf::Color3f cvfClr = flowDiagSolution.tracerColor(wellName);
        QColor qtClr = RiaColorTools::toQColor(cvfClr);

        cvf::Trace::show("wellName  min, max: %s   %f, %f", wellName.toStdString().c_str(), volArr.front(), volArr.back());

        for (size_t j = 0; j < yVals.size(); j++)
        {
            yVals[j] += volArr[j];
        }

        QwtPlotCurve* curve = new QwtPlotCurve;
        curve->setTitle(wellName);
        curve->setBrush(qtClr);
        curve->setZ(baseCurveZValue - i * 0.0001);
        curve->setSamples(tofValuesYears.data(), yVals.data(), static_cast<int>(tofValuesYears.size()));
        curve->attach(plotWidget);
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::onLoadDataAndUpdate()
{
    cvf::Trace::show("RimWellDistributionPlot::onLoadDataAndUpdate()");
    cvf::DebugTimer tim("RimWellDistributionPlot::onLoadDataAndUpdate()");

    updateMdiWindowVisibility();

    if (!m_myViewWidget)
    {
        return;
    }

    CVF_ASSERT(m_textBrowser);

    QString str = "<center><h1>MyViewWidget</h1></center>";
    str += "<br><br>";

    const QString caseName = m_case ? m_case->caseUserDescription() : "N/A";

    str += QString("case: %1<br>").arg(caseName);
    str += QString("timeStepIndex: %1<br>").arg(m_timeStepIndex);
    str += QString("wellName: %1<br>").arg(m_wellName);

    m_textBrowser->setText(str);


    for (QwtPlot* plotWidget : m_plotWidgets)
    {
        plotWidget->detachItems(QwtPlotItem::Rtti_PlotCurve);
    }

    if (m_case)
    {
        tim.reportLapTimeMS("about to start calc");
        RigTofWellDistributionCalculator calc(m_case, m_wellName, m_timeStepIndex);
        tim.reportLapTimeMS("calc");

        const QString timeStepName = m_case->timeStepName(m_timeStepIndex);

        const RimFlowDiagSolution* flowDiagSolution = m_case->defaultFlowDiagSolution();
        for (int i = 0; i < m_plotWidgets.size(); i++)
        {
            const RiaDefines::PhaseType phase = static_cast<RiaDefines::PhaseType>(i);

            QwtPlot* plotWidget = m_plotWidgets[i];
            populatePlotWidgetWithCurveData(calc, phase, *flowDiagSolution, plotWidget);
        
            QString phaseString = "N/A";
            if      (phase == RiaDefines::OIL_PHASE)   phaseString = "Oil";
            else if (phase == RiaDefines::GAS_PHASE)   phaseString = "Gas";
            else if (phase == RiaDefines::WATER_PHASE) phaseString = "Water";

            plotWidget->setAxisTitle(QwtPlot::xBottom, "TOF [years]");
            plotWidget->setAxisTitle(QwtPlot::yLeft, "Reservoir Volume [m3]");

            const QString plotTitle = QString("%1 Distribution: %2, %3").arg(phaseString).arg(m_wellName).arg(timeStepName);
            plotWidget->setTitle(plotTitle);
        }
    }

    for (QwtPlot* plotWidget : m_plotWidgets)
    {
        plotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* group = uiOrdering.addNewGroup("Plot Data");
    group->add(&m_case);
    group->add(&m_timeStepIndex);
    group->add(&m_wellName);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellDistributionPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* /*useOptionsOnly*/)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_case)
    {
        RimProject* ownerProj = nullptr;
        firstAncestorOrThisOfType(ownerProj);
        if (ownerProj)
        {
            std::vector<RimEclipseResultCase*> caseArr;
            ownerProj->descendantsIncludingThisOfType(caseArr);
            for (RimEclipseResultCase* c : caseArr)
            {
                options.push_back(caf::PdmOptionItemInfo(c->caseUserDescription(), c, true, c->uiIconProvider()));
            }
        }
    }

    else if (fieldNeedingOptions == &m_timeStepIndex)
    {
        if (m_case && m_case->eclipseCaseData())
        {
            const QStringList timeStepNames = m_case->timeStepStrings();
            for (int i = 0; i < timeStepNames.size(); i++)
            {
                options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
            }
        }

        if (options.size() == 0)
        {
            options.push_back(caf::PdmOptionItemInfo("None", -1));
        }
    }

    else if (fieldNeedingOptions == &m_wellName)
    {
        if (m_case && m_case->eclipseCaseData())
        {
            caf::QIconProvider simWellIcon(":/Well.png");
            const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
            for (const QString& name : sortedWellNameSet)
            {
                options.push_back(caf::PdmOptionItemInfo(name, name, true, simWellIcon));
            }
        }

        if (options.size() == 0)
        {
            options.push_back(caf::PdmOptionItemInfo("None", QVariant()));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_case)
    {
        fixupDependentFieldsAfterCaseChange();
    }

    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlot::fixupDependentFieldsAfterCaseChange()
{
    int newTimeStepIndex = -1;
    QString newWellName;

    if (m_case)
    {
        const int timeStepCount = m_case->timeStepStrings().size();
        if (timeStepCount > 0)
        {
            newTimeStepIndex = timeStepCount - 1;
        }

        const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
        if (sortedWellNameSet.size() > 0)
        {
            newWellName = *sortedWellNameSet.begin();
        }
    }

    m_timeStepIndex = newTimeStepIndex;
    m_wellName = newWellName;
}


