/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicFileHierarchyDialog.h"
#include "ExportCommands/RicSnapshotViewToClipboardFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "ExportCommands/RicSnapshotFilenameGenerator.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "Rim3dOverlayInfoConfig.h"

#include "RiuMainPlotWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuTools.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QGroupBox>

#include <vector>
#include <time.h>
#include <thread>


#define FIND_BUTTON_FIND_TEXT       "Find"
#define FIND_BUTTON_CANCEL_TEXT     "Cancel"
#define NO_FILES_FOUND_TEXT         "No files found"
#define WORKING_TEXT_1              "Working ."
#define WORKING_TEXT_2              "Working .."
#define WORKING_TEXT_3              "Working ..."

//--------------------------------------------------------------------------------------------------
/// Internal variables
//--------------------------------------------------------------------------------------------------
static QString separator = QString(QDir::separator());

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
static QStringList prefixStrings(const QStringList& strings, const QString& prefix);
static QString     relativePath(const QString& rootDir, const QString& dir);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicFileHierarchyDialog::RicFileHierarchyDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    // Create widgets
    m_rootDirLabel      = new QLabel();
    m_rootDir           = new QLineEdit();
    m_browseButton      = new QPushButton();
    m_pathFilterLabel   = new QLabel();
    m_pathFilter        = new QLineEdit();
    m_fileFilterLabel   = new QLabel();
    m_fileFilter        = new QLineEdit();
    m_fileExtension     = new QLabel();
    m_effectiveFilterLabel = new QLabel();
    m_effectiveFilter   = new QLabel();
    m_fileListLabel     = new QLabel();
    m_fileList          = new QTextEdit();
    m_findOrCancelButton = new QPushButton();

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect to signals
    connect(m_rootDir, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_pathFilter, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_fileFilter, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));

    connect(m_findOrCancelButton, SIGNAL(clicked()), this, SLOT(slotFindOrCancelButtonClicked()));
    connect(m_buttons, SIGNAL(accepted()), this, SLOT(slotDialogOkClicked()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(slotDialogCancelClicked()));
    connect(m_browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseButtonClicked()));

    // Set widget properties
    m_rootDirLabel->setText("Root folder");
    m_pathFilterLabel->setText("Path pattern");
    m_fileFilterLabel->setText("File pattern");
    m_effectiveFilterLabel->setText("Effective filter");
    m_effectiveFilter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_fileListLabel->setText("Files found");
    m_fileList->setLineWrapMode(QTextEdit::NoWrap);
    m_fileListLabel->setVisible(false);
    m_fileList->setVisible(false);
    m_browseButton->setText("...");
    m_browseButton->setFixedWidth(25);
    m_findOrCancelButton->setText("Find");
    m_findOrCancelButton->setFixedWidth(75);

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QGroupBox* inputGroup = new QGroupBox("Filter");
    QGridLayout* inputGridLayout = new QGridLayout();
    inputGridLayout->addWidget(m_rootDirLabel, 0, 0);
    inputGridLayout->addWidget(m_rootDir, 0, 1);
    inputGridLayout->addWidget(m_browseButton, 0, 2);
    inputGridLayout->addWidget(m_pathFilterLabel, 1, 0);
    inputGridLayout->addWidget(m_pathFilter, 1, 1);
    inputGridLayout->addWidget(m_fileFilterLabel, 2, 0);
    inputGridLayout->addWidget(m_fileFilter, 2, 1);
    inputGridLayout->addWidget(m_fileExtension, 2, 2);
    inputGroup->setLayout(inputGridLayout);

    QGroupBox* outputGroup = new QGroupBox("Files");
    QGridLayout* outputGridLayout = new QGridLayout();
    outputGridLayout->addWidget(m_effectiveFilterLabel, 0, 0);
    outputGridLayout->addWidget(m_effectiveFilter, 0, 1);
    outputGridLayout->addWidget(m_findOrCancelButton, 0, 2);
    outputGridLayout->addWidget(m_fileListLabel, 1, 0);
    outputGridLayout->addWidget(m_fileList, 1, 1, 1, 2);
    outputGroup->setLayout(outputGridLayout);

    dialogLayout->addWidget(inputGroup);
    dialogLayout->addWidget(outputGroup);
    dialogLayout->addWidget(m_buttons);

    setLayout(dialogLayout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicFileHierarchyDialog::~RicFileHierarchyDialog()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::files() const
{
    return m_files;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFileHierarchyDialog::rootDir() const
{
    QString rootDir = m_rootDir->text();
    if (!rootDir.endsWith(separator))
    {
        rootDir.append(separator);
    }
    return QDir::toNativeSeparators(rootDir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFileHierarchyDialog::pathFilter() const
{
    return QDir::toNativeSeparators(m_pathFilter->text());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFileHierarchyDialog::fileNameFilter() const
{
    return m_fileFilter->text();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::fileExtensions() const
{
    return m_fileExtension->text().split("|");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicFileHierarchyDialog::cancelPressed() const
{
    return m_cancelPressed;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::appendToFileList(const QString& fileName)
{
    QString text = m_fileList->toPlainText();
    if (text.startsWith(WORKING_TEXT_1)) clearFileList();

    m_fileList->append(fileName);
    QApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::clearFileList()
{
    m_files.clear();
    m_fileList->setText("");
    setOkButtonEnabled(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::updateStatus()
{
    static time_t lastStatusUpdate = 0;
    time_t now = time(0);

    if (now == lastStatusUpdate) return;
    lastStatusUpdate = now;

    QString currStatus = m_fileList->toPlainText();
    if      (currStatus == "") m_fileList->setText(WORKING_TEXT_1);
    else if (currStatus == WORKING_TEXT_1) m_fileList->setText(WORKING_TEXT_2);
    else if (currStatus == WORKING_TEXT_2) m_fileList->setText(WORKING_TEXT_3);
    else if (currStatus == WORKING_TEXT_3) m_fileList->setText(WORKING_TEXT_1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicFileHierarchyDialogResult RicFileHierarchyDialog::getOpenFileNames(QWidget *parent /*= 0*/, 
                                                                     const QString &caption /*= QString()*/, 
                                                                     const QString &dir /*= QString()*/,
                                                                     const QString &pathFilter /*= QString()*/,
                                                                     const QString &fileNameFilter /*= QString()*/,
                                                                     const QStringList &fileExtensions /*= QStringList()*/)
{
    QStringList             files;
    RicFileHierarchyDialog  dialog(parent);
    
    dialog.setWindowTitle(caption);

    dialog.m_rootDir->setText(QDir::toNativeSeparators(dir));
    dialog.m_pathFilter->setText(pathFilter);
    dialog.m_fileFilter->setText(fileNameFilter);
    dialog.m_fileExtension->setText(prefixStrings(fileExtensions, ".").join(" | "));
 
    dialog.updateEffectiveFilter();
    dialog.m_fileList->setText("");
    dialog.setOkButtonEnabled(false);

    dialog.resize(600, 150);
    dialog.exec();

    return RicFileHierarchyDialogResult(dialog.result() == QDialog::Accepted, dialog.files(), dialog.rootDir(), dialog.pathFilter(), dialog.fileNameFilter());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::findMatchingFiles()
{
    QStringList dirs = buildDirectoryListRecursive(rootDir());

    return findFilesInDirs(dirs);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::buildDirectoryListRecursive(const QString& currentDir)
{
    QStringList allDirs;

    if (cancelPressed()) return allDirs;

    updateStatus();

    QDir qdir(currentDir);
    QStringList subDirs = qdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (pathFilterMatch(currentDir))
    {
        allDirs.push_back(currentDir);
    }
    else
    {
        // If there is no match and filter string does not start with a wildcard, there is no need to enter sub directories
        if (!pathFilter().startsWith("*") && !relativePath(rootDir(), currentDir).isEmpty())
        {
            return QStringList();
        }
    }

    for (QString subDir : subDirs)
    {
        QApplication::processEvents();
        allDirs += buildDirectoryListRecursive(QDir::toNativeSeparators(qdir.absoluteFilePath(subDir)));
    }
    return cancelPressed() ? QStringList() : allDirs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::findFilesInDirs(const QStringList& dirs)
{
    QStringList allFiles;
    QStringList filters = createNameFilterList(fileNameFilter(), fileExtensions());

    for (const auto& dir : dirs)
    {
        QDir qdir(dir);
        QStringList files = qdir.entryList(filters, QDir::Files);

        updateStatus();

        for (QString file : files)
        {
            QString absFilePath = QDir::toNativeSeparators(qdir.absoluteFilePath(file));
            allFiles.append(absFilePath);
            appendToFileList(absFilePath);
        }
    }
    return allFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::createNameFilterList(const QString &fileNameFilter, const QStringList &fileExtensions)
{
    QStringList nameFilter;
    QString effectiveFileNameFilter = !fileNameFilter.isEmpty() ? fileNameFilter : "*";

    if (fileExtensions.size() == 0)
    {
        nameFilter.append(effectiveFileNameFilter);
    }
    else
    {
        for (QString fileExtension : fileExtensions)
        {
            nameFilter.append(effectiveFileNameFilter + fileExtension);
        }
    }
    return nameFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicFileHierarchyDialog::pathFilterMatch(const QString& dir)
{
    QRegExp regexp(pathFilter(), Qt::CaseInsensitive, QRegExp::Wildcard);
    QString relPath = relativePath(rootDir(), dir);

    return regexp.exactMatch(relPath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::updateEffectiveFilter()
{
    QString effFilter = QString("%1/%2/%3%4")
        .arg(m_rootDir->text())
        .arg(m_pathFilter->text())
        .arg(m_fileFilter->text())
        .arg(m_fileExtension->text());

    QString native(QDir::toNativeSeparators(effFilter));

    // Remove duplicate separators
    int len;
    do 
    {
        len = native.size();
        native.replace(separator + separator, separator);
    } while (native.size() != len);

    m_effectiveFilter->setText(native);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::setOkButtonEnabled(bool enabled)
{
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotFilterChanged(const QString& text)
{
    clearFileList();
    updateEffectiveFilter();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotFindOrCancelButtonClicked()
{
    if (m_findOrCancelButton->text() == FIND_BUTTON_FIND_TEXT)
    {
        clearFileList();

        if (!m_fileList->isVisible())
        {
            m_fileListLabel->setVisible(true);
            m_fileList->setVisible(true);
            resize(600, 350);
        }

        m_findOrCancelButton->setText(FIND_BUTTON_CANCEL_TEXT);

        m_cancelPressed = false;
        m_files = findMatchingFiles();

        m_findOrCancelButton->setText(FIND_BUTTON_FIND_TEXT);

        if (m_cancelPressed)
        {
            clearFileList();
        }
        else if(m_files.isEmpty())
        {
            m_fileList->setText(NO_FILES_FOUND_TEXT);
        }

        setOkButtonEnabled(!m_files.isEmpty());
    }
    else
    {
        m_cancelPressed = true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotDialogOkClicked()
{
    accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotDialogCancelClicked()
{
    m_files = QStringList();
    m_cancelPressed = true;
    reject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotBrowseButtonClicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Select root folder", m_rootDir->text());
    m_rootDir->setText(folder);
}


//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList prefixStrings(const QStringList& strings, const QString& prefix)
{
    QStringList prefixedStrings;
    for (auto string : strings)
    {
        if (!string.startsWith(prefix))
        {
            prefixedStrings.append(prefix + string);
        }
        else
        {
            prefixedStrings.append(string);
        }
    }
    return prefixedStrings;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString relativePath(const QString& rootDir, const QString& dir)
{
    if (dir.startsWith(rootDir))
    {
        QString relPath = dir;
        relPath.remove(0, rootDir.size());

        if (relPath.startsWith("/") || relPath.startsWith("\\")) relPath.remove(0, 1);
        return relPath;
    }
    else
    {
        return dir;
    }
}
