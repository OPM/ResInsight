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

#include "RiaEnsembleNameTools.h"
#include "RiaFilePathTools.h"
#include "RiaFileSearchTools.h"
#include "RiaGuiApplication.h"
#include "RiaStdStringTools.h"
#include "RiaStringListSerializer.h"

#include "RiuFileDialogTools.h"
#include "RiuTools.h"

#include <QAbstractItemView>
#include <QAction>
#include <QCheckBox>
#include <QClipboard>
#include <QCollator>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalBlocker>
#include <QTextEdit>
#include <QTime>
#include <QToolBar>
#include <QToolTip>
#include <QTreeView>
#include <QVBoxLayout>

#include <algorithm>
#include <vector>

// Anonymous namespace for internal functions
namespace
{

// Get all first level items, usually the ensemble items
auto firstLevelItems = []( QStandardItem* rootItem ) -> QList<QStandardItem*>
{
    QList<QStandardItem*> firstLevelItems;

    for ( int i = 0; i < rootItem->rowCount(); ++i )
    {
        QStandardItem* item = rootItem->child( i );
        if ( item )
        {
            firstLevelItems.append( item );
        }
    }

    return firstLevelItems;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void setCheckedStateChildItems( QStandardItem* parentItem, Qt::CheckState checkState )
{
    if ( !parentItem ) return;

    for ( int i = 0; i < parentItem->rowCount(); ++i )
    {
        auto childItem = parentItem->child( i );
        if ( childItem && childItem->isCheckable() )
        {
            childItem->setCheckState( checkState );
        }

        setCheckedStateChildItems( childItem, checkState );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void findItemsMatching( QStandardItem* parentItem, const QString& substring, QList<QStandardItem*>& matchingItems )
{
    if ( !parentItem ) return;

    for ( int i = 0; i < parentItem->rowCount(); ++i )
    {
        auto searchString = substring + "/";

        auto childItem = parentItem->child( i );
        if ( childItem )
        {
            auto textToMatch = childItem->text();
            textToMatch.replace( '\\', '/' );

            if ( childItem && textToMatch.contains( searchString, Qt::CaseInsensitive ) )
            {
                matchingItems.append( childItem );
            }
        }

        findItemsMatching( childItem, substring, matchingItems );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void getTextForCheckedItems( QStandardItem* root, QStandardItem* parentItem, const QString& prefix, QStringList& checkedItems )
{
    if ( !parentItem ) return;

    for ( int i = 0; i < parentItem->rowCount(); ++i )
    {
        auto childItem = parentItem->child( i );
        if ( childItem && childItem->isCheckable() && childItem->checkState() == Qt::Checked )
        {
            if ( parentItem != root )
            {
                checkedItems.append( prefix + childItem->text() );
            }

            getTextForCheckedItems( root, childItem, prefix, checkedItems );
        }
    }
}

int defaultDialogHeight()
{
    return 550;
}

QString findButtonText()
{
    return "Search";
}

QString filesFoundText()
{
    return "File Selection";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void sortStringsByLength( QStringList& strings, bool ascending )
{
    QStringList sorted   = strings;
    int         numItems = sorted.size();
    bool        swapped;
    do
    {
        swapped = false;
        for ( int i = 0; i < numItems - 1; i++ )
        {
            int s0 = strings[i].size();
            int s1 = strings[i + 1].size();
            if ( ( ascending && s0 > s1 ) || ( !ascending && s0 < s1 ) )
            {
                const QString temp = strings[i];
                strings[i]         = strings[i + 1];
                strings[i + 1]     = temp;
                swapped            = true;
            }
        }
    } while ( swapped );
}

} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialogResult RicRecursiveFileSearchDialog::runRecursiveSearchDialog( QWidget*                     parent,
                                                                                           const QString&               caption,
                                                                                           const QString&               dir,
                                                                                           const QString&               pathFilter,
                                                                                           const QString&               fileNameFilter,
                                                                                           const std::vector<FileType>& fileTypes )
{
    const QString filePathRegistryKey   = QString( "RicRecursiveFileSearchDialog %1" ).arg( caption ).replace( " ", "_" );
    const QString fileFilterRegistryKey = QString( "RicRecursiveFileSearchDialog file filter %1" ).arg( caption ).replace( " ", "_" );
    const QString useRealizationStarRegistryKey = "RecursiveFileSearchDialog_use_realization";
    QSettings     settings;

    RicRecursiveFileSearchDialog dialog( parent, fileTypes );
    {
        QSignalBlocker signalBlocker( dialog.m_pathFilterField );
        QSignalBlocker signalBlocker2( dialog.m_ensembleGroupingMode );

        dialog.setWindowTitle( caption );

        QString pathFilterText = dir;
        RiaFilePathTools::appendSeparatorIfNo( pathFilterText );
        pathFilterText += pathFilter;
        dialog.m_fileFilterField->addItem( fileNameFilter );
        dialog.m_pathFilterField->addItem( QDir::toNativeSeparators( pathFilterText ) );

        for ( const auto& fileType : fileTypes )
        {
            QString item = QString( "%1 (%2)" ).arg( fileNameForType( fileType ) ).arg( fileExtensionForType( fileType ) );
            dialog.m_fileTypeField->addItem( item, static_cast<int>( fileType ) );
        }

        dialog.m_fileFilterField->addItem( fileNameFilter );

        populateComboBoxHistoryFromRegistry( dialog.m_pathFilterField, filePathRegistryKey );

        populateComboBoxHistoryFromRegistry( dialog.m_fileFilterField, fileFilterRegistryKey );

        bool isChecked = settings.value( useRealizationStarRegistryKey, true ).toBool();
        dialog.m_useRealizationStarCheckBox->setChecked( isChecked );

        dialog.m_fileFilterField->setCurrentText( fileNameFilter );
        dialog.m_fileFilterField->setEditable( true );

        dialog.m_pathFilterField->setCurrentText( QDir::toNativeSeparators( pathFilterText ) );
        dialog.m_pathFilterField->setEditable( true );

        if ( !fileTypes.empty() )
        {
            dialog.m_fileExtensions = QStringList( fileExtensionForType( fileTypes.front() ) );
            dialog.m_fileType       = fileTypes.front();
        }

        for ( const auto& s : caf::AppEnum<RiaDefines::EnsembleGroupingMode>::uiTexts() )
        {
            dialog.m_ensembleGroupingMode->addItem( s );
        }

        dialog.updateEffectiveFilter();
        dialog.clearFileList();
        dialog.setOkButtonEnabled( false );

        dialog.resize( 800, 150 );
    }

    dialog.exec();

    if ( dialog.result() == QDialog::Accepted )
    {
        settings.setValue( useRealizationStarRegistryKey, dialog.m_useRealizationStarCheckBox->isChecked() );

        const int maxItemsInRegistry = 10;

        {
            RiaStringListSerializer stringListSerializer( filePathRegistryKey );
            stringListSerializer.addString( dialog.m_pathFilterField->currentText(), maxItemsInRegistry );
        }
        {
            RiaStringListSerializer stringListSerializer( fileFilterRegistryKey );
            stringListSerializer.addString( dialog.m_fileFilterField->currentText(), maxItemsInRegistry );
        }

        return { .ok             = dialog.result() == QDialog::Accepted,
                 .files          = dialog.m_foundFiles,
                 .rootDir        = dialog.rootDirWithEndSeparator(),
                 .pathFilter     = dialog.pathFilterWithoutStartSeparator(),
                 .fileNameFilter = dialog.fileNameFilter(),
                 .fileType       = dialog.fileType(),
                 .groupingMode   = dialog.ensembleGroupingMode() };
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialog::RicRecursiveFileSearchDialog( QWidget* parent, const std::vector<FileType>& fileTypes )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
    , m_incomingFileTypes( fileTypes )
    , m_blockUpdateOfOtherItems( false )
{
    // Create widgets
    m_browseButton = new QPushButton();

    m_useRealizationStarCheckBox = new QCheckBox( "Use 'realization-*' in filter" );
    connect( m_useRealizationStarCheckBox, SIGNAL( clicked() ), this, SLOT( slotUseRealizationStarClicked() ) );

    m_pathFilterLabel             = new QLabel();
    m_pathFilterField             = new QComboBox();
    m_fileFilterLabel             = new QLabel();
    m_fileFilterField             = new QComboBox();
    m_fileTypeLabel               = new QLabel();
    m_fileTypeField               = new QComboBox();
    m_fileExtensionLabel          = new QLabel();
    m_fileExtensionField          = new QLineEdit();
    m_effectiveFilterLabel        = new QLabel();
    m_effectiveFilterContentLabel = new QLabel();
    m_ensembleGroupingMode        = new QComboBox();
    m_findOrCancelButton          = new QPushButton();
    m_fileTreeView                = new QTreeView();
    m_treeViewFilterLabel         = new QLabel( "Selection Filter" );
    m_treeViewFilterLineEdit      = new QLineEdit();
    m_treeViewFilterButton        = new QPushButton( "Apply" );

    m_buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    // Connect to signals
    connect( m_pathFilterField, SIGNAL( currentTextChanged( const QString& ) ), this, SLOT( slotPathFilterChanged( const QString& ) ) );
    connect( m_pathFilterField, SIGNAL( editTextChanged( const QString& ) ), this, SLOT( slotPathFilterChanged( const QString& ) ) );

    connect( m_fileFilterField, SIGNAL( currentTextChanged( const QString& ) ), this, SLOT( slotFileFilterChanged( const QString& ) ) );
    connect( m_fileFilterField, SIGNAL( editTextChanged( const QString& ) ), this, SLOT( slotFileFilterChanged( const QString& ) ) );

    connect( m_fileTypeField, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotFileTypeChanged( int ) ) );

    connect( m_fileExtensionField, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFileExtensionChanged( const QString& ) ) );

    connect( m_fileTreeView,
             SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this,
             SLOT( slotFileListCustomMenuRequested( const QPoint& ) ) );

    connect( m_treeViewFilterButton, SIGNAL( clicked() ), this, SLOT( slotFilterTreeViewClicked() ) );
    connect( m_treeViewFilterLineEdit, &QLineEdit::returnPressed, m_treeViewFilterButton, &QPushButton::click );
    connect( m_treeViewFilterLineEdit, &QLineEdit::textEdited, m_treeViewFilterButton, &QPushButton::click );

    m_treeViewFilterLineEdit->setPlaceholderText( "Select Realizations: 1, 5-7, 9-18:3" );

    m_treeViewFilterLabel->setVisible( false );
    m_treeViewFilterButton->setVisible( false );
    m_treeViewFilterLineEdit->setVisible( false );

    connect( m_browseButton, SIGNAL( clicked() ), this, SLOT( slotBrowseButtonClicked() ) );

    connect( m_findOrCancelButton, SIGNAL( clicked() ), this, SLOT( slotFindOrCancelButtonClicked() ) );

    connect( m_buttons, SIGNAL( accepted() ), this, SLOT( slotDialogOkClicked() ) );
    connect( m_buttons, SIGNAL( rejected() ), this, SLOT( slotDialogCancelClicked() ) );

    m_buttons->button( QDialogButtonBox::Ok )->setDefault( true );

    // Set widget properties
    m_pathFilterLabel->setText( "Path pattern" );
    m_fileFilterLabel->setText( "File pattern" );
    m_fileTypeLabel->setText( "File type" );
    m_fileExtensionLabel->setText( "File extension" );
    m_effectiveFilterLabel->setText( "Effective filter" );

    m_effectiveFilterContentLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

    QObject::connect( &m_filePathModel,
                      &QStandardItemModel::itemChanged,
                      [this]( QStandardItem* item )
                      {
                          if ( m_blockUpdateOfOtherItems ) return;

                          if ( item->isCheckable() )
                          {
                              setCheckedStateChildItems( item, item->checkState() );
                          }

                          if ( item->checkState() == Qt::Checked )
                          {
                              auto parent = item->parent();
                              if ( parent )
                              {
                                  m_blockUpdateOfOtherItems = true;
                                  parent->setCheckState( Qt::Checked );
                                  m_blockUpdateOfOtherItems = false;
                              }
                          }
                      } );

    m_fileTreeView->setModel( &m_filePathModel );
    m_fileTreeView->setHeaderHidden( true );
    m_fileTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_fileTreeView->setContextMenuPolicy( Qt::CustomContextMenu );
    m_fileTreeView->setVisible( false );
    m_fileTreeView->setMinimumHeight( 350 );

    m_browseButton->setText( "..." );
    m_browseButton->setFixedWidth( 25 );

    m_findOrCancelButton->setText( findButtonText() );
    m_findOrCancelButton->setFixedWidth( 75 );

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QGroupBox*   inputGroup      = new QGroupBox( "Search" );
    QGridLayout* inputGridLayout = new QGridLayout();
    int          row             = 0;
    inputGridLayout->addWidget( m_pathFilterLabel, row, 0 );
    inputGridLayout->addWidget( m_pathFilterField, row, 1, 1, 2 );
    inputGridLayout->addWidget( m_browseButton, row, 3 );

    row++;
    inputGridLayout->addWidget( m_fileFilterLabel, row, 0 );
    inputGridLayout->addWidget( m_fileFilterField, row, 1, 1, 2 );

    row++;
    inputGridLayout->addWidget( m_fileTypeLabel, row, 0 );
    inputGridLayout->addWidget( m_fileTypeField, row, 1, 1, 2 );

    row++;
    inputGridLayout->addWidget( m_fileExtensionLabel, row, 0 );
    inputGridLayout->addWidget( m_fileExtensionField, row, 1, 1, 2 );

    row++;
    {
        QHBoxLayout* horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget( m_useRealizationStarCheckBox );
        QLabel* ensembleGroupingLabel = new QLabel( "Ensemble Grouping" );
        horizontalLayout->addWidget( ensembleGroupingLabel );
        horizontalLayout->addWidget( m_ensembleGroupingMode );
        horizontalLayout->addStretch( 1 );
        inputGridLayout->addLayout( horizontalLayout, row, 1 );
    }

    row++;
    inputGridLayout->addWidget( m_effectiveFilterLabel, row, 0 );
    inputGridLayout->addWidget( m_effectiveFilterContentLabel, row, 1 );
    inputGridLayout->addWidget( m_findOrCancelButton, row, 2 );

    inputGroup->setLayout( inputGridLayout );

    m_outputGroup                 = new QGroupBox( "Files Found" );
    QGridLayout* outputGridLayout = new QGridLayout();

    outputGridLayout->addWidget( m_treeViewFilterLabel, 1, 0 );
    outputGridLayout->addWidget( m_treeViewFilterLineEdit, 1, 1 );
    outputGridLayout->addWidget( m_treeViewFilterButton, 1, 2 );

    outputGridLayout->addWidget( m_fileTreeView, 2, 0, 1, 3 );

    m_outputGroup->setLayout( outputGridLayout );

    dialogLayout->addWidget( inputGroup );
    dialogLayout->addWidget( m_outputGroup );
    dialogLayout->addWidget( m_buttons );

    setLayout( dialogLayout );

    {
        QString text = "The path filter uses normal wildcard file globbing, like in any unix shell. \n"
                       "When the filter ends with a single \"*\" (eg. \"/home/*\"), however, ResInsight will \n"
                       "search recursively in all subdirectories from that point.\n"
                       "This is indicated by \"...\" in the Effective Filter label below.\n"
                       "\n"
                       "An asterix \"*\" matches any number of any characters, except the path separator.\n"
                       "A question mark \"?\" matches any single character, except the path separator.\n"
                       "Square brackets \"[]\" encloses a list of characters and matches one of the enclosed characters.\n"
                       "they are also used to escape the characters *,? and []";

        // https://doc.qt.io/qt-5/qregularexpression.html#wildcardToRegularExpression

        m_pathFilterLabel->setToolTip( text );
        m_pathFilterField->setToolTip( text );
    }

    {
        QString text = "Define the extension using \".EGRID|.GRDECL\"";
        m_fileExtensionLabel->setToolTip( text );
        m_fileExtensionField->setToolTip( text );
    }

    {
        QString text = "The file filter uses normal wild cards, but is not allowed to contain path separators. ";

        m_fileFilterLabel->setToolTip( text );
        m_fileFilterField->setToolTip( text );
    }

    {
        QString text = "This label displays the complete filter that is being applied. \n"
                       "The possible \"...\" indicates a complete recursive directory search.";

        m_effectiveFilterLabel->setToolTip( text );
        m_effectiveFilterContentLabel->setToolTip( text );
    }
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
QString RicRecursiveFileSearchDialog::cleanTextFromPathFilterField() const
{
    QString pathFilterText = m_pathFilterField->currentText().trimmed();
    pathFilterText         = RiaFilePathTools::toInternalSeparator( pathFilterText );
    pathFilterText         = RiaFilePathTools::removeDuplicatePathSeparators( pathFilterText );
    pathFilterText.replace( QString( "**" ), QString( "*" ) );

    if ( m_useRealizationStarCheckBox->isChecked() )
    {
        pathFilterText = replaceWithRealizationStar( pathFilterText );
    }

    return pathFilterText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::rootDirWithEndSeparator() const
{
    QString rootDir = cleanTextFromPathFilterField();
    rootDir         = RiaFilePathTools::rootSearchPathFromSearchFilter( rootDir );
    return RiaFilePathTools::appendSeparatorIfNo( rootDir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::pathFilterWithoutStartSeparator() const
{
    QString pathFilter = cleanTextFromPathFilterField();
    QString rootDir    = RiaFilePathTools::rootSearchPathFromSearchFilter( pathFilter );

    pathFilter.remove( 0, rootDir.size() );
    if ( pathFilter.startsWith( RiaFilePathTools::separator() ) ) pathFilter.remove( 0, 1 );
    return pathFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::fileNameFilter() const
{
    return m_fileFilterField->currentText().trimmed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::fileExtensions() const
{
    QString extFromFilter = extensionFromFileNameFilter();
    if ( !extFromFilter.isEmpty() )
    {
        return QStringList( { extFromFilter } );
    }

    QStringList exts = m_fileExtensions;
    sortStringsByLength( exts, true );
    return exts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialog::FileType RicRecursiveFileSearchDialog::fileType() const
{
    return m_fileType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::extensionFromFileNameFilter() const
{
    for ( const QString& ext : m_fileExtensions )
    {
        if ( m_fileFilterField->currentText().endsWith( ext, Qt::CaseInsensitive ) )
        {
            return ext;
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::updateFileListWidget()
{
    m_filePathModel.clear();

    if ( ensembleGroupingMode() != RiaDefines::EnsembleGroupingMode::NONE )
    {
        if ( m_fileType == RicRecursiveFileSearchDialog::FileType::STIMPLAN_SUMMARY ||
             m_fileType == RicRecursiveFileSearchDialog::FileType::REVEAL_SUMMARY )
        {
            auto                           fileType          = RicRecursiveFileSearchDialog::mapSummaryFileType( m_fileType );
            std::map<QString, QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByCustomEnsemble( m_foundFiles, fileType );
            for ( auto [ensembleName, groupedFileNames] : groupedByEnsemble )
            {
                addToTreeView( ensembleName, groupedFileNames );
            }
        }
        else
        {
            auto grouping = RiaEnsembleNameTools::groupFilesByEnsembleName( m_foundFiles, ensembleGroupingMode() );
            for ( const auto& [groupName, fileNames] : grouping )
            {
                addToTreeView( groupName, fileNames );
            }
        }
    }
    else
    {
        const auto groupName = "Files";
        addToTreeView( groupName, m_foundFiles );
    }

    QModelIndex index = m_filePathModel.index( 0, 0 );
    m_fileTreeView->expand( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::addToTreeView( const QString& ensembleName, const QStringList& fileNames )
{
    auto rootItem             = m_filePathModel.invisibleRootItem();
    int  rootSearchPathLength = rootDirWithEndSeparator().size();

    bool isEmpty = m_filePathModel.rowCount() == 0;

    auto ensembleItem = new QStandardItem( QDir::toNativeSeparators( ensembleName ) );
    ensembleItem->setCheckable( true );
    if ( isEmpty ) ensembleItem->setCheckState( Qt::Checked );
    rootItem->appendRow( ensembleItem );

    for ( const auto& fileName : fileNames )
    {
        auto itemText = fileName;
        itemText.remove( 0, rootSearchPathLength );

        auto fileItem = new QStandardItem( QDir::toNativeSeparators( itemText ) );
        fileItem->setCheckable( true );
        if ( isEmpty ) fileItem->setCheckState( Qt::Checked );
        ensembleItem->appendRow( fileItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::clearFileList()
{
    m_foundFiles.clear();
    m_filePathModel.clear();
    m_outputGroup->setTitle( filesFoundText() );
    setOkButtonEnabled( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::updateStatus( Status status, const QString& extraText )
{
    static int   progressLoopStep = 0;
    static QTime lastStatusUpdate = QTime::currentTime();
    QTime        now              = QTime::currentTime();

    // Do not update dialog more often than twice per second to avoid text update from slowing down search progress
    if ( status != NO_FILES_FOUND && lastStatusUpdate.msecsTo( now ) < 250 ) return;

    QString newStatus;
    if ( status == SEARCHING_FOR_DIRS || status == SEARCHING_FOR_FILES )
    {
        switch ( status )
        {
            case SEARCHING_FOR_DIRS:
                newStatus = "Scanning Directories";
                break;
            case SEARCHING_FOR_FILES:
                newStatus = "Finding Files";
                break;
        }

        for ( int progress = 0; progress < progressLoopStep; ++progress )
        {
            newStatus += " .";
        }

        if ( ++progressLoopStep >= 5 ) progressLoopStep = 0;

        if ( !extraText.isEmpty() ) newStatus += "\n" + extraText;
    }
    else if ( status == NO_FILES_FOUND )
    {
        newStatus = "No files found";
    }

    lastStatusUpdate = now;

    m_filePathModel.clear();

    // Show status message in the tree view
    auto rootItem     = m_filePathModel.invisibleRootItem();
    auto ensembleItem = new QStandardItem( newStatus );
    rootItem->appendRow( ensembleItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::findMatchingFiles()
{
    if ( cleanTextFromPathFilterField().isEmpty() ) return QStringList();

    auto updateDirSearchStatus = [&]( const QString& text ) -> bool
    {
        if ( m_isCancelPressed ) return false;
        updateStatus( SEARCHING_FOR_DIRS, text );
        QApplication::processEvents();
        return true;
    };

    auto updateFileSearchStatus = [&]( const QString& text ) -> bool
    {
        if ( m_isCancelPressed ) return false;
        updateStatus( SEARCHING_FOR_FILES, text );
        QApplication::processEvents();
        return true;
    };

    QString pathFilter = pathFilterWithoutStartSeparator();
    QString rootDir    = rootDirWithEndSeparator();
    if ( rootDir.size() > 1 && rootDir.endsWith( RiaFilePathTools::separator() ) ) rootDir.chop( 1 );

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( rootDir, pathFilter, matchingFolders, updateDirSearchStatus );
    if ( m_isCancelPressed ) return {};

    QStringList fileNameFilters = createFileNameFilterList();
    auto        files           = RiaFileSearchTools::findFilesInFolders( matchingFolders, fileNameFilters, updateFileSearchStatus );
    if ( m_isCancelPressed ) return {};

    return files;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::createFileNameFilterList()
{
    QString     fileNameFilter = this->fileNameFilter();
    QStringList fileExtensions = this->fileExtensions();

    QStringList nameFilter;
    QString     effectiveFileNameFilter = !fileNameFilter.isEmpty() ? fileNameFilter : "*";

    if ( fileExtensions.empty() || !extensionFromFileNameFilter().isEmpty() )
    {
        nameFilter.append( effectiveFileNameFilter );
    }
    else
    {
        for ( QString fileExtension : fileExtensions )
        {
            nameFilter.append( effectiveFileNameFilter + "." + fileExtension );
        }
    }
    return nameFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::replaceWithRealizationStar( const QString& text )
{
    const QString      pattern = "realization-\\d+";
    QRegularExpression regexp( pattern, QRegularExpression::CaseInsensitiveOption );

    QString textWithStar = text;
    textWithStar.replace( regexp, "realization-*" );

    return textWithStar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::populateComboBoxHistoryFromRegistry( QComboBox* comboBox, const QString& registryKey )
{
    RiaStringListSerializer stringListSerializer( registryKey );
    QStringList             files = stringListSerializer.textStrings();

    const int maxItemsInRegistry = 10;

    int numRecentFiles = std::min( (int)files.size(), maxItemsInRegistry );
    for ( int i = 0; i < numRecentFiles; i++ )
    {
        comboBox->addItem( files[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::updateEffectiveFilter()
{
    QString pathFilterText = pathFilterWithoutStartSeparator();
    if ( pathFilterText == "*" || pathFilterText.endsWith( QString( RiaFilePathTools::separator() ) + "*" ) )
    {
        pathFilterText.chop( 1 );
        pathFilterText = pathFilterText + "...";
    }

    QString effFilterText =
        QString( "%1%2/%3" ).arg( rootDirWithEndSeparator() ).arg( pathFilterText ).arg( createFileNameFilterList().join( "|" ) );

    effFilterText = RiaFilePathTools::removeDuplicatePathSeparators( effFilterText );

    // Present native separators to the user
    m_effectiveFilterContentLabel->setText( QDir::toNativeSeparators( effFilterText ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::setOkButtonEnabled( bool enabled )
{
    m_buttons->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::warningIfInvalidCharacters()
{
    if ( fileNameFilter().contains( QRegularExpression( "[\\\\/:]" ) ) )
    {
        QToolTip::showText( m_fileFilterField->mapToGlobal( QPoint( 0, 0 ) ), "File pattern contains invalid characters" );
        m_effectiveFilterContentLabel->setText( "(Invalid filter)" );
    }
    else
    {
        QToolTip::hideText();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotPathFilterChanged( const QString& text )
{
    updateEffectiveFilter();
    warningIfInvalidCharacters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFileFilterChanged( const QString& text )
{
    clearFileList();
    updateEffectiveFilter();
    warningIfInvalidCharacters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFileTypeChanged( int index )
{
    m_fileType = static_cast<FileType>( m_fileTypeField->itemData( index ).toInt() );

    QString extension = fileExtensionForType( m_fileType );
    m_fileExtensions  = QStringList( extension );

    m_fileExtensionField->setText( extension );

    updateEffectiveFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFileExtensionChanged( const QString& text )
{
    m_fileExtensions = QStringList( text );
    updateEffectiveFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFileListCustomMenuRequested( const QPoint& point )
{
    QMenu    menu;
    QAction* action;

    action = new QAction( QIcon( ":/Copy.svg" ), "&Copy", this );
    connect( action, SIGNAL( triggered() ), SLOT( slotCopyFileItemText() ) );
    menu.addAction( action );
    menu.addSeparator();

    action = new QAction( "On", this );
    connect( action, SIGNAL( triggered() ), SLOT( slotTurnOnFileListItems() ) );
    menu.addAction( action );

    action = new QAction( "Off", this );
    connect( action, SIGNAL( triggered() ), SLOT( slotTurnOffFileListItems() ) );
    menu.addAction( action );

    action = new QAction( "Toggle", this );
    connect( action, SIGNAL( triggered() ), SLOT( slotToggleFileListItems() ) );
    menu.addAction( action );

    QPoint globalPoint = m_fileTreeView->mapToGlobal( point );
    menu.exec( globalPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotToggleFileListItems()
{
    auto selectionModel = m_fileTreeView->selectionModel();
    auto indices        = selectionModel->selectedIndexes();
    for ( auto& index : indices )
    {
        if ( index.isValid() )
        {
            auto item = m_filePathModel.itemFromIndex( index );
            if ( item && item->isCheckable() )
            {
                item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotTurnOffFileListItems()
{
    auto selectionModel = m_fileTreeView->selectionModel();
    auto indices        = selectionModel->selectedIndexes();
    for ( auto& index : indices )
    {
        if ( index.isValid() )
        {
            auto item = m_filePathModel.itemFromIndex( index );
            if ( item && item->isCheckable() )
            {
                item->setCheckState( Qt::Unchecked );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotTurnOnFileListItems()
{
    auto selectionModel = m_fileTreeView->selectionModel();
    auto indices        = selectionModel->selectedIndexes();
    for ( auto& index : indices )
    {
        if ( index.isValid() )
        {
            auto item = m_filePathModel.itemFromIndex( index );
            if ( item && item->isCheckable() )
            {
                item->setCheckState( Qt::Checked );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotCopyFileItemText()
{
    auto index = m_fileTreeView->currentIndex();
    {
        if ( index.isValid() )
        {
            auto item = m_filePathModel.itemFromIndex( index );
            if ( item )
            {
                QString relativePathText = item->text();
                QApplication::clipboard()->setText( relativePathText );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFindOrCancelButtonClicked()
{
    if ( m_findOrCancelButton->text() == findButtonText() )
    {
        clearFileList();

        m_treeViewFilterLabel->setVisible( true );
        m_treeViewFilterButton->setVisible( true );
        m_treeViewFilterLineEdit->setVisible( true );

        if ( !m_fileTreeView->isVisible() )
        {
            m_fileTreeView->setVisible( true );

            if ( height() < defaultDialogHeight() ) resize( width(), defaultDialogHeight() );
        }

        m_findOrCancelButton->setText( "Cancel" );

        m_isCancelPressed = false;

        QStringList candidates = findMatchingFiles();

        // Sort by numbers instead of alphabetically
        QCollator collator;
        collator.setNumericMode( true );
        std::sort( candidates.begin(), candidates.end(), collator );

        m_foundFiles = candidates;

        updateFileListWidget();

        m_findOrCancelButton->setText( findButtonText() );

        if ( m_isCancelPressed )
        {
            clearFileList();
        }
        else if ( m_foundFiles.isEmpty() )
        {
            updateStatus( NO_FILES_FOUND );
        }
        else
        {
            m_outputGroup->setTitle( QString( "%1 (%2)" ).arg( filesFoundText() ).arg( m_foundFiles.size() ) );
        }

        setOkButtonEnabled( !m_foundFiles.isEmpty() );

        m_buttons->button( QDialogButtonBox::Ok )->setFocus();

        slotFilterTreeViewClicked();
    }
    else
    {
        m_isCancelPressed = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFilterTreeViewClicked()
{
    QString filterText = m_treeViewFilterLineEdit->text();

    auto values = RiaStdStringTools::valuesFromRangeSelection( filterText.toStdString() );

    auto items = firstLevelItems( m_filePathModel.invisibleRootItem() );
    for ( auto item : items )
    {
        if ( item->checkState() == Qt::Unchecked ) continue;

        if ( filterText.isEmpty() )
        {
            setCheckedStateChildItems( item, Qt::Checked );
        }
        else
        {
            setCheckedStateChildItems( item, Qt::Unchecked );

            for ( auto val : values )
            {
                QString searchString = "realization-" + QString::number( val );

                QList<QStandardItem*> matchingItems;
                findItemsMatching( item, searchString, matchingItems );
                for ( auto item : matchingItems )
                {
                    item->setCheckState( Qt::Checked );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotDialogOkClicked()
{
    QStringList checkedItems;
    getTextForCheckedItems( m_filePathModel.invisibleRootItem(), m_filePathModel.invisibleRootItem(), rootDirWithEndSeparator(), checkedItems );

    m_foundFiles = checkedItems;
    accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotDialogCancelClicked()
{
    m_foundFiles      = QStringList();
    m_isCancelPressed = true;
    reject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::showEvent( QShowEvent* event )
{
    m_findOrCancelButton->setFocus();
    QDialog::showEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotBrowseButtonClicked()
{
    QString folder = RiuFileDialogTools::getExistingDirectory( this, "Select folder", rootDirWithEndSeparator() );
    RiaFilePathTools::appendSeparatorIfNo( folder );
    folder += "*";
    if ( !folder.isEmpty() )
    {
        m_pathFilterField->addItem( QDir::toNativeSeparators( folder ) );
        m_pathFilterField->setCurrentText( QDir::toNativeSeparators( folder ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotUseRealizationStarClicked()
{
    updateEffectiveFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EnsembleGroupingMode RicRecursiveFileSearchDialog::ensembleGroupingMode() const
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
QStringList RicRecursiveFileSearchDialog::fileTypeToExtensionStrings( const std::vector<RicRecursiveFileSearchDialog::FileType>& fileTypes )
{
    QStringList extensions;
    for ( const auto& f : fileTypes )
    {
        extensions.append( RicRecursiveFileSearchDialog::fileExtensionForType( f ) );
    }
    return extensions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::fileExtensionForType( FileType fileType )
{
    switch ( fileType )
    {
        case FileType::GRDECL:
            return "GRDECL";
        case FileType::EGRID:
            return "EGRID";
        case FileType::GRID:
            return "GRID";
        case FileType::SMSPEC:
            return "SMSPEC";
        case FileType::ESMRY:
            return "ESMRY";
        case FileType::STIMPLAN_FRACTURE:
            return "XML";
        case FileType::LAS:
            return "LAS";
        case FileType::SURFACE:
            return "TS";
        case FileType::STIMPLAN_SUMMARY:
            return "CSV";
        case FileType::REVEAL_SUMMARY:
            return "CSV";
        default:
            return "*";
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::fileNameForType( FileType fileType )
{
    switch ( fileType )
    {
        case FileType::GRDECL:
            return "Eclipse Text File";
        case FileType::EGRID:
            return "Eclipse Grid File";
        case FileType::GRID:
            return "Eclipse Grid File";
        case FileType::SMSPEC:
            return "Eclipse Summary File";
        case FileType::ESMRY:
            return "ESMRY Summary File";
        case FileType::STIMPLAN_FRACTURE:
            return "StimPlan Fracture";
        case FileType::LAS:
            return "LAS File";
        case FileType::SURFACE:
            return "Surface File";
        case FileType::STIMPLAN_SUMMARY:
            return "StimPlan Summary File";
        case FileType::REVEAL_SUMMARY:
            return "Reveal Summary File";
        default:
            return "*";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::FileType RicRecursiveFileSearchDialog::mapSummaryFileType( RicRecursiveFileSearchDialog::FileType fileType )
{
    switch ( fileType )
    {
        case RicRecursiveFileSearchDialog::FileType::SMSPEC:
        case RicRecursiveFileSearchDialog::FileType::ESMRY:
            return RiaDefines::FileType::SMSPEC;
        case RicRecursiveFileSearchDialog::FileType::REVEAL_SUMMARY:
            return RiaDefines::FileType::REVEAL_SUMMARY;
        case RicRecursiveFileSearchDialog::FileType::STIMPLAN_SUMMARY:
            return RiaDefines::FileType::STIMPLAN_SUMMARY;
        default:
        {
            return RiaDefines::FileType::SMSPEC;
        }
    }
}
