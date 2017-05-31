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
#include "RimEclipseGeometrySelectionItem.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechGeometrySelectionItem.h"
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

CAF_PDM_SOURCE_INIT(RimGridTimeHistoryCurve, "GridTimeHistoryCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridTimeHistoryCurve::RimGridTimeHistoryCurve()
{
    CAF_PDM_InitObject("Grid Time History Curve", ":/SummaryCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_geometrySelectionText, "GeometrySelectionText", "Cell Reference", "", "", "");
    m_geometrySelectionText.registerGetMethod(this, &RimGridTimeHistoryCurve::geometrySelectionText);
    m_geometrySelectionText.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "EclipseResultDefinition", "Eclipse Result definition", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_geoMechResultDefinition, "GeoMechResultDefinition", "GeoMech Result definition", "", "", "");
    m_geoMechResultDefinition.uiCapability()->setUiHidden(true);
    m_geoMechResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_geometrySelectionItem, "GeometrySelectionItem", "Geometry Selection", "", "", "");
    m_geometrySelectionItem.uiCapability()->setUiTreeHidden(true);
    m_geometrySelectionItem.uiCapability()->setUiTreeChildrenHidden(true);

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
    if (m_geometrySelectionItem())
    {
        delete m_geometrySelectionItem();
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
        RimEclipseGeometrySelectionItem* geomSelectionItem = new RimEclipseGeometrySelectionItem;
        m_geometrySelectionItem = geomSelectionItem;

        geomSelectionItem->setFromSelectionItem(eclSelectionItem);

        if (eclSelectionItem->m_view)
        {
            m_eclipseResultDefinition = new RimEclipseResultDefinition;
            m_eclipseResultDefinition->simpleCopy(eclSelectionItem->m_view->cellResult());
        }
    }

    const RiuGeoMechSelectionItem* geoMechSelectionItem = dynamic_cast<const RiuGeoMechSelectionItem*>(selectionItem);
    if (geoMechSelectionItem)
    {
        RimGeoMechGeometrySelectionItem* geomSelectionItem = new RimGeoMechGeometrySelectionItem;
        m_geometrySelectionItem = geomSelectionItem;

        geomSelectionItem->setFromSelectionItem(geoMechSelectionItem);

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

    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if (eclTopItem && eclTopItem->eclipseCase())
    {
        size_t cellIndex = eclTopItem->cellIndex();
        size_t gridIndex = eclTopItem->gridIndex();

        CVF_ASSERT(m_eclipseResultDefinition());
        m_eclipseResultDefinition->loadResult();

        RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
        RigCaseCellResultsData* cellResultsData = cellResStorage->cellResults();

        std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates();

        values = RigTimeHistoryResultAccessor::timeHistoryValues(eclTopItem->eclipseCase()->eclipseCaseData(), m_eclipseResultDefinition(), gridIndex, cellIndex, timeStepDates.size());
    }

    if (geoMechGeomSelectionItem() && geoMechGeomSelectionItem()->geoMechCase())
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
    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if (eclTopItem)
    {
        CVF_ASSERT(m_eclipseResultDefinition());

        return m_eclipseResultDefinition->resultVariableUiName();
    }

    if (geoMechGeomSelectionItem())
    {
        CVF_ASSERT(m_geoMechResultDefinition());

        RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
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
    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if (eclTopItem && eclTopItem->eclipseCase())
    {
        return eclTopItem->eclipseCase()->caseUserDescription();
    }

    RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
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

    QString topoText = geometrySelectionText();

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
        std::vector<double> values;

        RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
        if (eclTopItem && eclTopItem->eclipseCase())
        {
            m_eclipseResultDefinition->loadResult();
        }

        RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
        if (geoMechTopItem && geoMechTopItem->geoMechCase())
        {
            m_geoMechResultDefinition->loadResult();
        }

        values = yValues();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfType(plot);
        bool isLogCurve = plot->isLogarithmicScaleEnabled(this->yAxis());

        if (plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE)
        {
            std::vector<time_t> dateTimes = timeStepValues();
            if (dateTimes.size() > 0 && dateTimes.size() == values.size())
            {
                m_qwtPlotCurve->setSamplesFromTimeTAndValues(dateTimes, values, isLogCurve);
            }
            else
            {
                m_qwtPlotCurve->setSamplesFromTimeTAndValues(std::vector<time_t>(), std::vector<double>(), isLogCurve);
            }
        }
        else
        {
            std::vector<double> days = daysSinceSimulationStart();
            if (days.size() > 0 && days.size() == values.size())
            {
                double timeScale = plot->timeAxisProperties()->fromDaysToDisplayUnitScale();

                std::vector<double> times;
                for (double day : days)
                {
                    times.push_back(timeScale * day);
                }

                m_qwtPlotCurve->setSamplesFromTimeAndValues(times, values, isLogCurve);
            }
            else
            {
                m_qwtPlotCurve->setSamplesFromTimeTAndValues(std::vector<time_t>(), std::vector<double>(), isLogCurve);
            }
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

    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if (eclTopItem && eclTopItem->eclipseCase())
    {
        RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
        RigCaseCellResultsData* cellResultsData = cellResStorage->cellResults();

        std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates();

        for (QDateTime dt : timeStepDates)
        {
            dateTimes.push_back(dt.toTime_t());
        }
    }

    RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
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
std::vector<double> RimGridTimeHistoryCurve::daysSinceSimulationStart() const
{
    std::vector<double> daysSinceSimulationStart;
    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if (eclTopItem && eclTopItem->eclipseCase())
    {
        RimReservoirCellResultsStorage* cellResStorage = m_eclipseResultDefinition->currentGridCellResults();
        RigCaseCellResultsData* cellResultsData = cellResStorage->cellResults();

        daysSinceSimulationStart = cellResultsData->daysSinceSimulationStart();
    }

    RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
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
                if (!dates.empty()) {
                    time_t startDate = dates[0].toTime_t();
                    double secondsToDaysConversion = (24.0 * 60.0 * 60.0);
                    for (QDateTime dt : dates)
                    {
                        double timeDifference = static_cast<double>(dt.toTime_t() - startDate);
                        daysSinceSimulationStart.push_back(timeDifference / secondsToDaysConversion);
                    }
                }
            }
            else
            {
                for (size_t i = 0; i < values.size(); i++)
                {
                    daysSinceSimulationStart.push_back(i);
                }
            }
        }
    }

    return daysSinceSimulationStart;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    uiOrdering.add(&m_geometrySelectionText);

    // Fields declared in RimResultDefinition
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    if (eclipseGeomSelectionItem())
    {
        CVF_ASSERT(m_eclipseResultDefinition());
        m_eclipseResultDefinition->uiOrdering(uiConfigName, *group1);
    }

    if (geoMechGeomSelectionItem())
    {
        CVF_ASSERT(m_geoMechResultDefinition());
        m_geoMechResultDefinition->uiOrdering(uiConfigName, *group1);
    }

    uiOrdering.add(&m_plotAxis);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
    nameGroup->setCollapsedByDefault(true);
    nameGroup->add(&m_showLegend);
    RimPlotCurve::curveNameUiOrdering(*nameGroup);

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
    if (eclipseGeomSelectionItem() && eclipseGeomSelectionItem()->eclipseCase() && eclipseGeomSelectionItem()->eclipseCase()->eclipseCaseData())
    {
        return eclipseGeomSelectionItem()->eclipseCase()->eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseGeometrySelectionItem* RimGridTimeHistoryCurve::eclipseGeomSelectionItem() const
{
    RimGeometrySelectionItem* pickItem = m_geometrySelectionItem();

    return dynamic_cast<RimEclipseGeometrySelectionItem*>(pickItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechGeometrySelectionItem* RimGridTimeHistoryCurve::geoMechGeomSelectionItem() const
{
    RimGeometrySelectionItem* pickItem = m_geometrySelectionItem();

    return dynamic_cast<RimGeoMechGeometrySelectionItem*>(pickItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateResultDefinitionFromCase()
{
    if (eclipseGeomSelectionItem())
    {
        CVF_ASSERT(m_eclipseResultDefinition());

        m_eclipseResultDefinition->setEclipseCase(eclipseGeomSelectionItem()->eclipseCase());
    }

    if (geoMechGeomSelectionItem())
    {
        CVF_ASSERT(m_geoMechResultDefinition());

        m_geoMechResultDefinition->setGeoMechCase(geoMechGeomSelectionItem()->geoMechCase());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::geometrySelectionText() const
{
    QString text;

    if (eclipseGeomSelectionItem() && m_geometrySelectionItem())
    {
        text = m_geometrySelectionItem->geometrySelectionText();
    }
    else if (geoMechGeomSelectionItem())
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();
        if (timeHistResultAccessor)
        {
            text = timeHistResultAccessor->geometrySelectionText();
        }
    }
    else
    {
        text = "No Geometry";
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

    if (   geoMechGeomSelectionItem()
        && geoMechGeomSelectionItem()->geoMechCase() 
        && geoMechGeomSelectionItem()->geoMechCase()->geoMechData())
    {
        RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
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
