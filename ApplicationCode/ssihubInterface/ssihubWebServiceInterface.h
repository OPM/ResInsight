/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include <QString>
#include <QVariant>
#include <QMap>

class QNetworkReply;
class QNetworkAccessManager;

namespace ssihub {

class WebServiceInterface : public QObject
{
    Q_OBJECT

public:
    WebServiceInterface(QObject *parent = 0);
    ~WebServiceInterface();

    void setUrl(const QString& url);

    QStringList fetchData(const QString& method, const QMap<QString, QVariant>& arguments);


private slots:
    void onResult(QNetworkReply* reply);

private:
    QString m_httpAddress;

    QNetworkAccessManager* m_networkManager;
};

    

}

