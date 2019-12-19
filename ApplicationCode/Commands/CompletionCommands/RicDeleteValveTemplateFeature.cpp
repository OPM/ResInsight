/////////////////////////////////////////////////////////////////////////////////
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
#include "RicDeleteValveTemplateFeature.h"

#include "RimProject.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"
#include "RimWellPathValve.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteValveTemplateFeature, "RicDeleteValveTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteValveTemplateFeature::isCommandEnabled()
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimValveTemplate>() )
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteValveTemplateFeature::onActionTriggered( bool isChecked )
{
    RimValveTemplate* valveTemplate = caf::SelectionManager::instance()->selectedItemOfType<RimValveTemplate>();

    if ( valveTemplate )
    {
        RimProject* project = nullptr;
        valveTemplate->firstAncestorOrThisOfTypeAsserted( project );
        std::vector<RimWellPathValve*> valves;
        project->descendantsIncludingThisOfType( valves );
        for ( RimWellPathValve* valve : valves )
        {
            if ( valve->valveTemplate() == valveTemplate )
            {
                valve->setValveTemplate( nullptr );
                valve->updateAllRequiredEditors();
            }
        }

        RimValveTemplateCollection* collection = nullptr;
        valveTemplate->firstAncestorOrThisOfTypeAsserted( collection );
        collection->removeAndDeleteValveTemplate( valveTemplate );
        collection->updateAllRequiredEditors();

        project->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteValveTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Valve Template" );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}
