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

#pragma once

#include "Rim3dOverlayInfoConfig.h"

#include "cafPdmPointer.h"

#include <QDialog>

class QLabel;
class QLineEdit;
class QTextEdit;
class QDialogButtonBox;
class QPushButton;
class QMainWindow;
class QListWidget;
class RicFileHierarchyDialogResult;

//==================================================================================================
///  
//==================================================================================================
class RicFileHierarchyDialog : public QDialog
{
    Q_OBJECT

    enum Status {SEARCHING_FOR_DIRS, SEARCHING_FOR_FILES, NO_FILES_FOUND};

public:
    RicFileHierarchyDialog(QWidget* parent);
    ~RicFileHierarchyDialog();

    static RicFileHierarchyDialogResult  runRecursiveSearchDialog(QWidget *parent = nullptr,
                                                                  const QString& caption = QString(),
                                                                  const QString& dir = QString(),
                                                                  const QString& pathFilter = QString(),
                                                                  const QString& fileNameFilter = QString(),
                                                                  const QStringList& fileExtensions = QStringList());

private:
    QStringList files() const;
    QString     rootDir() const;
    QString     pathFilter() const;
    QString     fileNameFilter() const;
    QStringList fileExtensions() const;
    bool        cancelPressed() const;
    void        appendToFileList(const QString& fileName);
    void        clearFileList();
    void        updateStatus(Status status, const QString& extraText = "");
    QString     currentStatus() const;

    QStringList findMatchingFiles();

    QStringList buildDirectoryListRecursive(const QString& currentDir, int level = 0);
    void buildDirectoryListRecursiveSimple(const QString& rootDir,
                                           const QString& remainingPathFilter,
                                           QStringList* accumulatedDirs);


    QStringList findFilesInDirs(const QStringList& dirs);

    QStringList createNameFilterList(const QString& fileNameFilter,
                                     const QStringList& fileExtensions);

    bool        pathFilterMatch(const QString& pathFilter, const QString& relPath);

    void        updateEffectiveFilter();

    void        setOkButtonEnabled(bool enabled);

private slots:
    void slotFilterChanged(const QString& text);
    void slotFileListCustomMenuRequested(const QPoint& point);
    void slotToggleFileListItems();
    void slotTurnOffFileListItems();
    void slotTurnOnFileListItems();
    void slotFindOrCancelButtonClicked();
    void slotDialogOkClicked();
    void slotDialogCancelClicked();
    void slotBrowseButtonClicked();

private:
    QLabel*                             m_rootDirLabel;
    QLineEdit*                          m_rootDir;
    QPushButton*                        m_browseButton;

    QLabel*                             m_pathFilterLabel;
    QLineEdit*                          m_pathFilter;

    QLabel*                             m_fileFilterLabel;
    QLineEdit*                          m_fileFilter;
    QLabel*                             m_fileExtension;

    QLabel*                             m_effectiveFilterLabel;
    QLabel*                             m_effectiveFilter;

    QLabel*                             m_fileListLabel;
    QListWidget*                        m_fileList;

    QPushButton*                        m_findOrCancelButton;
    QDialogButtonBox*                   m_buttons;

    QStringList                         m_files;

    bool                                m_cancelPressed;
};


//==================================================================================================
///  
//==================================================================================================
class RicFileHierarchyDialogResult
{
public:
    RicFileHierarchyDialogResult(bool ok, 
                                 const QStringList& files, 
                                 const QString& rootDir,
                                 const QString& pathFilter,
                                 const QString& fileNameFilter) :
        ok(ok), files(files), rootDir(rootDir), pathFilter(pathFilter), fileNameFilter(fileNameFilter) {}

    bool            ok;
    QStringList     files;
    QString         rootDir;
    QString         pathFilter;
    QString         fileNameFilter;
};