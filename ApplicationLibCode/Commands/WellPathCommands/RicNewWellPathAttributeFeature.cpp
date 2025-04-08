/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RicNewWellPathAttributeFeature.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellPathAttributeFeature, "RicNewWellPathAttributeFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathAttributeFeature::isCommandEnabled() const
{
    {
        const auto objects = caf::SelectionManager::instance()->objectsByType<RimWellPathAttribute>( caf::SelectionManager::FIRST_LEVEL );
        if ( !objects.empty() )
        {
            return true;
        }
    }

    {
        if ( caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>() ||
             caf::SelectionManager::instance()->selectedItemOfType<RimWellPathAttributeCollection>() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathAttributeFeature::onActionTriggered( bool isChecked )
{
    const auto attributes = caf::SelectionManager::instance()->objectsByType<RimWellPathAttribute>( caf::SelectionManager::FIRST_LEVEL );
    RimWellPathAttribute* attribute = nullptr;
    if ( attributes.size() == 1u )
    {
        auto attributeCollection = attributes[0]->firstAncestorOrThisOfTypeAsserted<RimWellPathAttributeCollection>();

        attribute = new RimWellPathAttribute;

        auto wellPath = attributeCollection->firstAncestorOrThisOfType<RimWellPath>();
        attribute->setDepthsFromWellPath( wellPath );
        attributeCollection->insertAttribute( attributes[0], attribute );

        attributeCollection->updateAllRequiredEditors();
    }
    else
    {
        RimWellPath* wellPath = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
        if ( wellPath )
        {
            std::vector<RimWellPathAttributeCollection*> attributeCollections =
                wellPath->descendantsIncludingThisOfType<RimWellPathAttributeCollection>();
            if ( !attributeCollections.empty() )
            {
                attribute = new RimWellPathAttribute;
                attribute->setDepthsFromWellPath( wellPath );

                attributeCollections[0]->insertAttribute( nullptr, attribute );
                attributeCollections[0]->updateAllRequiredEditors();

                wellPath->updateConnectedEditors();
                Riu3DMainWindowTools::selectAsCurrentItem( attributeCollections[0] );
            }
        }
    }

    if ( attribute )
    {
        RimProject* project = RimProject::current();
        project->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathAttributeFeature::setupActionLook( QAction* actionToSetup )
{
    const auto attributes = caf::SelectionManager::instance()->objectsByType<RimWellPathAttribute>( caf::SelectionManager::FIRST_LEVEL );
    if ( attributes.size() == 1u )
    {
        actionToSetup->setText( QString( "Insert New Attribute before %1" ).arg( attributes[0]->componentTypeLabel() ) );
        actionToSetup->setIcon( QIcon( ":/CasingDesign16x16.png" ) );
    }
    else if ( caf::SelectionManager::instance()->selectedItemOfType<RimWellPathAttributeCollection>() )
    {
        actionToSetup->setText( "Append New Attribute" );
        actionToSetup->setIcon( QIcon( ":/CasingDesign16x16.png" ) );
    }
    else if ( caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>() )
    {
        actionToSetup->setText( "Create Casing Design" );
        actionToSetup->setIcon( QIcon( ":/CasingDesign16x16.png" ) );
    }
}
