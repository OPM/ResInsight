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

#include "RiaEnsembleNameTools.h"
#include <QDialog>

class QLabel;
class QLineEdit;
class QTextEdit;
class QDialogButtonBox;
class QPushButton;
class QMainWindow;
class QListWidget;
class QGroupBox;
class QComboBox;
class QCheckBox;

class RicRecursiveFileSearchDialogResult;

//==================================================================================================
///
//==================================================================================================
class RicRecursiveFileSearchDialog : public QDialog
{
    Q_OBJECT
public:
    enum Status
    {
        SEARCHING_FOR_DIRS,
        SEARCHING_FOR_FILES,
        NO_FILES_FOUND
    };

    enum class FileType
    {
        GRDECL,
        EGRID,
        GRID,
        SMSPEC,
        STIMPLAN_FRACTURE,
        LAS,
        SURFACE,
        STIMPLAN_SUMMARY,
        REVEAL_SUMMARY
    };

    static RicRecursiveFileSearchDialogResult runRecursiveSearchDialog( QWidget*                     parent         = nullptr,
                                                                        const QString&               caption        = QString(),
                                                                        const QString&               dir            = QString(),
                                                                        const QString&               pathFilter     = QString(),
                                                                        const QString&               fileNameFilter = QString(),
                                                                        const std::vector<FileType>& fileTypes      = {} );

    static QString fileNameForType( FileType fileType );
    static QString fileExtensionForType( FileType fileType );

private:
    RicRecursiveFileSearchDialog( QWidget* parent, const std::vector<FileType>& fileTypes );
    ~RicRecursiveFileSearchDialog() override;

    QString cleanTextFromPathFilterField() const;
    QString rootDirWithEndSeparator() const;
    QString pathFilterWithoutStartSeparator() const;
    QString fileNameFilter() const;

    QStringList fileExtensions() const;
    QString     extensionFromFileNameFilter() const;

    RiaEnsembleNameTools::EnsembleGroupingMode ensembleGroupingMode() const;

    void setOkButtonEnabled( bool enabled );
    void warningIfInvalidCharacters();
    void updateEffectiveFilter();
    void updateStatus( Status status, const QString& extraText = "" );

    void updateFileListWidget();
    void clearFileList();
    void addToFileListWidget( const QStringList& fileNames );

    // File search methods

    QStringList findMatchingFiles();
    void buildDirectoryListRecursiveSimple( const QString& currentDir, const QString& currentPathFilter, QStringList* accumulatedDirs );
    QStringList    findFilesInDirs( const QStringList& dirs );
    QStringList    createFileNameFilterList();
    static QString replaceWithRealizationStar( const QString& text );

    static void populateComboBoxHistoryFromRegistry( QComboBox* comboBox, const QString& registryKey );

    static QStringList fileTypeToExtensionStrings( const std::vector<RicRecursiveFileSearchDialog::FileType>& fileTypes );

private slots:
    void slotPathFilterChanged( const QString& text );
    void slotFileFilterChanged( const QString& text );
    void slotFileExtensionChanged( const QString& text );
    void slotFileTypeChanged( int );
    void slotBrowseButtonClicked();
    void slotUseRealizationStarClicked();
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
    QComboBox*   m_pathFilterField;
    QPushButton* m_browseButton;
    QCheckBox*   m_useRealizationStarCheckBox;

    QLabel*    m_fileFilterLabel;
    QComboBox* m_fileFilterField;

    QLabel*    m_fileTypeLabel;
    QComboBox* m_fileTypeField;

    QLabel*    m_fileExtensionLabel;
    QLineEdit* m_fileExtensionField;

    QLabel*      m_effectiveFilterLabel;
    QLabel*      m_effectiveFilterContentLabel;
    QPushButton* m_findOrCancelButton;

    QComboBox* m_ensembleGroupingMode;

    QGroupBox*   m_outputGroup;
    QLabel*      m_searchRootLabel;
    QLabel*      m_searchRootContentLabel;
    QListWidget* m_fileListWidget;

    QDialogButtonBox* m_buttons;

    QStringList           m_foundFiles;
    std::vector<FileType> m_incomingFileTypes;
    QStringList           m_fileExtensions;

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
    RicRecursiveFileSearchDialogResult( bool                                       ok,
                                        const QStringList&                         files,
                                        const QString&                             rootDir,
                                        const QString&                             pathFilter,
                                        const QString&                             fileNameFilter,
                                        RiaEnsembleNameTools::EnsembleGroupingMode groupingMode )
        : ok( ok )
        , files( files )
        , rootDir( rootDir )
        , pathFilter( pathFilter )
        , fileNameFilter( fileNameFilter )
        , groupingMode( groupingMode )
    {
    }

    bool        ok;
    QStringList files;
    QString     rootDir;
    QString     pathFilter;
    QString     fileNameFilter;

    RiaEnsembleNameTools::EnsembleGroupingMode groupingMode;
};
