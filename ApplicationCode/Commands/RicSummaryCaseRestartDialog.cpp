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

#include "RicSummaryCaseRestartDialog.h"
#include "ExportCommands/RicSnapshotFilenameGenerator.h"
#include "ExportCommands/RicSnapshotViewToClipboardFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"

#include "RiaFilePathTools.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"

#include "RifEclipseSummaryTools.h"
#include "RifReaderEclipseSummary.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimEclipseView.h"

#include "RiuPlotMainWindow.h"
#include "RiuTools.h"

#include <QAbstractItemView>
#include <QAction>
#include <QCheckBox>
#include <QClipboard>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>

#include <cvfAssert.h>

#include <ctime>
#include <thread>
#include <vector>

#define DEFAULT_DIALOG_WIDTH 550
#define DEFAULT_DIALOG_INIT_HEIGHT 150

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<std::pair<RifRestartFileInfo, QString>>>
    makeShortPath( const std::vector<std::vector<RifRestartFileInfo>>& fileInfoLists )
{
    // Build output lists
    std::vector<std::vector<std::pair<RifRestartFileInfo, QString>>> output;

    QString currentFilePath =
        QFileInfo( fileInfoLists[CURRENT_FILES_LIST_INDEX].front().fileName ).absoluteDir().absolutePath();

    for ( const auto& fileInfoList : fileInfoLists )
    {
        std::vector<std::pair<RifRestartFileInfo, QString>> currList;

        for ( auto& fi : fileInfoList )
        {
            std::pair<RifRestartFileInfo, QString> newFi;
            newFi = std::make_pair( fi, fi.fileName );
            QFileInfo( fi.fileName ).fileName();

            QString absPath      = QFileInfo( fi.fileName ).absoluteDir().absolutePath();
            QString prefix       = RiaFilePathTools::equalPaths( currentFilePath, absPath ) ? "" : ".../";
            newFi.first.fileName = prefix + QFileInfo( fi.fileName ).fileName();
            currList.push_back( newFi );
        }
        output.push_back( currList );
    }
    return output;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::RicSummaryCaseRestartDialog( QWidget* parent )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
{
    // Create widgets
    m_currentFilesLayout      = new QGridLayout();
    m_summaryReadAllBtn       = new QRadioButton( this );
    m_summarySeparateCasesBtn = new QRadioButton( this );
    m_summaryNotReadBtn       = new QRadioButton( this );
    m_gridSeparateCasesBtn    = new QRadioButton( this );
    m_gridNotReadBtn          = new QRadioButton( this );
    m_warnings                = new QListWidget( this );
    m_showFullPathCheckBox    = new QCheckBox( this );
    m_buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply );

    // Connect to signals
    connect( m_showFullPathCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( slotShowFullPathToggled( int ) ) );
    connect( m_buttons, SIGNAL( clicked( QAbstractButton* ) ), this, SLOT( slotDialogButtonClicked( QAbstractButton* ) ) );

    // Set widget properties
    m_summaryReadAllBtn->setText( "Unified" );
    m_summarySeparateCasesBtn->setText( "Separate Cases" );
    m_summaryNotReadBtn->setText( "Skip" );
    m_gridSeparateCasesBtn->setText( "Separate Cases" );
    m_gridNotReadBtn->setText( "Skip" );
    m_showFullPathCheckBox->setText( "Show full paths" );
    m_buttons->button( QDialogButtonBox::Apply )->setText( "OK to All" );

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    m_currentFilesGroup = new QGroupBox( "Current Summary File" );
    m_currentFilesGroup->setStyleSheet( "QGroupBox { font-weight: bold; }" );
    m_currentFilesLayout = new QGridLayout();
    m_currentFilesGroup->setLayout( m_currentFilesLayout );

    // Summary files
    QGroupBox* summaryFilesGroup = new QGroupBox( "Origin Summary Files" );
    {
        summaryFilesGroup->setStyleSheet( "QGroupBox { font-weight: bold; }" );

        QVBoxLayout* filesGroupLayout = new QVBoxLayout();
        summaryFilesGroup->setLayout( filesGroupLayout );

        m_summaryFilesLayout = new QGridLayout();
        filesGroupLayout->addLayout( m_summaryFilesLayout );
        m_summaryFilesLayout->setContentsMargins( 0, 0, 0, 5 );

        QLabel* optionsLabel = new QLabel( "Import Options" );
        optionsLabel->setStyleSheet( "font-weight: bold;" );
        filesGroupLayout->addWidget( optionsLabel );

        QVBoxLayout* optionsLayout = new QVBoxLayout();
        optionsLayout->setSpacing( 0 );
        optionsLayout->addWidget( m_summaryReadAllBtn );
        optionsLayout->addWidget( m_summarySeparateCasesBtn );
        optionsLayout->addWidget( m_summaryNotReadBtn );
        filesGroupLayout->addLayout( optionsLayout );
    }

    // Grid files
    m_gridFilesGroup = new QGroupBox( "Origin Grid Files" );
    {
        m_gridFilesGroup->setStyleSheet( "QGroupBox { font-weight: bold; }" );

        QVBoxLayout* filesGroupLayout = new QVBoxLayout();
        m_gridFilesGroup->setLayout( filesGroupLayout );

        m_gridFilesLayout = new QGridLayout();
        filesGroupLayout->addLayout( m_gridFilesLayout );
        m_gridFilesLayout->setContentsMargins( 0, 0, 0, 5 );

        QLabel* optionsLabel = new QLabel( "Import Options" );
        optionsLabel->setStyleSheet( "font-weight: bold;" );
        filesGroupLayout->addWidget( optionsLabel );

        QVBoxLayout* optionsLayout = new QVBoxLayout();
        optionsLayout->setSpacing( 0 );
        optionsLayout->addWidget( m_gridSeparateCasesBtn );
        optionsLayout->addWidget( m_gridNotReadBtn );
        filesGroupLayout->addLayout( optionsLayout );
    }

    // Apply to all checkbox and buttons
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget( m_showFullPathCheckBox );
    buttonsLayout->addStretch( 1 );
    buttonsLayout->addWidget( m_buttons );

    QVBoxLayout* innerDialogLayout = new QVBoxLayout();
    innerDialogLayout->setSpacing( 20 );
    innerDialogLayout->addWidget( m_currentFilesGroup );
    innerDialogLayout->addWidget( summaryFilesGroup );
    innerDialogLayout->addWidget( m_gridFilesGroup );

    dialogLayout->addLayout( innerDialogLayout );
    dialogLayout->addWidget( m_warnings );
    dialogLayout->addLayout( buttonsLayout );

    setLayout( dialogLayout );

    m_okToAllPressed = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::~RicSummaryCaseRestartDialog()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialogResult RicSummaryCaseRestartDialog::openDialog( const QString& initialSummaryFile,
                                                                           const QString& initialGridFile,
                                                                           bool           failOnSummaryImportError,
                                                                           bool           showApplyToAllWidget,
                                                                           bool           useFirstSummaryCaseAsTemplate,
                                                                           ImportOptions  defaultSummaryImportOption,
                                                                           ImportOptions  defaultGridImportOption,
                                                                           RicSummaryCaseRestartDialogResult* lastResult,
                                                                           QWidget*                           parent )
{
    RicSummaryCaseRestartDialogResult dialogResult;
    if ( lastResult && lastResult->applyToAll && lastResult->summaryImportOption != SEPARATE_CASES )
    {
        dialogResult = *lastResult;
        dialogResult.summaryFiles.clear();
        dialogResult.gridFiles.clear();

        if ( !initialSummaryFile.isEmpty() ) dialogResult.summaryFiles.push_back( initialSummaryFile );
        if ( !initialGridFile.isEmpty() ) dialogResult.gridFiles.push_back( initialGridFile );
        return dialogResult;
    }

    RicSummaryCaseRestartDialog dialog( parent );
    bool                        handleSummaryFile = false;

    RifRestartFileInfo currentFileInfo;
    if ( !initialSummaryFile.isEmpty() )
    {
        currentFileInfo = dialog.getFileInfo( initialSummaryFile );

        if ( !currentFileInfo.valid() )
        {
            if ( failOnSummaryImportError )
            {
                return RicSummaryCaseRestartDialogResult( RicSummaryCaseRestartDialogResult::SUMMARY_ERROR );
            }
        }
        else
        {
            handleSummaryFile = true;
        }
    }

    bool handleGridFile = !initialGridFile.isEmpty();

    // If only grid file is present, return
    if ( !handleSummaryFile && !initialGridFile.isEmpty() )
    {
        RicSummaryCaseRestartDialogResult::Status status = RicSummaryCaseRestartDialogResult::SUMMARY_OK;
        if ( !initialSummaryFile.isEmpty() )
        {
            // We were meant to have a summary file but due to an error we don't.
            status = RicSummaryCaseRestartDialogResult::SUMMARY_WARNING;
        }
        return RicSummaryCaseRestartDialogResult( status,
                                                  defaultSummaryImportOption,
                                                  defaultGridImportOption,
                                                  {},
                                                  QStringList( {initialGridFile} ),
                                                  useFirstSummaryCaseAsTemplate ||
                                                      ( lastResult && lastResult->applyToAll ) );
    }

    RifReaderEclipseSummary         reader;
    bool                            hasWarnings     = false;
    std::vector<RifRestartFileInfo> originFileInfos = reader.getRestartFiles( initialSummaryFile, &hasWarnings );

    // If no restart files are found and no warnings, do not show dialog
    if ( originFileInfos.empty() && !hasWarnings )
    {
        return RicSummaryCaseRestartDialogResult( RicSummaryCaseRestartDialogResult::SUMMARY_OK,
                                                  NOT_IMPORT,
                                                  NOT_IMPORT,
                                                  QStringList( {initialSummaryFile} ),
                                                  QStringList( {initialGridFile} ),
                                                  useFirstSummaryCaseAsTemplate || lastResult->applyToAll );
    }

    if ( lastResult && lastResult->applyToAll )
    {
        dialogResult = *lastResult;
        dialogResult.summaryFiles.clear();
        dialogResult.gridFiles.clear();

        if ( hasWarnings )
        {
            for ( const QString& warning : reader.warnings() )
                RiaLogging::error( warning );
        }
    }
    else
    {
        std::vector<RifRestartFileInfo> currentFileInfos;
        std::vector<RifRestartFileInfo> originSummaryFileInfos;
        std::vector<RifRestartFileInfo> originGridFileInfos;

        // Grid file
        if ( handleGridFile )
        {
            dialog.m_currentFilesGroup->setTitle( "Current Grid and Summary Files" );
            currentFileInfos.push_back(
                RifRestartFileInfo( initialGridFile, currentFileInfo.startDate, currentFileInfo.endDate ) );

            for ( const auto& ofi : originFileInfos )
            {
                QString gridFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile( ofi.fileName );
                if ( QFileInfo( gridFile ).exists() )
                {
                    originGridFileInfos.push_back( RifRestartFileInfo( gridFile, ofi.startDate, ofi.endDate ) );
                }
            }
        }

        currentFileInfos.push_back( currentFileInfo );
        for ( const auto& ofi : originFileInfos )
        {
            originSummaryFileInfos.push_back( ofi );
        }

        // Set default import options
        switch ( defaultSummaryImportOption )
        {
            case ImportOptions::IMPORT_ALL:
                dialog.m_summaryReadAllBtn->setChecked( true );
                break;
            case ImportOptions::SEPARATE_CASES:
                dialog.m_summarySeparateCasesBtn->setChecked( true );
                break;
            case ImportOptions::NOT_IMPORT:
                dialog.m_summaryNotReadBtn->setChecked( true );
                break;
        }

        if ( handleGridFile )
        {
            switch ( defaultGridImportOption )
            {
                case ImportOptions::SEPARATE_CASES:
                    dialog.m_gridSeparateCasesBtn->setChecked( true );
                    break;
                case ImportOptions::NOT_IMPORT:
                    dialog.m_gridNotReadBtn->setChecked( true );
                    break;
            }
        }

        // Remove common root path
        std::vector<std::vector<std::pair<RifRestartFileInfo, QString>>> fileInfosNoRoot =
            makeShortPath( {currentFileInfos, originSummaryFileInfos, originGridFileInfos} );

        // Populate file list backing lists
        dialog.m_fileLists.push_back( fileInfosNoRoot[CURRENT_FILES_LIST_INDEX] );
        dialog.m_fileLists.push_back( fileInfosNoRoot[SUMMARY_FILES_LIST_INDEX] );
        dialog.m_fileLists.push_back( fileInfosNoRoot[GRID_FILES_LIST_INDEX] );

        // Update file list widgets
        dialog.updateFileListWidget( dialog.m_currentFilesLayout, CURRENT_FILES_LIST_INDEX );
        dialog.updateFileListWidget( dialog.m_summaryFilesLayout, SUMMARY_FILES_LIST_INDEX );
        dialog.updateFileListWidget( dialog.m_gridFilesLayout, GRID_FILES_LIST_INDEX );

        // Display warnings if any
        dialog.displayWarningsIfAny( reader.warnings() );

        // Set properties and show dialog
        dialog.setWindowTitle( "Origin Files" );
        dialog.m_buttons->button( QDialogButtonBox::Apply )->setVisible( !useFirstSummaryCaseAsTemplate && showApplyToAllWidget );
        dialog.resize( DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_INIT_HEIGHT );

        QApplication::setOverrideCursor( QCursor( Qt::ArrowCursor ) );
        dialog.exec();
        QApplication::restoreOverrideCursor();

        RicSummaryCaseRestartDialogResult::Status status = RicSummaryCaseRestartDialogResult::SUMMARY_OK;

        if ( dialog.result() == QDialog::Rejected )
        {
            status = RicSummaryCaseRestartDialogResult::SUMMARY_CANCELLED;
        }

        dialogResult = RicSummaryCaseRestartDialogResult( status,
                                                          dialog.selectedSummaryImportOption(),
                                                          dialog.selectedGridImportOption(),
                                                          {},
                                                          {},
                                                          useFirstSummaryCaseAsTemplate || dialog.okToAllSelected() );
    }

    if ( dialogResult.status != RicSummaryCaseRestartDialogResult::SUMMARY_OK )
    {
        return RicSummaryCaseRestartDialogResult( dialogResult.status,
                                                  NOT_IMPORT,
                                                  NOT_IMPORT,
                                                  QStringList(),
                                                  QStringList(),
                                                  false );
    }

    dialogResult.summaryFiles.push_back( RiaFilePathTools::toInternalSeparator( initialSummaryFile ) );
    if ( dialogResult.summaryImportOption == SEPARATE_CASES )
    {
        for ( const auto& ofi : originFileInfos )
        {
            dialogResult.summaryFiles.push_back( RiaFilePathTools::toInternalSeparator( ofi.fileName ) );
        }
    }

    if ( handleGridFile )
    {
        dialogResult.gridFiles.push_back( initialGridFile );

        if ( dialogResult.gridImportOption == SEPARATE_CASES )
        {
            for ( const auto& ofi : originFileInfos )
            {
                QString gridFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile( ofi.fileName );
                if ( handleGridFile ) dialogResult.gridFiles.push_back( gridFile );
            }
        }
    }
    return dialogResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::ImportOptions RicSummaryCaseRestartDialog::selectedSummaryImportOption() const
{
    return m_summaryReadAllBtn->isChecked() ? IMPORT_ALL
                                            : m_summarySeparateCasesBtn->isChecked() ? SEPARATE_CASES : NOT_IMPORT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::ImportOptions RicSummaryCaseRestartDialog::selectedGridImportOption() const
{
    return m_gridSeparateCasesBtn->isChecked() ? SEPARATE_CASES : NOT_IMPORT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSummaryCaseRestartDialog::okToAllSelected() const
{
    return m_okToAllPressed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::updateFileListWidget( QGridLayout* gridLayout, int listIndex )
{
    // Remove current items
    QLayoutItem* item;
    while ( item = gridLayout->takeAt( 0 ) )
    {
        gridLayout->removeItem( item );
        delete item->widget();
        delete item;
    }

    if ( m_fileLists[listIndex].empty() )
    {
        QWidget* parent = gridLayout->parentWidget();
        if ( parent ) parent->setVisible( false );
    }

    int maxFilesToDisplay = 4;
    int currFiles         = 0;
    for ( const auto& fileInfo : m_fileLists[listIndex] )
    {
        appendFileInfoToGridLayout( gridLayout, fileInfo.first, fileInfo.second );
        if ( ++currFiles == maxFilesToDisplay )
        {
            size_t remainingFileCount = m_fileLists[listIndex].size() - maxFilesToDisplay;
            appendTextToGridLayout( gridLayout, QString( "+ %1 more files" ).arg( remainingFileCount ) );
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::appendFileInfoToGridLayout( QGridLayout*              gridLayout,
                                                              const RifRestartFileInfo& fileInfo,
                                                              const QString&            fullPathFileName )
{
    CVF_ASSERT( gridLayout );

    QDateTime startDate       = QDateTime::fromTime_t( fileInfo.startDate );
    QString   startDateString = startDate.toString( RiaQDateTimeTools::dateFormatString() );
    QDateTime endDate         = QDateTime::fromTime_t( fileInfo.endDate );
    QString   endDateString   = endDate.toString( RiaQDateTimeTools::dateFormatString() );
    int       rowCount        = gridLayout->rowCount();

    QLabel* fileNameLabel = new QLabel();
    QLabel* dateLabel     = new QLabel();
    fileNameLabel->setText( m_showFullPathCheckBox->isChecked() ? fullPathFileName : fileInfo.fileName );
    dateLabel->setText( startDateString + " - " + endDateString );

    fileNameLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    gridLayout->addWidget( fileNameLabel, rowCount, 0 );
    gridLayout->addWidget( dateLabel, rowCount, 1 );

    // File name copy context menu
    fileNameLabel->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( fileNameLabel,
             SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this,
             SLOT( slotFileNameCopyCustomMenuRequested( const QPoint& ) ) );

    // Full path in tooltip
    fileNameLabel->setToolTip( fullPathFileName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::appendTextToGridLayout( QGridLayout* gridLayout, const QString& text )
{
    CVF_ASSERT( gridLayout );

    int rowCount = gridLayout->rowCount();

    QLabel* textLabel = new QLabel();
    textLabel->setText( text );

    textLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    gridLayout->addWidget( textLabel, rowCount, 0, 1, 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RicSummaryCaseRestartDialog::getFileInfo( const QString& summaryHeaderFile )
{
    RifReaderEclipseSummary reader;
    return reader.getFileInfo( summaryHeaderFile );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::displayWarningsIfAny( const QStringList& warnings )
{
    m_warnings->setVisible( !warnings.isEmpty() );
    for ( const auto& warning : warnings )
    {
        QListWidgetItem* item = new QListWidgetItem( warning, m_warnings );
        item->setForeground( Qt::red );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryCaseRestartDialog::fullFileName( const QString& shortOrFullFileName )
{
    for ( const auto& fileInfos : m_fileLists )
    {
        for ( const auto& fileInfo : fileInfos )
        {
            if ( fileInfo.first.fileName == shortOrFullFileName || fileInfo.second == shortOrFullFileName )
                return fileInfo.second;
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotShowFullPathToggled( int state )
{
    // Update file list widgets
    updateFileListWidget( m_currentFilesLayout, CURRENT_FILES_LIST_INDEX );
    updateFileListWidget( m_summaryFilesLayout, SUMMARY_FILES_LIST_INDEX );
    updateFileListWidget( m_gridFilesLayout, GRID_FILES_LIST_INDEX );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotDialogButtonClicked( QAbstractButton* button )
{
    bool cancelButtonClicked  = m_buttons->button( QDialogButtonBox::Cancel ) == button;
    bool okToAllButtonClicked = m_buttons->button( QDialogButtonBox::Apply ) == button;

    m_okToAllPressed = okToAllButtonClicked;
    if ( cancelButtonClicked )
        reject();
    else
        accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotFileNameCopyCustomMenuRequested( const QPoint& point )
{
    QMenu    menu;
    QAction* action;

    QLabel* sourceLabel = dynamic_cast<QLabel*>( sender() );

    action = new QAction( "Copy file name", this );
    action->setData( fullFileName( sourceLabel->text() ) );
    connect( action, SIGNAL( triggered() ), SLOT( slotCopyFileNameToClipboard() ) );
    menu.addAction( action );

    QPoint globalPoint = sourceLabel->mapToGlobal( point );
    menu.exec( globalPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotCopyFileNameToClipboard()
{
    QAction* a = dynamic_cast<QAction*>( sender() );

    QClipboard* cb = RiaGuiApplication::clipboard();
    if ( cb )
    {
        QString fullFileName = a->data().toString();
        cb->setText( QDir::toNativeSeparators( fullFileName ) );
    }
}
