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

#include "RicSaveEclipseResultAsInputPropertyFeature.h"

#include "RicSaveEclipseResultAsInputPropertyExec.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicSaveEclipseResultAsInputPropertyFeature, "RicSaveEclipseResultAsInputPropertyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveEclipseResultAsInputPropertyFeature::isCommandEnabled()
{
    return selectedEclipseCellColors() != nullptr || selectedEclipseView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicSaveEclipseResultAsInputPropertyFeature::selectedEclipseView() const
{
    std::vector<RimEclipseView*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( selection.size() > 0 )
    {
        return selection[0];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors* RicSaveEclipseResultAsInputPropertyFeature::selectedEclipseCellColors() const
{
    std::vector<RimEclipseCellColors*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( selection.size() > 0 )
    {
        return selection[0];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputPropertyFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RimEclipseCellColors* eclipseCellColors = selectedEclipseCellColors();
    if ( !eclipseCellColors )
    {
        // Get the cell colors from the eclipse view if possible.
        RimEclipseView* eclipseView = selectedEclipseView();
        if ( eclipseView )
        {
            eclipseCellColors = eclipseView->cellResult();
        }
    }

    if ( eclipseCellColors )
    {
        RicSaveEclipseResultAsInputPropertyExec* cellResultSaveExec =
            new RicSaveEclipseResultAsInputPropertyExec( eclipseCellColors );
        caf::CmdExecCommandManager::instance()->processExecuteCommand( cellResultSaveExec );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputPropertyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Property To File" );
}
