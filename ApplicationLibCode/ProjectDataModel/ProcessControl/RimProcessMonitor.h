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

#include <QString>
#include <QStringList>

class RimProcessMonitor : public QObject
{
    Q_OBJECT

public:
    explicit RimProcessMonitor( size_t processId, bool logStdOutErr = true );

    void               clearStdOutErr();
    const QStringList& stdOut() const;
    const QStringList& stdErr() const;

    virtual void appendStdOut( QString line );
    virtual void appendStdErr( QString line );

    void setProcessId( size_t processId );

signals:

public slots:
    virtual void error( QProcess::ProcessError error );
    virtual void finished( int exitCode, QProcess::ExitStatus exitStatus );
    virtual void readyReadStandardError();
    virtual void readyReadStandardOutput();
    virtual void started();

protected:
    QString     addPrefix( QString message );
    size_t      m_processId;
    bool        m_logStdOutErr;
    QStringList m_stdOut;
    QStringList m_stdErr;
};
