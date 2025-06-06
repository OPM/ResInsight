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

#include "cafCmdExecuteCommand.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class CmdSelectionChangeExecData : public PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    CmdSelectionChangeExecData()
    {
        CAF_PDM_InitObject( "CmdSelectionChangeExecData uiName",
                            "",
                            "CmdSelectionChangeExecData tooltip",
                            "CmdSelectionChangeExecData whatsthis" );

        CAF_PDM_InitFieldNoDefault( &m_selectionLevel, "selectionLevel", "selectionLevel" );
        CAF_PDM_InitField( &m_previousSelection, "previousSelection", std::vector<QString>(), "previousSelection" );
        CAF_PDM_InitField( &m_newSelection, "newSelection", std::vector<QString>(), "newSelection" );
    }

    PdmField<int>                  m_selectionLevel;
    PdmField<std::vector<QString>> m_previousSelection;
    PdmField<std::vector<QString>> m_newSelection;
};

//==================================================================================================
///
//==================================================================================================
class CmdSelectionChangeExec : public CmdExecuteCommand
{
public:
    explicit CmdSelectionChangeExec( NotificationCenter* notificationCenter );
    ~CmdSelectionChangeExec() override;
    ;

    CmdSelectionChangeExecData* commandData();

    QString name() override;
    void    redo() override;
    void    undo() override;

private:
    CmdSelectionChangeExecData* m_commandData;
};

} // end namespace caf
