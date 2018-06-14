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

#include "RiuWellImportWizard.h"

#include "RifJsonEncodeDecode.h"

#include "RimOilFieldEntry.h"
#include "RimOilRegionEntry.h"
#include "RimWellPathImport.h"

#include "cafPdmDocument.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmUiListView.h"
#include "cafPdmUiListViewEditor.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTreeView.h"
#include "cafPdmUiTreeViewEditor.h"
#include "cafUtils.h"

#include <QObject>
#include <QSslConfiguration>
#include <QtGui>
#include <QtNetwork>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellImportWizard::RiuWellImportWizard(const QString& webServiceAddress, const QString& downloadFolder, RimWellPathImport* wellPathImportObject, QWidget *parent /*= 0*/)
 : QWizard(parent)
{
    m_wellPathImportObject = wellPathImportObject;

    m_destinationFolder = downloadFolder;
    m_webServiceAddress = webServiceAddress;

    m_myProgressDialog = new QProgressDialog(this);
    m_firstTimeRequestingAuthentication = true;


    addPage(new AuthenticationPage(webServiceAddress, this));
    m_fieldSelectionPageId = addPage(new FieldSelectionPage(m_wellPathImportObject, this));
    m_wellSelectionPageId = addPage(new WellSelectionPage(m_wellPathImportObject, this));
    m_wellSummaryPageId = addPage(new WellSummaryPage(m_wellPathImportObject, this));

    connect(this, SIGNAL(currentIdChanged(int)), SLOT(slotCurrentIdChanged(int)));

    connect(&m_networkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
        this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(&m_networkAccessManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
        this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellImportWizard::~RiuWellImportWizard()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuWellImportWizard::jsonFieldsFilePath()
{
    return m_destinationFolder + "/fields.json";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuWellImportWizard::jsonWellsFilePath()
{
    return m_destinationFolder + "/wellpaths.json";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadFields()
{
    QString wellFileName = jsonWellsFilePath();
    if (caf::Utils::fileExists(wellFileName))
    {
        QFile::remove(wellFileName);
    }

    QString completeUrlText = m_webServiceAddress + "/resinsight/projects";
    QString destinationFileName = jsonFieldsFilePath();

    m_currentDownloadState = DOWNLOAD_FIELDS;
    issueHttpRequestToFile(completeUrlText, destinationFileName);

    return;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::issueHttpRequestToFile(QString completeUrlText, QString destinationFileName)
{
    setUrl(completeUrlText);
    m_file = new QFile(destinationFileName);
    if (!m_file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"),
            tr("Unable to save the file %1: %2.")
            .arg(destinationFileName).arg(m_file->errorString()));
        delete m_file;
        m_file = nullptr;
        return;
    }

    // schedule the request
    m_httpRequestAborted = false;
    startRequest(m_url);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::cancelDownload()
{
    //m_statusLabel->setText(tr("Download canceled."));
    m_httpRequestAborted = true;
    m_reply->abort();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::httpFinished()
{
    if (m_httpRequestAborted) {
        if (m_file) {
            m_file->close();
            m_file->remove();
            delete m_file;
            m_file = nullptr;
        }
        m_reply->deleteLater();
        m_myProgressDialog->hide();
        return;
    }

    m_file->flush();
    m_file->close();

    QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (m_reply->error()) {
        m_file->remove();
        QMessageBox::information(this, tr("HTTP"),
            tr("Download failed: %1.")
            .arg(m_reply->errorString()));
    } else if (!redirectionTarget.isNull()) {        
        QUrl newUrl = m_url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(this, tr("HTTP"),
            tr("Redirect to %1 ?").arg(newUrl.toString()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                m_url = newUrl;
                m_reply->deleteLater();
                m_file->open(QIODevice::WriteOnly);
                m_file->resize(0);
                startRequest(m_url);
                return;
        }
    } else {
        //m_statusLabel->setText(tr("Downloaded data to %1.").arg(m_destinationFolder));
    }

    if (m_currentDownloadState == DOWNLOAD_WELL_PATH)
    {
        QString singleWellPathFilePath = m_file->fileName();

        QFile file(singleWellPathFilePath);
        if (file.open(QFile::ReadOnly))
        {
            QString singleWellPathContent = file.readAll();

            // Strip leading and trailing []

            if (singleWellPathContent.indexOf('{') > 0)
            {
                singleWellPathContent = singleWellPathContent.right(singleWellPathContent.size() - singleWellPathContent.indexOf('{'));
            }

            if (singleWellPathContent[singleWellPathContent.size() - 1] == ']')
            {
                singleWellPathContent = singleWellPathContent.left(singleWellPathContent.size() - 1);
            }

            QString wellPathName = getValue("name", singleWellPathContent);
            if (!singleWellPathContent.isEmpty() && !wellPathName.isEmpty())
            {
                // Write out the content without leading/trailing []
                file.close();
                file.remove(singleWellPathFilePath);

                if (file.open(QFile::WriteOnly))
                {
                    QTextStream out(&file);
                    out << singleWellPathContent;
                }
            }

            m_myProgressDialog->setLabelText(QString("Downloaded well path : %1").arg(wellPathName));
        }

        int newValue = m_myProgressDialog->maximum() - m_wellRequestQueue.size();
        m_myProgressDialog->setValue(newValue);
    }

    m_reply->deleteLater();
    m_reply = nullptr;
    delete m_file;
    m_file = nullptr;

    if (m_currentDownloadState == DOWNLOAD_WELLS || m_currentDownloadState == DOWNLOAD_WELL_PATH)
    {
        checkDownloadQueueAndIssueRequests();
    }
    else if (m_currentDownloadState == DOWNLOAD_FIELDS)
    {
        updateFieldsModel();
        m_currentDownloadState = DOWNLOAD_UNDEFINED;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (m_file)
        m_file->write(m_reply->readAll());
}

//--------------------------------------------------------------------------------------------------
/// This slot will be called for the first network reply that will need authentication.
/// If the authentication is successful, the username/password is cached in the QNetworkAccessManager
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::slotAuthenticationRequired(QNetworkReply* networkReply, QAuthenticator* authenticator)
{
    if (m_firstTimeRequestingAuthentication)
    {
        // Use credentials from first wizard page
        authenticator->setUser(field("username").toString());
        authenticator->setPassword(field("password").toString());

        m_firstTimeRequestingAuthentication = false;
    }
    else
    {
        QMessageBox::information(this, "Authentication failed", "Failed to authenticate credentials. You will now be directed back to the first wizard page.");
        m_firstTimeRequestingAuthentication = true;

        restart();
    }
}

#ifndef QT_NO_OPENSSL
void RiuWellImportWizard::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
{
    QString errorString;
    foreach (const QSslError &error, errors) {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }

    if (QMessageBox::warning(this, tr("HTTP"),
        tr("One or more SSL errors has occurred: %1").arg(errorString),
        QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
            m_reply->ignoreSslErrors();
    }

}
#endif



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setUrl(const QString& httpAddress)
{
    m_url = httpAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::startRequest(QUrl url)
{
    auto request = QNetworkRequest(url);

#ifndef QT_NO_OPENSSL
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    request.setSslConfiguration(config);
#endif

    m_reply = m_networkAccessManager.get(request);
    connect(m_reply, SIGNAL(finished()),
        this, SLOT(httpFinished()));
    connect(m_reply, SIGNAL(readyRead()),
        this, SLOT(httpReadyRead()));
}


//--------------------------------------------------------------------------------------------------
/// Search for string, and find the associated value inside the next quoted string
//  text content : "A" : "B"
//  A search for key "A" returns B
//--------------------------------------------------------------------------------------------------
QString RiuWellImportWizard::getValue(const QString& key, const QString& stringContent)
{
    QString quotedKey = "\"" + key + "\"";

    int pos = stringContent.indexOf(quotedKey);
    if (pos >=0)
    {
        int valueStartPos = stringContent.indexOf("\"", pos + quotedKey.size());
        int valueEndPos = stringContent.indexOf("\"", valueStartPos + 1);

        if (valueStartPos >= 0 && valueEndPos > valueStartPos)
        {
            return stringContent.mid(valueStartPos + 1, valueEndPos - valueStartPos - 1);
        }
    }

    return QString();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::updateFieldsModel()
{
    QString fileName = jsonFieldsFilePath();

    if (caf::Utils::fileExists(fileName))
    {
        ResInsightInternalJson::JsonReader jsonReader;
        QMap<QString, QVariant> jsonMap = jsonReader.decodeFile(fileName);

        QStringList regions;
        QStringList fields;
        QStringList edmIds;
        QMapIterator<QString, QVariant> it(jsonMap);
        while (it.hasNext())
        {
            it.next();

            // If we have an array, skip to next node
            if (it.key() == "length")
                continue;

            QMap<QString, QVariant> fieldMap = it.value().toMap();

            regions.push_back(fieldMap["region"].toString());
            fields.push_back(fieldMap["name"].toString());
            edmIds.push_back(fieldMap["edmId"].toString());
        }

        m_wellPathImportObject->updateRegions(regions, fields, edmIds);

        for (size_t i = 0; i < m_wellPathImportObject->regions.size(); i++)
        {
            m_wellPathImportObject->regions[i]->updateState();
        }
        

        m_wellPathImportObject->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadWells()
{
    for (size_t rIdx = 0; rIdx < m_wellPathImportObject->regions.size(); rIdx++)
    {
        RimOilRegionEntry* oilRegion = m_wellPathImportObject->regions[rIdx];
        if (oilRegion->selected)
        {
            for (size_t fIdx = 0; fIdx < oilRegion->fields.size(); fIdx++)
            {
                RimOilFieldEntry* oilField = oilRegion->fields[fIdx];
                if (oilField->selected)
                {
                    DownloadEntity urlToFile;

                    QString wellRequest;
                    if (m_wellPathImportObject->utmFilterMode == RimWellPathImport::UTM_FILTER_OFF)
                    {
                        wellRequest = QString("/resinsight/projects/%1/wells").arg(oilField->edmId);
                    }
                    else
                    {
                        wellRequest = QString("/resinsight/projects/%1/wellsInArea?north=%2&south=%3&east=%4&west=%5&utmzone=32N")
                            .arg(oilField->edmId)
                            .arg(QString::number(m_wellPathImportObject->north, 'g', 10))
                            .arg(QString::number(m_wellPathImportObject->south, 'g', 10))
                            .arg(QString::number(m_wellPathImportObject->east, 'g', 10))
                            .arg(QString::number(m_wellPathImportObject->west, 'g', 10));
                    }

                    urlToFile.requestUrl = m_webServiceAddress + wellRequest;
                    urlToFile.responseFilename = m_destinationFolder + QString("/wells_%1.json").arg(oilField->edmId);

                    oilField->wellsFilePath = urlToFile.responseFilename;

                    m_wellRequestQueue.push_back(urlToFile);
                }
            }
        }
    }

    m_currentDownloadState = DOWNLOAD_WELLS;
    checkDownloadQueueAndIssueRequests();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadWellPaths()
{
    WellSelectionPage* wellSelectionPage = dynamic_cast<WellSelectionPage*>(page(m_wellSelectionPageId));
    std::vector<DownloadEntity> downloadEntities;
    wellSelectionPage->selectedWellPathEntries(downloadEntities, nullptr);

    for (size_t i = 0; i < downloadEntities.size(); i++)
    {
        m_wellRequestQueue.push_back(downloadEntities[i]);
    }

    m_currentDownloadState = DOWNLOAD_WELL_PATH;

    m_myProgressDialog->setMaximum(m_wellRequestQueue.size());
    m_myProgressDialog->setValue(0);
    m_myProgressDialog->show();


    checkDownloadQueueAndIssueRequests();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::checkDownloadQueueAndIssueRequests()
{
    if (m_wellRequestQueue.size() > 0)
    {
        DownloadEntity firstItem = m_wellRequestQueue[0];
        m_wellRequestQueue.pop_front();

        QString completeUrlText = firstItem.requestUrl;
        QString absoluteFilePath = firstItem.responseFilename;

        issueHttpRequestToFile(completeUrlText, absoluteFilePath);

        return;
    }

    if (m_currentDownloadState == DOWNLOAD_WELLS)
    {
        m_myProgressDialog->hide();

        // Update UI with downloaded wells

        for (size_t rIdx = 0; rIdx < m_wellPathImportObject->regions.size(); rIdx++)
        {
            RimOilRegionEntry* oilRegion = m_wellPathImportObject->regions[rIdx];
            if (oilRegion->selected)
            {
                for (size_t fIdx = 0; fIdx < oilRegion->fields.size(); fIdx++)
                {
                    RimOilFieldEntry* oilField = oilRegion->fields[fIdx];
                    if (oilField->selected)
                    {
                        parseWellsResponse(oilField);
                    }
                }
            }
        }

        m_wellPathImportObject->updateConnectedEditors();
    }
    else if (m_currentDownloadState == DOWNLOAD_WELL_PATH)
    {
        WellSummaryPage* wellSummaryPage = dynamic_cast<WellSummaryPage*>(page(m_wellSummaryPageId));
        if (wellSummaryPage)
        {
            wellSummaryPage->updateSummaryPage();
        }
    }

    m_currentDownloadState = DOWNLOAD_UNDEFINED;

    m_myProgressDialog->hide();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::resetAuthenticationCount()
{
    m_firstTimeRequestingAuthentication = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RiuWellImportWizard::absoluteFilePathsToWellPaths() const
{
    QStringList filePaths;

    WellSelectionPage* wellSelectionPage = dynamic_cast<WellSelectionPage*>(page(m_wellSelectionPageId));
    std::vector<DownloadEntity> downloadEntities;
    wellSelectionPage->selectedWellPathEntries(downloadEntities, nullptr);

    for (size_t i = 0; i < downloadEntities.size(); i++)
    {
        if (caf::Utils::fileExists(downloadEntities[i].responseFilename))
        {
            filePaths.push_back(downloadEntities[i].responseFilename);
        }
    }

    return filePaths;
}

//--------------------------------------------------------------------------------------------------
/// Set wells hidden from the field selection page
/// TODO: This can be refactored when UIOrdering for objects is created
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::slotCurrentIdChanged(int currentId)
{
    bool hideWells = true;
    if (currentId == m_wellSelectionPageId) hideWells = false;

    for (size_t rIdx = 0; rIdx < m_wellPathImportObject->regions.size(); rIdx++)
    {
        RimOilRegionEntry* oilRegion = m_wellPathImportObject->regions[rIdx];
        {
            for (size_t fIdx = 0; fIdx < oilRegion->fields.size(); fIdx++)
            {
                RimOilFieldEntry* oilField = oilRegion->fields[fIdx];
                oilField->wells.uiCapability()->setUiHidden(hideWells);
            }
        }
    }

    // Update the editors to propagate the changes to UI
    m_wellPathImportObject->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::parseWellsResponse(RimOilFieldEntry* oilFieldEntry)
{
    QStringList surveyNames;
    QStringList planNames;

    if (caf::Utils::fileExists(oilFieldEntry->wellsFilePath))
    {
        ResInsightInternalJson::JsonReader jsonReader;
        QMap<QString, QVariant> jsonMap = jsonReader.decodeFile(oilFieldEntry->wellsFilePath);

        QMapIterator<QString, QVariant> it(jsonMap);
        while (it.hasNext())
        {
            it.next();

            // If we have an array, skip to next node
            if (it.key() == "length")
                continue;

            QMap<QString, QVariant> rootMap = it.value().toMap();

            if (m_wellPathImportObject->wellTypeSurvey)
            {
                QMap<QString, QVariant> surveyMap = rootMap["survey"].toMap();
                QString name = surveyMap["name"].toString();

                if (!oilFieldEntry->find(name, RimWellPathEntry::WELL_SURVEY))
                {
                    QMap<QString, QVariant> linksMap = surveyMap["links"].toMap();
                    QString requestUrl = m_webServiceAddress + linksMap["entity"].toString();
                    QString surveyType = surveyMap["surveyType"].toString();
                    RimWellPathEntry* surveyWellPathEntry = RimWellPathEntry::createWellPathEntry(name, surveyType, requestUrl, m_destinationFolder, RimWellPathEntry::WELL_SURVEY);
                    oilFieldEntry->wells.push_back(surveyWellPathEntry);
                }

                surveyNames.push_back(name);
            }

            if (m_wellPathImportObject->wellTypePlans)
            {
                QList<QVariant> plansList = rootMap["plans"].toList();
                QListIterator<QVariant> planIt(plansList);
                while (planIt.hasNext())
                {
                    QMap<QString, QVariant> planMap = planIt.next().toMap();
                    QString name = planMap["name"].toString();

                    if (!oilFieldEntry->find(name, RimWellPathEntry::WELL_PLAN))
                    {
                        QMap<QString, QVariant> linksMap = planMap["links"].toMap();
                        QString requestUrl = m_webServiceAddress + linksMap["entity"].toString();
                        QString surveyType = planMap["surveyType"].toString();
                        RimWellPathEntry* surveyWellPathEntry = RimWellPathEntry::createWellPathEntry(name, surveyType, requestUrl, m_destinationFolder, RimWellPathEntry::WELL_PLAN);
                        oilFieldEntry->wells.push_back(surveyWellPathEntry);
                    }

                    planNames.push_back(name);
                }
            }
        }
    }

    // Delete the well path entries in the model that are not part of the reply from the web service
    std::vector<RimWellPathEntry*> wellsToRemove;

    for (size_t i = 0; i < oilFieldEntry->wells.size(); i++)
    {
        RimWellPathEntry* wellPathEntry = oilFieldEntry->wells[i];
        if (wellPathEntry->wellPathType == RimWellPathEntry::WELL_PLAN)
        {
            if (!planNames.contains(wellPathEntry->name))
            {
                wellsToRemove.push_back(wellPathEntry);
            }
        }
        else
        {
            if (!surveyNames.contains(wellPathEntry->name))
            {
                wellsToRemove.push_back(wellPathEntry);
            }
        }
    }

    for (size_t i = 0; i < wellsToRemove.size(); i++)
    {
        oilFieldEntry->wells.removeChildObject(wellsToRemove[i]);

        delete wellsToRemove[i];
    }

    WellSelectionPage* wellSelectionPage = dynamic_cast<WellSelectionPage*>(page(m_wellSelectionPageId));
    if (wellSelectionPage)
        wellSelectionPage->buildWellTreeView();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setCredentials(const QString& username, const QString& password)
{
    // Set the initial value of the fields defined in the Authorization page
    setField("username", username);
    setField("password", password);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuWellImportWizard::wellSelectionPageId()
{
    return m_wellSelectionPageId;
}












//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AuthenticationPage::AuthenticationPage(const QString& webServiceAddress, QWidget *parent /*= 0*/) : QWizardPage(parent)
{
    setTitle("SSIHUB - Login");

    QVBoxLayout* layout = new QVBoxLayout;

    QLabel* label = new QLabel("Please enter your login information for SSIHUB at : " + webServiceAddress);
    layout->addWidget(label);

    QFormLayout* formLayout = new QFormLayout;
    layout->addLayout(formLayout);

    QLineEdit* usernameLineEdit = new QLineEdit("", this);
    QLineEdit* passwordlLineEdit = new QLineEdit("", this);
    passwordlLineEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("&Username:", usernameLineEdit);
    formLayout->addRow("&Password:", passwordlLineEdit);

    setLayout(layout);

    // Make variables accessible to other pages in wizard
    // Use * at end of field name to indicate mandatory field
    registerField("username", usernameLineEdit);
    registerField("password", passwordlLineEdit);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AuthenticationPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>(wizard());
    wiz->resetAuthenticationCount();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FieldSelectionPage::FieldSelectionPage(RimWellPathImport* wellPathImport, QWidget *parent /*= 0*/)
{
    setTitle("Field Selection");

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QLabel* label = new QLabel("Select fields");
    layout->addWidget(label);


    // Tree view
    caf::PdmUiTreeView* treeView = new caf::PdmUiTreeView(this);
    treeView->setPdmItem(wellPathImport);
    layout->addWidget(treeView);
    layout->setStretchFactor(treeView, 10);

 
    // Property view
    m_propertyView = new caf::PdmUiPropertyView(this);
    layout->addWidget(m_propertyView);
    m_propertyView->showProperties(wellPathImport);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FieldSelectionPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>(wizard());
    wiz->downloadFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FieldSelectionPage::~FieldSelectionPage()
{
    m_propertyView->showProperties(nullptr);
}











//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WellSelectionPage::WellSelectionPage(RimWellPathImport* wellPathImport, QWidget* parent /*= 0*/)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QLabel* label = new QLabel("Select wells");
    layout->addWidget(label);

    m_wellSelectionTreeView = new caf::PdmUiTreeView(this);
    layout->addWidget(m_wellSelectionTreeView);

    m_wellPathImportObject = wellPathImport;

    m_regionsWithVisibleWells = new ObjectGroupWithHeaders;
    m_regionsWithVisibleWells->objects.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>(wizard());
    if (!wiz) return;

    wiz->downloadWells();

    setButtonText(QWizard::NextButton, "Download");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::buildWellTreeView()
{
    for (size_t rIdx = 0; rIdx < m_regionsWithVisibleWells->objects.size(); rIdx++)
    {
        caf::PdmObjectGroup* regGroup = dynamic_cast<caf::PdmObjectGroup*>(m_regionsWithVisibleWells->objects[rIdx]);
        if (!regGroup)
            continue;

        for (size_t fIdx = 0; fIdx < regGroup->objects.size(); fIdx++)
        {
            caf::PdmObjectGroup* fieldGroup = dynamic_cast<caf::PdmObjectGroup*>(regGroup->objects[fIdx]);
            if (!fieldGroup)
                continue;

            // RimWellPathEntry objects are present here, they must be taken out out the container, but not deleted
            // If fieldGroup->objects->deleteObjects is performed, the objects are deleted
            fieldGroup->objects.clear();
        }
    }

    // Delete all temporary pdm object groups
    m_regionsWithVisibleWells->objects.deleteAllChildObjects();

    for (size_t rIdx = 0; rIdx < m_wellPathImportObject->regions.size(); rIdx++)
    {
        RimOilRegionEntry* oilRegion = m_wellPathImportObject->regions[rIdx];
        if (oilRegion->selected)
        {
            caf::PdmObjectCollection* regGroup = new caf::PdmObjectCollection;
            regGroup->objects.uiCapability()->setUiHidden(true);

            regGroup->setUiName(oilRegion->userDescriptionField()->uiCapability()->uiValue().toString());

            m_regionsWithVisibleWells->objects.push_back(regGroup);

            for (size_t fIdx = 0; fIdx < oilRegion->fields.size(); fIdx++)
            {
                RimOilFieldEntry* oilField = oilRegion->fields[fIdx];
                if (oilField->selected)
                {
                    caf::PdmObjectCollection* fieldGroup = new caf::PdmObjectCollection;
                    fieldGroup->objects.uiCapability()->setUiHidden(true);

                    fieldGroup->setUiName(oilField->userDescriptionField()->uiCapability()->uiValue().toString());

                    regGroup->objects.push_back(fieldGroup);

                    for (size_t wIdx = 0; wIdx < oilField->wells.size(); wIdx++)
                    {
                        RimWellPathEntry* wellPathEntry = oilField->wells[wIdx];

                        // Create a copy of the PdmObject, as it is not supported to have multiple parents of any objects
                        QString objStr = wellPathEntry->writeObjectToXmlString();

                        RimWellPathEntry* wellPathCopy = new RimWellPathEntry;
                        wellPathCopy->readObjectFromXmlString(objStr, caf::PdmDefaultObjectFactory::instance());

                        fieldGroup->objects.push_back(wellPathCopy);
                    }

                    sortObjectsByDescription(fieldGroup);
                }
            }
        }
    }

    m_wellSelectionTreeView->setPdmItem(m_regionsWithVisibleWells);
    m_regionsWithVisibleWells->updateConnectedEditors();
    
    m_wellSelectionTreeView->treeView()->expandAll();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WellSelectionPage::~WellSelectionPage()
{
    if (m_wellSelectionTreeView)
    {
        m_wellSelectionTreeView->setPdmItem(nullptr);
    }
    delete m_regionsWithVisibleWells;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::selectedWellPathEntries(std::vector<DownloadEntity>& downloadEntities, caf::PdmObjectHandle* objHandle)
{
    if (objHandle == nullptr)
    {
        objHandle = m_regionsWithVisibleWells;
    }

    std::vector<caf::PdmFieldHandle*> childFields;
    objHandle->fields(childFields);
    for (size_t i = 0; i < childFields.size(); i++)
    {
        std::vector<caf::PdmObjectHandle*> childObjects;
        childFields[i]->childObjects(&childObjects);

        for (size_t j = 0; j < childObjects.size(); j++)
        {
            RimWellPathEntry* wellPathEntry = (dynamic_cast<RimWellPathEntry*>(childObjects[j]));
            if (wellPathEntry)
            {
                if (wellPathEntry->selected && wellPathEntry->isWellPathValid())
                {
                    DownloadEntity urlToFile;

                    urlToFile.name = wellPathEntry->name;
                    urlToFile.requestUrl = wellPathEntry->requestUrl;
                    urlToFile.responseFilename = wellPathEntry->wellPathFilePath;

                    downloadEntities.push_back(urlToFile);
                }
            }
            else
            {
                selectedWellPathEntries(downloadEntities, childObjects[j]);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool lessByDescription(const caf::PdmPointer<caf::PdmObjectHandle>& obj1, const caf::PdmPointer<caf::PdmObjectHandle>& obj2)
{
    caf::PdmUiFieldHandle* uiFieldHandle1 = nullptr;
    caf::PdmUiFieldHandle* uiFieldHandle2 = nullptr;

    if (obj1.notNull() && obj1->uiCapability() && obj1->uiCapability()->userDescriptionField())
    {
        uiFieldHandle1 = obj1->uiCapability()->userDescriptionField()->uiCapability();
    }

    if (obj2.notNull() && obj2->uiCapability() && obj2->uiCapability()->userDescriptionField())
    {
        uiFieldHandle2 = obj2->uiCapability()->userDescriptionField()->uiCapability();
    }

    if (uiFieldHandle1 && uiFieldHandle2)
    {
        QString string1 = uiFieldHandle1->uiValue().toString();
        QString string2 = uiFieldHandle2->uiValue().toString();

        return string1 < string2;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::sortObjectsByDescription(caf::PdmObjectCollection* objects)
{
    std::sort(objects->objects.begin(), objects->objects.end(), lessByDescription);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WellSummaryPage::WellSummaryPage(RimWellPathImport* wellPathImport, QWidget* parent /*= 0*/)
{
    m_wellPathImportObject = wellPathImport;
    m_wellPathImportObject->setUiHidden(true);

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    layout->addWidget(m_textEdit);

    QPushButton* button = new QPushButton("Show/hide details", this);
    connect(button, SIGNAL(clicked()), this, SLOT(slotShowDetails()));
    layout->addWidget(button);

    m_listView = new caf::PdmUiListView(this);
    layout->setStretchFactor(m_listView, 10);
    layout->addWidget(m_listView);
    m_listView->hide();

    m_objectGroup = new caf::PdmObjectCollection;

    setButtonText(QWizard::FinishButton, "Import");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSummaryPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>(wizard());
    wiz->downloadWellPaths();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSummaryPage::updateSummaryPage()
{
    m_objectGroup->objects.clear();

    m_textEdit->setText("Summary of imported wells\n\n");

    size_t wellPathCount = 0;
    QString errorString;

    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>(wizard());
    WellSelectionPage* wellSelectionPage = dynamic_cast<WellSelectionPage*>(wiz->page(wiz->wellSelectionPageId()));
    std::vector<DownloadEntity> downloadEntities;
    wellSelectionPage->selectedWellPathEntries(downloadEntities, nullptr);

    for (size_t i = 0; i < downloadEntities.size(); i++)
    {
        if (caf::Utils::fileExists(downloadEntities[i].responseFilename))
        {
            wellPathCount++;
        }
        else
        {
            errorString += QString("Failed to get file '%1' from well '%2'\n").arg(downloadEntities[i].responseFilename).arg(downloadEntities[i].name);
        }


        SummaryPageDownloadEntity* sumPageEntity = new SummaryPageDownloadEntity;
        sumPageEntity->name = downloadEntities[i].name;
        sumPageEntity->responseFilename = downloadEntities[i].responseFilename;
        sumPageEntity->requestUrl = downloadEntities[i].requestUrl;

        m_objectGroup->objects().push_back(sumPageEntity);
    }

    m_textEdit->setText(QString("Downloaded successfully %1 well paths.\nPlease push 'Import' button to import well paths into ResInsight.\n\n").arg(wellPathCount));
    if (!errorString.isEmpty())
    {
        m_textEdit->append("Detected following errors during well path download. See details below.");
        m_textEdit->append(errorString);

    }

    m_listView->setPdmObject(m_objectGroup);
    m_objectGroup->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSummaryPage::slotShowDetails()
{
    if (m_listView->isHidden())
    {
        m_listView->show();
    }
    else
    {
        m_listView->hide();
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ObjectGroupWithHeaders::defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    caf::PdmUiTreeViewEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTreeViewEditorAttribute*>(attribute);
    if (myAttr)
    {
        QStringList colHeaders;
        colHeaders << "Wells";
        myAttr->columnHeaders = colHeaders;
    }
}



CAF_PDM_SOURCE_INIT(SummaryPageDownloadEntity, "SummaryPageDownloadEntity");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SummaryPageDownloadEntity::SummaryPageDownloadEntity()
{
    CAF_PDM_InitObject("SummaryPageDownloadEntity", "", "", "");

    CAF_PDM_InitFieldNoDefault(&name, "Name", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&requestUrl, "RequestUrl", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&responseFilename, "ResponseFilename", "", "", "", "");
}
