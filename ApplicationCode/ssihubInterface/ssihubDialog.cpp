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
    urlSsiHubLineEdit = new QLineEdit;
    urlSsiHubLabel = new QLabel(tr("SSIHUB address:"));
    urlSsiHubLabel->setBuddy(urlSsiHubLineEdit);

    urlLineEdit = new QLineEdit;
    urlLabel = new QLabel(tr("SSIHUB complete request:"));
    urlLabel->setBuddy(urlLineEdit);

    statusLabel = new QLabel(tr("Status : idle"));
    statusLabel->setWordWrap(true);

    m_downloadFieldsButton = new QPushButton(tr("Get fields"));
    connect(m_downloadFieldsButton, SIGNAL(clicked()), this, SLOT(downloadFields()));

    // Fields data model and view
    m_fieldListView = new QListView(this);
    m_fieldModel = new QStringListModel;
    m_fieldListView->setModel(m_fieldModel);
    connect(m_fieldListView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection& )), this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection& )));

    // Well paths data model and view
    m_wellPathsView = new QListView(this);
    m_wellPathsModel = new QStandardItemModel;
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

    m_downloadWellPathsButton = new QPushButton(tr("Get well paths"));
    m_downloadWellPathsButton->setDefault(true);

    m_downloadFilterInfo = new QPushButton(tr("Get UTM filter info"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(m_downloadFieldsButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_downloadFilterInfo, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_downloadWellPathsButton, QDialogButtonBox::ActionRole);

    QDialogButtonBox* buttonBox1 = new QDialogButtonBox;
    buttonBox1->addButton(QDialogButtonBox::Cancel);
    buttonBox1->addButton("Import well paths", QDialogButtonBox::AcceptRole);

    connect(buttonBox1, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox1, SIGNAL(rejected()), this, SLOT(reject()));

    progressDialog = new QProgressDialog(this);

    connect(urlLineEdit, SIGNAL(textChanged(QString)),
        this, SLOT(refreshButtonStatus()));

    connect(&qnam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
        this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(&qnam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
        this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif

    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    connect(m_downloadWellPathsButton, SIGNAL(clicked()), this, SLOT(downloadWellPaths()));
    connect(m_downloadFilterInfo, SIGNAL(clicked()), this, SLOT(downloadUtmFilterInfo()));

    QVBoxLayout *topLayout1 = new QVBoxLayout;
    QVBoxLayout *topLayout2 = new QVBoxLayout;
    topLayout1->addWidget(urlSsiHubLabel);
    topLayout1->addWidget(urlSsiHubLineEdit);
    topLayout2->addWidget(urlLabel);
    topLayout2->addWidget(urlLineEdit);
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addLayout(topLayout1);
    topLayout->addLayout(topLayout2);


    QHBoxLayout *ssihubLayout = new QHBoxLayout;
    ssihubLayout->addWidget(m_fieldListView);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(ssihubLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(utmAreaGropBox);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(m_wellPathsView);
    mainLayout->addWidget(buttonBox1);
    setLayout(mainLayout);

    setWindowTitle(tr("Import Well Paths"));
    urlLineEdit->setFocus();

    refreshButtonStatus();

    resize(600, 400);

    m_north = HUGE_VAL;
    m_south = HUGE_VAL;
    m_east = HUGE_VAL;
    m_west = HUGE_VAL;

    m_downloadFilterInfo->setEnabled(m_filterWellsByUtmArea->isChecked());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::startRequest(QUrl url)
{
    reply = qnam.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()),
        this, SLOT(httpFinished()));
    connect(reply, SIGNAL(readyRead()),
        this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
        this, SLOT(updateDataReadProgress(qint64,qint64)));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::cancelDownload()
{
    statusLabel->setText(tr("Download canceled."));
    httpRequestAborted = true;
    reply->abort();

    refreshButtonStatus();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::httpFinished()
{
    if (httpRequestAborted) {
        if (m_file) {
            m_file->close();
            m_file->remove();
            delete m_file;
            m_file = 0;
        }
        reply->deleteLater();
        progressDialog->hide();
        return;
    }

    progressDialog->hide();
    m_file->flush();
    m_file->close();


    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        m_file->remove();
        QMessageBox::information(this, tr("HTTP"),
            tr("Download failed: %1.")
            .arg(reply->errorString()));
    } else if (!redirectionTarget.isNull()) {        
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(this, tr("HTTP"),
            tr("Redirect to %1 ?").arg(newUrl.toString()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                url = newUrl;
                reply->deleteLater();
                m_file->open(QIODevice::WriteOnly);
                m_file->resize(0);
                startRequest(url);
                return;
        }
    } else {
        statusLabel->setText(tr("Downloaded data to %1.").arg(m_destinationFolder));
    }

    updateFromDownloadedFiles();

    refreshButtonStatus();

    reply->deleteLater();
    reply = 0;
    delete m_file;
    m_file = 0;
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
        m_file->write(reply->readAll());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
        return;

    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(bytesRead);
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

    m_downloadFieldsButton->setEnabled(!urlSsiHubLineEdit->text().isEmpty());


    if (m_filterWellsByUtmArea->isChecked() &&
        m_fieldListView->selectionModel()->selectedIndexes().size() > 0)
    {
        m_downloadFilterInfo->setEnabled(true);
    }
    else
    {
        m_downloadFilterInfo->setEnabled(false);
    }
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
    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm()).arg(url.host()));

    // Did the URL have information? Fill the UI
    // This is only relevant if the URL-supplied credentials were wrong
    ui.userEdit->setText(url.userName());
    ui.passwordEdit->setText(url.password());

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
            reply->ignoreSslErrors();
    }
}



#endif



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::setUrl(const QString& httpAddress)
{
    urlLineEdit->setText(httpAddress);

    url = httpAddress;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::downloadWellPaths()
{
    QString fileName = jsonWellPathsFilePath();
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);

        m_wellPathsModel->clear();
    }

    QModelIndex mi = m_fieldListView->currentIndex();
    QString fieldName = m_fieldModel->data(mi, Qt::DisplayRole).toString();

    QString completeUrlText = urlSsiHubLineEdit->text() + "/resinsight/projects/" + fieldName + "/wellpaths";
    
    issueHttpRequestToFile(completeUrlText, fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::downloadFields()
{
    QString wellFileName = jsonWellPathsFilePath();
    if (QFile::exists(wellFileName))
    {
        QFile::remove(wellFileName);

        m_wellPathsModel->clear();
    }

    QString completeUrlText = urlSsiHubLineEdit->text() + "/resinsight/projects";
    QString destinationFileName = jsonFieldsFilePath();

    issueHttpRequestToFile(completeUrlText, destinationFileName);

    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::downloadUtmFilterInfo()
{
    QString fileName = jsonWellsByArea();
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
    }

    QModelIndex mi = m_fieldListView->currentIndex();
    QString fieldName = m_fieldModel->data(mi, Qt::DisplayRole).toString();

    QString completeUrlText = urlSsiHubLineEdit->text() + "/resinsight/projects/" + fieldName + "/wellsByArea";

    int north = m_northLineEdit->text().toInt();
    int south = m_southLineEdit->text().toInt();
    int east = m_eastLineEdit->text().toInt();
    int west = m_westLineEdit->text().toInt();

    completeUrlText += QString("?north=%1").arg(north);
    completeUrlText += QString("&south=%1").arg(south);
    completeUrlText += QString("&east=%1").arg(east);
    completeUrlText += QString("&west=%1").arg(west);
    completeUrlText += QString("&utmZone=32S&format=json");

    issueHttpRequestToFile(completeUrlText, fileName);
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

    urlSsiHubLineEdit->setText(validAddress);
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

        QString completeUrlText = urlSsiHubLineEdit->text() + "/resinsight/projects/" + fieldName;
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
QString FetchWellPathsDialog::jsonWellPathsFilePath()
{
    return m_destinationFolder + "/wellpaths.json";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString FetchWellPathsDialog::jsonWellsByArea()
{
    return m_destinationFolder + "/wellsbyarea.json";
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::updateFromDownloadedFiles()
{
    updateFieldsModel();
    extractAndUpdateSingleWellFiles();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::extractAndUpdateSingleWellFiles()
{
    QStringList filtereEntities;

    if (m_filterWellsByUtmArea->isChecked())
    {
        filtereEntities = filteredWellEntities();
    }

    QString filename = jsonWellPathsFilePath();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    QString fileContent = file.readAll();

    QStringList wellPathNames;
    QStringList wellPathFileNames;

    int pos = 0;
    pos = fileContent.indexOf('{', pos);
    while (pos >= 0)
    {
        int singleWellPathStart = pos;

        // Move to first char after starting brace
        pos++;

        int startBracket = 1;
        while (startBracket > 0 && pos < fileContent.size())
        {
            if (fileContent.at(pos) == '{')
            {
                startBracket++;
            }
            else if (fileContent.at(pos) == '}')
            {
                startBracket--;
            }

            pos++;
        }

        // Write out a single well path
        {
            QString singleWellPath = fileContent.mid(singleWellPathStart, pos - singleWellPathStart);
            QString singleWellPathName = getValue("well", singleWellPath);

            if (m_filterWellsByUtmArea->isChecked() && filtereEntities.indexOf(singleWellPathName) < 0)
            {
                // Outside UTM area
            }
            else
            {
                QUuid guid = QUuid::createUuid();

                QString singleWellPathFilePath = m_destinationFolder + QString("/wellpath_%1.json").arg(guid);
                QFile outputFile(singleWellPathFilePath);
                if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream out(&outputFile);
                    out << singleWellPath;
                }
                outputFile.close();

                QString wellPathName = getValue("name", singleWellPath);
                if (wellPathFileNames.indexOf(singleWellPathFilePath) < 0)
                {
                    wellPathNames.push_back(wellPathName);
                    wellPathFileNames.push_back(singleWellPathFilePath);
                }
            }
        }

        // Find next starting brace
        pos = fileContent.indexOf('{', pos);
    }

    m_wellPathsModel->clear();
    m_wellPathsModel->setRowCount(wellPathFileNames.size());
    m_wellPathsModel->setColumnCount(2);

    for (int i = 0; i < wellPathFileNames.size(); i++)
    {
        QModelIndex miName = m_wellPathsModel->index(i, 0);
        m_wellPathsModel->setData(miName, wellPathNames[i]);

        QModelIndex miFileName = m_wellPathsModel->index(i, 1);
        m_wellPathsModel->setData(miFileName, wellPathFileNames[i]);
    }
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
QString FetchWellPathsDialog::getValue(const QString& key, const QString& wellPathFileContent)
{
    QString quotedKey = "\"" + key + "\"";

    int pos = wellPathFileContent.indexOf(quotedKey);
    if (pos >=0)
    {
        int valueStartPos = wellPathFileContent.indexOf("\"", pos + quotedKey.size());
        int valueEndPos = wellPathFileContent.indexOf("\"", valueStartPos + 1);

        if (valueStartPos >= 0 && valueEndPos > valueStartPos)
        {
            return wellPathFileContent.mid(valueStartPos + 1, valueEndPos - valueStartPos - 1);
        }
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::setRegion(int north, int south, int east, int west)
{
    m_north = north;
    m_south = south;
    m_east = east;
    m_west = west;

    m_northLineEdit->setText(QString::number(static_cast<int>(m_north)));
    m_southLineEdit->setText(QString::number(static_cast<int>(m_south)));
    m_eastLineEdit->setText(QString::number(static_cast<int>(m_east)));
    m_westLineEdit->setText(QString::number(static_cast<int>(m_west)));
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

    progressDialog->setWindowTitle(tr("HTTP"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(destinationFileName));

    // schedule the request
    httpRequestAborted = false;
    startRequest(url);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList FetchWellPathsDialog::filteredWellEntities()
{
    QStringList entities;

    QString filename = jsonWellsByArea();
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
                QString entity = linkMap["entity"].toString();
                entities.push_back(entity);
            }
        }

        /*
        QList<QVariant> slotList = jsonMap["slot"].toList();
        foreach (QVariant slot, slotList)
        {
            QMap<QString, QVariant> slotMap = slot.toMap();
            QString entity = slotMap["entity"].toString();
            entities.push_back(entity);
        }
        */
    }

    return entities;
}



} // namespace ssihub



