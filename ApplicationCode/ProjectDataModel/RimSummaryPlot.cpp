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
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlotCollection.h"

#include "RiuSummaryQwtPlot.h"
#include "RiuSelectionColors.h"

#include "cvfBase.h"
#include "cvfColor3.h"

#include <QDateTime>
#include "RiuMainWindow.h"


CAF_PDM_SOURCE_INIT(RimSummaryPlot, "SummaryPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot()
{
    CAF_PDM_InitObject("Summary Plot", ":/WellLogPlot16x16.png", "", "");
    CAF_PDM_InitField(&m_showWindow, "ShowWindow", true, "Show Summary Plot", "", "", "");
    m_showWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Summary Plot"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curveFilters, "SummaryCurveFilters", "", "", "", "");
    m_curveFilters.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curves, "SummaryCurves", "",  "", "", "");
    m_curves.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::~RimSummaryPlot()
{
    RiuMainWindow::instance()->removeViewer(m_qwtPlot);

    deletePlotWidget();

    m_curves.deleteAllChildObjects();
    m_curveFilters.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deletePlotWidget()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->deleteLater();
        m_qwtPlot = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::handleViewerDeletion()
{
    m_showWindow = false;

    if (m_qwtPlot)
    {
        detachAllCurves();
    }

    uiCapability()->updateUiIconFromToggleField();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurve(RimSummaryCurve* curve)
{
    if (curve)
    {
        m_curves.push_back(curve);
        if (m_qwtPlot)
        {
            curve->setParentQwtPlot(m_qwtPlot);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveFilter(RimSummaryCurveFilter* curveFilter)
{
    if(curveFilter)
    {
        m_curveFilters.push_back(curveFilter);
        if(m_qwtPlot)
        {
            curveFilter->setParentQwtPlot(m_qwtPlot);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showWindow)
    {
        if (m_showWindow)
        {
            loadDataAndUpdate();
        }
        else
        {
            updateViewerWidget();
        }

        uiCapability()->updateUiIconFromToggleField();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setupBeforeSave()
{
    if (m_qwtPlot)
    {
        this->setMdiWindowGeometry(RiuMainWindow::instance()->windowGeometryForViewer(m_qwtPlot));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::loadDataAndUpdate()
{
   updateViewerWidget();    

   for (RimSummaryCurveFilter* curveFilter: m_curveFilters)
   {
        curveFilter->loadDataAndUpdate();
   }

    for (RimSummaryCurve* curve : m_curves)
    {
        curve->loadDataAndUpdate();
    }
 
    // Todo: Update zoom
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
        if (!m_qwtPlot)
        {
            m_qwtPlot = new RiuSummaryQwtPlot(this, RiuMainWindow::instance());

            for(RimSummaryCurveFilter* curveFilter: m_curveFilters)
            {
                curveFilter->setParentQwtPlot(m_qwtPlot);
            }

            for(RimSummaryCurve* curve : m_curves)
            {
                curve->setParentQwtPlot(m_qwtPlot);
            }

            RiuMainWindow::instance()->addViewer(m_qwtPlot, this->mdiWindowGeometry());
            RiuMainWindow::instance()->setActiveViewer(m_qwtPlot);
        }

        //updateViewerWidgetWindowTitle();
    }
    else
    {
        if (m_qwtPlot)
        {
            this->setMdiWindowGeometry( RiuMainWindow::instance()->windowGeometryForViewer(m_qwtPlot));

            RiuMainWindow::instance()->removeViewer(m_qwtPlot);
            detachAllCurves();

            deletePlotWidget();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::detachAllCurves()
{
    for(RimSummaryCurveFilter* curveFilter: m_curveFilters)
    {
        curveFilter->detachQwtCurve();
    }

    for(RimSummaryCurve* curve : m_curves)
    {
        curve->detachQwtCurve();
    }
}
