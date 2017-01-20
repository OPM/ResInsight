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

#include "RimWellAllocationPlot.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"

#include "RiuMainPlotWindow.h"
#include "RiuWellAllocationPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RigSingleWellResultsData.h"


CAF_PDM_SOURCE_INIT(RimWellAllocationPlot, "WellAllocationPlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::RimWellAllocationPlot()
{
    CAF_PDM_InitObject("Well Allocation Plot", ":/newIcon16x16.png", "", "");
    CAF_PDM_InitField(&m_showWindow, "ShowWindow", true, "Show Flow Diagnostics Plot", "", "", "");
    m_showWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Flow Diagnostics Plot"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_simulationWell, "SimulationWell", "Simulation Well", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_accumulatedWellFlowPlot, "AccumulatedWellFlowPlot", "Accumulated Well Flow", "", "", "");
    m_accumulatedWellFlowPlot.uiCapability()->setUiHidden(true);

    m_accumulatedWellFlowPlot = new RimWellLogPlot;
    this->setAsPlotMDI();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::~RimWellAllocationPlot()
{
    removeWidgetFromMDI();
    
    delete m_accumulatedWellFlowPlot();

    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::setSimulationWell(RimEclipseWell* simWell)
{
    m_simulationWell = simWell;

    updateFromWell();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::deleteViewWidget()
{
    if (m_wellAllocationPlotWidget)
    {
        m_wellAllocationPlotWidget->deleteLater();
        m_wellAllocationPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateFromWell()
{
    QString simName = "None";
    int branchCount = 0; 
    
    const RigWellResultFrame* wellResultFrame = nullptr;

    if (m_simulationWell && m_simulationWell->wellResults() )// && Timestep Ok )
    {
        simName = m_simulationWell->name();
        wellResultFrame =  &(m_simulationWell->wellResults()->wellResultFrame(1));
        branchCount = static_cast<int>( wellResultFrame->m_wellResultBranches.size());
    }

    int existingTrackCount = static_cast<int>(accumulatedWellFlowPlot()->trackCount());
    accumulatedWellFlowPlot()->setDescription("Accumulated Well Flow (" + simName + ")");

    int neededExtraTrackCount = branchCount - existingTrackCount;
    for (int etc = 0; etc < neededExtraTrackCount; ++etc)
    {
        RimWellLogTrack* plotTrack = new RimWellLogTrack();
        accumulatedWellFlowPlot()->addTrack(plotTrack);
    }

    for (int etc = branchCount; etc < existingTrackCount; ++etc)
    {
        accumulatedWellFlowPlot()->removeTrackByIndex(accumulatedWellFlowPlot()->trackCount()- 1);
    }

    for (size_t brIdx = 0; brIdx < branchCount; ++brIdx)
    {
        RimWellLogTrack* plotTrack = accumulatedWellFlowPlot()->trackByIndex(brIdx);

        plotTrack->setDescription(QString("Branch %1").arg(wellResultFrame->m_wellResultBranches[brIdx].m_ertBranchId));

    }

    setDescription("Well Allocation (" + simName + ")");
    accumulatedWellFlowPlot()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationPlot::viewWidget()
{
    return m_wellAllocationPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RimWellAllocationPlot::accumulatedWellFlowPlot()
{
    return m_accumulatedWellFlowPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellAllocationPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_simulationWell)
    {
        RimView* activeView = RiaApplication::instance()->activeReservoirView();
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(activeView);

        if (eclView && eclView->wellCollection())
        {
            RimEclipseWellCollection* coll = eclView->wellCollection();

            caf::PdmChildArrayField<RimEclipseWell*>& eclWells = coll->wells;

            QIcon simWellIcon(":/Well.png");
            for (RimEclipseWell* eclWell : eclWells)
            {
                options.push_back(caf::PdmOptionItemInfo(eclWell->name(), eclWell, false, simWellIcon));
            }
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showWindow)
    {
        updateViewerWidgetBasic();

        uiCapability()->updateUiIconFromToggleField();
    }
    else if (changedField == &m_userName ||
        changedField == &m_showPlotTitle)
    {
        updateViewerWidgetWindowTitle();
    }
    else if (changedField == &m_simulationWell)
    {
        updateFromWell();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimWellAllocationPlot::snapshotWindowContent()
{
    QImage image;

    // TODO

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::setDescription(const QString& description)
{
    m_userName = description;
    this->updateViewerWidgetWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::loadDataAndUpdate()
{
    updateViewerWidgetBasic();
    updateFromWell();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_wellAllocationPlotWidget = new RiuWellAllocationPlot(this, mainWindowParent);
    return m_wellAllocationPlotWidget;
}


