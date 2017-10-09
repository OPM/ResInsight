/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicWellLogTools.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseWell.h"
#include "RimProject.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "RifReaderEclipseRft.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionManager.h"

#include "WellLogCommands\RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicWellLogTools::selectedWellLogPlotTrack()
{
    std::vector<RimWellLogTrack*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell* RicWellLogTools::selectedSimulationWell(int *branchIndex)
{
    RiuSelectionItem* selItem = RiuSelectionManager::instance()->selectedItem(RiuSelectionManager::RUI_TEMPORARY);
    RiuSimWellSelectionItem* simWellSelItem = dynamic_cast<RiuSimWellSelectionItem*>(selItem);
    if (simWellSelItem)
    {
        (*branchIndex) = static_cast<int>(simWellSelItem->m_branchIndex);
        return simWellSelItem->m_simWell;
    }
    else
    {
        std::vector<RimEclipseWell*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        (*branchIndex) = 0;
        return selection.size() > 0 ? selection[0] : nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicWellLogTools::selectedWellPath()
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellLogTools::wellHasRftData(const QString& wellName)
{
    RimEclipseResultCase* resultCase;
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    for (RimCase* rimCase : cases)
    {
        if (resultCase = dynamic_cast<RimEclipseResultCase*>(rimCase))
        {
            return resultCase->rftReader()->wellHasRftData(wellName);
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogTools::addWellLogChannelsToPlotTrack(RimWellLogTrack* plotTrack, const std::vector<RimWellLogFileChannel*>& wellLogFileChannels)
{
    for (size_t cIdx = 0; cIdx < wellLogFileChannels.size(); cIdx++)
    {
        RimWellLogFileCurve* plotCurve = RicWellLogTools::addFileCurve(plotTrack);

        RimWellPath* wellPath;
        wellLogFileChannels[cIdx]->firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            plotCurve->setWellPath(wellPath);
            plotCurve->setWellLogChannelName(wellLogFileChannels[cIdx]->name());
            plotCurve->loadDataAndUpdate(true);
            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicWellLogTools::selectedWellPathWithLogFile()
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    if (selection.size() > 0)
    {
        RimWellPath* wellPath = selection[0];
        if (wellPath->wellLogFile())
        {
            return wellPath;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RicWellLogTools::addExtractionCurve(RimWellLogTrack* plotTrack, RimView* view, RimWellPath* wellPath, const RimEclipseWell* simWell, int branchIndex)
{
    CVF_ASSERT(plotTrack);
    RimWellLogExtractionCurve* curve = new RimWellLogExtractionCurve();

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plotTrack->curveCount());
    curve->setColor(curveColor);
    if (wellPath) curve->setWellPath(wellPath);
    if (simWell) curve->setFromSimulationWellName(simWell->name(), branchIndex);

    curve->setPropertiesFromView(view);

    plotTrack->addCurve(curve);

    plotTrack->updateConnectedEditors();

    // Make sure the summary plot window is created and visible
    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

    RiaApplication::instance()->project()->updateConnectedEditors();

    plotwindow->selectAsCurrentItem(curve);

    return curve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve* RicWellLogTools::addRftCurve(RimWellLogTrack* plotTrack, const RimEclipseWell* simWell)
{
    CVF_ASSERT(plotTrack);

    RimWellLogRftCurve* curve = new RimWellLogRftCurve();

    RimEclipseResultCase* resultCase;

    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    for (RimCase* rimCase : cases)
    {
        if (resultCase = dynamic_cast<RimEclipseResultCase*>(rimCase))
        {
            break;
        }
    }

    if (simWell && resultCase)
    {
        curve->setEclipseResultCase(resultCase);
        curve->setDefaultAddress(simWell->name());
    }

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plotTrack->curveCount());
    curve->setColor(curveColor);

    plotTrack->addCurve(curve);
    plotTrack->updateConnectedEditors();

    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

    RiaApplication::instance()->project()->updateConnectedEditors();

    plotwindow->selectAsCurrentItem(curve);

    return curve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve* RicWellLogTools::addFileCurve(RimWellLogTrack* plotTrack)
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
