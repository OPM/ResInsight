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
#include "RiaGuiApplication.h"
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
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QTextEdit>
#include <QTime>
#include <QToolBar>
#include <QToolTip>
#include <QVBoxLayout>

#include <algorithm>
#include <vector>

#define RECURSIVE_FILESEARCH_DEFAULT_DIALOG_HEIGHT 350
#define FIND_BUTTON_FIND_TEXT "Find"
#define FILES_FOUND_TEXT "Files Found"

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
static QStringList trimLeftStrings( const QStringList& strings, const QString& trimText );
static void        sortStringsByLength( QStringList& strings, bool ascending = true );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialogResult RicRecursiveFileSearchDialog::runRecursiveSearchDialog( QWidget*       parent,
                                                                                           const QString& caption,
                                                                                           const QString& dir,
                                                                                           const QString& pathFilter,
                                                                                           const QString& fileNameFilter,
                                                                                           const QStringList& fileExtensions )
{
    const QString filePathRegistryKey = QString( "RicRecursiveFileSearchDialog %1" ).arg( caption ).replace( " ", "_" );
    const QString fileFilterRegistryKey =
        QString( "RicRecursiveFileSearchDialog file filter %1" ).arg( caption ).replace( " ", "_" );
    const QString useRealizationStarRegistryKey = "RecursiveFileSearchDialog_use_realization";
    QSettings     settings;

    RicRecursiveFileSearchDialog dialog( parent, fileExtensions );
    {
        QSignalBlocker signalBlocker( dialog.m_pathFilterField );

        dialog.setWindowTitle( caption );

        QString pathFilterText = dir;
        RiaFilePathTools::appendSeparatorIfNo( pathFilterText );
        pathFilterText += pathFilter;
        dialog.m_fileFilterField->addItem( fileNameFilter );
        dialog.m_pathFilterField->addItem( QDir::toNativeSeparators( pathFilterText ) );

        for ( const auto& s : fileExtensions )
        {
            QString joined = fileExtensions.join( '|' );
            dialog.m_fileExtensionsField->setText( joined );
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

        dialog.m_fileExtensions = trimLeftStrings( fileExtensions, "." );

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
    }

    return RicRecursiveFileSearchDialogResult( dialog.result() == QDialog::Accepted,
                                               dialog.m_foundFiles,
                                               dialog.rootDirWithEndSeparator(),
                                               dialog.pathFilterWithoutStartSeparator(),
                                               dialog.fileNameFilter(),
                                               dialog.groupByEnsemble() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicRecursiveFileSearchDialog::RicRecursiveFileSearchDialog( QWidget* parent, const QStringList& fileExtensions )
    : QDialog( parent, RiuTools::defaultDialogFlags() )
    , m_incomingFileExtensions( fileExtensions )
{
    // Create widgets
    m_browseButton = new QPushButton();

    m_useRealizationStarCheckBox = new QCheckBox( "Use 'realization-*' in filter" );
    connect( m_useRealizationStarCheckBox, SIGNAL( clicked() ), this, SLOT( slotUseRealizationStarClicked() ) );

    m_groupByEnsembleCheckBox = new QCheckBox( "Group by ensemble" );
    m_groupByEnsembleCheckBox->setChecked( true );

    m_pathFilterLabel             = new QLabel();
    m_pathFilterField             = new QComboBox();
    m_fileFilterLabel             = new QLabel();
    m_fileFilterField             = new QComboBox();
    m_fileExtensionsLabel         = new QLabel();
    m_fileExtensionsField         = new QLineEdit();
    m_effectiveFilterLabel        = new QLabel();
    m_effectiveFilterContentLabel = new QLabel();
    m_searchRootLabel             = new QLabel();
    m_searchRootContentLabel      = new QLabel();
    m_findOrCancelButton          = new QPushButton();
    m_fileListWidget              = new QListWidget();

    m_buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    // Connect to signals
    connect( m_pathFilterField,
             SIGNAL( currentTextChanged( const QString& ) ),
             this,
             SLOT( slotPathFilterChanged( const QString& ) ) );
    connect( m_pathFilterField,
             SIGNAL( editTextChanged( const QString& ) ),
             this,
             SLOT( slotPathFilterChanged( const QString& ) ) );

    connect( m_fileFilterField,
             SIGNAL( currentTextChanged( const QString& ) ),
             this,
             SLOT( slotFileFilterChanged( const QString& ) ) );
    connect( m_fileFilterField,
             SIGNAL( editTextChanged( const QString& ) ),
             this,
             SLOT( slotFileFilterChanged( const QString& ) ) );

    connect( m_fileExtensionsField, SIGNAL( editingFinished() ), this, SLOT( slotFileExtensionsChanged() ) );

    connect( m_fileListWidget,
             SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this,
             SLOT( slotFileListCustomMenuRequested( const QPoint& ) ) );

    connect( m_browseButton, SIGNAL( clicked() ), this, SLOT( slotBrowseButtonClicked() ) );

    connect( m_findOrCancelButton, SIGNAL( clicked() ), this, SLOT( slotFindOrCancelButtonClicked() ) );

    connect( m_buttons, SIGNAL( accepted() ), this, SLOT( slotDialogOkClicked() ) );
    connect( m_buttons, SIGNAL( rejected() ), this, SLOT( slotDialogCancelClicked() ) );

    // Set widget properties
    m_pathFilterLabel->setText( "Path pattern" );
    m_fileFilterLabel->setText( "File pattern" );
    m_fileExtensionsLabel->setText( "File Extensions" );
    m_effectiveFilterLabel->setText( "Effective filter" );
    m_searchRootLabel->setText( "Root" );
    m_searchRootLabel->setVisible( false );

    m_effectiveFilterContentLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_searchRootContentLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_searchRootContentLabel->setVisible( false );

    m_fileListWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_fileListWidget->setVisible( false );
    m_fileListWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    m_fileListWidget->setMinimumHeight( 350 );

    m_browseButton->setText( "..." );
    m_browseButton->setFixedWidth( 25 );

    m_findOrCancelButton->setText( FIND_BUTTON_FIND_TEXT );
    m_findOrCancelButton->setFixedWidth( 75 );
    m_findOrCancelButton->setDefault( true );

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QGroupBox*   inputGroup      = new QGroupBox( "Filter" );
    QGridLayout* inputGridLayout = new QGridLayout();
    int          row             = 0;
    inputGridLayout->addWidget( m_pathFilterLabel, row, 0 );
    inputGridLayout->addWidget( m_pathFilterField, row, 1, 1, 2 );
    inputGridLayout->addWidget( m_browseButton, row, 3 );

    row++;
    inputGridLayout->addWidget( m_fileFilterLabel, row, 0 );
    inputGridLayout->addWidget( m_fileFilterField, row, 1, 1, 2 );

    row++;
    inputGridLayout->addWidget( m_fileExtensionsLabel, row, 0 );
    inputGridLayout->addWidget( m_fileExtensionsField, row, 1, 1, 2 );

    row++;
    {
        QHBoxLayout* horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget( m_useRealizationStarCheckBox );
        horizontalLayout->addWidget( m_groupByEnsembleCheckBox );
        inputGridLayout->addLayout( horizontalLayout, row, 1 );
    }

    row++;
    inputGridLayout->addWidget( m_effectiveFilterLabel, row, 0 );
    inputGridLayout->addWidget( m_effectiveFilterContentLabel, row, 1 );
    inputGridLayout->addWidget( m_findOrCancelButton, row, 2 );

    inputGroup->setLayout( inputGridLayout );

    m_outputGroup                 = new QGroupBox( "Files Found" );
    QGridLayout* outputGridLayout = new QGridLayout();
    outputGridLayout->addWidget( m_searchRootLabel, 1, 0 );
    outputGridLayout->addWidget( m_searchRootContentLabel, 1, 1 );

    // outputGridLayout->addWidget(m_fileListLabel, 2, 0);
    outputGridLayout->addWidget( m_fileListWidget, 2, 0, 1, 3 );
    m_outputGroup->setLayout( outputGridLayout );

    dialogLayout->addWidget( inputGroup );
    dialogLayout->addWidget( m_outputGroup );
    dialogLayout->addWidget( m_buttons );

    setLayout( dialogLayout );

    {
        QString text =
            "The path filter uses normal wildcard file globbing, like in any unix shell. \n"
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
        m_fileExtensionsLabel->setToolTip( text );
        m_fileExtensionsField->setToolTip( text );
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
    QString rootDir = this->cleanTextFromPathFilterField();
    rootDir         = RiaFilePathTools::rootSearchPathFromSearchFilter( rootDir );
    return RiaFilePathTools::appendSeparatorIfNo( rootDir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicRecursiveFileSearchDialog::pathFilterWithoutStartSeparator() const
{
    QString pathFilter = this->cleanTextFromPathFilterField();
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
    sortStringsByLength( exts );
    return exts;
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
    m_fileListWidget->clear();

    if ( m_groupByEnsembleCheckBox->isChecked() )
    {
        std::vector<QStringList> groupedByEnsemble = RiaEnsembleNameTools::groupFilesByEnsemble( m_foundFiles );
        for ( const QStringList& groupedFileNames : groupedByEnsemble )
        {
            QString          ensembleName = RiaEnsembleNameTools::findSuitableEnsembleName( groupedFileNames );
            QListWidgetItem* item = new QListWidgetItem( QDir::toNativeSeparators( ensembleName ), m_fileListWidget );
            addToFileListWidget( groupedFileNames );
        }
    }
    else
    {
        addToFileListWidget( m_foundFiles );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::addToFileListWidget( const QStringList& fileNames )
{
    int rootSearchPathLength = rootDirWithEndSeparator().size();

    for ( const auto& fileName : fileNames )
    {
        QString itemText = fileName;
        itemText.remove( 0, rootSearchPathLength );
        QListWidgetItem* item = new QListWidgetItem( QDir::toNativeSeparators( itemText ), m_fileListWidget );
        item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
        item->setCheckState( Qt::Checked );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::clearFileList()
{
    m_foundFiles.clear();
    m_fileListWidget->clear();
    m_outputGroup->setTitle( FILES_FOUND_TEXT );
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

    m_fileListWidget->clear();
    new QListWidgetItem( newStatus, m_fileListWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::findMatchingFiles()
{
    if ( cleanTextFromPathFilterField().isEmpty() ) return QStringList();

    QStringList dirs;

    QString pathFilter = this->pathFilterWithoutStartSeparator();
    QString rootDir    = this->rootDirWithEndSeparator();
    if ( rootDir.size() > 1 && rootDir.endsWith( RiaFilePathTools::separator() ) ) rootDir.chop( 1 );

    buildDirectoryListRecursiveSimple( rootDir, pathFilter, &dirs );

    return findFilesInDirs( dirs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::buildDirectoryListRecursiveSimple( const QString& currentDirFullPathNoEndSeparator,
                                                                      const QString& currentPathFilterNoEndSeparator,
                                                                      QStringList*   accumulatedDirs )
{
    QString currDir    = currentDirFullPathNoEndSeparator;
    QString pathFilter = currentPathFilterNoEndSeparator;

    if ( m_isCancelPressed )
    {
        accumulatedDirs->clear();
        return;
    }

    updateStatus( SEARCHING_FOR_DIRS, currDir );
    QApplication::processEvents();

    if ( pathFilter.isEmpty() )
    {
        accumulatedDirs->push_back( currentDirFullPathNoEndSeparator );
        return;
    }

    QStringList pathFilterPartList = pathFilter.split( RiaFilePathTools::separator() );
    QDir        qdir( currDir, pathFilterPartList[0], QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot );
    QStringList subDirs = qdir.entryList();

    if ( pathFilterPartList.size() == 1 && pathFilterPartList[0] == "*" )
    {
        accumulatedDirs->push_back( currDir );
    }

    for ( const QString& subDir : subDirs )
    {
        QString fullPath = qdir.absoluteFilePath( subDir );
        QString nextPathFilter;

        if ( pathFilterPartList.size() == 1 && pathFilterPartList[0] == "*" )
        {
            nextPathFilter = "*";
        }
        else
        {
            auto pf = pathFilterPartList;
            pf.removeFirst();
            nextPathFilter = pf.join( RiaFilePathTools::separator() );
        }

        buildDirectoryListRecursiveSimple( fullPath, nextPathFilter, accumulatedDirs );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::findFilesInDirs( const QStringList& dirs )
{
    QStringList allFiles;
    QStringList filters = createFileNameFilterList();

    for ( const auto& dir : dirs )
    {
        QDir        qdir( dir );
        QStringList files = qdir.entryList( filters, QDir::Files );

        if ( m_isCancelPressed ) return QStringList();

        updateStatus( SEARCHING_FOR_FILES, qdir.absolutePath() );
        QApplication::processEvents();

        for ( QString file : files )
        {
            QString absFilePath = qdir.absoluteFilePath( file );
            allFiles.append( absFilePath );
        }
    }
    return allFiles;
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

    if ( fileExtensions.size() == 0 || !extensionFromFileNameFilter().isEmpty() )
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
    const QString pattern = "realization-\\d+";
    QRegExp       regexp( pattern, Qt::CaseInsensitive );

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

    int numRecentFiles = std::min( files.size(), maxItemsInRegistry );
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
    m_searchRootContentLabel->setText( QDir::toNativeSeparators( rootDirWithEndSeparator() ) );
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
    if ( fileNameFilter().contains( QRegExp( "[\\\\/:]" ) ) )
    {
        QToolTip::showText( m_fileFilterField->mapToGlobal( QPoint( 0, 0 ) ), "File pattern contains invalid characters" );
        m_effectiveFilterContentLabel->setText( "(Invalid filter)" );
        m_searchRootContentLabel->setText( "" );
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
    m_findOrCancelButton->setDefault( true );

    slotFindOrCancelButtonClicked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFileFilterChanged( const QString& text )
{
    clearFileList();
    updateEffectiveFilter();
    warningIfInvalidCharacters();
    m_findOrCancelButton->setDefault( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFileExtensionsChanged()
{
    QStringList items = m_fileExtensionsField->text().split( '|' );

    m_fileExtensions = trimLeftStrings( items, "." );

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

    QPoint globalPoint = m_fileListWidget->mapToGlobal( point );
    menu.exec( globalPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotToggleFileListItems()
{
    for ( auto& item : m_fileListWidget->selectedItems() )
    {
        if ( ( item->flags() & Qt::ItemIsUserCheckable ) != 0 )
        {
            item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotTurnOffFileListItems()
{
    for ( auto& item : m_fileListWidget->selectedItems() )
    {
        if ( ( item->flags() & Qt::ItemIsUserCheckable ) != 0 )
        {
            item->setCheckState( Qt::Unchecked );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotTurnOnFileListItems()
{
    for ( auto& item : m_fileListWidget->selectedItems() )
    {
        if ( ( item->flags() & Qt::ItemIsUserCheckable ) != 0 )
        {
            item->setCheckState( Qt::Checked );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotCopyFileItemText()
{
    if ( m_fileListWidget->currentItem() )
    {
        QString relativePathText = m_fileListWidget->currentItem()->text();
        RiaGuiApplication::instance()->clipboard()->setText( relativePathText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotFindOrCancelButtonClicked()
{
    if ( m_findOrCancelButton->text() == FIND_BUTTON_FIND_TEXT )
    {
        clearFileList();

        if ( !m_fileListWidget->isVisible() )
        {
            m_fileListWidget->setVisible( true );
            m_searchRootLabel->setVisible( true );
            m_searchRootContentLabel->setVisible( true );

            if ( height() < RECURSIVE_FILESEARCH_DEFAULT_DIALOG_HEIGHT )
                resize( width(), RECURSIVE_FILESEARCH_DEFAULT_DIALOG_HEIGHT );
        }

        m_findOrCancelButton->setText( "Cancel" );

        m_isCancelPressed = false;

        QStringList candidates = findMatchingFiles();

        // Sort by numbers instead of alphabetically
        QCollator collator;
        collator.setNumericMode( true );
        std::sort( candidates.begin(), candidates.end(), collator );

        m_foundFiles = candidates;

        this->updateFileListWidget();

        m_findOrCancelButton->setText( FIND_BUTTON_FIND_TEXT );

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
            m_outputGroup->setTitle( QString( "%1 (%2)" ).arg( FILES_FOUND_TEXT ).arg( m_foundFiles.size() ) );
        }

        setOkButtonEnabled( !m_foundFiles.isEmpty() );
        m_buttons->button( QDialogButtonBox::Ok )->setDefault( true );
    }
    else
    {
        m_isCancelPressed = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRecursiveFileSearchDialog::slotDialogOkClicked()
{
    m_foundFiles.clear();

    int itemCount = m_fileListWidget->count();
    for ( int i = 0; i < itemCount; i++ )
    {
        const QListWidgetItem* item = m_fileListWidget->item( i );
        if ( ( item->flags() & Qt::ItemIsUserCheckable ) != 0 && item->checkState() )
        {
            m_foundFiles.push_back( rootDirWithEndSeparator() + RiaFilePathTools::toInternalSeparator( item->text() ) );
        }
    }
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
bool RicRecursiveFileSearchDialog::groupByEnsemble() const
{
    return m_groupByEnsembleCheckBox->isChecked();
}

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList trimLeftStrings( const QStringList& strings, const QString& trimText )
{
    QStringList trimmedStrings;
    for ( const auto& string : strings )
    {
        QString trimmedString = string;
        if ( string.startsWith( trimText ) )
        {
            trimmedString = string.right( string.size() - trimText.size() );
        }
        trimmedStrings.append( trimmedString );
    }
    return trimmedStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void sortStringsByLength( QStringList& strings, bool ascending /*= true*/ )
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

//--------------------------------------------------------------------------------------------------
/// Obsolete
//--------------------------------------------------------------------------------------------------
QStringList RicRecursiveFileSearchDialog::buildDirectoryListRecursive( const QString& currentDir, int level )
{
    QStringList allDirs;

    if ( m_isCancelPressed ) return allDirs;

    QString currPathFilter  = pathFilterWithoutStartSeparator();
    bool    subStringFilter = false;

    // Optimizing for speed by a refined match at first directory level
    if ( level == 1 )
    {
        QString pathFilter = this->pathFilterWithoutStartSeparator();
        if ( !pathFilter.startsWith( "*" ) )
        {
            int wildcardIndex = pathFilter.indexOf( QRegExp( QString( "[*%1]" ).arg( RiaFilePathTools::separator() ) ) );
            if ( wildcardIndex >= 0 )
            {
                currPathFilter  = pathFilter.left( wildcardIndex + 1 );
                subStringFilter = true;
            }
        }
    }

    QString currRelPath = RiaFilePathTools::relativePath( rootDirWithEndSeparator(), currentDir );
    if ( pathFilterMatch( currPathFilter, currRelPath ) )
    {
        allDirs.push_back( currentDir );
    }
    else if ( level == 1 && subStringFilter )
    {
        return QStringList();
    }

    QDir        qdir( currentDir );
    QStringList subDirs = qdir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
    for ( QString subDir : subDirs )
    {
        QString subDirFullPath = qdir.absoluteFilePath( subDir );
        updateStatus( SEARCHING_FOR_DIRS, subDirFullPath );
        QApplication::processEvents();
        allDirs += buildDirectoryListRecursive( subDirFullPath, level + 1 );
    }
    return m_isCancelPressed ? QStringList() : allDirs;
}

//--------------------------------------------------------------------------------------------------
/// Obsolete
//--------------------------------------------------------------------------------------------------
bool RicRecursiveFileSearchDialog::pathFilterMatch( const QString& pathFilter, const QString& relPath )
{
    QString pattern = pathFilter;
    if ( relPath.endsWith( RiaFilePathTools::separator() ) && !pathFilter.endsWith( RiaFilePathTools::separator() ) )
        pattern += RiaFilePathTools::separator();

    QRegExp regexp( pattern, Qt::CaseInsensitive, QRegExp::Wildcard );
    return regexp.exactMatch( relPath );
}
