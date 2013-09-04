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

#include <QObject>
#include <QtGui>
#include <QtNetwork>


/*
#include <QWizard>
#include <QVBoxLayout>
#include <QTreeView>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QFile>
#include <QUuid>
*/

#include "cafPdmUiPropertyView.h"
#include "RimWellPathImport.h"
#include "cafUiTreeModelPdm.h"
#include "ssihubDialog.h"

#include "RifJsonEncodeDecode.h"
#include "cafPdmUiTreeView.h"
#include "RimWellCollection.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellImportWizard::RiuWellImportWizard(const QString& webServiceAddress, const QString& downloadFolder, RimWellPathImport* wellPathImportObject, QWidget *parent /*= 0*/)
 : QWizard(parent)
{
    m_wellPathImport = wellPathImportObject;

    m_destinationFolder = downloadFolder;
    m_webServiceAddress = webServiceAddress;

//    m_treeModelPdm = new caf::UiTreeModelPdm(this);

    addPage(new IntroPage(webServiceAddress, this));

    m_pdmTreeView = new caf::PdmUiTreeView(this);
    m_pdmTreeView->showTree(wellPathImportObject);

    addPage(new FieldSelectionPage(m_wellPathImport, NULL, m_pdmTreeView, this));

    addPage(new WellSelectionPage(m_wellPathImport, this));
    m_wellSummaryPageId = addPage(new WellSummaryPage(m_wellPathImport, this));

    m_statusLabel = new QLabel(tr("Status : idle"));
    m_statusLabel->setWordWrap(true);

    m_progressDialog = new QProgressDialog(this);

    m_wellCollection = new RimWellCollection;


    connect(&m_networkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
        this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(&m_networkAccessManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
        this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif

    connect(m_progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setWebServiceAddress(const QString& wsAddress)
{
    m_webServiceAddress = wsAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setJsonDestinationFolder(const QString& folder)
{
    m_destinationFolder = folder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setWellPathImportObject(RimWellPathImport* wellPathImportObject)
{
    m_wellPathImport = wellPathImportObject;
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
    if (QFile::exists(wellFileName))
    {
        QFile::remove(wellFileName);
    }

    m_wellPathImport->regions.deleteAllChildObjects();
    m_wellPathImport->updateConnectedEditors();

    QString completeUrlText = m_webServiceAddress + "/resinsight/projects";
    QString destinationFileName = jsonFieldsFilePath();

    m_currentDownloadState = DOWNLOAD_FIELDS;
    issueHttpRequestToFile(completeUrlText, destinationFileName);

    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::checkDownloadQueueAndIssueRequests()
{
    if (m_wellPathRequestQueue.size() > 0)
    {
        QString link = m_wellPathRequestQueue[0];
        m_wellPathRequestQueue.pop_front();

        QString completeUrlText = m_webServiceAddress + link;

        QUuid guid = QUuid::createUuid();
        QString singleWellPathFilePath = m_destinationFolder + QString("/wellpath_%1.json").arg(guid);

        m_currentDownloadState = DOWNLOAD_WELL_PATH;
        issueHttpRequestToFile(completeUrlText, singleWellPathFilePath);
    }

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
        m_file = 0;
        return;
    }

    m_progressDialog->setWindowTitle(tr("HTTP"));
    m_progressDialog->setLabelText(tr("Downloading %1.").arg(destinationFileName));

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

    refreshButtonStatus();

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
            m_file = 0;
        }
        m_reply->deleteLater();
        m_progressDialog->hide();
        return;
    }

    if (m_wellPathRequestQueue.size() == 0)
    {
        m_progressDialog->hide();
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
        m_statusLabel->setText(tr("Downloaded data to %1.").arg(m_destinationFolder));
    }

    if (m_currentDownloadState == DOWNLOAD_WELL_PATH)
    {
        
        qDebug() << "Downloaded " << m_file->fileName();

        /*
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
//                 int currentRowCount = m_wellPathsModel->rowCount();
//                 m_wellPathsModel->setRowCount(m_wellPathsModel->rowCount() + 1);
// 
//                 QModelIndex miName = m_wellPathsModel->index(currentRowCount, 0);
//                 m_wellPathsModel->setData(miName, wellPathName);
// 
//                 QModelIndex miFileName = m_wellPathsModel->index(currentRowCount, 1);
//                 m_wellPathsModel->setData(miFileName, singleWellPathFilePath);


                // Write out the content without leading/trailing []
                file.close();
                file.remove(singleWellPathFilePath);

                if (file.open(QFile::WriteOnly))
                {
                    QTextStream out(&file);
                    out << singleWellPathContent;
                }
            }
        }
        */
    }


    refreshButtonStatus();

    m_reply->deleteLater();
    m_reply = 0;
    delete m_file;
    m_file = 0;

    if (m_currentDownloadState == DOWNLOAD_WELLS || m_currentDownloadState == DOWNLOAD_WELL_PATH)
    {
        checkDownloadQueueAndIssueRequests_v2();
    }
    else if (m_currentDownloadState == DOWNLOAD_FIELDS)
    {
        updateFieldsModel();
        m_currentDownloadState = DOWNLOAD_UNDEFINED;
    }
    else
    {
        checkDownloadQueueAndIssueRequests();
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
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (m_httpRequestAborted)
        return;

    m_progressDialog->setMaximum(totalBytes);
    m_progressDialog->setValue(bytesRead);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::refreshButtonStatus()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::slotAuthenticationRequired(QNetworkReply* networkReply, QAuthenticator* authenticator)
{
    /*
     QDialog dlg;
     Ui::Dialog ui;
     ui.setupUi(&dlg);
     dlg.adjustSize();
     ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm()).arg(m_url.host()));

    // Did the URL have information? Fill the UI
    // This is only relevant if the URL-supplied credentials were wrong
    ui.userEdit->setText(m_url.userName());
    ui.passwordEdit->setText(m_url.password());
    ui.passwordEdit->setEchoMode(QLineEdit::Password);

    if (dlg.exec() == QDialog::Accepted) {
        authenticator->setUser(ui.userEdit->text());
        authenticator->setPassword(ui.passwordEdit->text());
    }

    */

    authenticator->setUser(field("username").toString());
    authenticator->setPassword(field("password").toString());
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
void RiuWellImportWizard::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{

}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setUrl(const QString& httpAddress)
{
//    m_urlLineEdit->setText(httpAddress);

    m_url = httpAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::startRequest(QUrl url)
{
    m_reply = m_networkAccessManager.get(QNetworkRequest(url));
    connect(m_reply, SIGNAL(finished()),
        this, SLOT(httpFinished()));
    connect(m_reply, SIGNAL(readyRead()),
        this, SLOT(httpReadyRead()));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)),
        this, SLOT(updateDataReadProgress(qint64,qint64)));

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
void RiuWellImportWizard::getWellPathLinks(QStringList* surveyLinks, QStringList* planLinks)
{
    QStringList entities;

    QString filename = jsonWellsFilePath();
    if (QFile::exists(filename))
    {
        JsonReader jsonReader;
        QMap<QString, QVariant> jsonMap = jsonReader.decodeFile(filename);

        QMapIterator<QString, QVariant> it(jsonMap);
        while (it.hasNext())
        {
            it.next();

            QString key = it.key();
            if (key[0].isDigit())
            {
                QMap<QString, QVariant> slotMap = it.value().toMap();
                QMap<QString, QVariant> linkMap = slotMap["links"].toMap();

                QString surveyLink = linkMap["survey"].toString();
                surveyLinks->push_back(surveyLink);

                QString planLink = linkMap["plans"].toString();
                planLinks->push_back(planLink);
            }
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::issueDownloadOfWellPaths(const QStringList& surveyLinks, const QStringList& planLinks)
{
    m_wellPathRequestQueue.clear();

//    if (m_importSurveyCheckBox->isChecked())
    {
        m_wellPathRequestQueue += surveyLinks;
    }

//    if (m_importPlansCheckBox->isChecked())
    {
        m_wellPathRequestQueue += planLinks;
    }

    checkDownloadQueueAndIssueRequests();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::updateFieldsModel()
{
    QString fileName = jsonFieldsFilePath();

    if (QFile::exists(fileName))
    {
        JsonReader jsonReader;
        QMap<QString, QVariant> jsonMap = jsonReader.decodeFile(fileName);

        QStringList regions;
        QStringList fields;
        QStringList edmIds;
        QMapIterator<QString, QVariant> it(jsonMap);
        while (it.hasNext())
        {
            it.next();

            QString key = it.key();
            if (key[0].isDigit())
            {
                QMap<QString, QVariant> fieldMap = it.value().toMap();

                regions.push_back(fieldMap["region"].toString());
                fields.push_back(fieldMap["name"].toString());
                edmIds.push_back(fieldMap["edmId"].toString());
            }
        }

        m_wellPathImport->updateRegions(regions, fields, edmIds);
        m_wellPathImport->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadWells()
{
    for (size_t rIdx = 0; rIdx < m_wellPathImport->regions.size(); rIdx++)
    {
        RimOilRegionEntry* oilRegion = m_wellPathImport->regions[rIdx];
        if (oilRegion->selected)
        {
            for (size_t fIdx = 0; fIdx < oilRegion->fields.size(); fIdx++)
            {
                RimOilFieldEntry* oilField = oilRegion->fields[fIdx];
                if (oilField->selected)
                {
                    DownloadEntity urlToFile;

                    QString wellRequest;
                    if (m_wellPathImport->utmFilterMode == RimWellPathImport::UTM_FILTER_OFF)
                    {
                        wellRequest = QString("/resinsight/projects/%1/wells").arg(oilField->edmId);
                    }
                    else
                    {
                        wellRequest = QString("/resinsight/projects/%1/wellsInArea?north=%2&south=%3&east=%4&west=%5&utmZone=32N")
                            .arg(oilField->edmId)
                            .arg(QString::number(m_wellPathImport->north, 'g', 10))
                            .arg(QString::number(m_wellPathImport->south, 'g', 10))
                            .arg(QString::number(m_wellPathImport->east, 'g', 10))
                            .arg(QString::number(m_wellPathImport->west, 'g', 10));
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
    checkDownloadQueueAndIssueRequests_v2();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadWellPaths()
{
    for (size_t rIdx = 0; rIdx < m_wellPathImport->regions.size(); rIdx++)
    {
        RimOilRegionEntry* oilRegion = m_wellPathImport->regions[rIdx];
        if (oilRegion->selected)
        {
            for (size_t fIdx = 0; fIdx < oilRegion->fields.size(); fIdx++)
            {
                RimOilFieldEntry* oilField = oilRegion->fields[fIdx];
                if (oilField->selected)
                {
                    for (size_t wIdx = 0; wIdx < oilField->wells.size(); wIdx++)
                    {
                        RimWellPathEntry* wellPathEntry = oilField->wells[wIdx];
                        
                        if (!wellPathEntry->isWellPathValid())
                            continue;
                        
                        DownloadEntity urlToFile;

                        urlToFile.requestUrl = wellPathEntry->requestUrl;
                        urlToFile.responseFilename = wellPathEntry->wellPathFilePath;

                        m_wellRequestQueue.push_back(urlToFile);
                    }
                }
            }
        }
    }

    m_currentDownloadState = DOWNLOAD_WELL_PATH;

    checkDownloadQueueAndIssueRequests_v2();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::checkDownloadQueueAndIssueRequests_v2()
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
        m_wellCollection->wells.clear();

        // Update UI with downloaded wells

        for (size_t rIdx = 0; rIdx < m_wellPathImport->regions.size(); rIdx++)
        {
            RimOilRegionEntry* oilRegion = m_wellPathImport->regions[rIdx];
            if (oilRegion->selected)
            {
                for (size_t fIdx = 0; fIdx < oilRegion->fields.size(); fIdx++)
                {
                    RimOilFieldEntry* oilField = oilRegion->fields[fIdx];
                    if (oilField->selected)
                    {
                        oilField->parseWellsResponse(m_destinationFolder, m_webServiceAddress);

                        for (size_t wIdx = 0; wIdx < oilField->wells.size(); wIdx++)
                        {
                            m_wellCollection->wells.push_back(oilField->wells[wIdx]);
                        }
                    }
                }
            }
        }


        m_wellCollection->updateConnectedEditors();
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellImportWizard::~RiuWellImportWizard()
{
    delete m_wellCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellCollection* RiuWellImportWizard::wellCollection()
{
    return m_wellCollection;
}











//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
IntroPage::IntroPage(const QString& webServiceAddress, QWidget *parent /*= 0*/) : QWizardPage(parent)
{
    setTitle("SSIHUB - Login");

    QVBoxLayout* layout = new QVBoxLayout;

    QLabel* label = new QLabel("Please enter your login information for SSIHUB at : " + webServiceAddress);
    layout->addWidget(label);

    QFormLayout* formLayout = new QFormLayout;
    layout->addLayout(formLayout);

    QLineEdit* usernameLineEdit = new QLineEdit("admin", this);
    QLineEdit* passwordlLineEdit = new QLineEdit("resinsight", this);
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
FieldSelectionPage::FieldSelectionPage(RimWellPathImport* wellPathImport, caf::UiTreeModelPdm* treeModelPdm, caf::PdmUiTreeView* pdmUiTreeView, QWidget *parent /*= 0*/)
{
    setTitle("Field Selection");

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QLabel* label = new QLabel("Select fields");
    layout->addWidget(label);

    layout->addWidget(pdmUiTreeView);

    // Property view
    caf::PdmUiPropertyView* propertyView = new caf::PdmUiPropertyView(this);
    layout->addWidget(propertyView);
    propertyView->showProperties(wellPathImport);
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
WellSelectionPage::WellSelectionPage(RimWellPathImport* wellPathImport, QWidget* parent /*= 0*/)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>(wizard());
    wiz->downloadWells();

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QLabel* label = new QLabel("Select wells");
    layout->addWidget(label);

    m_wellSelectionTreeView = new caf::PdmUiTreeView(this);
    m_wellSelectionTreeView->showTree(wiz->wellCollection());

    layout->addWidget(m_wellSelectionTreeView);

}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WellSummaryPage::WellSummaryPage(RimWellPathImport* wellPathImport, QWidget* parent /*= 0*/)
{
    m_wellPathImportObject = wellPathImport;

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    m_textEdit = new QTextEdit(this);

    layout->addWidget(m_textEdit);
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
    m_textEdit->setText("Summary of imported wells\n\n");

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
                    QString oilFieldText = QString("\nRegion : %1  -  Field : %2").arg(oilRegion->name).arg(oilField->name);
                    m_textEdit->append(oilFieldText);

                    for (size_t wIdx = 0; wIdx < oilField->wells.size(); wIdx++)
                    {
                        RimWellPathEntry* wellPathEntry = oilField->wells[wIdx];
                        QString wellStatus = QString("%1 %2").arg(oilField->wells[wIdx]->name).arg(oilField->wells[wIdx]->wellPathFilePath);

                        m_textEdit->append(wellStatus);
                    }
                }
            }
        }
    }
}

