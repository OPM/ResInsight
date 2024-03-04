/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

// #include <QAbstractOAuth>
// #include <QJsonDocument>
// #include <QJsonObject>
// #include <QNetworkAccessManager>
// #include <QNetworkReply>
// #include <QNetworkRequest>
// #include <QOAuth2AuthorizationCodeFlow>
#include <QObject>

// #include <QDesktopServices>
#include <QOAuthHttpServerReplyHandler>
#include <QString>
// #include <QUrl>
// #include <QUrlQuery>

//==================================================================================================
///
//==================================================================================================
class RiaOsduOAuthHttpServerReplyHandler : public QOAuthHttpServerReplyHandler
{
public:
    RiaOsduOAuthHttpServerReplyHandler( quint16 port, QObject* parent );

    QString callback() const override;

private:
    quint16 m_port;
};
