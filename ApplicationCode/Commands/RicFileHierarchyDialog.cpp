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
#include "RiaFilePathTools.h"

#include "RimEclipseView.h"
#include "Rim3dOverlayInfoConfig.h"

#include "RiuPlotMainWindow.h"
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
#define SCANNING_DIRS_TEXT          "Scanning Directories"
#define FINDING_FILES_TEXT          "Finding Files"

//--------------------------------------------------------------------------------------------------
/// Internal variables
//--------------------------------------------------------------------------------------------------
static QString SEPARATOR = "/";

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
static QStringList  prefixStrings(const QStringList& strings, const QString& prefix);

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
RicFileHierarchyDialogResult RicFileHierarchyDialog::runRecursiveSearchDialog(QWidget *parent /*= 0*/,
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
    QString rootDir = RiaFilePathTools::toInternalSeparator(m_rootDir->text());
    return RiaFilePathTools::appendSeparatorIfNo(rootDir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFileHierarchyDialog::pathFilter() const
{
    return RiaFilePathTools::toInternalSeparator(m_pathFilter->text());
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
    QString itemText = fileName;
    itemText.remove(0, rootDir().size());
    QListWidgetItem* item = new QListWidgetItem(itemText, m_fileList);
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
void RicFileHierarchyDialog::updateStatus(Status status, const QString& extraText)
{
    static int progressLoopStep = 0;
    static time_t lastStatusUpdate = 0;
    time_t now = time(nullptr);

    QString newStatus;
    if (status == SEARCHING_FOR_DIRS || status == SEARCHING_FOR_FILES )
    {
        switch ( status )
        {
            case SEARCHING_FOR_DIRS: newStatus = SCANNING_DIRS_TEXT; break;
            case SEARCHING_FOR_FILES: newStatus = FINDING_FILES_TEXT; break;
        }

        for (int progress = 0; progress < progressLoopStep; ++progress)
        {
            newStatus += " .";
        }

        if (now != lastStatusUpdate) progressLoopStep++;    // If less than one second since last update, do not increment
        
        if (progressLoopStep >= 5) progressLoopStep = 0;

        if (!extraText.isEmpty()) newStatus += "\n" + extraText;
    }
    else if (status == NO_FILES_FOUND)
    {
        newStatus = NO_FILES_FOUND_TEXT;
    }

    lastStatusUpdate = now;

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

    //const QStringList& dirs = buildDirectoryListRecursive(rootDir());

    QStringList dirs;

    if ( this->pathFilter().isEmpty() ) dirs.append(this->rootDir());

    buildDirectoryListRecursiveSimple(this->rootDir(), this->pathFilter(), &dirs);

    const QStringList& files = findFilesInDirs(dirs);

    this->clearFileList();

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

    QString currRelPath = RiaFilePathTools::relativePath(rootDir(), currentDir);
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
        QString subDirFullPath = qdir.absoluteFilePath(subDir);
        updateStatus(SEARCHING_FOR_DIRS, subDirFullPath);
        QApplication::processEvents();
        allDirs += buildDirectoryListRecursive(subDirFullPath, level + 1);
    }
    return cancelPressed() ? QStringList() : allDirs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::buildDirectoryListRecursiveSimple(const QString& rootDir, 
                                                               const QString& remainingPathFilter, 
                                                               QStringList* accumulatedDirs)
{
    if (cancelPressed()) return;

    QString currentRemainingpathFilter = remainingPathFilter;
    {
        // Remove prefixing or trailing path separators from filter

        int pathSepIdx = currentRemainingpathFilter.indexOf(SEPARATOR);
        while ( pathSepIdx == 0 )
        {
            currentRemainingpathFilter.remove(pathSepIdx, 1);
            pathSepIdx = currentRemainingpathFilter.indexOf(SEPARATOR);
        }

        if ( currentRemainingpathFilter.endsWith(SEPARATOR) )
        {
            currentRemainingpathFilter.chop(1);
        }
    }

    QString effectiveRootDir = rootDir;
    {
        // Remove trailing path separator from root
        if ( effectiveRootDir.endsWith(SEPARATOR) )
        {
            effectiveRootDir.chop(1);
        }
    }

    // Search pathfilter for the first wildcard. 
    // Use the path up to that directory directly, short-cutting the search
    {
        std::set<int> sortedWildCardPositions;
        sortedWildCardPositions.insert(currentRemainingpathFilter.indexOf("*"));
        sortedWildCardPositions.insert(currentRemainingpathFilter.indexOf("?"));
        sortedWildCardPositions.insert(currentRemainingpathFilter.indexOf("["));

        int minWildCardPos = -1;
        for ( int wildCardPos : sortedWildCardPositions )
        {
            if ( wildCardPos == -1 ) continue;

            minWildCardPos = wildCardPos;
            break;
        }

        if ( minWildCardPos == -1 )
        {
            effectiveRootDir += SEPARATOR + currentRemainingpathFilter;
            currentRemainingpathFilter = "";
        }
        else
        {
            int pathSepPos = currentRemainingpathFilter.indexOf(SEPARATOR);
            while ( pathSepPos != -1 && pathSepPos < minWildCardPos )
            {
                effectiveRootDir += SEPARATOR + currentRemainingpathFilter.left(pathSepPos);
                currentRemainingpathFilter.remove(0, pathSepPos + 1); // include the separator
                minWildCardPos -= (pathSepPos + 1);
                pathSepPos = currentRemainingpathFilter.indexOf(SEPARATOR);
            }
        }
    }

    // Find the filter to use for this directory level 
    // Assumes no prefixed path separator

    QStringList subDirsFullPath;
    {
        int pathSepIdx = currentRemainingpathFilter.indexOf(SEPARATOR);
        QString currentDirNameFilter = currentRemainingpathFilter.left(pathSepIdx);
        
        if ( pathSepIdx == -1 ) 
        {
            currentRemainingpathFilter = "";
        }
        else
        {
            currentRemainingpathFilter.remove(0, pathSepIdx);
        }

        if ( currentDirNameFilter.isEmpty() ) currentDirNameFilter = "*";



        QDir qdir(effectiveRootDir, currentDirNameFilter, QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot);

        // Add effectiveRoot if current dir name filter =""or"*" or "?" and currentRemainingpathFilter = ""
        // and remainingPathFilter not empty
        if ( (currentDirNameFilter == "*" || currentDirNameFilter == "?") 
            && currentRemainingpathFilter.isEmpty()
            && !remainingPathFilter.isEmpty())
        {
            (*accumulatedDirs) += qdir.absolutePath();
        }

        QStringList subDirs = qdir.entryList();

        for ( const QString& subDir : subDirs )
        {
            QString fullPath = qdir.absoluteFilePath(subDir);
            subDirsFullPath += fullPath;
            (*accumulatedDirs) += fullPath;
        }
    }

    for (const QString& subDir : subDirsFullPath)
    {
        updateStatus(SEARCHING_FOR_DIRS, subDir);
        QApplication::processEvents();
        buildDirectoryListRecursiveSimple(subDir, currentRemainingpathFilter, accumulatedDirs);
    }
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

        updateStatus(SEARCHING_FOR_FILES, qdir.absolutePath());
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
    QString pattern = pathFilter;
    if (relPath.endsWith(SEPARATOR) && !pathFilter.endsWith(SEPARATOR)) pattern += SEPARATOR;
    QRegExp regexp(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
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

    QString internalFilter(RiaFilePathTools::toInternalSeparator(effFilter));

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
            updateStatus(NO_FILES_FOUND);
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
    if(!folder.isEmpty()) m_rootDir->setText(folder);
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
