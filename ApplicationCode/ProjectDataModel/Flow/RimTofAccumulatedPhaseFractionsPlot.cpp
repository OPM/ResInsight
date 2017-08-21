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

#include "RimTofAccumulatedPhaseFractionsPlot.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimWellAllocationPlot.h"

#include "RigSingleWellResultsData.h"
#include "RigTofAccumulatedPhaseFractionsCalculator.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainPlotWindow.h"
#include "RiuTofAccumulatedPhaseFractionsPlot.h"
#include "RiuWellAllocationPlot.h"

#include "cvfColor3.h"


CAF_PDM_SOURCE_INIT(RimTofAccumulatedPhaseFractionsPlot, "TofAccumulatedPhaseFractionsPlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTofAccumulatedPhaseFractionsPlot::RimTofAccumulatedPhaseFractionsPlot()
{
    CAF_PDM_InitObject("Cumulative Saturation by Time of Flight", ":/WellAllocPie16x16.png", "", "");

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Cumulative Saturation by Time of Flight"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");
    m_showWindow = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTofAccumulatedPhaseFractionsPlot::~RimTofAccumulatedPhaseFractionsPlot()
{
    removeMdiWindowFromMdiArea();
    
    deleteViewWidget();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::deleteViewWidget()
{
    if (m_tofAccumulatedPhaseFractionsPlotWidget)
    {
        m_tofAccumulatedPhaseFractionsPlotWidget->deleteLater();
        m_tofAccumulatedPhaseFractionsPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::reloadFromWell()
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimTofAccumulatedPhaseFractionsPlot::resultCase()
{
    RimWellAllocationPlot* allocationPlot;
    firstAncestorOrThisOfTypeAsserted(allocationPlot);

    return allocationPlot->rimCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTofAccumulatedPhaseFractionsPlot::tracerName()
{
    RimWellAllocationPlot* allocationPlot;
    firstAncestorOrThisOfTypeAsserted(allocationPlot);

    return allocationPlot->wellName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimTofAccumulatedPhaseFractionsPlot::timeStep()
{
    RimWellAllocationPlot* allocationPlot;
    firstAncestorOrThisOfTypeAsserted(allocationPlot);

    return static_cast<size_t>(allocationPlot->timeStep());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimTofAccumulatedPhaseFractionsPlot::viewWidget()
{
    return m_tofAccumulatedPhaseFractionsPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::zoomAll()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_userName ||
        changedField == &m_showPlotTitle)
    {
        updateMdiWindowTitle();
    }
 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimTofAccumulatedPhaseFractionsPlot::snapshotWindowContent()
{
    QImage image;

    // TODO

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::setDescription(const QString& description)
{
    m_userName = description;
    this->updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTofAccumulatedPhaseFractionsPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::loadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if (m_tofAccumulatedPhaseFractionsPlotWidget && m_showWindow())
    {
        RigTofAccumulatedPhaseFractionsCalculator calc(resultCase(), tracerName(), timeStep());

        const std::vector<double>& xValues = calc.sortedUniqueTOFValues();
        const std::vector<double>& watValues = calc.accumulatedPhaseFractionsSwat();
        const std::vector<double>& oilValues = calc.accumulatedPhaseFractionsSoil();
        const std::vector<double>& gasValues = calc.accumulatedPhaseFractionsSgas();

        m_tofAccumulatedPhaseFractionsPlotWidget->setSamples(xValues, watValues, oilValues, gasValues);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimTofAccumulatedPhaseFractionsPlot::createViewWidget(QWidget* mainWindowParent)
{
    if (!m_tofAccumulatedPhaseFractionsPlotWidget)
    {
        m_tofAccumulatedPhaseFractionsPlotWidget = new RiuTofAccumulatedPhaseFractionsPlot(this, mainWindowParent);
    }
    return m_tofAccumulatedPhaseFractionsPlotWidget;
}


