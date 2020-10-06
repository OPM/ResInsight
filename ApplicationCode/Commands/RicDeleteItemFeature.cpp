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
bool RicDeleteItemFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems( items );

    if ( items.empty() ) return false;

    for ( caf::PdmUiItem* item : items )
    {
        caf::PdmObject* currentPdmObject = dynamic_cast<caf::PdmObject*>( item );
        if ( !currentPdmObject ) return false;

        if ( !currentPdmObject->isDeletable() ) return false;

        caf::PdmChildArrayFieldHandle* childArrayFieldHandle =
            dynamic_cast<caf::PdmChildArrayFieldHandle*>( currentPdmObject->parentField() );
        if ( !childArrayFieldHandle ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems( items );
    assert( items.size() > 0 );

    for ( caf::PdmUiItem* item : items )
    {
        caf::PdmObject* currentPdmObject = dynamic_cast<caf::PdmObject*>( item );
        if ( !currentPdmObject ) continue;

        if ( !currentPdmObject->isDeletable() ) continue;

        caf::PdmChildArrayFieldHandle* childArrayFieldHandle =
            dynamic_cast<caf::PdmChildArrayFieldHandle*>( currentPdmObject->parentField() );
        if ( !childArrayFieldHandle ) continue;

        int indexAfter = -1;

        std::vector<caf::PdmObjectHandle*> childObjects;
        childArrayFieldHandle->childObjects( &childObjects );

        for ( size_t i = 0; i < childObjects.size(); i++ )
        {
            if ( childObjects[i] == currentPdmObject )
            {
                indexAfter = static_cast<int>( i );
            }
        }

        // Did not find currently selected pdm object in the current list field
        assert( indexAfter != -1 );

        RicDeleteItemExec* executeCmd = new RicDeleteItemExec( caf::SelectionManager::instance()->notificationCenter() );

        RicDeleteItemExecData* data = executeCmd->commandData();
        data->m_rootObject          = caf::PdmReferenceHelper::findRoot( childArrayFieldHandle );
        data->m_pathToField =
            caf::PdmReferenceHelper::referenceFromRootToField( data->m_rootObject, childArrayFieldHandle );
        data->m_indexToObject = indexAfter;

        caf::CmdExecCommandManager::instance()->processExecuteCommand( executeCmd );
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
