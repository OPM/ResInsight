/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaLogging.h"

#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class RiuWorkflowJobRunner : public QObject
{
    Q_OBJECT
public:
    RiuWorkflowJobRunner( const QString& label, QObject* parent = nullptr );

    void start( const QString& program, const QStringList& arguments, const QProcessEnvironment& env );
    void cancel();
    bool isRunning() const;

private slots:
    void onReadyReadStdout();
    void onReadyReadStderr();
    void onProcessFinished( int exitCode, QProcess::ExitStatus status );

private:
    void drainLines( QByteArray& buffer, RILogLevel level );

    QString    m_label;
    QProcess   m_process;
    QByteArray m_stdoutBuf;
    QByteArray m_stderrBuf;
};
