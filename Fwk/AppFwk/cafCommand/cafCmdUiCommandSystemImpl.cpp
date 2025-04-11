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
#include "cafPdmChildArrayField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmReferenceHelper.h"
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
bool CmdUiCommandSystemImpl::isFieldWritable( PdmFieldHandle* fieldToUpdate ) const
{
    if ( !fieldToUpdate ) return false;

    auto xmlCapability = fieldToUpdate->xmlCapability();
    if ( !xmlCapability ) return false;

    if ( !xmlCapability->isIOWritable() ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdUiCommandSystemImpl::fieldChangedCommand( const std::vector<PdmFieldHandle*>& fieldsToUpdate,
                                                  const QVariant&                     newUiValue )
{
    if ( fieldsToUpdate.empty() ) return;

    std::vector<CmdExecuteCommand*> commands;

    PdmChildArrayFieldHandle* childArrayFieldHandle  = nullptr;
    PdmObjectHandle*          ownerOfChildArrayField = nullptr;
    PdmObjectHandle*          rootObjHandle          = nullptr;

    auto firstField = fieldsToUpdate.front();
    if ( firstField )
    {
        // Find the first childArrayField by traversing parent field and objects. Usually, the childArrayField is
        // the parent, but in some cases when we change fields in a sub-object of the object we need to traverse
        // more levels

        ownerOfChildArrayField = firstField->ownerObject();
        while ( ownerOfChildArrayField )
        {
            if ( ownerOfChildArrayField->parentField() )
            {
                childArrayFieldHandle =
                    dynamic_cast<caf::PdmChildArrayFieldHandle*>( ownerOfChildArrayField->parentField() );
                ownerOfChildArrayField = ownerOfChildArrayField->parentField()->ownerObject();

                if ( childArrayFieldHandle && ownerOfChildArrayField ) break;
            }
            else
            {
                ownerOfChildArrayField = nullptr;
            }
        }

        rootObjHandle = PdmReferenceHelper::findRoot( firstField );
    }

    std::vector<QString> pathsToFields;
    for ( caf::PdmFieldHandle* field : fieldsToUpdate )
    {
        PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if ( uiFieldHandle )
        {
            QVariant fieldCurrentUiValue = uiFieldHandle->uiValue();
            if ( fieldCurrentUiValue != newUiValue )
            {
                QString pathToField = PdmReferenceHelper::referenceFromRootToField( rootObjHandle, field );
                if ( !pathToField.isEmpty() )
                {
                    pathsToFields.push_back( pathToField );
                }
            }
        }
    }

    auto* fieldChangeExec = new CmdFieldChangeExec( SelectionManager::instance()->notificationCenter() );

    fieldChangeExec->commandData()->m_newUiValue             = newUiValue;
    fieldChangeExec->commandData()->m_pathToFields           = pathsToFields;
    fieldChangeExec->commandData()->m_rootObject             = rootObjHandle;
    fieldChangeExec->commandData()->m_ownerOfChildArrayField = ownerOfChildArrayField;
    fieldChangeExec->commandData()->m_childArrayFieldHandle  = childArrayFieldHandle;

    CmdExecCommandManager::instance()->processExecuteCommand( fieldChangeExec );
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
        commandIdList << "PdmListField_AddItem" << "PdmListField_DeleteItem";
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
