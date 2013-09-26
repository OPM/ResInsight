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
#include <QNetworkReply>

class QFile;
class QProgressDialog;
class QLabel;
class QTextEdit;


class RimWellPathImport;
class RimOilFieldEntry;


namespace caf
{
    class UiTreeModelPdm;
    class PdmUiTreeView;
    class PdmUiListView;
    class PdmObjectGroup;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class AuthenticationPage : public QWizardPage
{
    Q_OBJECT

public:
    AuthenticationPage(const QString& webServiceAddress, QWidget *parent = 0);

    virtual void initializePage();
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class FieldSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    FieldSelectionPage(RimWellPathImport* wellPathImport, QWidget* parent = 0);

    virtual void initializePage();
};



class ObjectGroupWithHeaders;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class WellSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    WellSelectionPage(RimWellPathImport* wellPathImport, QWidget* parent = 0);
    ~WellSelectionPage();

    virtual void initializePage();
    void buildWellTreeView();

private:
    ObjectGroupWithHeaders*  m_regionsWithVisibleWells;
    RimWellPathImport*  m_wellPathImportObject;
    caf::PdmUiTreeView* m_wellSelectionTreeView;

};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class WellSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    WellSummaryPage(RimWellPathImport* wellPathImport, QWidget* parent = 0);

    virtual void initializePage();

    void updateSummaryPage();

private slots:
    void slotShowDetails();

private:
    RimWellPathImport*  m_wellPathImportObject;
    QTextEdit*          m_textEdit;
    caf::PdmUiListView* m_listView;
    caf::PdmObjectGroup*m_objectGroup;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class DownloadEntity
{
public:
    QString requestUrl;
    QString responseFilename;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuWellImportWizard : public QWizard
{
    Q_OBJECT

public:
    enum DownloadState{ DOWNLOAD_FIELDS, DOWNLOAD_WELLS, DOWNLOAD_WELL_PATH, DOWNLOAD_UNDEFINED};

public:
    RiuWellImportWizard(const QString& webServiceAddress, const QString& downloadFolder, RimWellPathImport* wellPathImportObject, QWidget *parent = 0);

    void        setCredentials(const QString& username, const QString& password);
    QStringList absoluteFilePathsToWellPaths() const;

    // Methods used from the wizard pages
    caf::PdmObjectGroup* wellCollection();
    void        resetAuthenticationCount();

public slots:
    void        downloadWellPaths();
    void        downloadWells();
    void        downloadFields();
    
    void        checkDownloadQueueAndIssueRequests();

    void        issueHttpRequestToFile( QString completeUrlText, QString destinationFileName );
    void        cancelDownload();

    void        httpFinished();
    void        httpReadyRead();

    void        slotAuthenticationRequired(QNetworkReply* networkReply, QAuthenticator* authenticator);


#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
#endif

private slots:
    void        slotCurrentIdChanged(int currentId);

private:
    void        startRequest(QUrl url);
    void        setUrl(const QString& httpAddress);

    QString     jsonFieldsFilePath();
    QString     jsonWellsFilePath();

    void        updateFieldsModel();
    void        parseWellsResponse(RimOilFieldEntry* oilFieldEntry);


    QString     getValue(const QString& key, const QString& stringContent);


private:
    QString                 m_webServiceAddress;
    QString                 m_destinationFolder;

    RimWellPathImport*      m_wellPathImportObject;
    caf::PdmUiTreeView*     m_pdmTreeView;

    QProgressDialog*        m_myProgressDialog;

    QUrl                    m_url;
    QNetworkAccessManager   m_networkAccessManager;
    QNetworkReply*          m_reply;
    QFile*                  m_file;
    bool                    m_httpRequestAborted;

    bool                    m_firstTimeRequestingAuthentication;

    QList<DownloadEntity>   m_wellRequestQueue;

    DownloadState           m_currentDownloadState;
    
    int                     m_fieldSelectionPageId;
    int                     m_wellSelectionPageId;
    int                     m_wellSummaryPageId;
};

