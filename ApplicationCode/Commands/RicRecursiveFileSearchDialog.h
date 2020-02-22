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

    enum Status
    {
        SEARCHING_FOR_DIRS,
        SEARCHING_FOR_FILES,
        NO_FILES_FOUND
    };

public:
    static RicRecursiveFileSearchDialogResult runRecursiveSearchDialog( QWidget*           parent         = nullptr,
                                                                        const QString&     caption        = QString(),
                                                                        const QString&     dir            = QString(),
                                                                        const QString&     pathFilter     = QString(),
                                                                        const QString&     fileNameFilter = QString(),
                                                                        const QStringList& fileExtensions = QStringList() );

private:
    RicRecursiveFileSearchDialog( QWidget* parent );
    ~RicRecursiveFileSearchDialog() override;

    QString cleanTextFromPathFilterField() const;
    QString rootDirWithEndSeparator() const;
    QString pathFilterWithoutStartSeparator() const;
    QString fileNameFilter() const;

    QStringList fileExtensions() const;
    QString     extensionFromFileNameFilter() const;

    void setOkButtonEnabled( bool enabled );
    void warningIfInvalidCharacters();
    void updateEffectiveFilter();
    void updateStatus( Status status, const QString& extraText = "" );

    void updateFileListWidget();
    void clearFileList();

    // File search methods

    QStringList findMatchingFiles();
    void        buildDirectoryListRecursiveSimple( const QString& currentDir,
                                                   const QString& currentPathFilter,
                                                   QStringList*   accumulatedDirs );
    QStringList findFilesInDirs( const QStringList& dirs );
    QStringList createFileNameFilterList();

private slots:
    void slotFilterChanged( const QString& text );
    void slotBrowseButtonClicked();
    void slotFindOrCancelButtonClicked();

    void slotFileListCustomMenuRequested( const QPoint& point );
    void slotCopyFileItemText();
    void slotToggleFileListItems();
    void slotTurnOffFileListItems();
    void slotTurnOnFileListItems();

    void slotDialogOkClicked();
    void slotDialogCancelClicked();

private:
    QLabel*      m_pathFilterLabel;
    QLineEdit*   m_pathFilterField;
    QPushButton* m_browseButton;

    QLabel*    m_fileFilterLabel;
    QLineEdit* m_fileFilterField;

    QLabel*      m_effectiveFilterLabel;
    QLabel*      m_effectiveFilterContentLabel;
    QPushButton* m_findOrCancelButton;

    QGroupBox*   m_outputGroup;
    QLabel*      m_searchRootLabel;
    QLabel*      m_searchRootContentLabel;
    QListWidget* m_fileListWidget;

    QDialogButtonBox* m_buttons;

    QStringList m_foundFiles;
    QStringList m_fileExtensions;

    bool m_isCancelPressed;

    // Obsolete. Here for reference if this search mode is needed later
    QStringList buildDirectoryListRecursive( const QString& currentDir, int level = 0 );
    bool        pathFilterMatch( const QString& pathFilter, const QString& relPath );
};

//==================================================================================================
///
//==================================================================================================
class RicRecursiveFileSearchDialogResult
{
public:
    RicRecursiveFileSearchDialogResult( bool               ok,
                                        const QStringList& files,
                                        const QString&     rootDir,
                                        const QString&     pathFilter,
                                        const QString&     fileNameFilter )
        : ok( ok )
        , files( files )
        , rootDir( rootDir )
        , pathFilter( pathFilter )
        , fileNameFilter( fileNameFilter )
    {
    }

    bool        ok;
    QStringList files;
    QString     rootDir;
    QString     pathFilter;
    QString     fileNameFilter;
};