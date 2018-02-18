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
#include <QListWidget>
#include <QAbstractItemView>
#include <QMenu>

#include <vector>
#include <time.h>
#include <thread>

#define DEFAULT_DIALOG_WIDTH        750
#define DEFAULT_DIALOG_INIT_HEIGHT  150
#define DEFAULT_DIALOG_FIND_HEIGHT  350
#define FIND_BUTTON_FIND_TEXT       "Find"
#define FIND_BUTTON_CANCEL_TEXT     "Cancel"
#define NO_FILES_FOUND_TEXT         "No files found"
#define WORKING_TEXT_1              "Working ."
#define WORKING_TEXT_2              "Working .."
#define WORKING_TEXT_3              "Working ..."

//--------------------------------------------------------------------------------------------------
/// Internal variables
//--------------------------------------------------------------------------------------------------
static QString SEPARATOR = "/";

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
static QString      toInternalSeparator(const QString& path);
static QString&     appendSeparatorIfNo(QString& path);
static QStringList  prefixStrings(const QStringList& strings, const QString& prefix);
static QString      relativePath(const QString& rootDir, const QString& dir);
static bool         equalPaths(const QString& path1, const QString& path2);

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
    m_fileList          = new QListWidget();
    m_findOrCancelButton = new QPushButton();

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect to signals
    connect(m_rootDir, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_pathFilter, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_fileFilter, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_fileList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotFileListCustomMenuRequested(const QPoint&)));

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
    m_fileListLabel->setVisible(false);
    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileList->setVisible(false);
    m_fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_browseButton->setText("...");
    m_browseButton->setFixedWidth(25);
    m_findOrCancelButton->setText(FIND_BUTTON_FIND_TEXT);
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
    dialog.clearFileList();
    dialog.setOkButtonEnabled(false);

    dialog.resize(DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_INIT_HEIGHT);
    dialog.exec();

    return RicFileHierarchyDialogResult(dialog.result() == QDialog::Accepted, dialog.files(), dialog.rootDir(), dialog.pathFilter(), dialog.fileNameFilter());
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
    QString rootDir = toInternalSeparator(m_rootDir->text());
    return appendSeparatorIfNo(rootDir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFileHierarchyDialog::pathFilter() const
{
    return toInternalSeparator(m_pathFilter->text());
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
    QStringList exts = m_fileExtension->text().split("|");
    for (QString& ext : exts)
    {
        ext = ext.trimmed();
    }
    return exts;
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
    if (currentStatus().startsWith(WORKING_TEXT_1)) clearFileList();

    QString itemText = fileName;
    QListWidgetItem* item = new QListWidgetItem(itemText.remove(0, rootDir().size()), m_fileList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::clearFileList()
{
    m_files.clear();
    m_fileList->clear();
    setOkButtonEnabled(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::updateStatus(Status status, bool force)
{
    static time_t lastStatusUpdate = 0;
    time_t now = time(nullptr);

    // If less than one second since last update, then return
    if (!force && now == lastStatusUpdate) return;
    lastStatusUpdate = now;

    QString newStatus;
    if (status == WORKING)
    {
        QString currStatus = currentStatus();
        if (currStatus == "") newStatus = WORKING_TEXT_1;
        else if (currStatus == WORKING_TEXT_1) newStatus = WORKING_TEXT_2;
        else if (currStatus == WORKING_TEXT_2) newStatus = WORKING_TEXT_3;
        else if (currStatus == WORKING_TEXT_3) newStatus = WORKING_TEXT_1;
    }
    else if (status == NO_FILES_FOUND)
    {
        newStatus = NO_FILES_FOUND_TEXT;
    }

    m_fileList->clear();
    new QListWidgetItem(newStatus, m_fileList);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFileHierarchyDialog::currentStatus() const
{
    return m_fileList->item(0) ? m_fileList->item(0)->text() : "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::findMatchingFiles()
{
    if (m_rootDir->text().isEmpty()) return QStringList();

    const QStringList& dirs = buildDirectoryListRecursive(rootDir());
    const QStringList& files = findFilesInDirs(dirs);

    for (const auto& file : files)
    {
        appendToFileList(file);
    }
    return files;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::buildDirectoryListRecursive(const QString& currentDir, int level)
{
    QStringList allDirs;

    if (cancelPressed()) return allDirs;

    QString currPathFilter = pathFilter();
    bool subStringFilter = false;

    // Optimizing for speed by a refined match at first directory level
    if (level == 1)
    {
        QString pathFilter = this->pathFilter();
        if (!pathFilter.startsWith("*"))
        {
            int wildcardIndex = pathFilter.indexOf(QRegExp(QString("[*%1]").arg(SEPARATOR)));
            if (wildcardIndex >= 0)
            {
                currPathFilter = pathFilter.left(wildcardIndex + 1);
                subStringFilter = true;
            }
        }
    }

    QString currRelPath = relativePath(rootDir(), currentDir);
    if (pathFilterMatch(currPathFilter, currRelPath))
    {
        allDirs.push_back(currentDir);
    }
    else if(level == 1 && subStringFilter)
    {
        return QStringList();
    }

    QDir qdir(currentDir);
    QStringList subDirs = qdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (QString subDir : subDirs)
    {
        updateStatus(WORKING);
        QApplication::processEvents();
        allDirs += buildDirectoryListRecursive(qdir.absoluteFilePath(subDir), level + 1);
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

        updateStatus(WORKING);
        QApplication::processEvents();

        for (QString file : files)
        {
            QString absFilePath = qdir.absoluteFilePath(file);
            allFiles.append(absFilePath);
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
bool RicFileHierarchyDialog::pathFilterMatch(const QString& pathFilter, const QString& relPath)
{
    QRegExp regexp(pathFilter, Qt::CaseInsensitive, QRegExp::Wildcard);
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

    QString internalFilter(toInternalSeparator(effFilter));

    // Remove duplicate separators
    int len;
    do
    {
        len = internalFilter.size();
        internalFilter.replace(SEPARATOR + SEPARATOR, SEPARATOR);
    } while (internalFilter.size() != len);

    // Present native separators to the user
    m_effectiveFilter->setText(QDir::toNativeSeparators(internalFilter));
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
void RicFileHierarchyDialog::slotFileListCustomMenuRequested(const QPoint& point)
{
    QMenu menu;
    QPoint globalPoint = point;
    QAction* action;

    action = new QAction("On", this);
    connect(action, SIGNAL(triggered()), SLOT(slotTurnOnFileListItems()));
    menu.addAction(action);

    action = new QAction("Off", this);
    connect(action, SIGNAL(triggered()), SLOT(slotTurnOffFileListItems()));
    menu.addAction(action);

    action = new QAction("Toggle", this);
    connect(action, SIGNAL(triggered()), SLOT(slotToggleFileListItems()));
    menu.addAction(action);

    globalPoint = m_fileList->mapToGlobal(point);
    menu.exec(globalPoint);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotToggleFileListItems()
{
    for (auto& item : m_fileList->selectedItems())
    {
        if ((item->flags() & Qt::ItemIsUserCheckable) != 0)
        {
            item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotTurnOffFileListItems()
{
    for (auto& item : m_fileList->selectedItems())
    {
        if ((item->flags() & Qt::ItemIsUserCheckable) != 0)
        {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotTurnOnFileListItems()
{
    for (auto& item : m_fileList->selectedItems())
    {
        if ((item->flags() & Qt::ItemIsUserCheckable) != 0)
        {
            item->setCheckState(Qt::Checked);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotFindOrCancelButtonClicked()
{
    if (m_findOrCancelButton->text() == FIND_BUTTON_FIND_TEXT)
    {
        clearFileList();

        if(!m_fileList->isVisible())
        {
            m_fileListLabel->setVisible(true);
            m_fileList->setVisible(true);

            if(height() < DEFAULT_DIALOG_FIND_HEIGHT) resize(width(), DEFAULT_DIALOG_FIND_HEIGHT);
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
            updateStatus(NO_FILES_FOUND, true);
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
    m_files.clear();

    int itemCount = m_fileList->count();
    for (int i = 0; i < itemCount; i++)
    {
        const QListWidgetItem* item = m_fileList->item(i);
        if ((item->flags() & Qt::ItemIsUserCheckable) != 0 && item->checkState())
        {
            m_files.push_back(rootDir() + item->text());
        }
    }
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
QString toInternalSeparator(const QString& path)
{
    QString currNativeSep = QDir::separator();

    if (currNativeSep == "/")
    {
        // On Linux like system -> Do not convert separators
        return path;
    }

    // On other systems (i.e. Windows) -> Convert to internal separator (/)
    QString output = path;
    return output.replace(QString("\\"), SEPARATOR);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString& appendSeparatorIfNo(QString& path)
{
    if (!path.endsWith(SEPARATOR))
    {
        path.append(SEPARATOR);
    }
    return path;
}

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

        if (relPath.startsWith(SEPARATOR)) relPath.remove(0, 1);
        return appendSeparatorIfNo(relPath);
    }
    else
    {
        return dir;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool equalPaths(const QString& path1, const QString& path2)
{
    QString p1 = path1;
    QString p2 = path2;
    appendSeparatorIfNo(p1);
    appendSeparatorIfNo(p2);
    return p1 == p2;
}
