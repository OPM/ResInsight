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

#include <QString>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RicNewWellLogPlotFeatureImpl::createWellLogPlot(bool showAfterCreation, const QString& plotDescription)
{
    RimWellLogPlotCollection* wellLogPlotColl = wellLogPlotCollection();
    CVF_ASSERT(wellLogPlotColl);

    RimWellLogPlot* plot = new RimWellLogPlot();
    plot->setAsPlotMdiWindow();
    
    wellLogPlotColl->wellLogPlots().push_back(plot);

    // Make sure the summary plot window is created
    RiaApplication::instance()->getOrCreateMainPlotWindow();

    if (!plotDescription.isEmpty())
    {
        plot->setDescription(plotDescription);
    }
    else
    {
        plot->setDescription(QString("Well Log Plot %1").arg(wellLogPlotCollection()->wellLogPlots.size()));
    }

    if (showAfterCreation)
    {
        RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
    }

    return plot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack(bool updateAfter, const QString& trackDescription, RimWellLogPlot* existingPlot)
{
    RimWellLogPlot* plot = existingPlot;
    if (plot == nullptr)
    {
        plot = createWellLogPlot();
    }

    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addTrack(plotTrack);
    if (!trackDescription.isEmpty())
    {
        plotTrack->setDescription(trackDescription);
    }
    else
    {
        plotTrack->setDescription(QString("Track %1").arg(plot->trackCount()));
    }

    if (updateAfter)
    {
        updateAfterCreation(plot);
    }

    return plotTrack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotFeatureImpl::updateAfterCreation(RimWellLogPlot* plot)
{
    CVF_ASSERT(plot);
    plot->loadDataAndUpdate();
    plot->updateDepthZoom();
    plot->updateConnectedEditors();
    RiaApplication::instance()->project()->updateConnectedEditors();
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
