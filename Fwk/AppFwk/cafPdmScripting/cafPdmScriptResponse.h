//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>

#include <memory>

namespace caf
{
//==================================================================================================
//
// Command response which contains status and possibly a result
//
//==================================================================================================
class PdmScriptResponse
{
public:
    // Status in order of severity from ok to critical
    enum Status
    {
        COMMAND_OK,
        COMMAND_WARNING,
        COMMAND_ERROR
    };

public:
    PdmScriptResponse( Status status = COMMAND_OK, const QString& message = "" );
    explicit PdmScriptResponse( PdmObject* ok_result );

    Status      status() const;
    QString     sanitizedResponseMessage() const;
    QStringList messages() const;
    PdmObject*  result() const;
    void        setResult( PdmObject* result );
    void        updateStatus( Status status, const QString& message );

private:
    static QString statusLabel( Status status );

private:
    Status                     m_status;
    QStringList                m_messages;
    std::unique_ptr<PdmObject> m_result;
};
} // namespace caf
