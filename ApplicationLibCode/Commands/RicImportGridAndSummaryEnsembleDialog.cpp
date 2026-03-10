/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicImportGridAndSummaryEnsembleDialog.h"

#include "RicRecursiveFileSearchDialog.h"

#include "RiaApplication.h"
#include "RiaEnsembleNameTools.h"
#include "RiaFilePathTools.h"
#include "RiaFileSearchTools.h"
#include "RiaPreferences.h"
#include "RiaStdStringTools.h"
#include "RiaStringListSerializer.h"

#include "RiuFileDialogTools.h"
#include "RiuTools.h"

#include "cafAppEnum.h"

#include <QCheckBox>
#include <QCollator>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportGridAndSummaryEnsembleDialogResult
    RicImportGridAndSummaryEnsembleDialog::runDialog( QWidget* parent, bool defaultGridChecked, bool defaultSummaryChecked )
{
    const QString pathRegistryKey = "RicImportGridAndSummaryEnsembleDialog_path";

    auto* app = RiaApplication::instance();

    RicImportGridAndSummaryEnsembleDialog dialog( parent );
    {
        QSignalBlocker blocker1( dialog.m_pathFilterField );

        dialog.setWindowTitle( "Import Grid and Summary Ensemble" );

        // Default path (index 0 fallback)
        QString defaultDir = app->lastUsedDialogDirectory( "GRID_SUMMARY_ENSEMBLE" );
        RiaFilePathTools::appendSeparatorIfNo( defaultDir );
        defaultDir += "*";
        dialog.m_pathFilterField->addItem( QDir::toNativeSeparators( defaultDir ) );

        // Registry history starts at index 1
        RicRecursiveFileSearchDialog::populateComboBoxHistoryFromRegistry( dialog.m_pathFilterField, pathRegistryKey );

        // Use most recently used path from registry (index 1) when preference is enabled
        if ( RiaPreferences::current()->useRecentlyUsedFolderAsDefault() && dialog.m_pathFilterField->count() > 1 )
        {
            dialog.m_pathFilterField->setCurrentIndex( 1 );
        }

        dialog.m_pathFilterField->setEditable( true );

        for ( const auto& s : caf::AppEnum<RiaDefines::EnsembleGroupingMode>::uiTexts() )
        {
            dialog.m_ensembleGroupingMode->addItem( s );
        }

        dialog.m_createGridEnsembleCheckBox->setChecked( defaultGridChecked );
        dialog.m_createSummaryEnsembleCheckBox->setChecked( defaultSummaryChecked );

        dialog.updateEffectiveFilter();
        dialog.clearFileList();
        dialog.setOkButtonEnabled( false );

        dialog.resize( 800, 150 );
    }

    if ( dialog.exec() != QDialog::Accepted )
    {
        return {};
    }

    // Save registry history
    const int maxItemsInRegistry = 10;
    {
        RiaStringListSerializer s( pathRegistryKey );
        s.addString( dialog.m_pathFilterField->currentText(), maxItemsInRegistry );
    }

    // Save last used directory
    QString rootDir = dialog.rootDirWithSeparator();
    if ( !rootDir.isEmpty() )
    {
        QString dirToSave = rootDir;
        if ( dirToSave.endsWith( RiaFilePathTools::separator() ) ) dirToSave.chop( 1 );
        app->setLastUsedDialogDirectory( "GRID_SUMMARY_ENSEMBLE", dirToSave );
    }

    // Collect checked realization items
    RicImportGridAndSummaryEnsembleDialogResult result;
    result.ok                    = true;
    result.rootDir               = dialog.rootDirWithSeparator();
    result.pathFilter            = dialog.pathFilterWithoutRoot();
    result.groupingMode          = dialog.ensembleGroupingMode();
    result.createGridEnsemble    = dialog.m_createGridEnsembleCheckBox->isChecked();
    result.createSummaryEnsemble = dialog.m_createSummaryEnsembleCheckBox->isChecked();

    auto* rootItem = dialog.m_filePathModel.invisibleRootItem();
    for ( int i = 0; i < rootItem->rowCount(); ++i )
    {
        auto ensembleItem = rootItem->child( i );
        if ( !ensembleItem ) continue;

        for ( int j = 0; j < ensembleItem->rowCount(); ++j )
        {
            auto childItem = ensembleItem->child( j );
            if ( !childItem || !childItem->isCheckable() ) continue;
            if ( childItem->checkState() != Qt::Checked ) continue;

            QString basePath = childItem->data( Qt::UserRole ).toString();
            if ( basePath.isEmpty() ) continue;

            auto it = dialog.m_foundRealizations.find( basePath );
            if ( it == dialog.m_foundRealizations.end() ) continue;

            if ( !it->gridFile.isEmpty() ) result.gridFiles.append( it->gridFile );
            if ( !it->summaryFile.isEmpty() ) result.summaryFiles.append( it->summaryFile );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportGridAndSummaryEnsembleDialog::RicImportGridAndSummaryEnsembleDialog( QWidget* parent )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
    , m_blockItemUpdates( false )
{
    // Create widgets
    m_pathFilterField      = new QComboBox();
    m_browseButton         = new QPushButton( "..." );
    m_effectiveFilterLabel = new QLabel();
    m_effectiveFilterLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_searchButton = new QPushButton( "Search" );
    m_searchButton->setFixedWidth( 75 );

    m_useRealizationStarCheckBox = new QCheckBox( "Use 'realization-*' in filter" );
    m_useRealizationStarCheckBox->setChecked( true );
    m_ensembleGroupingMode = new QComboBox();

    m_createGridEnsembleCheckBox    = new QCheckBox( "Create Grid Ensemble" );
    m_createSummaryEnsembleCheckBox = new QCheckBox( "Create Summary Ensemble" );
    m_createGridEnsembleCheckBox->setChecked( true );
    m_createSummaryEnsembleCheckBox->setChecked( true );

    m_outputGroup        = new QGroupBox( "Files Found" );
    m_treeFilterLineEdit = new QLineEdit();
    m_treeFilterLineEdit->setPlaceholderText( "Select Realizations: 1, 5-7, !4, 9-18:3" );
    m_treeFilterButton = new QPushButton( "Apply" );
    m_fileTreeView     = new QTreeView();

    m_buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    // Tree view setup
    m_fileTreeView->setModel( &m_filePathModel );
    m_fileTreeView->setHeaderHidden( true );
    m_fileTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_fileTreeView->setContextMenuPolicy( Qt::CustomContextMenu );
    m_fileTreeView->setVisible( false );
    m_fileTreeView->setMinimumHeight( 350 );
    m_fileTreeView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_browseButton->setFixedWidth( 25 );

    // Initially hide filter widgets
    m_treeFilterLineEdit->setVisible( false );
    m_treeFilterButton->setVisible( false );

    // Connect signals
    connect( m_pathFilterField, SIGNAL( currentTextChanged( const QString& ) ), this, SLOT( slotPathFilterChanged( const QString& ) ) );
    connect( m_pathFilterField, SIGNAL( editTextChanged( const QString& ) ), this, SLOT( slotPathFilterChanged( const QString& ) ) );

    connect( m_browseButton, SIGNAL( clicked() ), this, SLOT( slotBrowseClicked() ) );
    connect( m_useRealizationStarCheckBox, SIGNAL( clicked() ), this, SLOT( slotUseRealizationStarClicked() ) );
    connect( m_searchButton, SIGNAL( clicked() ), this, SLOT( slotSearchClicked() ) );
    connect( m_treeFilterButton, SIGNAL( clicked() ), this, SLOT( slotFilterTreeViewClicked() ) );
    connect( m_treeFilterLineEdit, &QLineEdit::returnPressed, m_treeFilterButton, &QPushButton::click );
    connect( m_treeFilterLineEdit, &QLineEdit::textEdited, m_treeFilterButton, &QPushButton::click );

    connect( m_buttons, SIGNAL( accepted() ), this, SLOT( slotOkClicked() ) );
    connect( m_buttons, SIGNAL( rejected() ), this, SLOT( slotCancelClicked() ) );

    m_buttons->button( QDialogButtonBox::Ok )->setDefault( true );

    // itemChanged: propagate check state to children and upward
    QObject::connect( &m_filePathModel,
                      &QStandardItemModel::itemChanged,
                      [this]( QStandardItem* item )
                      {
                          if ( m_blockItemUpdates ) return;

                          if ( item->isCheckable() )
                          {
                              RicRecursiveFileSearchDialog::setCheckedStateChildItems( item, item->checkState() );
                          }

                          if ( item->checkState() == Qt::Checked )
                          {
                              auto parentItem = item->parent();
                              if ( parentItem )
                              {
                                  m_blockItemUpdates = true;
                                  parentItem->setCheckState( Qt::Checked );
                                  m_blockItemUpdates = false;
                              }
                          }
                      } );

    // Build layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    // Search group
    QGroupBox*   searchGroup      = new QGroupBox( "Search" );
    QGridLayout* searchGridLayout = new QGridLayout();
    int          row              = 0;

    searchGridLayout->addWidget( new QLabel( "Path pattern" ), row, 0 );
    searchGridLayout->addWidget( m_pathFilterField, row, 1, 1, 2 );
    searchGridLayout->addWidget( m_browseButton, row, 3 );

    row++;
    {
        QHBoxLayout* hLayout = new QHBoxLayout();
        hLayout->addWidget( m_useRealizationStarCheckBox );
        hLayout->addWidget( new QLabel( "Ensemble Grouping" ) );
        hLayout->addWidget( m_ensembleGroupingMode );
        hLayout->addStretch( 1 );
        searchGridLayout->addLayout( hLayout, row, 1 );
    }

    row++;
    searchGridLayout->addWidget( new QLabel( "Effective filter" ), row, 0 );
    searchGridLayout->addWidget( m_effectiveFilterLabel, row, 1, 1, 2 );
    searchGridLayout->addWidget( m_searchButton, row, 3 );

    searchGroup->setLayout( searchGridLayout );

    // Import group
    QGroupBox*   importGroup  = new QGroupBox( "Import" );
    QHBoxLayout* importLayout = new QHBoxLayout();
    importLayout->addWidget( m_createGridEnsembleCheckBox );
    importLayout->addWidget( m_createSummaryEnsembleCheckBox );
    importLayout->addStretch( 1 );
    importGroup->setLayout( importLayout );

    // Files Found group
    QGridLayout* outputGridLayout = new QGridLayout();
    outputGridLayout->addWidget( m_treeFilterLineEdit, 0, 0 );
    outputGridLayout->addWidget( m_treeFilterButton, 0, 1 );
    outputGridLayout->addWidget( m_fileTreeView, 1, 0, 1, 2 );
    outputGridLayout->setRowStretch( 0, 0 );
    outputGridLayout->setRowStretch( 1, 1 );
    m_outputGroup->setLayout( outputGridLayout );

    dialogLayout->addWidget( searchGroup, 0 );
    dialogLayout->addWidget( importGroup, 0 );
    dialogLayout->addWidget( m_outputGroup, 1 );
    dialogLayout->addWidget( m_buttons, 0 );

    setLayout( dialogLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportGridAndSummaryEnsembleDialog::cleanPathFilter() const
{
    QString pathFilterText = m_pathFilterField->currentText().trimmed();
    pathFilterText         = RiaFilePathTools::toInternalSeparator( pathFilterText );
    pathFilterText         = RiaFilePathTools::removeDuplicatePathSeparators( pathFilterText );
    pathFilterText.replace( QString( "**" ), QString( "*" ) );

    if ( m_useRealizationStarCheckBox->isChecked() )
    {
        const QString      pattern = "realization-\\d+";
        QRegularExpression regexp( pattern, QRegularExpression::CaseInsensitiveOption );
        pathFilterText.replace( regexp, "realization-*" );
    }

    return pathFilterText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportGridAndSummaryEnsembleDialog::rootDirWithSeparator() const
{
    QString rootDir = RiaFilePathTools::rootSearchPathFromSearchFilter( cleanPathFilter() );
    return RiaFilePathTools::appendSeparatorIfNo( rootDir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportGridAndSummaryEnsembleDialog::pathFilterWithoutRoot() const
{
    QString pathFilter = cleanPathFilter();
    QString rootDir    = RiaFilePathTools::rootSearchPathFromSearchFilter( pathFilter );

    pathFilter.remove( 0, rootDir.size() );
    if ( pathFilter.startsWith( RiaFilePathTools::separator() ) ) pathFilter.remove( 0, 1 );
    return pathFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::updateEffectiveFilter()
{
    QString pathFilter = pathFilterWithoutRoot();
    if ( pathFilter == "*" || pathFilter.endsWith( QString( RiaFilePathTools::separator() ) + "*" ) )
    {
        pathFilter.chop( 1 );
        pathFilter = pathFilter + "...";
    }

    QString effFilter = QString( "%1%2/*.EGRID|SMSPEC|ESMRY" ).arg( rootDirWithSeparator() ).arg( pathFilter );
    effFilter         = RiaFilePathTools::removeDuplicatePathSeparators( effFilter );

    m_effectiveFilterLabel->setText( QDir::toNativeSeparators( effFilter ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::setOkButtonEnabled( bool enabled )
{
    m_buttons->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportGridAndSummaryEnsembleDialog::findMatchingFiles( const QStringList& extensions )
{
    if ( cleanPathFilter().isEmpty() ) return {};

    QString rootDir    = rootDirWithSeparator();
    QString pathFilter = pathFilterWithoutRoot();
    if ( rootDir.size() > 1 && rootDir.endsWith( RiaFilePathTools::separator() ) ) rootDir.chop( 1 );

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( rootDir, pathFilter, matchingFolders );

    QStringList nameFilters;
    for ( const auto& ext : extensions )
    {
        nameFilters.append( "*." + ext );
    }

    return RiaFileSearchTools::findFilesInFolders( matchingFolders, nameFilters );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::updateFileListWidget()
{
    m_filePathModel.clear();
    m_foundRealizations.clear();

    QStringList gridFiles    = findMatchingFiles( { "EGRID" } );
    QStringList summaryFiles = findMatchingFiles( { "SMSPEC", "ESMRY" } );

    // Sort so ESMRY comes before SMSPEC (alphabetically E < S)
    std::sort( summaryFiles.begin(), summaryFiles.end() );

    QString rootDir = rootDirWithSeparator();
    int     rootLen = rootDir.size();

    // Build m_foundRealizations map using relative base path (no extension) as key
    for ( const auto& filePath : gridFiles )
    {
        QString   rel = filePath.mid( rootLen );
        QFileInfo fi( rel );
        QString   basePath = QDir::cleanPath( fi.path() + "/" + fi.completeBaseName() );
        if ( !m_foundRealizations.contains( basePath ) )
        {
            m_foundRealizations[basePath].gridFile = filePath;
        }
        else if ( m_foundRealizations[basePath].gridFile.isEmpty() )
        {
            m_foundRealizations[basePath].gridFile = filePath;
        }
    }

    for ( const auto& filePath : summaryFiles )
    {
        QString   rel = filePath.mid( rootLen );
        QFileInfo fi( rel );
        QString   basePath = QDir::cleanPath( fi.path() + "/" + fi.completeBaseName() );
        if ( !m_foundRealizations.contains( basePath ) )
        {
            m_foundRealizations[basePath].summaryFile = filePath;
        }
        else if ( m_foundRealizations[basePath].summaryFile.isEmpty() )
        {
            // Only store first match per base (ESMRY wins since it sorts before SMSPEC alphabetically)
            m_foundRealizations[basePath].summaryFile = filePath;
        }
    }

    if ( m_foundRealizations.isEmpty() ) return;

    auto mode = ensembleGroupingMode();

    auto ensembleLabel = []( const QString& name, int gridCount, int summaryCount )
    { return QString( "%1 (%2 grids, %3 summary cases)" ).arg( name ).arg( gridCount ).arg( summaryCount ); };

    if ( mode == RiaDefines::EnsembleGroupingMode::NONE )
    {
        // Single "Files" ensemble item, one child per base path
        auto rootItem     = m_filePathModel.invisibleRootItem();
        auto ensembleItem = new QStandardItem();
        ensembleItem->setCheckable( true );
        ensembleItem->setCheckState( Qt::Checked );
        rootItem->appendRow( ensembleItem );

        int gridCount = 0, summaryCount = 0;
        for ( auto it = m_foundRealizations.begin(); it != m_foundRealizations.end(); ++it )
        {
            if ( !it.value().gridFile.isEmpty() ) gridCount++;
            if ( !it.value().summaryFile.isEmpty() ) summaryCount++;

            auto childItem = new QStandardItem( QDir::toNativeSeparators( it.key() ) );
            childItem->setCheckable( true );
            childItem->setCheckState( Qt::Checked );
            childItem->setData( it.key(), Qt::UserRole );
            ensembleItem->appendRow( childItem );
        }

        ensembleItem->setText( ensembleLabel( "Files", gridCount, summaryCount ) );
    }
    else
    {
        // Build list of representative files for grouping (grid if available, else summary)
        QStringList            representativeFiles;
        QMap<QString, QString> repFileToBasePath;
        for ( auto it = m_foundRealizations.begin(); it != m_foundRealizations.end(); ++it )
        {
            QString rep = it.value().gridFile.isEmpty() ? it.value().summaryFile : it.value().gridFile;
            if ( !rep.isEmpty() )
            {
                representativeFiles.append( rep );
                repFileToBasePath[rep] = it.key();
            }
        }

        // Sort numerically
        QCollator collator;
        collator.setNumericMode( true );
        std::sort( representativeFiles.begin(), representativeFiles.end(), collator );

        auto grouping = RiaEnsembleNameTools::groupFilesByEnsembleName( representativeFiles, mode );

        auto rootItem = m_filePathModel.invisibleRootItem();
        for ( const auto& [groupName, groupFiles] : grouping )
        {
            auto ensembleItem = new QStandardItem();
            ensembleItem->setCheckable( true );
            ensembleItem->setCheckState( Qt::Checked );
            rootItem->appendRow( ensembleItem );

            int gridCount = 0, summaryCount = 0;
            for ( const auto& repFile : groupFiles )
            {
                QString basePath = repFileToBasePath.value( repFile );
                if ( basePath.isEmpty() ) continue;

                auto it = m_foundRealizations.find( basePath );
                if ( it != m_foundRealizations.end() )
                {
                    if ( !it.value().gridFile.isEmpty() ) gridCount++;
                    if ( !it.value().summaryFile.isEmpty() ) summaryCount++;
                }

                auto childItem = new QStandardItem( QDir::toNativeSeparators( basePath ) );
                childItem->setCheckable( true );
                childItem->setCheckState( Qt::Checked );
                childItem->setData( basePath, Qt::UserRole );
                ensembleItem->appendRow( childItem );
            }

            ensembleItem->setText( ensembleLabel( QDir::toNativeSeparators( groupName ), gridCount, summaryCount ) );
        }
    }

    // Expand first item
    if ( m_filePathModel.rowCount() > 0 )
    {
        QModelIndex index = m_filePathModel.index( 0, 0 );
        m_fileTreeView->expand( index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::clearFileList()
{
    m_foundRealizations.clear();
    m_filePathModel.clear();
    m_outputGroup->setTitle( "Files Found" );
    setOkButtonEnabled( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EnsembleGroupingMode RicImportGridAndSummaryEnsembleDialog::ensembleGroupingMode() const
{
    if ( m_ensembleGroupingMode->currentIndex() == 0 ) return RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE;
    if ( m_ensembleGroupingMode->currentIndex() == 1 ) return RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE;
    if ( m_ensembleGroupingMode->currentIndex() == 2 ) return RiaDefines::EnsembleGroupingMode::NONE;
    if ( m_ensembleGroupingMode->currentIndex() == 3 ) return RiaDefines::EnsembleGroupingMode::RESINSIGHT_OPMFLOW_STRUCTURE;
    return RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::slotPathFilterChanged( const QString& /*text*/ )
{
    updateEffectiveFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::slotBrowseClicked()
{
    QString folder = RiuFileDialogTools::getExistingDirectory( this, "Select folder", rootDirWithSeparator() );
    if ( folder.isEmpty() ) return;
    RiaFilePathTools::appendSeparatorIfNo( folder );
    folder += "*";
    m_pathFilterField->addItem( QDir::toNativeSeparators( folder ) );
    m_pathFilterField->setCurrentText( QDir::toNativeSeparators( folder ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::slotUseRealizationStarClicked()
{
    updateEffectiveFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/// Strip trailing realization number and separator from a filename stem, e.g.
/// "ECLIPSE_001" -> "ECLIPSE", "MODEL-3" -> "MODEL", "BASE" -> "BASE"
//--------------------------------------------------------------------------------------------------
static QString stripTrailingRealizationNumber( const QString& name )
{
    static const QRegularExpression re( "[_\\-]?\\d+$" );
    QString                         stripped = name;
    stripped.remove( re );
    return stripped;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportGridAndSummaryEnsembleDialog::checkForMultipleFilenamesAndUncheckOutliers()
{
    QStringList warnings;

    auto* rootItem = m_filePathModel.invisibleRootItem();
    for ( int i = 0; i < rootItem->rowCount(); ++i )
    {
        auto ensembleItem = rootItem->child( i );
        if ( !ensembleItem ) continue;

        // Count occurrences of each canonical filename (trailing realization number stripped)
        QMap<QString, int> canonicalCounts;
        for ( int j = 0; j < ensembleItem->rowCount(); ++j )
        {
            auto childItem = ensembleItem->child( j );
            if ( !childItem ) continue;

            QString basePath = childItem->data( Qt::UserRole ).toString();
            if ( basePath.isEmpty() ) continue;

            QString canonical = stripTrailingRealizationNumber( QFileInfo( basePath ).fileName() );
            canonicalCounts[canonical]++;
        }

        if ( canonicalCounts.size() <= 1 ) continue;

        // Find the most common canonical filename
        QString mostCommon;
        int     maxCount = 0;
        for ( auto it = canonicalCounts.constBegin(); it != canonicalCounts.constEnd(); ++it )
        {
            if ( it.value() > maxCount )
            {
                maxCount   = it.value();
                mostCommon = it.key();
            }
        }

        // Untick outliers
        QStringList outlierNames;
        m_blockItemUpdates = true;
        for ( int j = 0; j < ensembleItem->rowCount(); ++j )
        {
            auto childItem = ensembleItem->child( j );
            if ( !childItem ) continue;

            QString basePath  = childItem->data( Qt::UserRole ).toString();
            QString canonical = stripTrailingRealizationNumber( QFileInfo( basePath ).fileName() );
            if ( canonical != mostCommon )
            {
                childItem->setCheckState( Qt::Unchecked );
                if ( !outlierNames.contains( canonical ) ) outlierNames.append( canonical );
            }
        }
        m_blockItemUpdates = false;

        QStringList allNames = canonicalCounts.keys();
        warnings.append( QString( "Ensemble '%1' contains multiple filenames: '%2'.\n"
                                  "Only '%3' (%4 realizations) is selected. Deselected: '%5'." )
                             .arg( ensembleItem->text() )
                             .arg( allNames.join( ", " ) )
                             .arg( mostCommon )
                             .arg( maxCount )
                             .arg( outlierNames.join( ", " ) ) );
    }

    return warnings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::slotSearchClicked()
{
    clearFileList();

    m_treeFilterLineEdit->setVisible( true );
    m_treeFilterButton->setVisible( true );

    if ( !m_fileTreeView->isVisible() )
    {
        m_fileTreeView->setVisible( true );
        if ( height() < 550 ) resize( width(), 550 );
    }

    updateFileListWidget();

    int realizationCount = m_foundRealizations.size();
    if ( realizationCount > 0 )
    {
        m_outputGroup->setTitle( QString( "Files Found (%1)" ).arg( realizationCount ) );
        setOkButtonEnabled( true );
        m_buttons->button( QDialogButtonBox::Ok )->setFocus();
    }
    else
    {
        m_outputGroup->setTitle( "Files Found" );

        // Add "No files found" status item
        auto rootItem   = m_filePathModel.invisibleRootItem();
        auto statusItem = new QStandardItem( "No files found" );
        rootItem->appendRow( statusItem );
    }

    QStringList warnings = checkForMultipleFilenamesAndUncheckOutliers();
    if ( !warnings.isEmpty() )
    {
        QMessageBox msgBox( QMessageBox::Warning, "Multiple Filenames Found", warnings.join( "\n\n" ), QMessageBox::Ok, this );
        // Force the dialog wide enough so LF-separated lines are not word-wrapped
        auto* layout = qobject_cast<QGridLayout*>( msgBox.layout() );
        if ( layout )
        {
            auto* spacer = new QSpacerItem( 600, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
            layout->addItem( spacer, layout->rowCount(), 0, 1, layout->columnCount() );
        }
        msgBox.exec();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::slotFilterTreeViewClicked()
{
    QString filterText = m_treeFilterLineEdit->text();
    auto    values     = RiaStdStringTools::valuesFromRangeSelection( filterText.toStdString() );

    auto items = RicRecursiveFileSearchDialog::firstLevelItems( m_filePathModel.invisibleRootItem() );
    for ( auto item : items )
    {
        if ( item->checkState() == Qt::Unchecked ) continue;

        if ( filterText.isEmpty() )
        {
            RicRecursiveFileSearchDialog::setCheckedStateChildItems( item, Qt::Checked );
        }
        else
        {
            RicRecursiveFileSearchDialog::setCheckedStateChildItems( item, Qt::Unchecked );

            for ( auto val : values )
            {
                QString searchString = "realization-" + QString::number( val );

                QList<QStandardItem*> matchingItems;
                RicRecursiveFileSearchDialog::findItemsMatching( item, searchString, matchingItems );
                for ( auto matchedItem : matchingItems )
                {
                    matchedItem->setCheckState( Qt::Checked );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::slotOkClicked()
{
    accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::slotCancelClicked()
{
    reject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridAndSummaryEnsembleDialog::showEvent( QShowEvent* event )
{
    m_searchButton->setFocus();
    QDialog::showEvent( event );
}
