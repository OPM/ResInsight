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

#include "cafCmdSelectionChangeExec.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

namespace caf
{
CAF_PDM_SOURCE_INIT( CmdSelectionChangeExecData, "CmdSelectionChangeExecData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString CmdSelectionChangeExec::name()
{
    return m_commandData->classKeyword();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdSelectionChangeExec::redo()
{
    SelectionManager::instance()->setSelectionAtLevelFromReferences( m_commandData->m_newSelection.v(),
                                                                     m_commandData->m_selectionLevel.v() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdSelectionChangeExec::undo()
{
    SelectionManager::instance()->setSelectionAtLevelFromReferences( m_commandData->m_previousSelection.v(),
                                                                     m_commandData->m_selectionLevel.v() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdSelectionChangeExec::CmdSelectionChangeExec( NotificationCenter* notificationCenter )
    : CmdExecuteCommand( notificationCenter )
{
    m_commandData = new CmdSelectionChangeExecData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdSelectionChangeExec::~CmdSelectionChangeExec()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdSelectionChangeExecData* CmdSelectionChangeExec::commandData()
{
    return m_commandData;
}

} // end namespace caf
