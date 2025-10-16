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

#include <QString>
#include <QStringList>

#include <utility>
#include <vector>

class RimProcessMonitor;
class QProcess;

class RimProcess : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimProcess( bool logStdOutErr = true, RimProcessMonitor* monitor = nullptr );
    ~RimProcess() override;

    void setDescription( QString desc );
    void setCommand( QString cmdStr );
    void addParameter( QString paramStr );
    void addParameters( QStringList parameterList );

    void addEnvironmentVariable( QString name, QString value );

    void setWorkingDirectory( QString directory );

    QString commandLine() const;

    QString     command() const;
    QStringList parameters() const;
    int         ID() const;

    // blocking run
    bool execute( bool enableStdOut = true, bool enableStdErr = true );

    // async run
    bool start( bool enableStdOut = true, bool enableStdErr = true );
    void cleanUpAfterRun();
    void terminate();

    QStringList stdErr() const;
    QStringList stdOut() const;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    QString optionalCommandInterpreter() const;
    QString optionalPreParameters() const;
    QString handleSpaces( QString argument ) const;

    bool isWindowsBatchFile() const;

    caf::PdmField<QString> m_command;
    QStringList            m_arguments;
    caf::PdmField<QString> m_description;
    caf::PdmField<int>     m_id;
    caf::PdmField<QString> m_workDir;

    std::vector<std::pair<QString, QString>> m_environmentVariables;

    static int         m_nextProcessId;
    RimProcessMonitor* m_monitor;
    bool               m_enableLogging;
    QProcess*          m_qProcess;
};
