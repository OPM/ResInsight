/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicLinkWellPathFeature.h"

#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"

#include "Riu3dSelectionManager.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicLinkWellPathFeature, "RicLinkWellPathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkWellPathFeature::isCommandEnabled()
{
    return ( !wellPaths().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkWellPathFeature::onActionTriggered( bool isChecked )
{
    for ( auto w : wellPaths() )
    {
        if ( auto modeledWell = dynamic_cast<RimModeledWellPath*>( w ) )
        {
            auto geoDef = modeledWell->geometryDefinition();
            geoDef->enableLinkOfReferencePointUpdates( isChecked );
            geoDef->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    QString text = "Link Reference Point";
    actionToSetup->setText( text );
    actionToSetup->setCheckable( true );
    actionToSetup->setChecked( isCommandChecked() );

    actionToSetup->setIcon( QIcon( ":/chain.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkWellPathFeature::isCommandChecked()
{
    if ( !wellPaths().empty() )
    {
        auto firstWell = dynamic_cast<RimModeledWellPath*>( wellPaths().front() );
        if ( firstWell )
        {
            if ( auto geoDef = firstWell->geometryDefinition() )
            {
                return geoDef->isReferencePointUpdatesLinked();
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicLinkWellPathFeature::wellPaths()
{
    std::vector<RimWellPath*> wellPaths;

    auto wellPathSelectionItem = RiuWellPathSelectionItem::wellPathSelectionItem();
    if ( wellPathSelectionItem && wellPathSelectionItem->m_wellpath )
    {
        if ( auto modeledWellPath =
                 dynamic_cast<RimModeledWellPath*>( wellPathSelectionItem->m_wellpath->topLevelWellPath() ) )
        {
            wellPaths.push_back( modeledWellPath );
        }
    }

    auto selectedWells = caf::selectedObjectsByTypeStrict<RimWellPath*>();
    for ( auto w : selectedWells )
    {
        wellPaths.push_back( w->topLevelWellPath() );
    }

    return wellPaths;
}
