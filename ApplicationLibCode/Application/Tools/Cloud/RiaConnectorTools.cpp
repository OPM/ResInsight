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

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaConnectorTools::tokenDataAsJson( QOAuth2AuthorizationCodeFlow* authCodeFlow )
{
    QJsonObject obj;
    obj.insert( "token", authCodeFlow->token() );
    obj.insert( "refreshToken", authCodeFlow->refreshToken() );

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

    authCodeFlow->setToken( obj["token"].toString() );
    authCodeFlow->setRefreshToken( obj["refreshToken"].toString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaConnectorTools::writeTokenData( const QString& filePath, const QString& tokenDataJson )
{
    QFile file( filePath );
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
QString RiaConnectorTools::readTokenData( const QString& filePath )
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
