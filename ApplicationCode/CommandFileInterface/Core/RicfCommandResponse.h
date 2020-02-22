/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>

#include <memory>

//==================================================================================================
//
// Command response which contains status and possibly a result
//
//==================================================================================================
class RicfCommandResponse
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
    RicfCommandResponse( Status status = COMMAND_OK, const QString& message = "" );
    explicit RicfCommandResponse( caf::PdmObject* ok_result );

    Status          status() const;
    QString         sanitizedResponseMessage() const;
    QStringList     messages() const;
    caf::PdmObject* result() const;
    void            setResult( caf::PdmObject* result );
    void            updateStatus( Status status, const QString& message );

private:
    static QString statusLabel( Status status );

private:
    Status                          m_status;
    QStringList                     m_messages;
    std::unique_ptr<caf::PdmObject> m_result;
};
