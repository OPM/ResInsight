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

    m_downloadWellPathsButton = new QPushButton(tr("Get well paths"));
    m_downloadWellPathsButton->setDefault(true);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(m_downloadFieldsButton, QDialogButtonBox::ActionRole);
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
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(m_wellPathsView);
    mainLayout->addWidget(buttonBox1);
    setLayout(mainLayout);

    setWindowTitle(tr("Import Well Paths"));
    urlLineEdit->setFocus();

    refreshButtonStatus();

    resize(600, 400);
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
    QModelIndex mi = m_fieldListView->currentIndex();
    QString fieldName = m_fieldModel->data(mi, Qt::DisplayRole).toString();

    QString completeUrlText = urlSsiHubLineEdit->text() + "/resinsight/projects/" + fieldName + "/wellpaths";
    setUrl(completeUrlText);

    QString fileName = jsonWellPathsFilePath();
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);

        m_wellPathsModel->clear();
    }

    m_file = new QFile(fileName);
    if (!m_file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"),
            tr("Unable to save the file %1: %2.")
            .arg(fileName).arg(m_file->errorString()));
        delete m_file;
        m_file = 0;
        return;
    }

    progressDialog->setWindowTitle(tr("HTTP"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));
    m_downloadWellPathsButton->setEnabled(false);

    // schedule the request
    httpRequestAborted = false;
    startRequest(url);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FetchWellPathsDialog::downloadFields()
{
    QString completeUrlText = urlSsiHubLineEdit->text() + "/resinsight/projects";
    setUrl(completeUrlText);

    // Delete already wellPathList
    {
        QString wellFileName = jsonWellPathsFilePath();
        if (QFile::exists(wellFileName))
        {
            QFile::remove(wellFileName);

            m_wellPathsModel->clear();
        }
    }

    QString fieldsFileName = jsonFieldsFilePath();
    m_file = new QFile(fieldsFileName);
    if (!m_file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"),
            tr("Unable to save the file %1: %2.")
            .arg(fieldsFileName).arg(m_file->errorString()));
        delete m_file;
        m_file = 0;
        return;
    }

    progressDialog->setWindowTitle(tr("HTTP"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(fieldsFileName));
    m_downloadFieldsButton->setEnabled(false);

    // schedule the request
    httpRequestAborted = false;
    startRequest(url);
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

            QUuid guid = QUuid::createUuid();

            QString singleWellPathFilePath = m_destinationFolder + QString("/wellpath_%1.json").arg(guid);
            QFile outputFile(singleWellPathFilePath);
            if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&outputFile);
                out << singleWellPath;
            }
            outputFile.close();

            QString key = "\"name\"";
            QString wellPathName = getValue(key, singleWellPath);
            if (wellPathFileNames.indexOf(singleWellPathFilePath) < 0)
            {
                wellPathNames.push_back(wellPathName);
                wellPathFileNames.push_back(singleWellPathFilePath);
            }
        }

        // Find next starting brace
        pos = fileContent.indexOf('{', pos);
    }

    m_wellPathsModel->clear();
    m_wellPathsModel->setRowCount(wellPathFileNames.size());
    m_wellPathsModel->setColumnCount(2);
    //m_wellPathsView->hideColumn(1);

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
    int pos = wellPathFileContent.indexOf(key);
    if (pos >=0)
    {
        int valueStartPos = wellPathFileContent.indexOf("\"", pos + key.size());
        int valueEndPos = wellPathFileContent.indexOf("\"", valueStartPos + 1);

        if (valueStartPos >= 0 && valueEndPos > valueStartPos)
        {
            return wellPathFileContent.mid(valueStartPos + 1, valueEndPos - valueStartPos - 1);
        }
    }

    return QString();
}



} // namespace ssihub



