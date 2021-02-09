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

#include "cafCmdExecCommandManager.h"

#include "cafCmdExecuteCommand.h"
#include "cafCmdFieldChangeExec.h"
#include "cafCmdUiCommandSystemImpl.h"
#include "cafPdmUiCommandSystemProxy.h"

#include <QUndoCommand>

//--------------------------------------------------------------------------------------------------
/// Classed used to take over ownership of an execute command and wrap it in a QUndoCommand
//--------------------------------------------------------------------------------------------------
class UndoRedoWrapper : public QUndoCommand
{
public:
    explicit UndoRedoWrapper( caf::CmdExecuteCommand* executeCommand )
    {
        m_executeCommand = executeCommand;

        setText( m_executeCommand->name() );
    }

    ~UndoRedoWrapper() override { delete m_executeCommand; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void undo() override { m_executeCommand->undo(); }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void redo() override { m_executeCommand->redo(); }

private:
    caf::CmdExecuteCommand* m_executeCommand;
};

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdExecCommandManager::CmdExecCommandManager()
{
    m_commandFeatureInterface = nullptr;

    m_undoStack = new QUndoStack();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdExecCommandManager* CmdExecCommandManager::instance()
{
    static CmdExecCommandManager* singleton = new CmdExecCommandManager;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdExecCommandManager::activateCommandSystem()
{
    if ( !m_commandFeatureInterface )
    {
        m_commandFeatureInterface = new CmdUiCommandSystemImpl;
    }

    PdmUiCommandSystemProxy::instance()->setCommandInterface( m_commandFeatureInterface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdExecCommandManager::deactivateCommandSystem()
{
    PdmUiCommandSystemProxy::instance()->setCommandInterface( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdExecCommandManager::enableUndoCommandSystem( bool enable )
{
    this->activateCommandSystem();

    m_commandFeatureInterface->enableUndoFeature( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QUndoStack* CmdExecCommandManager::undoStack()
{
    return m_undoStack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdExecCommandManager::processExecuteCommand( CmdExecuteCommand* executeCommand )
{
    if ( isUndoEnabledForCommand( executeCommand ) )
    {
        // Transfer ownership of execute command to wrapper object
        UndoRedoWrapper* undoRedoWrapper = new UndoRedoWrapper( executeCommand );

        m_undoStack->push( undoRedoWrapper );
    }
    else
    {
        // Execute command and delete the execute command

        executeCommand->redo();
        delete executeCommand;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdExecCommandManager::processExecuteCommandsAsMacro( const std::vector<CmdExecuteCommand*>& commands )
{
    if ( !commands.empty() && isUndoEnabledForCommand( commands.front() ) )
    {
        auto firstCommand = commands.front();
        if ( isUndoEnabledForCommand( firstCommand ) )
        {
            QString macroName = firstCommand->name() + " (multiple objects)";

            m_undoStack->beginMacro( macroName );
            for ( auto command : commands )
            {
                UndoRedoWrapper* undoRedoWrapper = new UndoRedoWrapper( command );
                m_undoStack->push( undoRedoWrapper );
            }
            m_undoStack->endMacro();

            return;
        }
    }

    for ( caf::CmdExecuteCommand* executeCommand : commands )
    {
        if ( executeCommand )
        {
            executeCommand->redo();
            delete executeCommand;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdExecCommandManager::isUndoEnabledForCommand( CmdExecuteCommand* command )
{
    bool useUndo = false;

    if ( dynamic_cast<CmdFieldChangeExec*>( command ) && m_commandFeatureInterface->disableUndoForFieldChange() )
    {
        useUndo = false;
    }
    else if ( m_commandFeatureInterface && m_commandFeatureInterface->isUndoEnabled() )
    {
        useUndo = true;
    }

    return useUndo;
}

} // end namespace caf
