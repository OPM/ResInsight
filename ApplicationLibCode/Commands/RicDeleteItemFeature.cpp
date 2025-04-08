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

#include "RicDeleteItemFeature.h"
#include "RicDeleteItemExec.h"
#include "RicDeleteItemExecData.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdSelectionHelper.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteItemFeature, "RicDeleteItemFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteItemFeature::isCommandEnabled() const
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();

    if ( selectedItems.empty() ) return false;

    for ( caf::PdmUiItem* item : selectedItems )
    {
        auto* currentPdmObject = dynamic_cast<caf::PdmObject*>( item );
        if ( !currentPdmObject ) return false;

        if ( !currentPdmObject->isDeletable() ) return false;

        auto* childArrayFieldHandle = dynamic_cast<caf::PdmChildArrayFieldHandle*>( currentPdmObject->parentField() );
        if ( !childArrayFieldHandle ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::onActionTriggered( bool isChecked )
{
    for ( caf::PdmUiItem* item : caf::SelectionManager::instance()->selectedItems() )
    {
        auto* currentPdmObject = dynamic_cast<caf::PdmObject*>( item );
        if ( !currentPdmObject ) continue;

        deleteObject( currentPdmObject );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::deleteObject( caf::PdmObject* objectToDelete )
{
    if ( !objectToDelete || !objectToDelete->isDeletable() ) return;

    auto* childArrayFieldHandle = dynamic_cast<caf::PdmChildArrayFieldHandle*>( objectToDelete->parentField() );
    if ( !childArrayFieldHandle ) return;

    int indexToObject = -1;

    std::vector<caf::PdmObjectHandle*> childObjects = childArrayFieldHandle->children();

    for ( size_t i = 0; i < childObjects.size(); i++ )
    {
        if ( childObjects[i] == objectToDelete )
        {
            indexToObject = static_cast<int>( i );
        }
    }

    // Did not find object in the current list field
    if ( indexToObject == -1 ) return;

    auto* executeCmd = new RicDeleteItemExec( caf::SelectionManager::instance()->notificationCenter() );

    RicDeleteItemExecData& data = executeCmd->commandData();
    data.m_rootObject           = caf::PdmReferenceHelper::findRoot( childArrayFieldHandle );
    data.m_pathToField          = caf::PdmReferenceHelper::referenceFromRootToField( data.m_rootObject, childArrayFieldHandle );
    data.m_indexToObject        = indexToObject;

    {
        QString desc;
        if ( objectToDelete->userDescriptionField() )
        {
            desc = objectToDelete->userDescriptionField()->uiCapability()->uiValue().toString();
        }
        else
        {
            desc = objectToDelete->uiName();
        }

        data.m_description = desc + " (delete)";
    }

    // When the delete feature is ready for redo/undo, activate by using the line below
    // Temporarily do not insert these object into undo/redo system as this requires a lot of testing
    // for reinserting objects.
    bool useUndoRedo = false;
    if ( useUndoRedo )
    {
        caf::CmdExecCommandManager::instance()->processExecuteCommand( executeCmd );
    }
    else
    {
        executeCmd->redo();
        delete executeCmd;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}
