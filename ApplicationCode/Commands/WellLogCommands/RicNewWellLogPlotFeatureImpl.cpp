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

#include "RicNewWellLogPlotFeatureImpl.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"

#include "RiaApplication.h"

#include "cvfAssert.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RicNewWellLogPlotFeatureImpl::createWellLogPlot()
{
    RimWellLogPlotCollection* wellLogPlotColl = wellLogPlotCollection();
    CVF_ASSERT(wellLogPlotColl);

    RimWellLogPlot* plot = new RimWellLogPlot();
    plot->setAsPlotMdiWindow();
    wellLogPlotColl->wellLogPlots().push_back(plot);

    // Make sure the summary plot window is created and visible
    RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

    plot->setDescription(QString("Well Log Plot %1").arg(wellLogPlotCollection()->wellLogPlots.size()));

    return plot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack()
{
    RimWellLogPlot* plot = createWellLogPlot();

    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addTrack(plotTrack);
    plotTrack->setDescription(QString("Track %1").arg(plot->trackCount()));

    plot->loadDataAndUpdate();
    plot->updateConnectedEditors();
    RiaApplication::instance()->project()->updateConnectedEditors();

    return plotTrack;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RicNewWellLogPlotFeatureImpl::wellLogPlotCollection()
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimWellLogPlotCollection* wellLogPlotColl = mainPlotColl->wellLogPlotCollection();
    CVF_ASSERT(wellLogPlotColl);

    return mainPlotColl->wellLogPlotCollection();
}
