//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafCmdUiCommandSystemImpl.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdExecuteCommand.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFieldChangeExec.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmUiObjectHandle.h"

#include "cafSelectionManager.h"

#include <QMenu>

#include <vector>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdUiCommandSystemImpl::CmdUiCommandSystemImpl()
{
    m_undoFeatureEnabled        = false;
    m_disableUndoForFieldChange = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::fieldChangedCommand( const std::vector<PdmFieldHandle*>& fieldsToUpdate,
                                                  const QVariant&                     newUiValue )
{
    if ( fieldsToUpdate.empty() ) return;

    std::vector<CmdExecuteCommand*> commands;

    for ( size_t i = 0; i < fieldsToUpdate.size(); i++ )
    {
        PdmFieldHandle*   field         = fieldsToUpdate[i];
        PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if ( uiFieldHandle )
        {
            QVariant fieldCurrentUiValue = uiFieldHandle->uiValue();

            if ( fieldCurrentUiValue != newUiValue )
            {
                PdmObjectHandle* rootObjHandle = PdmReferenceHelper::findRoot( field );

                QString reference = PdmReferenceHelper::referenceFromRootToField( rootObjHandle, field );
                if ( reference.isEmpty() )
                {
                    CAF_ASSERT( false );
                    return;
                }

                CmdFieldChangeExec* fieldChangeExec =
                    new CmdFieldChangeExec( SelectionManager::instance()->notificationCenter() );

                fieldChangeExec->commandData()->m_newUiValue  = newUiValue;
                fieldChangeExec->commandData()->m_pathToField = reference;
                fieldChangeExec->commandData()->m_rootObject  = rootObjHandle;

                commands.push_back( fieldChangeExec );
            }
        }
    }

    caf::PdmUiObjectHandle* uiOwnerObjectHandle = uiObj( fieldsToUpdate[0]->ownerObject() );
    if ( uiOwnerObjectHandle && !uiOwnerObjectHandle->useUndoRedoForFieldChanged() )
    {
        // Temporarily disable undo framework as requested by the PdmUiObjectHandle
        m_disableUndoForFieldChange = true;
    }

    if ( commands.size() == 1 )
    {
        CmdExecCommandManager::instance()->processExecuteCommand( commands[0] );
    }
    else
    {
        CmdExecCommandManager::instance()->processExecuteCommandsAsMacro( commands );
    }

    if ( uiOwnerObjectHandle && !uiOwnerObjectHandle->useUndoRedoForFieldChanged() )
    {
        // Restore undo feature to normal operation
        m_disableUndoForFieldChange = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::populateMenuWithDefaultCommands( const QString& uiConfigName, QMenu* menu )
{
    if ( uiConfigName == "PdmUiTreeViewEditor" || uiConfigName == "PdmUiTableViewEditor" )
    {
        caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();

        menu->addAction( commandManager->action( "PdmListField_AddItem" ) );
        menu->addAction( commandManager->action( "PdmListField_DeleteItem" ) );

        QStringList commandIdList;
        commandIdList << "PdmListField_AddItem"
                      << "PdmListField_DeleteItem";
        commandManager->refreshStates( commandIdList );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdUiCommandSystemImpl::isUndoEnabled()
{
    return m_undoFeatureEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::enableUndoFeature( bool enable )
{
    m_undoFeatureEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdUiCommandSystemImpl::disableUndoForFieldChange()
{
    return m_disableUndoForFieldChange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::setCurrentContextMenuTargetWidget( QWidget* targetWidget )
{
    caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget( targetWidget );
}

} // end namespace caf
