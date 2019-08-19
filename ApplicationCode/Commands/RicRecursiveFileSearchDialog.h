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
class QGroupBox;

class RicRecursiveFileSearchDialogResult;

//==================================================================================================
///  
//==================================================================================================
class RicRecursiveFileSearchDialog : public QDialog
{
    Q_OBJECT

    enum Status {SEARCHING_FOR_DIRS, SEARCHING_FOR_FILES, NO_FILES_FOUND};

public:
    RicRecursiveFileSearchDialog(QWidget* parent);
    ~RicRecursiveFileSearchDialog() override;

    static RicRecursiveFileSearchDialogResult  runRecursiveSearchDialog(QWidget *parent = nullptr,
                                                                  const QString& caption = QString(),
                                                                  const QString& dir = QString(),
                                                                  const QString& pathFilter = QString(),
                                                                  const QString& fileNameFilter = QString(),
                                                                  const QStringList& fileExtensions = QStringList());

private:
    QString     cleanTextFromPathFilterField() const;
    QString     rootDirWithEndSeparator() const;
    QString     pathFilterWithoutStartSeparator() const;
    QString     fileNameFilter() const;
    QStringList fileExtensions() const;
    QString     fileExtensionsText() const;
    QString     extensionFromFileNameFilter() const;

    void        updateFileListWidget();
    void        clearFileList();
    void        updateStatus(Status status, const QString& extraText = "");

    QStringList findMatchingFiles();

    QStringList buildDirectoryListRecursive(const QString& currentDir, int level = 0);

    void buildDirectoryListRecursiveSimple(const QString& currentDir,
                                           const QString& currentPathFilter,
                                           QStringList* accumulatedDirs);

    QStringList findFilesInDirs(const QStringList& dirs);

    QStringList createFileNameFilterList();

    bool        pathFilterMatch(const QString& pathFilter, const QString& relPath);

    void        updateEffectiveFilter();

    void        setOkButtonEnabled(bool enabled);

    void        warningIfInvalidCharacters();

private slots:
    void slotFilterChanged(const QString& text);
    
    void slotFileListCustomMenuRequested(const QPoint& point);
    void slotToggleFileListItems();
    void slotTurnOffFileListItems();
    void slotTurnOnFileListItems();
    void slotCopyFileItemText();

    void slotFindOrCancelButtonClicked();
    void slotDialogOkClicked();
    void slotDialogCancelClicked();
    void slotBrowseButtonClicked();

private:

    QLabel*                             m_pathFilterLabel;
    QLineEdit*                          m_pathFilterField;
    QPushButton*                        m_browseButton;

    QLabel*                             m_fileFilterLabel;
    QLineEdit*                          m_fileFilterField;

    QLabel*                             m_effectiveFilterLabel;
    QLabel*                             m_effectiveFilterContentLabel;

    QGroupBox*                          m_outputGroup;
    QLabel*                             m_searchRootLabel;
    QLabel*                             m_searchRootContentLabel;

    QLabel*                             m_fileListLabel;
    QListWidget*                        m_fileListWidget;

    QPushButton*                        m_findOrCancelButton;
    QDialogButtonBox*                   m_buttons;

    QStringList                         m_foundFiles;
    QStringList                         m_fileExtensions;

    bool                                m_isCancelPressed;
};


//==================================================================================================
///  
//==================================================================================================
class RicRecursiveFileSearchDialogResult
{
public:
    RicRecursiveFileSearchDialogResult(bool ok, 
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