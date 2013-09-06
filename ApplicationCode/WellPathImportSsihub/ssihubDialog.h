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

#include <QDialog>
#include <QNetworkAccessManager>
#include <QUrl>

#include <QItemSelection>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QFile;
class QLabel;
class QLineEdit;
class QProgressDialog;
class QPushButton;
class QSslError;
class QAuthenticator;
class QNetworkReply;
class QStringListModel;
class QListView;
class QStandardItemModel;
class QCheckBox;
QT_END_NAMESPACE




namespace ssihub {


class FetchWellPathsDialog : public QDialog
{
    Q_OBJECT

public:
    enum DownloadState{ DOWNLOAD_FIELDS, DOWNLOAD_WELLS, DOWNLOAD_WELL_PATH, DOWNLOAD_UNDEFINED};

public:
    FetchWellPathsDialog(QWidget *parent = 0);

    void        setSsiHubUrl(const QString& httpAddress);
    void        setDestinationFolder(const QString& folder);
    void        setRegion(int north, int south, int east, int west);

    QStringList downloadedJsonWellPathFiles();

protected:
    virtual void showEvent(QShowEvent* event);

public:
    void        startRequest(QUrl url);
    void        setUrl(const QString& httpAddress);
    
    QString     jsonFieldsFilePath();
    QString     jsonWellsFilePath();
    QString     jsonWellsInArea();

    void        updateFieldsModel();

    QString     getValue(const QString& key, const QString& stringContent);

    void        getWellPathLinks(QStringList* surveyLinks, QStringList* planLinks);
    void        issueDownloadOfWellPaths(const QStringList& surveyLinks, const QStringList& planLinks);
    
    void        requestFieldData(QStringList& regions, QStringList& fields, QStringList& edmIds);

signals:
    void        signalFieldsDownloaded();


private slots:
    void        downloadWellPaths();
    void        downloadFields();
    void        checkDownloadQueueAndIssueRequests();

    void        issueHttpRequestToFile( QString completeUrlText, QString fieldsFileName );
    void        cancelDownload();
    void        httpFinished();

    void        httpReadyRead();
    void        updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
    void        refreshButtonStatus();
    void        slotAuthenticationRequired(QNetworkReply*,QAuthenticator *);

    void        slotSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );


#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply*,const QList<QSslError> &errors);

#endif

private:
    QLabel*             m_statusLabel;
    QLabel*             m_urlLabel;
    QLineEdit*          m_urlLineEdit;
    QLabel*             m_urlSsiHubLabel;
    QLineEdit*          m_urlSsiHubLineEdit;
    
    QPushButton*        m_downloadFieldsButton;
    QListView*          m_fieldListView;

    QListView*          m_wellPathsView;
    QStandardItemModel* m_wellPathsModel;


    QCheckBox*          m_filterWellsByUtmArea;
    QLineEdit*          m_northLineEdit;
    QLineEdit*          m_southLineEdit;
    QLineEdit*          m_eastLineEdit;
    QLineEdit*          m_westLineEdit;

    QCheckBox*          m_importSurveyCheckBox;
    QCheckBox*          m_importPlansCheckBox;

    QProgressDialog*    m_progressDialog;
    QPushButton*        m_downloadWellPathsButton;
    QDialogButtonBox*   m_buttonBox;

    QUrl                    m_url;
    QNetworkAccessManager   m_networkAccessManager;
    QNetworkReply*          m_reply;
    QFile*                  m_file;
    bool                    m_httpRequestAborted;


    QString             m_destinationFolder;
    QStringListModel*   m_fieldModel;
    
    QStringList         m_wellPathRequestQueue;

    DownloadState       m_currentDownloadState;
};

} // namespace ssihub
