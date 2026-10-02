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
/// RiaApplication). It is started when Sumo cloud authentication is performed and from the Cloud Data
/// user interface, bound to the first available port at or above the wanted port, polled for liveness
/// on its /alive endpoint, restarted if it stops responding, and killed when ResInsight closes.
///
/// A restart scans for a port again, so the port in use can change during a session. Always read the
/// current address from serverUrl() instead of caching it.
///
//==================================================================================================
class RiaCloudApiService : public QObject
{
    Q_OBJECT
public:
    // The server address is the base URL without the port, e.g. "http://127.0.0.1". The wanted port is
    // where the search for an available port starts.
    RiaCloudApiService( const QString& serverAddress, int wantedPort, QObject* parent = nullptr );
    ~RiaCloudApiService() override;

    // Manual: an explicit user action (Start/Restart button). Automatic: triggered by application logic
    // (e.g. Sumo authentication completing, or waitUntilResponding()).
    //
    // Does not affect whether autoRestart() gives up (see autoRestart()), only what happens after it has:
    // a later Automatic call is then a no-op (see m_hasGivenUp), while a Manual one tries again.
    enum class StartTrigger
    {
        Manual,
        Automatic
    };

    void start( StartTrigger trigger = StartTrigger::Manual );
    void stop();
    void restart();

    // True as soon as the process has been launched, which is not the same as the service being
    // usable: uvicorn may still be booting, or may be about to exit on a missing dependency.
    bool isRunning() const;

    // True once the service has answered a health check, i.e. it is actually serving requests.
    bool isResponding() const;

    // Block until the service answers a health check, starting it when it is not running. Returns
    // immediately when it is already responding, and false when it does not answer within the timeout.
    //
    // Requests must not be issued before this returns true: isRunning() is true as soon as the process is
    // launched, while uvicorn may still be booting. Call from the thread owning this object, as the health
    // check uses its network access manager.
    bool waitUntilResponding( int timeoutMs );

    int port() const;

    // Base URL of the service, including the port actually in use, e.g. "http://127.0.0.1:8001". Empty
    // when the service is not running.
    QString serverUrl() const;

signals:
    // Emitted when isRunning() or isResponding() changes, so that user interface elements showing
    // the server status can refresh. The status changes without any user interaction, driven by the
    // health check and by the process exiting on its own.
    void statusChanged();

private slots:
    void onHealthCheck();
    void onReadyReadStandardOutput();

private:
    // Called after a crash or an unresponsive health check. Gives up immediately if the service has never
    // responded since this start (see m_hasRespondedSinceStart) -- likely a config problem, not worth
    // retrying. Otherwise retries indefinitely, treating it as a crash of a previously working server.
    void autoRestart();

    // Shared process-launch implementation used by start() and by autoRestart()'s retry. Unlike start(),
    // leaves m_consecutiveLaunchFailures and m_hasRespondedSinceStart untouched.
    void launchProcess();

    static int                 findAvailablePortNumber( int firstPort );
    static QString             serviceWorkingDirectory();
    static QProcessEnvironment buildProcessEnvironment( const QString& workingDirectory );

private:
    QPointer<QProcess>     m_process;
    QNetworkAccessManager* m_networkAccessManager;
    QTimer                 m_startupTimer;
    QTimer                 m_healthTimer;

    const QString m_serverAddress;
    const int     m_wantedPort;

    int  m_port;
    int  m_consecutiveFailures;
    int  m_consecutiveLaunchFailures;
    bool m_isResponding;

    // True once the current run has answered a health check; autoRestart() then retries indefinitely
    // instead of giving up on the first failure. Reset to false in start().
    bool m_hasRespondedSinceStart;

    // True once autoRestart() has given up on a run that never responded. Blocks a later Automatic
    // start() (e.g. Sumo's tokenReady) from re-arming the loop; a Manual start/restart clears it.
    bool m_hasGivenUp;
};
