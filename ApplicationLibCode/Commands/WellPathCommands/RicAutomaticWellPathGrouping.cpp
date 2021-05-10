/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RicAutomaticWellPathGrouping.h"

#include "RiaPreferences.h"
#include "RigWellPath.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGroup.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QRegExp>

RICF_SOURCE_INIT( RicAutomaticWellPathGrouping, "RicAutomaticWellPathGroupingFeature", "autoGroupWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicAutomaticWellPathGrouping::RicAutomaticWellPathGrouping()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPaths, "wellPaths", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicAutomaticWellPathGrouping::execute()
{
    caf::PdmScriptResponse response;

    auto wellPaths = m_wellPaths.ptrReferencedObjects();
    if ( wellPaths.empty() )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_WARNING, "No well paths provided" );
    }

    RimProject* project = RimProject::current();

    if ( project )
    {
        project->scheduleCreateDisplayModelAndRedrawAllViews();
        RimOilField* oilField = project->activeOilField();

        if ( oilField )
        {
            // oilField->wellPathCollection->groupWellPaths( wellPaths, true );

            oilField->wellPathCollection->rebuildWellPathNodes();

            return caf::PdmScriptResponse();
        }
    }
    return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "No project open" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAutomaticWellPathGrouping::isCommandEnabled()
{
    return !selectedWellPaths().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAutomaticWellPathGrouping::onActionTriggered( bool isChecked )
{
    m_wellPaths.setValue( selectedWellPaths() );
    execute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAutomaticWellPathGrouping::setupActionLook( QAction* actionToSetup )
{
    auto wellPathCollection = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCollection>();
    if ( wellPathCollection )
    {
        actionToSetup->setText( "Automatically Create Multi-Lateral Wells" );
        actionToSetup->setIcon( QIcon( ":/WellPathGroup.svg" ) );
    }
    else
    {
        actionToSetup->setText( "Automatically Create Multi-Lateral Wells from Selected Paths" );
        actionToSetup->setIcon( QIcon( ":/WellPathGroup.svg" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicAutomaticWellPathGrouping::selectedWellPaths()
{
    std::vector<RimWellPath*> wellPaths;

    auto wellPathCollection = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCollection>();
    if ( wellPathCollection )
    {
        wellPaths = wellPathCollection->allWellPaths();
    }
    else
    {
        caf::SelectionManager::instance()->objectsByTypeStrict( &wellPaths );
    }

    QString multiLateralWellPathPattern = RiaPreferences::current()->multiLateralWellNamePattern();
    QRegExp re( multiLateralWellPathPattern, Qt::CaseInsensitive, QRegExp::Wildcard );

    std::vector<RimWellPath*> multiLateralWellPaths;
    for ( auto wellPath : wellPaths )
    {
        if ( re.exactMatch( wellPath->name() ) )
        {
            multiLateralWellPaths.push_back( wellPath );
        }
    }

    return multiLateralWellPaths;
}
