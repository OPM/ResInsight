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

#include "RicNewGridPlotWindowFeature.h"

#include "RiaApplication.h"

#include "RimGridPlotWindow.h"
#include "RimGridPlotWindowCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"
#include <QAction>

#include "cvfAssert.h"

CAF_CMD_SOURCE_INIT( RicNewGridPlotWindowFeature, "RicNewGridPlotWindowFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewGridPlotWindowFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGridPlotWindowFeature::onActionTriggered( bool isChecked )
{
    RimProject*                  project        = RiaApplication::instance()->project();
    RimGridPlotWindowCollection* plotCollection = project->mainPlotCollection()->combinationPlotCollection();

    RimGridPlotWindow* plotWindow = new RimGridPlotWindow;
    plotWindow->setDescription( QString( "Combination Plot %1" ).arg( plotCollection->gridPlotWindows().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addGridPlotWindow( plotWindow );
    plotCollection->updateAllRequiredEditors();

    plotWindow->loadDataAndUpdate();

    RiuPlotMainWindowTools::setExpanded( plotCollection, true );
    RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewGridPlotWindowFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Empty Combination Plot" );
    actionToSetup->setIcon( QIcon( ":/WellLogPlot16x16.png" ) );
}
