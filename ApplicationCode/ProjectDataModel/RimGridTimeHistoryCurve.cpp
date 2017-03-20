/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimGridTimeHistoryCurve.h"

#include "RiaApplication.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigTimeHistoryResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseTopologyItem.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSelectionManager.h"

#include "qwt_plot.h"

CAF_PDM_SOURCE_INIT(RimGridTimeHistoryCurve, "TimeHistoryCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridTimeHistoryCurve::RimGridTimeHistoryCurve()
{
    CAF_PDM_InitObject("Grid Time History Curve", ":/SummaryCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_topologyText, "TopologyText", "TopologyText", "", "", "");
    m_topologyText.registerGetMethod(this, &RimGridTimeHistoryCurve::topologyText);
    m_topologyText.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "ResultDefinition", "Result definition", "", "", "");
    m_eclipseResultDefinition = new RimEclipseResultDefinition();

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_pickingTopologyItem, "PickingTopologyItem", "Picking Topology Item", "", "", "");
    m_pickingTopologyItem.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitField(&m_plotAxis, "PlotAxis", caf::AppEnum< RimDefines::PlotAxis >(RimDefines::PLOT_AXIS_LEFT), "Axis", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridTimeHistoryCurve::~RimGridTimeHistoryCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::setFromSelectionItem(const RiuSelectionItem* selectionItem)
{
    if (m_pickingTopologyItem())
    {
        delete m_pickingTopologyItem();
    }

    const RiuEclipseSelectionItem* eclSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>(selectionItem);
    if (eclSelectionItem)
    {
        RimEclipseTopologyItem* topologyItem = new RimEclipseTopologyItem;
        m_pickingTopologyItem = topologyItem;

        topologyItem->setFromSelectionItem(eclSelectionItem);

        if (eclSelectionItem->m_view)
        {
            m_eclipseResultDefinition->simpleCopy(eclSelectionItem->m_view->cellResult());
        }
    }

    updateResultDefinitionFromCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDefines::PlotAxis RimGridTimeHistoryCurve::yAxis() const
{
    return m_plotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::setYAxis(RimDefines::PlotAxis plotAxis)
{
    m_plotAxis = plotAxis;

    updateQwtPlotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridTimeHistoryCurve::yValues() const
{
    std::vector<double> values;

    RimEclipseTopologyItem* eclTopItem = eclipseTopologyItem();
    if (eclTopItem && eclTopItem->eclipseCase())
    {
        size_t cellIndex = eclTopItem->cellIndex();
        size_t gridIndex = eclTopItem->gridIndex();

        m_eclipseResultDefinition->loadResult();

        RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
        RigCaseCellResultsData* cellResultsData = cellResStorage->cellResults();

        std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates(m_eclipseResultDefinition->scalarResultIndex());

        values = RigTimeHistoryResultAccessor::timeHistoryValues(eclTopItem->eclipseCase()->eclipseCaseData(), m_eclipseResultDefinition(), gridIndex, cellIndex, timeStepDates.size());
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::quantityName() const
{
    CVF_ASSERT(m_eclipseResultDefinition);

    return m_eclipseResultDefinition->resultVariableUiName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::caseName() const
{
    RimEclipseCase* eclCase = nullptr;
    RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
    cellResStorage->firstAncestorOrThisOfType(eclCase);

    if (eclCase)
    {
        return eclCase->caseUserDescription();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::createCurveAutoName()
{
    QString text;

    QString resName = m_eclipseResultDefinition->resultVariableUiName();
    if (!resName.isEmpty())
    {
        text += resName;
    }

    if (m_pickingTopologyItem())
    {
        if (!text.isEmpty())
        {
            text += ", ";
        }
        text += m_pickingTopologyItem->topologyText();
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateZoomInParentPlot()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);

    plot->updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::onLoadDataAndUpdate()
{
    this->RimPlotCurve::updateCurvePresentation();

    if (isCurveVisible())
    {
        RimEclipseTopologyItem* eclTopItem = eclipseTopologyItem();
        if (eclTopItem && eclTopItem->eclipseCase())
        {
            m_eclipseResultDefinition->loadResult();

            std::vector<time_t> dateTimes = timeStepValues();

            std::vector<double> values = yValues();

            RimSummaryPlot* plot = nullptr;
            firstAncestorOrThisOfType(plot);
            bool isLogCurve = plot->isLogarithmicScaleEnabled(this->yAxis());

            if (dateTimes.size() > 0 && dateTimes.size() == values.size())
            {
                if (plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE)
                {
                    m_qwtPlotCurve->setSamplesFromTimeTAndValues(dateTimes, values, isLogCurve);
                }
                else
                {
                    double timeScale = plot->timeAxisProperties()->fromTimeTToDisplayUnitScale();

                    std::vector<double> times;
                    if (dateTimes.size())
                    {
                        time_t startDate = dateTimes[0];
                        for (time_t& date : dateTimes)
                        {
                            times.push_back(timeScale*(date - startDate));
                        }
                    }

                    m_qwtPlotCurve->setSamplesFromTimeAndValues(times, values, isLogCurve);
                }
            }
            else
            {
                m_qwtPlotCurve->setSamplesFromTimeTAndValues(std::vector<time_t>(), std::vector<double>(), isLogCurve);
            }

            updateZoomInParentPlot();

            if (m_parentQwtPlot) m_parentQwtPlot->replot();
        }

        updateQwtPlotAxis();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfTypeAsserted(plot);

        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimGridTimeHistoryCurve::timeStepValues() const
{
    std::vector<time_t> dateTimes;

    RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
    RigCaseCellResultsData* cellResultsData = cellResStorage->cellResults();

    std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates(m_eclipseResultDefinition->scalarResultIndex());

    for (QDateTime dt : timeStepDates)
    {
        dateTimes.push_back(dt.toTime_t());
    }

    return dateTimes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    uiOrdering.add(&m_topologyText);

    // Fields declared in RimResultDefinition
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    m_eclipseResultDefinition->defineUiOrdering(uiConfigName, *group1);

    uiOrdering.add(&m_plotAxis);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::initAfterRead()
{
    updateResultDefinitionFromCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_plotAxis)
    {
        updateQwtPlotAxis();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfTypeAsserted(plot);

        plot->updateAxes();
    }
    else
    {
        RimPlotCurve::fieldChangedByUi(changedField, oldValue, newValue);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimGridTimeHistoryCurve::mainGrid()
{
    if (eclipseTopologyItem() && eclipseTopologyItem()->eclipseCase() && eclipseTopologyItem()->eclipseCase()->eclipseCaseData())
    {
        return eclipseTopologyItem()->eclipseCase()->eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseTopologyItem* RimGridTimeHistoryCurve::eclipseTopologyItem() const
{
    RimPickingTopologyItem* pickItem = m_pickingTopologyItem();

    return dynamic_cast<RimEclipseTopologyItem*>(pickItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateResultDefinitionFromCase()
{
    if (eclipseTopologyItem())
    {
        m_eclipseResultDefinition->setEclipseCase(eclipseTopologyItem()->eclipseCase());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::topologyText() const
{
    QString text;

    if (m_pickingTopologyItem())
    {
        text = m_pickingTopologyItem->topologyText();
    }
    else
    {
        text = "No topology";
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateQwtPlotAxis()
{
    if (m_qwtPlotCurve)
    {
        if (this->yAxis() == RimDefines::PLOT_AXIS_LEFT)
        {
            m_qwtPlotCurve->setYAxis(QwtPlot::yLeft);
        }
        else
        {
            m_qwtPlotCurve->setYAxis(QwtPlot::yRight);
        }
    }
}

