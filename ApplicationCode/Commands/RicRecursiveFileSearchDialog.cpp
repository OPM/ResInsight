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

#include "RicRecursiveFileSearchDialog.h"
#include "ExportCommands/RicSnapshotViewToClipboardFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "ExportCommands/RicSnapshotFilenameGenerator.h"

#include "RiaGuiApplication.h"
#include "RiaFilePathTools.h"

#include "RimEclipseView.h"
#include "Rim3dOverlayInfoConfig.h"

#include "RiuPlotMainWindow.h"
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
#include <QTime>
#include <QToolTip>
#include <QClipboard>

#include <vector>
#include <ctime>
#include <thread>

#define DEFAULT_DIALOG_WIDTH        750
#define DEFAULT_DIALOG_INIT_HEIGHT  150
#define DEFAULT_DIALOG_FIND_HEIGHT  350
#define FIND_BUTTON_FIND_TEXT       "Find"
#define FIND_BUTTON_CANCEL_TEXT     "Cancel"
#define NO_FILES_FOUND_TEXT         "No files found"
#define SCANNING_DIRS_TEXT          "Scanning Directories"
#define FINDING_FILES_TEXT          "Finding Files"
#define FILES_FOUND_TEXT            "Files found"

//--------------------------------------------------------------------------------------------------
/// Internal variables
//--------------------------------------------------------------------------------------------------
static const QChar SEPARATOR = RiaFilePathTools::SEPARATOR;

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
static QStringList  prefixStrings(const QStringList& strings, const QString& prefix);
static QStringList  trimLeftStrings(const QStringList& strings, const QString& trimText);
static void         sortStringsByLength(QStringList& strings, bool ascending = true);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialog::RicRecursiveFileSearchDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    // Create widgets
    m_rootDirLabel      = new QLabel();
    m_rootDirField           = new QLineEdit();
    m_browseButton      = new QPushButton();
    m_pathFilterLabel   = new QLabel();
    m_pathFilterField        = new QLineEdit();
    m_fileFilterLabel   = new QLabel();
    m_fileFilterField        = new QLineEdit();
    //m_fileExtensionLabel = new QLabel();
    m_effectiveFilterLabel = new QLabel();
    m_effectiveFilterContentLabel   = new QLabel();
    m_findOrCancelButton = new QPushButton();
    m_fileListLabel     = new QLabel();
    m_fileListWidget          = new QListWidget();

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect to signals
    connect(m_rootDirField, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_pathFilterField, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_fileFilterField, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_fileListWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotFileListCustomMenuRequested(const QPoint&)));

    connect(m_findOrCancelButton, SIGNAL(clicked()), this, SLOT(slotFindOrCancelButtonClicked()));
    connect(m_buttons, SIGNAL(accepted()), this, SLOT(slotDialogOkClicked()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(slotDialogCancelClicked()));
    connect(m_browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseButtonClicked()));

    // Set widget properties
    m_rootDirLabel->setText("Root folder");
    m_pathFilterLabel->setText("Path pattern");

    m_fileFilterLabel->setText("File pattern");
    m_effectiveFilterLabel->setText("Effective filter");
    m_effectiveFilterContentLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_fileListLabel->setText("Files found");
    m_fileListLabel->setVisible(false);
    
    m_fileListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileListWidget->setVisible(false);
    m_fileListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_fileListWidget->setSortingEnabled(true);
    
    m_browseButton->setText("...");
    m_browseButton->setFixedWidth(25);
    m_findOrCancelButton->setText(FIND_BUTTON_FIND_TEXT);
    m_findOrCancelButton->setFixedWidth(75);
    m_findOrCancelButton->setDefault(true);

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QGroupBox* inputGroup = new QGroupBox("Filter");
    QGridLayout* inputGridLayout = new QGridLayout();
    inputGridLayout->addWidget(m_rootDirLabel, 0, 0);
    inputGridLayout->addWidget(m_rootDirField, 0, 1);
    inputGridLayout->addWidget(m_browseButton, 0, 2);
    inputGridLayout->addWidget(m_pathFilterLabel, 1, 0);
    inputGridLayout->addWidget(m_pathFilterField, 1, 1);
    inputGridLayout->addWidget(m_fileFilterLabel, 2, 0);
    inputGridLayout->addWidget(m_fileFilterField, 2, 1);
    //inputGridLayout->addWidget(m_fileExtensionLabel, 2, 2);
    inputGroup->setLayout(inputGridLayout);

    QGroupBox* outputGroup = new QGroupBox("Files");
    QGridLayout* outputGridLayout = new QGridLayout();
    outputGridLayout->addWidget(m_effectiveFilterLabel, 0, 0);
    outputGridLayout->addWidget(m_effectiveFilterContentLabel, 0, 1);
    outputGridLayout->addWidget(m_findOrCancelButton, 0, 2);
    outputGridLayout->addWidget(m_fileListLabel, 1, 0);
    outputGridLayout->addWidget(m_fileListWidget, 1, 1, 1, 2);
    outputGroup->setLayout(outputGridLayout);

    dialogLayout->addWidget(inputGroup);
    dialogLayout->addWidget(outputGroup);
    dialogLayout->addWidget(m_buttons);

    setLayout(dialogLayout);

    QString pathFilterHelpText = 
        "The path filter uses normal wildcard file globbing, like in any unix shell. \n"
        "\n"
        "An asterix \"*\" matches any number of any characters, except the path separator.\n"
        "A question mark \"?\" matches any single character, except the path separator.\n"
        "\n"
        "When the filter ends with a single \"*\", however, ResInsight will \n"
        "search recursively in all subdirectories from that point.\n"
        "This is indicated by \"...\" in the Effective Filter label below.";

    m_pathFilterLabel->setToolTip(pathFilterHelpText);
    m_pathFilterField->setToolTip(pathFilterHelpText);

    QString fileFilterHelpText = 
        "The file filter uses normal wildcards, but is not allowed to contain path separators. ";

    m_fileFilterLabel->setToolTip(fileFilterHelpText);
    m_fileFilterField->setToolTip(fileFilterHelpText);

    QString effectiveFilterHelpText = 
        "This label displays the complete filter that is being applied. \n"
        "The possible \"...\" indicates a complete recursive directory search.";

    m_effectiveFilterLabel->setToolTip(effectiveFilterHelpText);
    m_effectiveFilterContentLabel->setToolTip(effectiveFilterHelpText);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialog::~RicRecursiveFileSearchDialog()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialogResult RicRecursiveFileSearchDialog::runRecursiveSearchDialog(QWidget *parent /*= 0*/,
                                                                              const QString &caption /*= QString()*/,
                                                                              const QString &dir /*= QString()*/,
                                                                              const QString &pathFilter /*= QString()*/,
                                                                              const QString &fileNameFilter /*= QString()*/,
                                                                              const QStringList &fileExtensions /*= QStringList()*/)
{
    QStringList             files;
    RicRecursiveFileSearchDialog  dialog(parent);

    dialog.setWindowTitle(caption);

    dialog.m_rootDirField->setText(QDir::toNativeSeparators(dir));
    dialog.m_pathFilterField->setText(QDir::toNativeSeparators(pathFilter));
    dialog.m_fileFilterField->setText(fileNameFilter);
    dialog.m_fileExtensions = trimLeftStrings(fileExtensions, ".");

    dialog.updateEffectiveFilter();
    dialog.clearFileList();
    dialog.setOkButtonEnabled(false);

    dialog.resize(DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_INIT_HEIGHT);
    dialog.exec();

    return RicRecursiveFileSearchDialogResult(dialog.result() == QDialog::Accepted, 
                                        dialog.files(), 
                                        dialog.rootDirWithEndSeparator(), 
                                        dialog.pathFilter(), 
                                        dialog.fileNameFilter());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::files() const
{
    return m_foundFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::rootDirWithEndSeparator() const
{
    QString rootDir = RiaFilePathTools::toInternalSeparator(m_rootDirField->text().trimmed());
    rootDir = RiaFilePathTools::removeDuplicatePathSeparators(rootDir);
    return RiaFilePathTools::appendSeparatorIfNo(rootDir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::pathFilter() const
{
    QString pathFilter = m_pathFilterField->text().trimmed();
    pathFilter = RiaFilePathTools::toInternalSeparator(pathFilter);
    pathFilter.replace(QString("**"), QString("*"));
    return  RiaFilePathTools::removeDuplicatePathSeparators(pathFilter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::fileNameFilter() const
{
    return m_fileFilterField->text().trimmed();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::fileExtensions() const
{
    QString extFromFilter = extensionFromFileNameFilter();
    if (!extFromFilter.isEmpty())
    {
        return QStringList({ extFromFilter });
    }

    QStringList exts = m_fileExtensions;
    sortStringsByLength(exts);
    return exts;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::fileExtensionsText() const
{
    QString extFromFilter = extensionFromFileNameFilter();
    if (!extFromFilter.isEmpty())   return "";
    else                            return prefixStrings(fileExtensions(), ".").join(" | ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::extensionFromFileNameFilter() const
{
    for (const QString& ext : m_fileExtensions)
    {
        if (m_fileFilterField->text().endsWith(ext, Qt::CaseInsensitive))
        {
            return ext;
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicRecursiveFileSearchDialog::cancelPressed() const
{
    return m_cancelPressed;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::appendToFileList(const QString& fileName)
{
    QString itemText = fileName;
    itemText.remove(0, rootDirWithEndSeparator().size());
    QListWidgetItem* item = new QListWidgetItem(QDir::toNativeSeparators(itemText), m_fileListWidget);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::clearFileList()
{
    m_foundFiles.clear();
    m_fileListWidget->clear();
    m_fileListLabel->setText(FILES_FOUND_TEXT);
    setOkButtonEnabled(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::updateStatus(Status status, const QString& extraText)
{
    static int progressLoopStep = 0;
    static QTime lastStatusUpdate;
    QTime now = QTime::currentTime();

    // Do not update dialog more often than twice per second to avoid text update from slowing down search progress
    if (status != NO_FILES_FOUND && lastStatusUpdate.msecsTo(now) < 500) return;

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

        if (++progressLoopStep >= 5) progressLoopStep = 0;

        if (!extraText.isEmpty()) newStatus += "\n" + extraText;
    }
    else if (status == NO_FILES_FOUND)
    {
        newStatus = NO_FILES_FOUND_TEXT;
    }

    lastStatusUpdate = now;

    m_fileListWidget->clear();
    new QListWidgetItem(newStatus, m_fileListWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::findMatchingFiles()
{
    if (m_rootDirField->text().isEmpty()) return QStringList();

    //const QStringList& dirs = buildDirectoryListRecursive(rootDir());

    QStringList dirs;

    QString pathFilter = this->pathFilter();
    if (pathFilter.startsWith(SEPARATOR)) pathFilter.remove(0, 1);
    QString rootDir = this->rootDirWithEndSeparator();
    if (rootDir.endsWith(SEPARATOR)) rootDir.chop(1);

    buildDirectoryListRecursiveSimple(rootDir, pathFilter, &dirs);

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
QStringList RicRecursiveFileSearchDialog::buildDirectoryListRecursive(const QString& currentDir, int level)
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

    QString currRelPath = RiaFilePathTools::relativePath(rootDirWithEndSeparator(), currentDir);
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
void RicRecursiveFileSearchDialog::buildDirectoryListRecursiveSimple(const QString& currentDirFullPathNoEndSeparator,
                                                               const QString& currentPathFilterNoEndSeparator,
                                                               QStringList* accumulatedDirs)
{
    QString currDir = currentDirFullPathNoEndSeparator;
    QString pathFilter = currentPathFilterNoEndSeparator;

    if (cancelPressed())
    {
        accumulatedDirs->clear();
        return;
    }

    updateStatus(SEARCHING_FOR_DIRS, currDir);
    QApplication::processEvents();

    if (pathFilter.isEmpty())
    {
        accumulatedDirs->push_back(currentDirFullPathNoEndSeparator);
        return;
    }

    QStringList pathFilterPartList = pathFilter.split(SEPARATOR);
    QDir        qdir(currDir, pathFilterPartList[0], QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList subDirs = qdir.entryList();

    if (pathFilterPartList.size() == 1 && pathFilterPartList[0] == "*")
    {
        accumulatedDirs->push_back(currDir);
    }

    for (const QString& subDir : subDirs)
    {
        QString     fullPath = qdir.absoluteFilePath(subDir);
        QString     nextPathFilter;

        if (pathFilterPartList.size() == 1 && pathFilterPartList[0] == "*")
        {
            nextPathFilter = "*";
        }
        else
        {
            auto pf = pathFilterPartList;
            pf.removeFirst();
            nextPathFilter = pf.join(SEPARATOR);
        }

        buildDirectoryListRecursiveSimple(fullPath, nextPathFilter, accumulatedDirs);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::findFilesInDirs(const QStringList& dirs)
{
    QStringList allFiles;
    QStringList filters = createFileNameFilterList();

    for (const auto& dir : dirs)
    {
        QDir qdir(dir);
        QStringList files = qdir.entryList(filters, QDir::Files);

        if (cancelPressed()) return QStringList();

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
QStringList RicRecursiveFileSearchDialog::createFileNameFilterList()
{
    QString fileNameFilter = this->fileNameFilter();
    QStringList fileExtensions = this->fileExtensions();

    QStringList nameFilter;
    QString effectiveFileNameFilter = !fileNameFilter.isEmpty() ? fileNameFilter : "*";

    if (fileExtensions.size() == 0 || !extensionFromFileNameFilter().isEmpty())
    {
        nameFilter.append(effectiveFileNameFilter);
    }
    else
    {
        for (QString fileExtension : fileExtensions)
        {
            nameFilter.append(effectiveFileNameFilter + "." + fileExtension);
        }
    }
    return nameFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicRecursiveFileSearchDialog::pathFilterMatch(const QString& pathFilter, const QString& relPath)
{
    QString pattern = pathFilter;
    if (relPath.endsWith(SEPARATOR) && !pathFilter.endsWith(SEPARATOR)) pattern += SEPARATOR;
    QRegExp regexp(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
    return regexp.exactMatch(relPath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::updateEffectiveFilter()
{
    QString pathFilterText = pathFilter();
    if (pathFilterText == "*" || pathFilterText.endsWith(QString(SEPARATOR) + "*")) 
    {
         pathFilterText.chop(1);
         pathFilterText = pathFilterText + "...";
    }

    QString effFilterText = QString("%1%2/%3")
        .arg(rootDirWithEndSeparator())
        .arg(pathFilterText)
        .arg(createFileNameFilterList().join("|"));

    effFilterText = RiaFilePathTools::removeDuplicatePathSeparators(effFilterText);

    // Present native separators to the user
    m_effectiveFilterContentLabel->setText(QDir::toNativeSeparators(effFilterText));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::setOkButtonEnabled(bool enabled)
{
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::warningIfInvalidCharacters()
{
    if (m_fileFilterField->text().contains(QRegExp("[\\\\/:]")))
    {
        QToolTip::showText(m_fileFilterField->mapToGlobal(QPoint(0, 0)), "File pattern contains invalid characters");
        m_effectiveFilterContentLabel->setText("(Invalid filter)");
    }
    else
    {
        QToolTip::hideText();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFilterChanged(const QString& text)
{
    updateEffectiveFilter();
    warningIfInvalidCharacters();
    m_findOrCancelButton->setDefault(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFileListCustomMenuRequested(const QPoint& point)
{
    QMenu menu;
    QPoint globalPoint = point;
    QAction* action;

    action = new QAction(QIcon(":/Copy.png"), "&Copy", this);
    connect(action, SIGNAL(triggered()), SLOT(slotCopyFileItemText()));
    menu.addAction(action);
    menu.addSeparator();

    action = new QAction("On", this);
    connect(action, SIGNAL(triggered()), SLOT(slotTurnOnFileListItems()));
    menu.addAction(action);

    action = new QAction("Off", this);
    connect(action, SIGNAL(triggered()), SLOT(slotTurnOffFileListItems()));
    menu.addAction(action);

    action = new QAction("Toggle", this);
    connect(action, SIGNAL(triggered()), SLOT(slotToggleFileListItems()));
    menu.addAction(action);

    globalPoint = m_fileListWidget->mapToGlobal(point);
    menu.exec(globalPoint);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotToggleFileListItems()
{
    for (auto& item : m_fileListWidget->selectedItems())
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
void RicRecursiveFileSearchDialog::slotTurnOffFileListItems()
{
    for (auto& item : m_fileListWidget->selectedItems())
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
void RicRecursiveFileSearchDialog::slotTurnOnFileListItems()
{
    for (auto& item : m_fileListWidget->selectedItems())
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
void RicRecursiveFileSearchDialog::slotCopyFileItemText()
{
    if (m_fileListWidget->currentItem())
    {
        QString relativePathText = m_fileListWidget->currentItem()->text();
        RiaGuiApplication::instance()->clipboard()->setText(relativePathText);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFindOrCancelButtonClicked()
{
    if (m_findOrCancelButton->text() == FIND_BUTTON_FIND_TEXT)
    {
        clearFileList();

        if(!m_fileListWidget->isVisible())
        {
            m_fileListLabel->setVisible(true);
            m_fileListWidget->setVisible(true);

            if(height() < DEFAULT_DIALOG_FIND_HEIGHT) resize(width(), DEFAULT_DIALOG_FIND_HEIGHT);
        }

        m_findOrCancelButton->setText(FIND_BUTTON_CANCEL_TEXT);

        m_cancelPressed = false;
        m_foundFiles = findMatchingFiles();

        m_findOrCancelButton->setText(FIND_BUTTON_FIND_TEXT);

        if (m_cancelPressed)
        {
            clearFileList();
        }
        else if(m_foundFiles.isEmpty())
        {
            updateStatus(NO_FILES_FOUND);
        }
        else
        {
            m_fileListLabel->setText(QString("%1\n(%2)").arg(FILES_FOUND_TEXT).arg(m_foundFiles.size()));
        }

        setOkButtonEnabled(!m_foundFiles.isEmpty());
        m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    }
    else
    {
        m_cancelPressed = true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotDialogOkClicked()
{
    m_foundFiles.clear();

    int itemCount = m_fileListWidget->count();
    for (int i = 0; i < itemCount; i++)
    {
        const QListWidgetItem* item = m_fileListWidget->item(i);
        if ((item->flags() & Qt::ItemIsUserCheckable) != 0 && item->checkState())
        {
            m_foundFiles.push_back(rootDirWithEndSeparator() + RiaFilePathTools::toInternalSeparator(item->text()));
        }
    }
    accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotDialogCancelClicked()
{
    m_foundFiles = QStringList();
    m_cancelPressed = true;
    reject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotBrowseButtonClicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Select root folder", m_rootDirField->text());
    if(!folder.isEmpty()) m_rootDirField->setText(QDir::toNativeSeparators(folder));
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
QStringList  trimLeftStrings(const QStringList& strings, const QString& trimText)
{
    QStringList trimmedStrings;
    for (const auto& string : strings)
    {
        QString trimmedString = string;
        if (string.startsWith(trimText))
        {
            trimmedString = string.right(string.size() - trimText.size());
        }
        trimmedStrings.append(trimmedString);
    }
    return trimmedStrings;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void sortStringsByLength(QStringList& strings, bool ascending /*= true*/)
{
    QStringList sorted = strings;
    int numItems = sorted.size();
    bool swapped;
    do
    {
        swapped = false;
        for (int i = 0; i < numItems - 1; i++)
        {
            int s0 = strings[i].size();
            int s1 = strings[i + 1].size();
            if (ascending && s0 > s1 || !ascending && s0 < s1)
            {
                const QString temp = strings[i];
                strings[i] = strings[i + 1];
                strings[i + 1] = temp;
                swapped = true;
            }
        }
    } while (swapped);


}

