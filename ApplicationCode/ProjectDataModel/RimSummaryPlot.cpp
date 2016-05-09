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

#include "RimSummaryCurve.h"
#include "RimSummaryPlotCollection.h"

#include "RiuResultQwtPlot.h"
#include "RiuSelectionColors.h"

#include "cvfBase.h"
#include "cvfColor3.h"

#include <QDateTime>
#include "RiuMainWindow.h"


CAF_PDM_SOURCE_INIT(RimSummaryPlot, "GraphPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot()
{
    CAF_PDM_InitObject("Graph", ":/WellLogPlot16x16.png", "", "");
    CAF_PDM_InitField(&m_showWindow, "ShowWindow", true, "Show Summary Plot", "", "", "");
    m_showWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Summary Plot"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&curves, "SummaryCurves", "",  "", "", "");
    curves.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::~RimSummaryPlot()
{
    deletePlotWidget();

    curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryPlot::createPlotWidget(QWidget* parent)
{
    assert(m_viewer.isNull());

    m_viewer = new RiuResultQwtPlot(parent);

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deletePlotWidget()
{
    if (m_viewer)
    {
        m_viewer->deleteLater();
        m_viewer = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuResultQwtPlot* RimSummaryPlot::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::loadDataAndUpdate()
{
    m_viewer->deleteAllCurves();

    for (size_t i = 0; i < curves.size(); i++)
    {
        RimSummaryCurve* curve = curves[i];

        std::vector<QDateTime> dateTimes;
        std::vector<double> values;

        curve->curveData(&dateTimes, &values);

        cvf::Color3f curveColor = RiuSelectionColors::curveColorFromTable();

        m_viewer->addCurve(curve->m_variableName(), curveColor, dateTimes, values);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateViewerWidget()
{
    if (m_showWindow())
    {
        if (!m_viewer)
        {
            m_viewer = new RiuResultQwtPlot(RiuMainWindow::instance());


            RiuMainWindow::instance()->addViewer(m_viewer, std::vector<int>());
            RiuMainWindow::instance()->setActiveViewer(m_viewer);
        }

        //updateViewerWidgetWindowTitle();
    }
    else
    {
        if (m_viewer)
        {
            //windowGeometry = RiuMainWindow::instance()->windowGeometryForViewer(m_viewer);

            RiuMainWindow::instance()->removeViewer(m_viewer);
            detachAllCurves();

            delete m_viewer;
            m_viewer = NULL;

        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::detachAllCurves()
{
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        //curves[cIdx]->detachQwtCurve();
    }
}
