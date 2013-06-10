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


#include "ssihubDialog.h"


#include <QtGui>
#include <QtNetwork>

#include "httpwindow.h"
#include "ui_authenticationdialog.h"
#include "RifJsonEncodeDecode.h"


namespace ssihub {



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FetchWellPathsDialog::FetchWellPathsDialog(QWidget *parent)
    : QDialog(parent)
{
    m_urlSsiHubLineEdit = new QLineEdit;
    m_urlSsiHubLineEdit->setReadOnly(true);
    m_urlSsiHubLabel = new QLabel(tr("SSIHUB address:"));
    m_urlSsiHubLabel->setBuddy(m_urlSsiHubLineEdit);

    m_urlLineEdit = new QLineEdit;
    m_urlLineEdit->setReadOnly(true);
    m_urlLabel = new QLabel(tr("SSIHUB complete request:"));
    m_urlLabel->setBuddy(m_urlLineEdit);

    m_statusLabel = new QLabel(tr("Status : idle"));
    m_statusLabel->setWordWrap(true);

    m_downloadFieldsButton = new QPushButton(tr("Get fields"));
    connect(m_downloadFieldsButton, SIGNAL(clicked()), this, SLOT(downloadFields()));

    // Fields data model and view
    m_fieldListView = new QListView(this);
    m_fieldModel = new QStringListModel;
    m_fieldListView->setModel(m_fieldModel);
    connect(m_fieldListView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection& )), this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection& )));

    // Well paths data model and view
    m_wellPathsModel = new QStandardItemModel;
    m_wellPathsModel->setColumnCount(2);
    m_wellPathsView = new QListView(this);
    m_wellPathsView->setModel(m_wellPathsModel);

    // Filter by Utm coordinates
    m_filterWellsByUtmArea = new QCheckBox("Filter by UTM area");
    connect(m_filterWellsByUtmArea, SIGNAL(clicked()), this, SLOT(refreshButtonStatus()));

    m_northLineEdit = new QLineEdit;
    m_southLineEdit = new QLineEdit;
    m_eastLineEdit = new QLineEdit;
    m_westLineEdit = new QLineEdit;



    QGroupBox* utmAreaGropBox = new QGroupBox("UTM filter by area");
    QGridLayout *utmAreaLayout = new QGridLayout;
    utmAreaLayout->addWidget(m_filterWellsByUtmArea, 0, 1);
    utmAreaLayout->addWidget(new QLabel("North"),   1, 0);
    utmAreaLayout->addWidget(m_northLineEdit,       1, 1);
    utmAreaLayout->addWidget(new QLabel("South"),   1, 2);
    utmAreaLayout->addWidget(m_southLineEdit,       1, 3);
    utmAreaLayout->addWidget(new QLabel("East"),    2, 0);
    utmAreaLayout->addWidget(m_eastLineEdit,        2, 1);
    utmAreaLayout->addWidget(new QLabel("West"),    2, 2);
    utmAreaLayout->addWidget(m_westLineEdit,        2, 3);
    utmAreaGropBox->setLayout(utmAreaLayout);


    // Well types

    m_importSurveyCheckBox = new QCheckBox("Survey");
    m_importSurveyCheckBox->setChecked(true);
    m_importPlansCheckBox = new QCheckBox("Plans");
    m_importPlansCheckBox->setChecked(true);

    QGroupBox* wellTypeGropBox = new QGroupBox("Include well types");
    QHBoxLayout* wellTypeLayout = new QHBoxLayout;
    wellTypeLayout->addWidget(m_importSurveyCheckBox);
    wellTypeLayout->addWidget(m_importPlansCheckBox);
    wellTypeGropBox->setLayout(wellTypeLayout);



    m_downloadWellPathsButton = new QPushButton(tr("Get well paths"));
    m_downloadWellPathsButton->setDefault(true);

    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->addButton(m_downloadFieldsButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_downloadWellPathsButton, QDialogButtonBox::ActionRole);

    QDialogButtonBox* buttonBox1 = new QDialogButtonBox;
    buttonBox1->addButton(QDialogButtonBox::Cancel);
    buttonBox1->addButton("Import well paths", QDialogButtonBox::AcceptRole);

    connect(buttonBox1, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox1, SIGNAL(rejected()), this, SLOT(reject()));

    m_progressDialog = new QProgressDialog(this);

    connect(m_urlLineEdit, SIGNAL(textChanged(QString)),
        this, SLOT(refreshButtonStatus()));

    connect(&m_networkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
        this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(&m_networkAccessManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
        this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif

    connect(m_progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    connect(m_downloadWellPathsButton, SIGNAL(clicked()), this, SLOT(downloadWellPaths()));

    QVBoxLayout *topLayout1 = new QVBoxLayout;
    QVBoxLayout *topLayout2 = new QVBoxLayout;
    topLayout1->addWidget(m_urlSsiHubLabel);
    topLayout1->addWidget(m_urlSsiHubLineEdit);
    topLayout2->addWidget(m_urlLabel);
    topLayout2->addWidget(m_urlLineEdit);
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addLayout(topLayout1);
    topLayout->addLayout(topLayout2);


    QHBoxLayout *ssihubLayout = new QHBoxLayout;
    ssihubLayout->addWidget(m_fieldListView);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(ssihubLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(utmAreaGropBox);
    mainLayout->addWidget(wellTypeGropBox);
    mainLayout->addWidget(m_buttonBox);
    mainLayout->addWidget(m_wellPathsView);
    mainLayout->addWidget(buttonBox1);
    setLayout(mainLayout);

    setWindowTitle(tr("Import Well Paths"));
    m_urlLineEdit->setFocus();

    refreshButtonStatus();

    resize(600, 400);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::startRequest(QUrl url)
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
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::cancelDownload()
{
    m_statusLabel->setText(tr("Download canceled."));
    m_httpRequestAborted = true;
    m_reply->abort();

    refreshButtonStatus();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::httpFinished()
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
                int currentRowCount = m_wellPathsModel->rowCount();
                m_wellPathsModel->setRowCount(m_wellPathsModel->rowCount() + 1);

                QModelIndex miName = m_wellPathsModel->index(currentRowCount, 0);
                m_wellPathsModel->setData(miName, wellPathName);

                QModelIndex miFileName = m_wellPathsModel->index(currentRowCount, 1);
                m_wellPathsModel->setData(miFileName, singleWellPathFilePath);


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
    }


    refreshButtonStatus();

    m_reply->deleteLater();
    m_reply = 0;
    delete m_file;
    m_file = 0;

    if (m_currentDownloadState == DOWNLOAD_WELLS)
    {
        QStringList survey;
        QStringList plans;

        getWellPathLinks(&survey, &plans);

        m_currentDownloadState = DOWNLOAD_UNDEFINED;

        issueDownloadOfWellPaths(survey, plans);
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
void FetchWellPathsDialog::httpReadyRead()
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
void FetchWellPathsDialog::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (m_httpRequestAborted)
        return;

    m_progressDialog->setMaximum(totalBytes);
    m_progressDialog->setValue(bytesRead);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::refreshButtonStatus()
{
    if (m_fieldListView->selectionModel()->selectedIndexes().size() > 0)
    {
        m_downloadWellPathsButton->setEnabled(true);
    }
    else
    {
        m_downloadWellPathsButton->setEnabled(false);
    }

    m_downloadFieldsButton->setEnabled(!m_urlSsiHubLineEdit->text().isEmpty());

    bool enableUtmEditors = m_filterWellsByUtmArea->isChecked();

    m_northLineEdit->setEnabled(enableUtmEditors);
    m_southLineEdit->setEnabled(enableUtmEditors);
    m_eastLineEdit->setEnabled(enableUtmEditors);
    m_westLineEdit->setEnabled(enableUtmEditors);



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::slotAuthenticationRequired(QNetworkReply*,QAuthenticator *authenticator)
{
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
}

#ifndef QT_NO_OPENSSL
void FetchWellPathsDialog::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
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
void FetchWellPathsDialog::setUrl(const QString& httpAddress)
{
    m_urlLineEdit->setText(httpAddress);

    m_url = httpAddress;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::downloadWellPaths()
{
    QString fileName = jsonWellsFilePath();
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);

        m_wellPathsModel->clear();
        m_wellPathsModel->setColumnCount(2);
    }

    m_currentDownloadState = DOWNLOAD_WELLS;

    QModelIndex mi = m_fieldListView->currentIndex();
    QString fieldName = m_fieldModel->data(mi, Qt::DisplayRole).toString();

    QString completeUrlText = m_urlSsiHubLineEdit->text() + "/resinsight/projects/" + fieldName;

    if (m_filterWellsByUtmArea->isChecked())
    {
        completeUrlText += "/wellsInArea";

        int north = m_northLineEdit->text().toInt();
        int south = m_southLineEdit->text().toInt();
        int east = m_eastLineEdit->text().toInt();
        int west = m_westLineEdit->text().toInt();

        completeUrlText += QString("?north=%1").arg(north);
        completeUrlText += QString("&south=%1").arg(south);
        completeUrlText += QString("&east=%1").arg(east);
        completeUrlText += QString("&west=%1").arg(west);
        completeUrlText += QString("&utmZone=32S&format=json");
    }
    else
    {
         completeUrlText += "/wells";
    }
    
    issueHttpRequestToFile(completeUrlText, fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::downloadFields()
{
    QString wellFileName = jsonWellsFilePath();
    if (QFile::exists(wellFileName))
    {
        QFile::remove(wellFileName);

        m_wellPathsModel->clear();
        m_wellPathsModel->setColumnCount(2);
    }

    QString completeUrlText = m_urlSsiHubLineEdit->text() + "/resinsight/projects";
    QString destinationFileName = jsonFieldsFilePath();

    m_currentDownloadState = DOWNLOAD_FIELDS;
    issueHttpRequestToFile(completeUrlText, destinationFileName);

    return;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::setDestinationFolder(const QString& folder)
{
    m_destinationFolder = folder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::setSsiHubUrl(const QString& httpAddress)
{
    QString validAddress(httpAddress);
    if (validAddress.endsWith('/'))
    {
        validAddress = validAddress.left(validAddress.size() - 1);
    }

    m_urlSsiHubLineEdit->setText(validAddress);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    QModelIndexList idxList = selected.indexes();

    if (idxList.size() == 1)
    {
        QString fieldName = m_fieldModel->data(idxList[0], Qt::DisplayRole).toString();

        QString completeUrlText = m_urlSsiHubLineEdit->text() + "/resinsight/projects/" + fieldName;
        setUrl(completeUrlText);
    }
    else
    {
        setUrl("");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::updateFieldsModel()
{
    QString fileName = jsonFieldsFilePath();

    if (QFile::exists(fileName))
    {
        JsonReader jsonReader;
        QMap<QString, QVariant> jsonMap = jsonReader.decodeFile(fileName);

        QStringList fieldNames;
        QMapIterator<QString, QVariant> it(jsonMap);
        while (it.hasNext())
        {
            it.next();

            QString key = it.key();
            if (key[0].isDigit())
            {
                QMap<QString, QVariant> fieldMap = it.value().toMap();

                fieldNames.push_back(fieldMap["name"].toString());
            }
        }

        m_fieldModel->setStringList(fieldNames);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString FetchWellPathsDialog::jsonFieldsFilePath()
{
    return m_destinationFolder + "/fields.json";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString FetchWellPathsDialog::jsonWellsFilePath()
{
    return m_destinationFolder + "/wellpaths.json";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString FetchWellPathsDialog::jsonWellsInArea()
{
    return m_destinationFolder + "/wellsInArea.json";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList FetchWellPathsDialog::downloadedJsonWellPathFiles()
{
    QStringList fileNames;

    for (int i = 0; i < m_wellPathsModel->rowCount(); i++)
    {
        QModelIndex mi = m_wellPathsModel->index(i, 1);
        fileNames.push_back(m_wellPathsModel->data(mi).toString());
    }

    return fileNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::showEvent(QShowEvent* event)
{
    refreshButtonStatus();

    QDialog::showEvent(event);

}

//--------------------------------------------------------------------------------------------------
/// Search for string, and find the associated value inside the next quoted string
//  text content : "A" : "B"
//  A search for key "A" returns B
//--------------------------------------------------------------------------------------------------
QString FetchWellPathsDialog::getValue(const QString& key, const QString& stringContent)
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
void FetchWellPathsDialog::setRegion(int north, int south, int east, int west)
{
    m_northLineEdit->setText(QString::number(north));
    m_southLineEdit->setText(QString::number(south));
    m_eastLineEdit->setText(QString::number(east));
    m_westLineEdit->setText(QString::number(west));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::issueHttpRequestToFile(QString completeUrlText, QString destinationFileName)
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
void FetchWellPathsDialog::getWellPathLinks(QStringList* surveyLinks, QStringList* planLinks)
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
void FetchWellPathsDialog::issueDownloadOfWellPaths(const QStringList& surveyLinks, const QStringList& planLinks)
{
    m_wellPathRequestQueue.clear();
    
    if (m_importSurveyCheckBox->isChecked())
    {
        m_wellPathRequestQueue += surveyLinks;
    }

    if (m_importPlansCheckBox->isChecked())
    {
        m_wellPathRequestQueue += planLinks;
    }

    checkDownloadQueueAndIssueRequests();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::checkDownloadQueueAndIssueRequests()
{
    if (m_wellPathRequestQueue.size() > 0)
    {
        QString link = m_wellPathRequestQueue[0];
        m_wellPathRequestQueue.pop_front();

        QString completeUrlText = m_urlSsiHubLineEdit->text() + link;

        QUuid guid = QUuid::createUuid();
        QString singleWellPathFilePath = m_destinationFolder + QString("/wellpath_%1.json").arg(guid);

        m_currentDownloadState = DOWNLOAD_WELL_PATH;
        issueHttpRequestToFile(completeUrlText, singleWellPathFilePath);
    }
}



} // namespace ssihub



