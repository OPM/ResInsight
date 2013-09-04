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
#include <QWizard>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QItemSelection>

class QFile;
class QProgressDialog;
class QLabel;
class QTextEdit;


class RimWellPathImport;
class RimWellCollection;


namespace caf
{
    class UiTreeModelPdm;
    class PdmUiTreeView;
}


class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    IntroPage(const QString& webServiceAddress, QWidget *parent = 0);

};


class FieldSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    FieldSelectionPage(RimWellPathImport* wellPathImport, caf::UiTreeModelPdm* treeModelPdm, caf::PdmUiTreeView* pdmUiTreeView, QWidget* parent = 0);

    virtual void initializePage();

private:
    //RimWellPathImport* m_wellPathImportObject;
};


class WellSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    WellSelectionPage(RimWellPathImport* wellPathImport, QWidget* parent = 0);

    virtual void initializePage();

private:
    caf::PdmUiTreeView* m_wellSelectionTreeView;

};

class WellSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    WellSummaryPage(RimWellPathImport* wellPathImport, QWidget* parent = 0);

    virtual void initializePage();

    void updateSummaryPage();

private:
    RimWellPathImport*  m_wellPathImportObject;
    QTextEdit*          m_textEdit;
};


class DownloadEntity
{
public:
    QString requestUrl;
    QString responseFilename;
};

class RiuWellImportWizard : public QWizard
{
    Q_OBJECT

public:
    enum DownloadState{ DOWNLOAD_FIELDS, DOWNLOAD_WELLS, DOWNLOAD_WELL_PATH, DOWNLOAD_UNDEFINED};

public:
    RiuWellImportWizard(const QString& webServiceAddress, const QString& downloadFolder, RimWellPathImport* wellPathImportObject, QWidget *parent = 0);
    ~RiuWellImportWizard();

    void setWebServiceAddress(const QString& wsAdress);
    void setJsonDestinationFolder(const QString& folder);
    void setWellPathImportObject(RimWellPathImport* wellPathImportObject);

    RimWellCollection* wellCollection();

private:
    void        startRequest(QUrl url);
    void        setUrl(const QString& httpAddress);

    QString     jsonFieldsFilePath();
    QString     jsonWellsFilePath();

    void        updateFieldsModel();

    QString     getValue(const QString& key, const QString& stringContent);

    void        getWellPathLinks(QStringList* surveyLinks, QStringList* planLinks);
    void        issueDownloadOfWellPaths(const QStringList& surveyLinks, const QStringList& planLinks);




public slots:
    void        downloadWellPaths();
    void        downloadWells();
    void        downloadFields();
    void        checkDownloadQueueAndIssueRequests();
    
    void        checkDownloadQueueAndIssueRequests_v2();

    void        issueHttpRequestToFile( QString completeUrlText, QString destinationFileName );
    void        cancelDownload();
    void        httpFinished();

    void        httpReadyRead();
    void        updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
    void        refreshButtonStatus();
    void        slotAuthenticationRequired(QNetworkReply* networkReply, QAuthenticator* authenticator);

    void        slotSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );


#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply*,const QList<QSslError> &errors);

#endif

private:
    QString m_webServiceAddress;
    QString m_destinationFolder;

    RimWellPathImport* m_wellPathImport;
    caf::PdmUiTreeView* m_pdmTreeView;

    RimWellCollection* m_wellCollection;

    QProgressDialog*    m_progressDialog;

    QUrl                    m_url;
    QNetworkAccessManager   m_networkAccessManager;
    QNetworkReply*          m_reply;
    QFile*                  m_file;
    bool                    m_httpRequestAborted;


    QStringList             m_wellPathRequestQueue;
    QList<DownloadEntity>   m_wellRequestQueue;

    DownloadState       m_currentDownloadState;


    // To be deleted
    QLabel*             m_statusLabel;

    int                 m_wellSummaryPageId;

};









