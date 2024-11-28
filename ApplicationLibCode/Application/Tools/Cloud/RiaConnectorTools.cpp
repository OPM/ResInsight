/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RiaConnectorTools.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaPreferencesOsdu.h"
#include "RiaPreferencesSumo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>

#include <QCoreApplication>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaConnectorTools::tokenDataAsJson( QOAuth2AuthorizationCodeFlow* authCodeFlow )
{
    QJsonObject obj;
    obj.insert( "token", authCodeFlow->token() );
    obj.insert( "refreshToken", authCodeFlow->refreshToken() );
    if ( authCodeFlow->expirationAt().isValid() )
    {
        obj.insert( "expiration", authCodeFlow->expirationAt().toSecsSinceEpoch() );
    }

    QJsonDocument doc( obj );
    return doc.toJson( QJsonDocument::Indented );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConnectorTools::initializeTokenDataFromJson( QOAuth2AuthorizationCodeFlow* authCodeFlow, const QString& tokenDataJson )
{
    QJsonDocument doc = QJsonDocument::fromJson( tokenDataJson.toUtf8() );
    QJsonObject   obj = doc.object();

    if ( obj.contains( "expiration" ) && obj.contains( "token" ) )
    {
        quint64   secondsSinceEpoch = obj["expiration"].toVariant().toULongLong();
        QDateTime expiration        = QDateTime::fromSecsSinceEpoch( secondsSinceEpoch );
        if ( expiration.isValid() && expiration > QDateTime::currentDateTime() )
        {
            authCodeFlow->setToken( obj["token"].toString() );
        }
    }

    authCodeFlow->setRefreshToken( obj["refreshToken"].toString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConnectorTools::writeTokenData( const QString& filePath, const QString& tokenDataJson )
{
    QFile file( filePath );

    // Ensure the directory exists (create it if it doesn't)
    QString dirPath = QFileInfo( file ).absolutePath();
    QDir    dir( dirPath );
    if ( !dir.exists() )
    {
        dir.mkpath( dirPath );
    }

    if ( file.open( QIODevice::WriteOnly ) )
    {
        QTextStream stream( &file );
        stream << tokenDataJson;
        file.close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaConnectorTools::readStringFromFile( const QString& filePath )
{
    QFile file( filePath );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QTextStream stream( &file );
        QString     result = stream.readAll();
        file.close();
        return result;
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RiaConnectorTools::readKeyValuePairs( const QString& filePath )
{
    auto content = readStringFromFile( filePath );

    QJsonDocument doc = QJsonDocument::fromJson( content.toUtf8() );
    QJsonObject   obj = doc.object();

    std::map<QString, QString> keyValuePairs;
    for ( auto it = obj.begin(); it != obj.end(); ++it )
    {
        keyValuePairs[it.key()] = it.value().toString();
    }

    return keyValuePairs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConnectorTools::readCloudConfigFiles( RiaPreferences* preferences )
{
    if ( preferences == nullptr ) return;

    // Check multiple locations for configuration files. The first valid configuration file is used. Currently, using Qt5 the ResInsight
    // binary file is stored at the root of the installation folder. When moving to Qt6, we will probably use sub folders /bin /lib and
    // others. Support both one and two search levels to support Qt6.
    //
    // home_folder/.resinsight/*_config.json
    // location_of_resinsight_executable/../share/cloud_services/*_config.json
    // location_of_resinsight_executable/../../share/cloud_services/*_config.json
    //

    {
        QStringList osduFilePathCandidates;
        osduFilePathCandidates << QDir::homePath() + "/.resinsight/osdu_config.json";
        osduFilePathCandidates << QCoreApplication::applicationDirPath() + "/../share/cloud_services/osdu_config.json";
        osduFilePathCandidates << QCoreApplication::applicationDirPath() + "/../../share/cloud_services/osdu_config.json";

        for ( const auto& osduFileCandidate : osduFilePathCandidates )
        {
            auto keyValuePairs = RiaConnectorTools::readKeyValuePairs( osduFileCandidate );
            if ( !keyValuePairs.empty() )
            {
                RiaLogging::info( QString( "Imported OSDU configuration from : '%1'" ).arg( osduFileCandidate ) );

                preferences->osduPreferences()->setData( keyValuePairs );
                preferences->osduPreferences()->setFieldsReadOnly();
                break;
            }
        }
    }

    {
        QStringList sumoFilePathCandidates;
        sumoFilePathCandidates << QDir::homePath() + "/.resinsight/sumo_config.json";
        sumoFilePathCandidates << QCoreApplication::applicationDirPath() + "/../share/cloud_services/sumo_config.json";
        sumoFilePathCandidates << QCoreApplication::applicationDirPath() + "/../../share/cloud_services/sumo_config.json";

        for ( const auto& sumoFileCandidate : sumoFilePathCandidates )
        {
            auto keyValuePairs = RiaConnectorTools::readKeyValuePairs( sumoFileCandidate );
            if ( !keyValuePairs.empty() )
            {
                RiaLogging::info( QString( "Imported SUMO configuration from : '%1'" ).arg( sumoFileCandidate ) );

                preferences->sumoPreferences()->setData( keyValuePairs );
                preferences->sumoPreferences()->setFieldsReadOnly();
                break;
            }
        }
    }
}
