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
#include "RicToggleWellPathGrouping.h"

#include "RigWellPath.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafSelectionManager.h"

#include <QAction>

RICF_SOURCE_INIT( RicToggleWellPathGrouping, "RicToggleWellPathGroupingFeature", "groupWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicToggleWellPathGrouping::RicToggleWellPathGrouping()
{
    CAF_PDM_InitScriptableField( &m_groupeWellPaths, "group", false, "", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPaths, "wellPaths", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicToggleWellPathGrouping::execute()
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
            if ( m_groupeWellPaths() )
            {
                oilField->wellPathCollection->groupWellPaths( wellPaths );
                return caf::PdmScriptResponse();
            }
            else
            {
                oilField->wellPathCollection->ungroupWellPaths( wellPaths );
                return caf::PdmScriptResponse();
            }
        }
    }
    return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "No project open" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleWellPathGrouping::isCommandEnabled()
{
    auto wellPaths = selectedWellPaths();
    return wellPaths.size() > 1u && containsUngroupedWellPathsWithCommonGeometry( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleWellPathGrouping::onActionTriggered( bool isChecked )
{
    auto wellPaths = selectedWellPaths();

    if ( containsUngroupedWellPathsWithCommonGeometry( wellPaths ) )
    {
        m_groupeWellPaths = true;
    }
    else if ( containsGroupedWellPaths( wellPaths ) )
    {
        m_groupeWellPaths = false;
    }
    else
    {
        return;
    }

    m_wellPaths.setValue( wellPaths );
    execute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleWellPathGrouping::setupActionLook( QAction* actionToSetup )
{
    auto wellPaths = selectedWellPaths();
    if ( containsUngroupedWellPathsWithCommonGeometry( wellPaths ) )
    {
        actionToSetup->setText( "Group the selected well paths into a Well Tree" );
        actionToSetup->setIcon( QIcon( ":/Well.png" ) );
    }
    else if ( containsGroupedWellPaths( wellPaths ) )
    {
        actionToSetup->setText( "Ungroup the selected well paths" );
        actionToSetup->setIcon( QIcon( ":/Well.png" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicToggleWellPathGrouping::selectedWellPaths()
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByTypeStrict( &wellPaths );
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleWellPathGrouping::containsGroupedWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    // TODO: add real check
    return std::any_of( wellPaths.begin(), wellPaths.end(), []( RimWellPath* wellPath ) { return true; } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleWellPathGrouping::containsUngroupedWellPathsWithCommonGeometry( const std::vector<RimWellPath*>& wellPaths )
{
    // TODO: add check for whether the well paths are grouped.
    std::vector<const RigWellPath*> geometries;
    for ( auto wellPath : wellPaths )
    {
        geometries.push_back( wellPath->wellPathGeometry() );
    }
    RigWellPath commonGeometry = RigWellPath::commonGeometry( geometries );
    return !commonGeometry.wellPathPoints().empty();
}
