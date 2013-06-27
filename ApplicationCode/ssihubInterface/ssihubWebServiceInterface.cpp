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

#include "ssihubWebServiceInterface.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include "ssihubDialog.h"
//#include <QObject>


namespace ssihub {

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WebServiceInterface::WebServiceInterface(QObject *parent /*= 0*/)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    //connect(m_openProjectAction,	    SIGNAL(triggered()), SLOT(slotOpenProject()));

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WebServiceInterface::~WebServiceInterface()
{
    //if (m_networkManager) delete m_networkManager;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WebServiceInterface::setUrl(const QString& url)
{
    m_httpAddress = url;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList WebServiceInterface::fetchData(const QString& method, const QMap<QString, QVariant>& arguments)
{

    FetchWellPathsDialog httpWin;
    httpWin.setSsiHubUrl(m_httpAddress);
    httpWin.exec();


    /*
    QNetworkReply* reply = m_networkManager->get(QNetworkRequest(QUrl("http://qt.nokia.com")));

    while (!reply->isFinished())
    {

    }


    QString data = (QString) reply->readAll();

    /*
    QUrl url(m_httpAddress);
//     url.setPath(QString("%1%2").arg(url.path()).arg(method));
// 
//     foreach(QString param, arguments.keys()) {
//         url.addQueryItem(param, arguments[param].toString());
//     }

    QNetworkRequest request;
    request.setUrl(url);

    QNetworkReply* reply = m_networkManager->get(request);

    QString data = (QString) reply->readAll();


    QStringList fileContent;
    fileContent.push_back(data);

    return fileContent;

    */



     QStringList strings;
     return strings;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WebServiceInterface::onResult(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return;  // ...only in a blog post

    QString data = (QString) reply->readAll();
}



}; // namespace ssihub



