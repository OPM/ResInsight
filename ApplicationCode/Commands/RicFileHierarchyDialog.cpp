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

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect to close button signal
    connect(m_buttons, SIGNAL(accepted()), this, SLOT(slotDialogOkClicked()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(slotDialogCancelClicked()));
    connect(m_browseButton, SIGNAL(clicked()), this, SLOT(slotBrowseButtonClicked()));

    // Set widget properties
    m_rootDirLabel->setText("Root folder");
    m_pathFilterLabel->setText("Path pattern");
    m_pathFilter->setText("*");
    m_fileFilterLabel->setText("File pattern");
    m_browseButton->setText("...");

    m_browseButton->setFixedWidth(25);

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();
    //dialogLayout->addWidget(m_rootDirLabel);
    //dialogLayout->addWidget(m_rootDir);
    //dialogLayout->addWidget(m_browseButton);
    //dialogLayout->addWidget(m_pathFilterLabel);
    //dialogLayout->addWidget(m_pathFilter);
    //dialogLayout->addWidget(m_fileFilterLabel);
    //dialogLayout->addWidget(m_fileFilter);
    //dialogLayout->addWidget(m_fileExtension);
    //dialogLayout->addWidget(m_buttons);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget(m_rootDirLabel, 0, 0);
    gridLayout->addWidget(m_rootDir, 0, 1);
    gridLayout->addWidget(m_browseButton, 0, 2);
    gridLayout->addWidget(m_pathFilterLabel, 1, 0);
    gridLayout->addWidget(m_pathFilter, 1, 1);
    gridLayout->addWidget(m_fileFilterLabel, 2, 0);
    gridLayout->addWidget(m_fileFilter, 2, 1);
    gridLayout->addWidget(m_fileExtension, 2, 2);

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
RicFileHierarchyDialogResult RicFileHierarchyDialog::getOpenFileNames(QWidget *parent /*= 0*/, 
                                                                     const QString &caption /*= QString()*/, 
                                                                     const QString &dir /*= QString()*/, 
                                                                     const QString &fileNameFilter /*= QString()*/,
                                                                     const QStringList &fileExtensions /*= QStringList()*/)
{
    QStringList             files;
    RicFileHierarchyDialog  dialog(parent);
    
    dialog.setWindowTitle(caption);

    dialog.m_rootDir->setText(dir);
    dialog.m_fileFilter->setText(fileNameFilter);
    dialog.m_fileExtension->setText(prefixStrings(fileExtensions, ".").join(" | "));

    dialog.resize(600, 150);
    dialog.exec();

    return RicFileHierarchyDialogResult(dialog.result() == QDialog::Accepted, dialog.files(), dialog.rootDir());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::findMatchingFiles(const QString& rootDir, const QString& pathFilter, const QString& fileNameFilter, const QStringList& fileExtensions)
{
    QStringList files = findFilesRecursive(rootDir, fileNameFilter, fileExtensions);
    return filterByPathFilter(files, pathFilter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RicFileHierarchyDialog::findFilesRecursive(const QString &dir, const QString &fileNameFilter, const QStringList &fileExtensions)
{
    QStringList allFiles;

    QDir qdir(dir);
    QStringList subDirs = qdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList files = qdir.entryList(createNameFilterList(fileNameFilter, fileExtensions), QDir::Files);

    for (QString file : files)
    {
        allFiles.append(qdir.absoluteFilePath(file));
    }

    for (QString subDir : subDirs)
    {
        allFiles += findFilesRecursive(qdir.absoluteFilePath(subDir), fileNameFilter, fileExtensions);
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
QStringList RicFileHierarchyDialog::filterByPathFilter(const QStringList& files, const QString& pathFilter)
{
    QStringList filteredFiles;
    QRegExp regexp(pathFilter, Qt::CaseInsensitive, QRegExp::Wildcard);

    for (QString file : files)
    {
        QFileInfo fileInfo(file);
        QString path = fileInfo.absolutePath();
        
        if (regexp.exactMatch(path))
        {
            filteredFiles.append(file);
        }
    }

    return filteredFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotDialogOkClicked()
{
    m_files = findMatchingFiles(m_rootDir->text(), m_pathFilter->text(), m_fileFilter->text(), m_fileExtension->text().split("|"));
    accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFileHierarchyDialog::slotDialogCancelClicked()
{
    m_files = QStringList();
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
