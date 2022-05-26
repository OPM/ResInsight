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

#include <vector>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class CmdFieldChangeExecData
{
public:
    CmdFieldChangeExecData()
        : m_rootObject( nullptr )
        , m_childArrayFieldHandle( nullptr )
        , m_ownerOfChildArrayField( nullptr )
    {
    }

    caf::PdmPointer<PdmObjectHandle> m_rootObject;
    PdmChildArrayFieldHandle*        m_childArrayFieldHandle;
    PdmObjectHandle*                 m_ownerOfChildArrayField;

    std::vector<QString> m_pathToFields;
    std::vector<QString> m_undoFieldValueSerialized;

    QVariant m_newUiValue;
    QString  m_redoFieldValueSerialized;
};

//==================================================================================================
///
//==================================================================================================
class CmdFieldChangeExec : public CmdExecuteCommand
{
public:
    explicit CmdFieldChangeExec( NotificationCenter* notificationCenter );
    ~CmdFieldChangeExec() override;

    CmdFieldChangeExecData* commandData();

    QString name() override;
    void    redo() override;
    void    undo() override;

private:
    void readFieldValueFromValidXmlDocument( QXmlStreamReader& xmlStream, PdmXmlFieldHandle* xmlFieldHandle );
    void writeFieldDataToValidXmlDocument( QXmlStreamWriter& xmlStream, PdmXmlFieldHandle* xmlFieldHandle );

private:
    CmdFieldChangeExecData* m_commandData;
};

} // end namespace caf
