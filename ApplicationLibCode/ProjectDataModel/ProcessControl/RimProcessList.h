/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026    Equinor ASA
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

#include "RiaWslTools.h"

#include <QString>
#include <QStringList>

#include <memory>
#include <utility>
#include <vector>

class RimProcessMonitor;
class RimProcess;

class RimProcessList
{
public:
    RimProcessList( std::shared_ptr<RimProcessMonitor> monitor );
    ~RimProcessList();

    void addProcessCommand( QString command, QStringList arguments );
    void addProcessCommand( QStringList arguments );
    void addEnvironmentVariable( QString name, QString value );
    void setWorkingDirectory( QString filePath );
    void setName( QString name );

    std::vector<std::shared_ptr<RimProcess>> createProcesses();

    const std::vector<QStringList>& commands();

    void                           setWslOptions( const RiaWslTools::WslOptions& wslOpt );
    const RiaWslTools::WslOptions& wslOptions() const;

    const QStringList& stdErr() const;
    const QStringList& stdOut() const;

private:
    std::vector<QStringList>           m_commandList;
    std::shared_ptr<RimProcessMonitor> m_monitor;
    std::map<QString, QString>         m_environment;
    QString                            m_workingDirectory;
    QStringList                        m_emptyList;
    QString                            m_name;

    RiaWslTools::WslOptions m_wslOptions;
};
