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

#include <QObject>
#include <QProcess>

class RimProcessMonitor : public QObject
{
    Q_OBJECT

public:
    explicit RimProcessMonitor( int processId );

signals:

public slots:
    void error( QProcess::ProcessError error );
    void finished( int exitCode, QProcess::ExitStatus exitStatus );
    void readyReadStandardError();
    void readyReadStandardOutput();
    void started();

private:
    QString addPrefix( QString message );
    int     m_processId;
};
