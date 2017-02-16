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

#include "RicNewWellLogFileCurveFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RiaApplication.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewWellLogFileCurveFeature, "RicNewWellLogFileCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogFileCurveFeature::isCommandEnabled()
{
    return (selectedWellLogPlotTrack() != NULL && wellLogFilesAvailable()) || selectedWellPathWithLogFile() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogFileCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogTrack* wellLogPlotTrack = selectedWellLogPlotTrack();
    if (wellLogPlotTrack)
    {
        addCurve(wellLogPlotTrack);
    }
    else
    {
        RimWellPath* wellPath = selectedWellPathWithLogFile();
        if (wellPath)
        {
            RimWellLogTrack* wellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogFileCurve* plotCurve = addCurve(wellLogPlotTrack);
            plotCurve->setWellPath(wellPath);
            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogFileCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Log LAS Curve");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewWellLogFileCurveFeature::selectedWellLogPlotTrack() const
{
    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewWellLogFileCurveFeature::selectedWellPathWithLogFile() const
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    if (selection.size() > 0)
    {
        RimWellPath* wellPath = selection[0];
        if (wellPath->m_wellLogFile())
        {
            return wellPath;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogFileCurveFeature::wellLogFilesAvailable() const
{
    RimProject* project = RiaApplication::instance()->project();
    if (project->activeOilField()->wellPathCollection())
    {
        caf::PdmChildArrayField<RimWellPath*>& wellPaths = project->activeOilField()->wellPathCollection()->wellPaths;

        for (size_t i = 0; i < wellPaths.size(); i++)
        {
            if (wellPaths[i]->m_wellLogFile())
            {
                if (wellPaths[i]->m_wellLogFile()->wellLogFile())
                {
                    return true;
                }
            }
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve* RicNewWellLogFileCurveFeature::addCurve(RimWellLogTrack* plotTrack)
{
    CVF_ASSERT(plotTrack);

    RimWellLogFileCurve* curve = new RimWellLogFileCurve();

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plotTrack->curveCount());
    curve->setColor(curveColor);

    plotTrack->addCurve(curve);

    plotTrack->updateConnectedEditors();

    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
    plotwindow->selectAsCurrentItem(curve);

    return curve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogFileCurveFeature::addWellLogChannelsToPlotTrack(RimWellLogTrack* plotTrack, const std::vector<RimWellLogFileChannel*>& wellLogFileChannels)
{
    for (size_t cIdx = 0; cIdx < wellLogFileChannels.size(); cIdx++)
    {
        RimWellLogFileCurve* plotCurve = addCurve(plotTrack);
    
        RimWellPath* wellPath;
        wellLogFileChannels[cIdx]->firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            plotCurve->setWellPath(wellPath);
            plotCurve->setWellLogChannelName(wellLogFileChannels[cIdx]->name());
            plotCurve->loadDataAndUpdate();
            plotCurve->updateConnectedEditors();
        }
    }
}
