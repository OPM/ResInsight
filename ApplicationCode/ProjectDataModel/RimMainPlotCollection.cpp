/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimMainPlotCollection.h"

#include "RimProject.h"
#include "RimSummaryPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "RiuMainWindow.h"
#include "RiuProjectPropertyView.h"

CAF_PDM_SOURCE_INIT(RimMainPlotCollection, "MainPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::RimMainPlotCollection()
{
    CAF_PDM_InitObject("Plots", "", "", "");

    CAF_PDM_InitField(&show, "Show", true, "Show 2D Plot Window", "", "", "");
    show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogPlotCollection, "WellLogPlotCollection", "",  "", "", "");
    m_wellLogPlotCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_summaryPlotCollection, "SummaryPlotCollection", "Summary Plots", "", "", "");
    m_summaryPlotCollection.uiCapability()->setUiHidden(true);

    m_wellLogPlotCollection = new RimWellLogPlotCollection();
    m_summaryPlotCollection = new RimSummaryPlotCollection();

    //m_plotMainWindow = NULL;
    //m_plotManagerMainWindow = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::~RimMainPlotCollection()
{
    if (m_wellLogPlotCollection()) delete m_wellLogPlotCollection();
    if (m_summaryPlotCollection()) delete m_summaryPlotCollection();

    //m_plotManagerMainWindow->close();
    //m_plotManagerMainWindow->deleteLater();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
#if 0
    if (changedField == &showWindow)
    {
        if (showWindow)
        {
            showPlotWindow();
        }
        else
        {
            hidePlotWindow();
        }
    }
    #endif
}
#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::showPlotWindow()
{
    if (!m_plotManagerMainWindow)
    {
        m_plotManagerMainWindow = new QMainWindow;
        m_plotManagerMainWindow->setDockNestingEnabled(true);

        m_plotMainWindow = new QMainWindow;
        m_plotMainWindow->setDockNestingEnabled(true);

        // NOTE! setCentralWidget takes ownership of widget
        m_plotManagerMainWindow->setCentralWidget(m_plotMainWindow);

        {
            QDockWidget* dockWidget = new QDockWidget("Plots", m_plotManagerMainWindow);
            dockWidget->setObjectName("dockWidget");

            RiuMainWindow* mainWindow = RiuMainWindow::instance();

            RiuProjectAndPropertyView* projPropView = new RiuProjectAndPropertyView(dockWidget);
            dockWidget->setWidget(projPropView);

            RimProject* proj = NULL;
            this->firstAncestorOrThisOfType(proj);

            projPropView->setPdmItem(this);

            m_plotManagerMainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        }
    }

    m_plotMainWindow->show();

    m_plotManagerMainWindow->showNormal();
    m_plotManagerMainWindow->raise();
}

#endif
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMainPlotCollection::objectToggleField()
{
    return &show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RimMainPlotCollection::wellLogPlotCollection()
{
    return m_wellLogPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection* RimMainPlotCollection::summaryPlotCollection()
{
    return m_summaryPlotCollection();
}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::createDockWindowsForAllPlots()
{
    for (size_t i = 0; i < m_graphPlots.size(); i++)
    {
        if (!dockWidgetFromPlot(m_graphPlots[i]))
        {
            createPlotDockWidget(m_graphPlots[i]);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDockWidget* RimMainPlotCollection::dockWidgetFromPlot(RimSummaryPlot* graphPlot)
{
    foreach(QDockWidget* dockW, m_plotViewDockWidgets)
    {
        if (dockW && dockW->widget() == graphPlot->widget())
        {
            return dockW;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::createPlotDockWidget(RimSummaryPlot* graphPlot)
{
    assert(m_plotMainWindow != NULL);

    QDockWidget* dockWidget = new QDockWidget(QString("Plot Widget Tree (%1)").arg(m_plotViewDockWidgets.size() + 1), m_plotMainWindow);
    dockWidget->setObjectName("dockWidget");
    // dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* widget = graphPlot->createPlotWidget(m_plotMainWindow);

    dockWidget->setWidget(widget);

    m_plotMainWindow->addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    m_plotViewDockWidgets.push_back(dockWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::eraseDockWidget(RimSummaryPlot* graphPlot)
{
    QDockWidget* dockW = dockWidgetFromPlot(graphPlot);
    if (dockW)
    {
        m_plotMainWindow->removeDockWidget(dockW);
        dockW->setWidget(NULL);
        dockW->deleteLater();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::redrawAllPlots()
{
    for (size_t i = 0; i < m_graphPlots.size(); i++)
    {
        m_graphPlots[i]->redrawAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMainWindow* RimMainPlotCollection::windowWithGraphPlots()
{
    return m_plotMainWindow;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::initAfterRead()
{
    if (show())
    {
        showPlotWindow();
    }
    else
    {
        hidePlotWindow();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::hidePlotWindow()
{
    if (m_plotManagerMainWindow)
    {
        m_plotManagerMainWindow->hide();
    }
}
#endif