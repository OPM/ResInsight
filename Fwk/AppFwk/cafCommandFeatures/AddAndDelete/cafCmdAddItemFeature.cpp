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

#include "cafCmdAddItemFeature.h"

#include "cafCmdAddItemExec.h"
#include "cafCmdAddItemExecData.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdSelectionHelper.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

#include "cafCmdExecCommandManager.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

#include "cafAssert.h"

#include <QAction>

namespace caf
{
CAF_CMD_SOURCE_INIT( CmdAddItemFeature, "PdmListField_AddItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdExecuteCommand* CmdAddItemFeature::createExecuteCommand()
{
    caf::PdmChildArrayFieldHandle* childArrayFieldHandle = SelectionManager::instance()->activeChildArrayFieldHandle();
    if ( !childArrayFieldHandle ) return nullptr;

    int             indexAfter  = -1;
    CmdAddItemExec* addItemExec = new CmdAddItemExec( SelectionManager::instance()->notificationCenter() );

    CmdAddItemExecData* data = addItemExec->commandData();
    data->m_rootObject       = PdmReferenceHelper::findRoot( childArrayFieldHandle );
    data->m_pathToField = PdmReferenceHelper::referenceFromRootToField( data->m_rootObject, childArrayFieldHandle );
    data->m_indexAfter  = indexAfter;

    return addItemExec;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdAddItemFeature::isCommandEnabled()
{
    caf::PdmChildArrayFieldHandle* childArrayFieldHandle = SelectionManager::instance()->activeChildArrayFieldHandle();

    if ( childArrayFieldHandle )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdAddItemFeature::onActionTriggered( bool isChecked )
{
    if ( isCommandEnabled() )
    {
        CmdExecuteCommand* exeCmd = createExecuteCommand();
        if ( exeCmd )
        {
            CmdExecCommandManager::instance()->processExecuteCommand( exeCmd );
        }
        else
        {
            CAF_ASSERT( 0 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdAddItemFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add new object" );
}

} // end namespace caf
