/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RiaSumoExplorerDefines.h"

#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QString>

#include <vector>

class QNetworkAccessManager;
class QNetworkReply;

//==================================================================================================
//
// Sumo Explorer Connector
//
// Manages a local Python FastAPI server that wraps the Sumo Explorer API.
// Provides synchronous and asynchronous access to Sumo data via HTTP.
//
//==================================================================================================
class RiaSumoExplorerConnector : public QObject
{
    Q_OBJECT

public:
    RiaSumoExplorerConnector( QObject* parent, const QString& pythonPath, unsigned int port );
    ~RiaSumoExplorerConnector() override;

    // Server lifecycle management
    bool startServer();
    void stopServer();
    bool isServerRunning() const;
    bool waitForServerReady( int timeoutMs = 10000 );

    // Synchronous data requests (blocking)
    void       requestAssetsBlocking();
    void       requestCasesForFieldBlocking( const QString& fieldName );
    void       requestEnsemblesForCaseBlocking( const QString& caseId );
    void       requestVectorNamesBlocking( const QString& caseId, const QString& ensembleName );
    void       requestRealizationIdsBlocking( const QString& caseId, const QString& ensembleName );
    QByteArray requestSummaryDataBlocking( const QString& caseId, const QString& ensembleName, const QString& vectorName );
    QByteArray requestParametersBlocking( const QString& caseId, const QString& ensembleName );

    // Asynchronous data requests (non-blocking)
    void requestAssets();
    void requestCasesForField( const QString& fieldName );
    void requestEnsemblesForCase( const QString& caseId );
    void requestVectorNames( const QString& caseId, const QString& ensembleName );
    void requestRealizationIds( const QString& caseId, const QString& ensembleName );

    // Data accessors
    std::vector<SumoExplorerAsset>           assets() const;
    std::vector<SumoExplorerCase>            cases() const;
    std::vector<SumoExplorerEnsemble>        ensembles() const;
    std::vector<SumoExplorerVectorInfo>      vectorNames() const;
    std::vector<SumoExplorerRealizationInfo> realizationIds() const;

    // Server info
    QString serverUrl() const;
    QString lastError() const;

signals:
    void serverStarted();
    void serverStopped();
    void serverError( const QString& error );
    void assetsFinished();
    void casesFinished();
    void ensemblesFinished();
    void vectorNamesFinished();
    void realizationIdsFinished();

private slots:
    void onProcessError( QProcess::ProcessError error );
    void onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();

    void parseAssetsReply( QNetworkReply* reply );
    void parseCasesReply( QNetworkReply* reply );
    void parseEnsemblesReply( QNetworkReply* reply );
    void parseVectorNamesReply( QNetworkReply* reply );
    void parseRealizationIdsReply( QNetworkReply* reply );

private:
    QString makeUrl( const QString& path ) const;
    void    logServerOutput( const QString& output );
    void    setError( const QString& error );

    QByteArray executeGetRequest( const QString& url );
    void wrapAndCallNetworkRequest( std::function<void()> requestCallable, const std::function<void( QNetworkReply* )>& replyHandler );
    QByteArray base64ToBytes( const QString& base64 );

private:
    QProcess*              m_serverProcess;
    QNetworkAccessManager* m_networkManager;
    QString                m_pythonPath;
    unsigned int           m_port;
    bool                   m_serverRunning;
    QString                m_lastError;

    std::vector<SumoExplorerAsset>           m_assets;
    std::vector<SumoExplorerCase>            m_cases;
    std::vector<SumoExplorerEnsemble>        m_ensembles;
    std::vector<SumoExplorerVectorInfo>      m_vectorNames;
    std::vector<SumoExplorerRealizationInfo> m_realizationIds;
};
