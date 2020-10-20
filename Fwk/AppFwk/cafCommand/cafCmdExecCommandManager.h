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

#pragma once

#include <QPointer>

#include <vector>

class QUndoStack;
class QString;

namespace caf
{
class CmdExecuteCommand;
class CmdUiCommandSystemImpl;

//==================================================================================================
///
//==================================================================================================
class CmdExecCommandManager
{
public:
    static CmdExecCommandManager* instance();

    CmdExecCommandManager( const CmdExecCommandManager& ) = delete;
    CmdExecCommandManager& operator=( const CmdExecCommandManager& ) = delete;

    // When the undoFeature is enabled, execute commands are inserted in the undo stack
    // The application can use the QUndoStack to display/modify execute commands wrapped in QUndoCommand objects
    void        enableUndoCommandSystem( bool enable );
    QUndoStack* undoStack();

    // If undo system is enabled, the PdmExecuteCommand is wrapped in a QUndoCommand and inserted in the QUndoStack.
    // If undo is not possible (undo system not enabled, or pdm object has disabled undo),
    // the PdmExecuteCommand is executed and deleted
    void processExecuteCommand( CmdExecuteCommand* executeCommand );
    void processExecuteCommandsAsMacro( const QString& macroName, std::vector<CmdExecuteCommand*>& commands );

private:
    CmdExecCommandManager();

    // Creates the object (CmdUiCommandSystemImpl) used to communicate from UI editors to advanced parts of the command
    // system This includes support for undo system and default command features for add/delete of items in
    // PdmChildArrayFieldHandle and creation of field changed commands so a change in an editor can be put into
    // undo/redo CmdUiCommandSystemImpl is a requirement for using the undo system
    void activateCommandSystem();
    void deactivateCommandSystem();

    bool isUndoEnabledForCurrentCommand( CmdExecuteCommand* command );

    friend class CmdExecCommandSystemActivator;
    friend class CmdExecCommandSystemDeactivator;

private:
    QPointer<QUndoStack> m_undoStack;

    CmdUiCommandSystemImpl* m_commandFeatureInterface;
};

//==================================================================================================
/// Helper class used to temporarily disable the command system including undo/redo functionality
//==================================================================================================
class CmdExecCommandSystemDeactivator
{
public:
    CmdExecCommandSystemDeactivator() { CmdExecCommandManager::instance()->deactivateCommandSystem(); }

    ~CmdExecCommandSystemDeactivator() { CmdExecCommandManager::instance()->activateCommandSystem(); }
};

//==================================================================================================
/// Helper class used to temporarily enable the command system including undo/redo functionality
//==================================================================================================
class CmdExecCommandSystemActivator
{
public:
    CmdExecCommandSystemActivator() { CmdExecCommandManager::instance()->activateCommandSystem(); }

    ~CmdExecCommandSystemActivator() { CmdExecCommandManager::instance()->deactivateCommandSystem(); }
};

} // end namespace caf
