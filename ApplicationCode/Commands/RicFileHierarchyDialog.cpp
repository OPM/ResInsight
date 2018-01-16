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

#include <vector>

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
QStringList prefixStrings(const QStringList& strings, const QString& prefix);

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

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect to close button signal
    connect(m_rootDir, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_pathFilter, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));
    connect(m_fileFilter, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilterChanged(const QString&)));

    connect(m_buttons, SIGNAL(accepted()), this, SLOT(slotDialogOkClicked()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(slotDialogCancelClicked()));
    connect(m_browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseButtonClicked()));

    // Set widget properties
    m_rootDirLabel->setText("Root folder");
    m_pathFilterLabel->setText("Path pattern");
    m_fileFilterLabel->setText("File pattern");
    m_effectiveFilterLabel->setText("Effective filter");
    m_fileListLabel->setText("Files found");
    m_fileList->setLineWrapMode(QTextEdit::NoWrap);
    m_browseButton->setText("...");
    m_browseButton->setFixedWidth(25);

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget(m_rootDirLabel, 0, 0);
    gridLayout->addWidget(m_rootDir, 0, 1);
    gridLayout->addWidget(m_browseButton, 0, 2);
    gridLayout->addWidget(m_pathFilterLabel, 1, 0);
    gridLayout->addWidget(m_pathFilter, 1, 1);
    gridLayout->addWidget(m_fileFilterLabel, 2, 0);
    gridLayout->addWidget(m_fileFilter, 2, 1);
    gridLayout->addWidget(m_fileExtension, 2, 2);
    gridLayout->addWidget(m_effectiveFilterLabel, 3, 0);
    gridLayout->addWidget(m_effectiveFilter, 3, 1);
    gridLayout->addWidget(m_fileListLabel, 4, 0);
    gridLayout->addWidget(m_fileList, 4, 1);

    dialogLayout->addLayout(gridLayout);
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
    return m_rootDir->text();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicFileHierarchyDialog::pathFilter() const
{
    return m_pathFilter->text();
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
    m_fileList->append(fileName);
    QApplication::processEvents();
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

    dialog.resize(600, 250);
    dialog.exec();

    return RicFileHierarchyDialogResult(dialog.result() == QDialog::Accepted, dialog.files(), dialog.rootDir(), dialog.pathFilter(), dialog.fileNameFilter());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::findMatchingFiles(RicFileHierarchyDialog& dialog)
{
    return findFilesRecursive(dialog.rootDir(), dialog);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::findFilesRecursive(const QString &currentDir, RicFileHierarchyDialog& dialog)
{
    QStringList allFiles;

    if (dialog.cancelPressed()) return allFiles;

    QDir qdir(currentDir);
    QStringList subDirs = qdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList files = qdir.entryList(createNameFilterList(dialog.fileNameFilter(), dialog.fileExtensions()), QDir::Files);

    for (QString file : files)
    {
        QString absFilePath = qdir.absoluteFilePath(file);
        if (pathFilterMatch(absFilePath, dialog.pathFilter()))
        {
            allFiles.append(absFilePath);
            dialog.appendToFileList(absFilePath);
        }
    }

    for (QString subDir : subDirs)
    {
        QApplication::processEvents();
        allFiles += findFilesRecursive(qdir.absoluteFilePath(subDir), dialog);
    }
    return dialog.cancelPressed() ? QStringList() : allFiles;
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
bool RicFileHierarchyDialog::pathFilterMatch(const QString& file, const QString& pathFilter)
{
    QRegExp regexp(pathFilter, Qt::CaseInsensitive, QRegExp::Wildcard);

    QFileInfo fileInfo(file);
    QString path = fileInfo.absolutePath();

    return regexp.exactMatch(path);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::updateEffectiveFilter()
{
    QString sep(QDir::separator());

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
        native.replace(sep + sep, sep);
    } while (native.size() != len);

    m_effectiveFilter->setText(native);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotFilterChanged(const QString& text)
{
    updateEffectiveFilter();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotDialogOkClicked()
{
    m_cancelPressed = false;
    m_files = findMatchingFiles(*this);
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
