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
#include "RimGeoMechCase.h"
#include "RimGeoMechTopologyItem.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuFemTimeHistoryResultAccessor.h"
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

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "EclipseResultDefinition", "Eclipse Result definition", "", "", "");

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_geoMechResultDefinition, "GeoMechResultDefinition", "GeoMech Result definition", "", "", "");

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    m_geoMechResultDefinition.uiCapability()->setUiHidden(true);
    m_geoMechResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

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

    if (m_eclipseResultDefinition())
    {
        delete m_eclipseResultDefinition();
    }

    if (m_geoMechResultDefinition())
    {
        delete m_geoMechResultDefinition();
    }

    const RiuEclipseSelectionItem* eclSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>(selectionItem);
    if (eclSelectionItem)
    {
        RimEclipseTopologyItem* topologyItem = new RimEclipseTopologyItem;
        m_pickingTopologyItem = topologyItem;

        topologyItem->setFromSelectionItem(eclSelectionItem);

        if (eclSelectionItem->m_view)
        {
            m_eclipseResultDefinition = new RimEclipseResultDefinition;
            m_eclipseResultDefinition->simpleCopy(eclSelectionItem->m_view->cellResult());
        }
    }

    const RiuGeoMechSelectionItem* geoMechSelectionItem = dynamic_cast<const RiuGeoMechSelectionItem*>(selectionItem);
    if (geoMechSelectionItem)
    {
        RimGeoMechTopologyItem* topologyItem = new RimGeoMechTopologyItem;
        m_pickingTopologyItem = topologyItem;

        topologyItem->setFromSelectionItem(geoMechSelectionItem);

        if (geoMechSelectionItem->m_view)
        {
            m_geoMechResultDefinition = new RimGeoMechResultDefinition;
            m_geoMechResultDefinition->setGeoMechCase(geoMechSelectionItem->m_view->geoMechCase());
            m_geoMechResultDefinition->setResultAddress(geoMechSelectionItem->m_view->cellResultResultDefinition()->resultAddress());
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

        CVF_ASSERT(m_eclipseResultDefinition());
        m_eclipseResultDefinition->loadResult();

        RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
        RigCaseCellResultsData* cellResultsData = cellResStorage->cellResults();

        std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates(m_eclipseResultDefinition->scalarResultIndex());

        values = RigTimeHistoryResultAccessor::timeHistoryValues(eclTopItem->eclipseCase()->eclipseCaseData(), m_eclipseResultDefinition(), gridIndex, cellIndex, timeStepDates.size());
    }

    if (geoMechTopologyItem() && geoMechTopologyItem()->geoMechCase())
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();

        if (timeHistResultAccessor)
        {
            values = timeHistResultAccessor->timeHistoryValues();
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::quantityName() const
{
    RimEclipseTopologyItem* eclTopItem = eclipseTopologyItem();
    if (eclTopItem)
    {
        CVF_ASSERT(m_eclipseResultDefinition());

        return m_eclipseResultDefinition->resultVariableUiName();
    }

    if (geoMechTopologyItem())
    {
        CVF_ASSERT(m_geoMechResultDefinition());

        RimGeoMechTopologyItem* geoMechTopItem = geoMechTopologyItem();
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();

        QString text;

        caf::AppEnum<RigFemResultPosEnum> resPosAppEnum = m_geoMechResultDefinition()->resultPositionType();
        text.append(resPosAppEnum.uiText() + ", ");
        text.append(m_geoMechResultDefinition()->resultFieldUiName() + ", ");
        text.append(m_geoMechResultDefinition()->resultComponentUiName() + " ");

        if (resPosAppEnum == RIG_ELEMENT_NODAL_FACE)
        {
            if (geoMechTopItem->m_elementFace >= 0)
            {
                text.append(", " + caf::AppEnum<cvf::StructGridInterface::FaceType>::textFromIndex(geoMechTopItem->m_elementFace));
            }
            else
            {
                text.append(", from N[" + QString::number(timeHistResultAccessor->closestNodeId()) + "] transformed onto intersection");
            }
        }
    
        return text;
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::caseName() const
{
    RimEclipseTopologyItem* eclTopItem = eclipseTopologyItem();
    if (eclTopItem && eclTopItem->eclipseCase())
    {
        return eclTopItem->eclipseCase()->caseUserDescription();
    }

    RimGeoMechTopologyItem* geoMechTopItem = geoMechTopologyItem();
    if (geoMechTopItem && geoMechTopItem->geoMechCase())
    {
        return geoMechTopItem->geoMechCase()->caseUserDescription();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::createCurveAutoName()
{
    QString text;

    text += quantityName();

    QString topoText = topologyText();

    if (!topoText.isEmpty())
    {
        text += ", ";
        text += topoText;
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
        std::vector<time_t> dateTimes;
        std::vector<double> values;

        RimEclipseTopologyItem* eclTopItem = eclipseTopologyItem();
        if (eclTopItem && eclTopItem->eclipseCase())
        {
            m_eclipseResultDefinition->loadResult();
        }

        RimGeoMechTopologyItem* geoMechTopItem = geoMechTopologyItem();
        if (geoMechTopItem && geoMechTopItem->geoMechCase())
        {
            m_geoMechResultDefinition->loadResult();
        }

        dateTimes = timeStepValues();
        values = yValues();

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

        updateQwtPlotAxis();
        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimGridTimeHistoryCurve::timeStepValues() const
{
    std::vector<time_t> dateTimes;

    RimEclipseTopologyItem* eclTopItem = eclipseTopologyItem();
    if (eclTopItem && eclTopItem->eclipseCase())
    {
        RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
        RigCaseCellResultsData* cellResultsData = cellResStorage->cellResults();

        std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates(m_eclipseResultDefinition->scalarResultIndex());

        for (QDateTime dt : timeStepDates)
        {
            dateTimes.push_back(dt.toTime_t());
        }
    }

    RimGeoMechTopologyItem* geoMechTopItem = geoMechTopologyItem();
    if (geoMechTopItem && geoMechTopItem->geoMechCase())
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();
        if (timeHistResultAccessor)
        {
            std::vector<double> values = timeHistResultAccessor->timeHistoryValues();

            QStringList stepNames = geoMechTopItem->geoMechCase()->timeStepStrings();
            std::vector<QDateTime> dates = RimGeoMechCase::dateTimeVectorFromTimeStepStrings(stepNames);
            if (dates.size() == values.size())
            {
                for (QDateTime dt : dates)
                {
                    dateTimes.push_back(dt.toTime_t());
                }
            }
            else
            {
                for (size_t i = 0; i < values.size(); i++)
                {
                    dateTimes.push_back(i);
                }
            }
        }
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
    if (eclipseTopologyItem())
    {
        CVF_ASSERT(m_eclipseResultDefinition());
        m_eclipseResultDefinition->uiOrdering(uiConfigName, *group1);
    }

    if (geoMechTopologyItem())
    {
        CVF_ASSERT(m_geoMechResultDefinition());
        m_geoMechResultDefinition->uiOrdering(uiConfigName, *group1);
    }

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
RimGeoMechTopologyItem* RimGridTimeHistoryCurve::geoMechTopologyItem() const
{
    RimPickingTopologyItem* pickItem = m_pickingTopologyItem();

    return dynamic_cast<RimGeoMechTopologyItem*>(pickItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateResultDefinitionFromCase()
{
    if (eclipseTopologyItem())
    {
        CVF_ASSERT(m_eclipseResultDefinition());

        m_eclipseResultDefinition->setEclipseCase(eclipseTopologyItem()->eclipseCase());
    }

    if (geoMechTopologyItem())
    {
        CVF_ASSERT(m_geoMechResultDefinition());

        m_geoMechResultDefinition->setGeoMechCase(geoMechTopologyItem()->geoMechCase());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::topologyText() const
{
    QString text;

    if (eclipseTopologyItem() && m_pickingTopologyItem())
    {
        text = m_pickingTopologyItem->topologyText();
    }
    else if (geoMechTopologyItem())
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();
        if (timeHistResultAccessor)
        {
            text = timeHistResultAccessor->topologyText();
        }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RiuFemTimeHistoryResultAccessor> RimGridTimeHistoryCurve::femTimeHistoryResultAccessor() const
{
    std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor;

    if (   geoMechTopologyItem()
        && geoMechTopologyItem()->geoMechCase() 
        && geoMechTopologyItem()->geoMechCase()->geoMechData())
    {
        RimGeoMechTopologyItem* geoMechTopItem = geoMechTopologyItem();
        if (geoMechTopItem->m_hasIntersectionTriangle)
        {
            std::array<cvf::Vec3f, 3> intersectionTriangle;
            intersectionTriangle[0] = cvf::Vec3f(geoMechTopItem->m_intersectionTriangle_0());
            intersectionTriangle[1] = cvf::Vec3f(geoMechTopItem->m_intersectionTriangle_1());
            intersectionTriangle[2] = cvf::Vec3f(geoMechTopItem->m_intersectionTriangle_2());

            timeHistResultAccessor = std::unique_ptr<RiuFemTimeHistoryResultAccessor>(
                new RiuFemTimeHistoryResultAccessor(geoMechTopItem->geoMechCase()->geoMechData(),
                    m_geoMechResultDefinition()->resultAddress(),
                    geoMechTopItem->m_gridIndex,
                    static_cast<int>(geoMechTopItem->m_cellIndex),
                    geoMechTopItem->m_elementFace,
                    geoMechTopItem->m_localIntersectionPoint,
                    intersectionTriangle));
        }
        else
        {
            timeHistResultAccessor = std::unique_ptr<RiuFemTimeHistoryResultAccessor>(
                new RiuFemTimeHistoryResultAccessor(geoMechTopItem->geoMechCase()->geoMechData(),
                    m_geoMechResultDefinition()->resultAddress(),
                    geoMechTopItem->m_gridIndex,
                    static_cast<int>(geoMechTopItem->m_cellIndex),
                    geoMechTopItem->m_elementFace,
                    geoMechTopItem->m_localIntersectionPoint));
        }

    }

    return timeHistResultAccessor;
}
