/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RicNewRftPlotFeature.h"

#include "RiaApplication.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSimWellInView.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellRftPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewRftPlotFeature, "RicNewRftPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewRftPlotFeature::isCommandEnabled()
{
    RimRftPlotCollection* simWell = caf::firstAncestorOfTypeFromSelectedObject<RimRftPlotCollection*>();
    if (simWell) return true;
    
    if (selectedWellName().isEmpty())
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewRftPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* proj = RiaApplication::instance()->project();

    RimRftPlotCollection* rftPlotColl = proj->mainPlotCollection()->rftPlotCollection();
    if (rftPlotColl)
    {
        QString wellName = selectedWellName();

        QString plotName = QString(RimWellRftPlot::plotNameFormatString()).arg(wellName);

        RimWellRftPlot* rftPlot = new RimWellRftPlot();
        rftPlot->setSimWellOrWellPathName(wellName);

        RimWellLogTrack* plotTrack = new RimWellLogTrack();
        rftPlot->wellLogPlot()->addTrack(plotTrack);
        plotTrack->setDescription(QString("Track %1").arg(rftPlot->wellLogPlot()->trackCount()));

        rftPlotColl->addPlot(rftPlot);
        rftPlot->setDescription(plotName);

        rftPlot->applyInitialSelections();
        rftPlot->loadDataAndUpdate();
        rftPlotColl->updateConnectedEditors();

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::setExpanded(plotTrack);
        RiuPlotMainWindowTools::selectAsCurrentItem(rftPlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewRftPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New RFT Plot");
    actionToSetup->setIcon(QIcon(":/FlowCharPlot16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewRftPlotFeature::selectedWellName()
{
    RimSimWellInView* simWell = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView*>();
    if (simWell) return simWell->name();

    RimWellPath* rimWellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>();
    if (rimWellPath) return rimWellPath->name();

    return QString();
}
