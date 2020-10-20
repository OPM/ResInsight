////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicReloadPlotTemplatesFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "PlotTemplates/RimPlotTemplateFolderItem.h"
#include "RimProject.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadPlotTemplatesFeature, "RicReloadPlotTemplatesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPlotTemplatesFeature::rebuildFromDisc()
{
    RimProject*     proj  = RimProject::current();
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    proj->setPlotTemplateFolders( prefs->plotTemplateFolders() );
    proj->rootPlotTemlateItem()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadPlotTemplatesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPlotTemplatesFeature::onActionTriggered( bool isChecked )
{
    RicReloadPlotTemplatesFeature::rebuildFromDisc();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPlotTemplatesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload Templates" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );
}
