/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021    Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QProcess>
#include <QString>
#include <QStringList>

class RimProcessMonitor;

class RimProcess : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimProcess();
    ~RimProcess() override;

    void setDescription( QString desc );
    void setCommand( QString cmdStr );
    void addParameter( QString paramStr );
    void setParameters( QStringList parameterList );

    QString commandLine() const;

    QString     command() const;
    QStringList parameters() const;
    int         ID() const;

    bool execute();

protected:
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    QString optionalCommandInterpreter() const;
    QString handleSpaces( QString argument ) const;

    caf::PdmField<QString> m_command;
    QStringList            m_arguments;
    caf::PdmField<QString> m_description;
    caf::PdmField<int>     m_id;

    static int         m_nextProcessId;
    RimProcessMonitor* m_monitor;
};
