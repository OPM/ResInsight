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

#include "RimTotalWellAllocationPlot.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"

#include "RigSingleWellResultsData.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainPlotWindow.h"
#include "RiuNightchartsWidget.h"
#include "RiuWellAllocationPlot.h"


CAF_PDM_SOURCE_INIT(RimTotalWellAllocationPlot, "TotalWellAllocationPlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTotalWellAllocationPlot::RimTotalWellAllocationPlot()
{
    CAF_PDM_InitObject("Total Well Allocation Plot", ":/newIcon16x16.png", "", "");

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Total Well Allocation Plot"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_simulationWell, "SimulationWell", "Simulation Well", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTotalWellAllocationPlot::~RimTotalWellAllocationPlot()
{
    removeMdiWindowFromMdiArea();
    
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::setSimulationWell(RimEclipseWell* simWell)
{
    m_simulationWell = simWell;

    updateFromWell();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::deleteViewWidget()
{
    if (m_wellTotalAllocationPlotWidget)
    {
        m_wellTotalAllocationPlotWidget->deleteLater();
        m_wellTotalAllocationPlotWidget= nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::updateFromWell()
{
/*
    QString simName = "None";
    int branchCount = 0; 
    
    const RigWellResultFrame* wellResultFrame = nullptr;

    if (m_simulationWell && m_simulationWell->wellResults() )// && Timestep Ok )
    {
        simName = m_simulationWell->name();
        wellResultFrame =  &(m_simulationWell->wellResults()->wellResultFrame(1));
        branchCount = static_cast<int>( wellResultFrame->m_wellResultBranches.size());
       // // Todo : Use this instead, and use to calculate accumulated flow
       //
       // size_t timeStep = 0; // make field
       // std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
       // std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;
       // m_simulationWell->calculateWellPipeDynamicCenterLine(timeStep, pipeBranchesCLCoords, pipeBranchesCellIds);
       // branchCount = static_cast<int>( pipeBranchesCLCoords.size());
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

        plotTrack->setDescription(QString("Branch %1").arg(brIdx+1));

    }

    setDescription("Well Allocation (" + simName + ")");
    accumulatedWellFlowPlot()->updateConnectedEditors();
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimTotalWellAllocationPlot::viewWidget()
{
    return m_wellTotalAllocationPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimTotalWellAllocationPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
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
void RimTotalWellAllocationPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_userName ||
        changedField == &m_showPlotTitle)
    {
        updateMdiWindowTitle();
    }
    else if (changedField == &m_simulationWell)
    {
        updateFromWell();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimTotalWellAllocationPlot::snapshotWindowContent()
{
    QImage image;

    // TODO

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::setDescription(const QString& description)
{
    m_userName = description;
    this->updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTotalWellAllocationPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::loadDataAndUpdate()
{
    updateMdiWindowVisibility();
    updateFromWell();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimTotalWellAllocationPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_wellTotalAllocationPlotWidget = new RiuNightchartsWidget(mainWindowParent);
    m_wellTotalAllocationPlotWidget->addItem("Item1", QColor(200, 10, 50), 34);
    m_wellTotalAllocationPlotWidget->addItem("Item2", Qt::green, 27);
    m_wellTotalAllocationPlotWidget->addItem("Item3", Qt::cyan, 14);
    m_wellTotalAllocationPlotWidget->addItem("Item4", Qt::yellow, 7);
    m_wellTotalAllocationPlotWidget->addItem("Item5", Qt::blue, 4);

    return m_wellTotalAllocationPlotWidget;
}


