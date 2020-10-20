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

#include "cafCmdSelectionHelper.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdSelectionChangeExec.h"

#include "cafSelectionManager.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdSelectionHelper::executeSelectionCommand( const std::vector<PdmObjectHandle*>& selection, int selectionLevel )
{
    CmdSelectionChangeExec* selectionChangeExec = createSelectionCommand( selection, selectionLevel );

    CmdExecCommandManager::instance()->processExecuteCommand( selectionChangeExec );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdSelectionChangeExec* CmdSelectionHelper::createSelectionCommand( const std::vector<PdmObjectHandle*>& selection,
                                                                    int selectionLevel )
{
    CmdSelectionChangeExec* selectionChangeExec =
        new CmdSelectionChangeExec( SelectionManager::instance()->notificationCenter() );
    selectionChangeExec->commandData()->m_selectionLevel.v() = selectionLevel;

    SelectionManager::instance()->selectionAsReferences( selectionChangeExec->commandData()->m_previousSelection.v(),
                                                         selectionLevel );

    for ( size_t i = 0; i < selection.size(); i++ )
    {
        QString itemRef =
            PdmReferenceHelper::referenceFromRootToObject( PdmReferenceHelper::findRoot( selection[i] ), selection[i] );
        selectionChangeExec->commandData()->m_newSelection.v().push_back( itemRef );
    }

    return selectionChangeExec;
}

} // end namespace caf
