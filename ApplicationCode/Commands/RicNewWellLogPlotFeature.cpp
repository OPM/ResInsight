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

#include "RicNewWellLogPlotFeature.h"

#include "RimProject.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrace.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"

#include <QAction>

#include "cvfAssert.h"


CAF_CMD_SOURCE_INIT(RicNewWellLogPlotFeature, "RicNewWellLogPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotFeature::onActionTriggered(bool isChecked)
{
    RimWellLogPlotCollection* wellLogPlotColl = wellLogPlotCollection();
    CVF_ASSERT(wellLogPlotColl);

    RimWellLogPlot* plot = new RimWellLogPlot();
    wellLogPlotColl->wellLogPlots().push_back(plot);

    RimWellLogPlotTrace* plotrace = new RimWellLogPlotTrace();
    plot->addTrace(plotrace);
        
    plot->setDescription(QString("Well Log Plot %1").arg(wellLogPlotCollection()->wellLogPlots.size()));
    plot->loadDataAndUpdate();

    RiaApplication::instance()->project()->updateConnectedEditors();
    RiuMainWindow::instance()->setCurrentObjectInTreeView(plot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Log Plot");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection* RicNewWellLogPlotFeature::mainPlotCollection()
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    if (!mainPlotColl)
    {
        project->recreateMainPlotCollection();
    }

    return project->mainPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RicNewWellLogPlotFeature::wellLogPlotCollection()
{
    RimMainPlotCollection* mainPlotColl = mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimWellLogPlotCollection* wellLogPlotColl = mainPlotColl->wellLogPlotCollection();
    if (!wellLogPlotColl)
    {
        mainPlotColl->recreateWellLogPlotCollection();
    }

    return mainPlotColl->wellLogPlotCollection();
}
