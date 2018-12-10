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

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RiaApplication.h"

#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewWellLogFileCurveFeature, "RicNewWellLogFileCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogFileCurveFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()) return false;
    return (caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellLogTrack>() != nullptr && wellLogFilesAvailable()) || RicWellLogTools::selectedWellPathWithLogFile() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogFileCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogTrack* wellLogPlotTrack = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellLogTrack>();
    if (wellLogPlotTrack)
    {
        RicWellLogTools::addFileCurve(wellLogPlotTrack);
    }
    else
    {
        RimWellPath* wellPath = RicWellLogTools::selectedWellPathWithLogFile();
        if (wellPath)
        {
            RimWellLogTrack* newWellLogPlotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
            RimWellLogFileCurve* plotCurve = RicWellLogTools::addFileCurve(newWellLogPlotTrack);
            plotCurve->setWellPath(wellPath);

            if (wellPath->wellLogFiles().size() == 1)
            {
                plotCurve->setWellLogFile(wellPath->wellLogFiles().front());
            }
            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogFileCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
    actionToSetup->setText("New Well Log LAS Curve");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogFileCurveFeature::wellLogFilesAvailable()
{
    auto wellPathCollection = RimTools::wellPathCollection();
    if (wellPathCollection)
    {
        caf::PdmChildArrayField<RimWellPath*>& wellPaths = wellPathCollection->wellPaths;

        for (size_t i = 0; i < wellPaths.size(); i++)
        {
            if (!wellPaths[i]->wellLogFiles().empty())
            {
                return true;
            }
        }
    }

    return false;
}
