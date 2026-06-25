/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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
#include <QPointer>
#include <QProcessEnvironment>
#include <QTimer>

class QProcess;
class QNetworkAccessManager;

//==================================================================================================
///
/// Manages the life cycle of the local "ri_cloud_api" uvicorn service.
///
/// The service is a long-lived localhost process (unlike the single-shot script worker process in
/// RiaApplication). It is started when Sumo cloud authentication is performed, bound to an
/// automatically selected free port, polled for liveness on its /alive endpoint, restarted on a
/// new port if it stops responding, and killed when ResInsight closes.
///
//==================================================================================================
class RiaCloudApiService : public QObject
{
    Q_OBJECT
public:
    explicit RiaCloudApiService( QObject* parent = nullptr );
    ~RiaCloudApiService() override;

    void start();
    void stop();
    void restart();

    bool isRunning() const;
    int  port() const;

private slots:
    void onHealthCheck();

private:
    static int                 findAvailablePortNumber();
    static QString             serviceWorkingDirectory();
    static QProcessEnvironment buildProcessEnvironment( const QString& workingDirectory );

private:
    QPointer<QProcess>     m_process;
    QNetworkAccessManager* m_networkAccessManager;
    QTimer                 m_startupTimer;
    QTimer                 m_healthTimer;

    int m_port;
    int m_consecutiveFailures;
};
