/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicShowWellPlanFeature.h"

#include "ApplicationCommands/RicShowPlotDataFeature.h"

#include "RimModeledWellPath.h"

#include "RiuTextDialog.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowWellPlanFeature, "RicShowWellPlanFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowWellPlanFeature::isCommandEnabled() const
{
    auto selectedWellPaths = caf::selectedObjectsByType<RimModeledWellPath*>();
    return !selectedWellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowWellPlanFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    std::vector<RimModeledWellPath*> selectedWellPaths = caf::selectedObjectsByType<RimModeledWellPath*>();
    if ( selectedWellPaths.empty() )
    {
        return;
    }

    for ( auto wellPath : selectedWellPaths )
    {
        QString title = "Well Plan for " + wellPath->name();

        RiuTextDialog* textDialog = new RiuTextDialog();
        textDialog->setMinimumSize( 800, 600 );
        textDialog->setWindowTitle( title );
        textDialog->setText( wellPath->wellPlanText() );
        textDialog->show();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowWellPlanFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Show Well Plan" );
    actionToSetup->setIcon( QIcon( ":/PlotWindow.svg" ) );
}
