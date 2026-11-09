/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiuReportWindow.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaQStringFormatter.h"
#include "RiaReportDataProducer.h"

#include "SummaryPlotCommands/RicAsciiExportSummaryPlotFeature.h"

#include "RiuFileDialogTools.h"
#include "RiuGuiTheme.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuReportHeaderView.h"
#include "RiuReportTableModel.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafUtils.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableView>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <format>
#include <limits>
#include <map>

namespace
{
QColor blendColors( const QColor& foreground, const QColor& background, double foregroundWeight )
{
    const double w = std::clamp( foregroundWeight, 0.0, 1.0 );
    return QColor::fromRgbF( static_cast<float>( foreground.redF() * w + background.redF() * ( 1.0 - w ) ),
                             static_cast<float>( foreground.greenF() * w + background.greenF() * ( 1.0 - w ) ),
                             static_cast<float>( foreground.blueF() * w + background.blueF() * ( 1.0 - w ) ) );
}

QColor themeColor( const QString& variableName )
{
    return RiuGuiTheme::getColorByVariableName( variableName );
}

QColor mutedTextColor()
{
    return blendColors( themeColor( "textColor" ), themeColor( "mainBackgroundColor" ), 0.55 );
}

//--------------------------------------------------------------------------------------------------
/// Quote text containing tabs, line breaks or quotes, so spreadsheets keep it in one cell
//--------------------------------------------------------------------------------------------------
QString escapeClipboardCell( const QString& text )
{
    if ( !text.contains( '\t' ) && !text.contains( '\n' ) && !text.contains( '\r' ) && !text.contains( '"' ) ) return text;

    QString escaped = text;
    escaped.replace( "\"", "\"\"" );
    return "\"" + escaped + "\"";
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuReportWindow::RiuReportWindow( QWidget* parent )
    : QWidget( parent )
    , m_isRefreshPending( false )
{
    auto mainLayout = new QVBoxLayout( this );
    mainLayout->setContentsMargins( 4, 4, 4, 4 );
    mainLayout->setSpacing( 4 );

    m_titleLabel    = new QLabel( this );
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold( true );
    titleFont.setPointSizeF( titleFont.pointSizeF() * 1.15 );
    m_titleLabel->setFont( titleFont );
    m_titleLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );

    m_sourceLabel = new QLabel( this );
    m_sourceLabel->setTextFormat( Qt::RichText );
    m_sourceLabel->setTextInteractionFlags( Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard );
    m_sourceLabel->setToolTip( "Select the source object in the project tree" );
    connect( m_sourceLabel, &QLabel::linkActivated, this, &RiuReportWindow::selectProducerObject );

    mainLayout->addWidget( createToolBar() );
    mainLayout->addWidget( m_titleLabel );
    mainLayout->addWidget( m_sourceLabel );

    m_stackedWidget = new QStackedWidget( this );

    m_placeholderLabel = new QLabel( "Select a plot and use <b>Show Plot Data</b> to see the plot data here.", this );
    m_placeholderLabel->setAlignment( Qt::AlignCenter );
    m_placeholderLabel->setWordWrap( true );
    m_stackedWidget->addWidget( m_placeholderLabel );

    m_tabWidget = new QTabWidget( this );
    m_tabWidget->setDocumentMode( true );
    connect( m_tabWidget, &QTabWidget::currentChanged, this, &RiuReportWindow::onCurrentTabChanged );
    m_stackedWidget->addWidget( m_tabWidget );

    mainLayout->addWidget( m_stackedWidget, 1 );

    m_refreshTimer = new QTimer( this );
    m_refreshTimer->setSingleShot( true );
    m_refreshTimer->setInterval( 200 );
    connect( m_refreshTimer, &QTimer::timeout, this, &RiuReportWindow::refresh );

    // Filtering large tables is slow, so wait until the user stops typing
    m_filterTimer = new QTimer( this );
    m_filterTimer->setSingleShot( true );
    m_filterTimer->setInterval( 300 );
    connect( m_filterTimer, &QTimer::timeout, this, &RiuReportWindow::onFilterTextChanged );

    rebuildContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RiuReportWindow::findProducerObject( caf::PdmObjectHandle* object )
{
    while ( object )
    {
        if ( dynamic_cast<RiaReportDataProducer*>( object ) ) return object;

        auto parentField = object->parentField();
        object           = parentField ? parentField->ownerObject() : nullptr;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::setProducerObject( caf::PdmObjectHandle* producerObject )
{
    if ( producerObject && !dynamic_cast<RiaReportDataProducer*>( producerObject ) ) producerObject = nullptr;

    if ( producerObject != m_producerObject.p() ) m_tableViewStates.clear();

    m_producerObject = producerObject;

    if ( isVisible() )
    {
        refresh();
    }
    else
    {
        m_isRefreshPending = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RiuReportWindow::producerObject() const
{
    return m_producerObject.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::showProducerObject( caf::PdmObjectHandle* producerObject )
{
    if ( producerObject && !dynamic_cast<RiaReportDataProducer*>( producerObject ) ) producerObject = nullptr;

    m_producerObject = producerObject;
    refresh();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::refresh()
{
    m_refreshTimer->stop();
    m_isRefreshPending = false;

    m_title.clear();
    m_sections.clear();

    if ( auto dataProducer = producer() )
    {
        m_title    = dataProducer->reportTitle();
        m_sections = dataProducer->reportSections();
    }

    rebuildContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::scheduleRefreshIfShowing( const caf::PdmObjectHandle* object )
{
    if ( !object || m_producerObject.isNull() ) return;

    // Containers such as multi plots show data from their child plots, so updates of descendants are relevant
    bool isProducerOrDescendant = false;
    for ( auto current = object; current; )
    {
        if ( current == m_producerObject.p() )
        {
            isProducerOrDescendant = true;
            break;
        }
        auto parentField = current->parentField();
        current          = parentField ? parentField->ownerObject() : nullptr;
    }
    if ( !isProducerOrDescendant ) return;

    auto dataProducer = producer();
    if ( !dataProducer || !dataProducer->supportsLiveUpdate() ) return;

    if ( isVisible() )
    {
        m_refreshTimer->start();
    }
    else
    {
        m_isRefreshPending = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );

    if ( m_isRefreshPending ) refresh();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RiuReportWindow::createToolBar()
{
    auto toolBar = new QWidget( this );
    auto layout  = new QHBoxLayout( toolBar );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 6 );

    m_displayModeCombo = new QComboBox( toolBar );
    m_displayModeCombo->addItem( "Table", static_cast<int>( DisplayMode::TABLE ) );
    m_displayModeCombo->addItem( "CSV", static_cast<int>( DisplayMode::CSV ) );
    m_displayModeCombo->addItem( "Plain Text", static_cast<int>( DisplayMode::TEXT ) );

    m_numberFormatCombo = new QComboBox( toolBar );
    m_numberFormatCombo->addItem( "Default", static_cast<int>( RifReportFormatOptions::DoubleFormat::COLUMN_DEFAULT ) );
    m_numberFormatCombo->addItem( "Fixed", static_cast<int>( RifReportFormatOptions::DoubleFormat::FLOAT ) );
    m_numberFormatCombo->addItem( "Scientific", static_cast<int>( RifReportFormatOptions::DoubleFormat::SCIENTIFIC ) );
    m_numberFormatCombo->addItem( "Concise", static_cast<int>( RifReportFormatOptions::DoubleFormat::CONCISE ) );

    m_precisionSpinBox = new QSpinBox( toolBar );
    m_precisionSpinBox->setRange( -1, 16 );
    m_precisionSpinBox->setValue( -1 );
    m_precisionSpinBox->setSpecialValueText( "Auto" );
    m_precisionSpinBox->setToolTip( "Number of decimals. Auto uses the default precision of each column." );

    m_delimiterCombo = new QComboBox( toolBar );
    m_delimiterCombo->addItem( "Comma", "," );
    m_delimiterCombo->addItem( "Semicolon", ";" );
    m_delimiterCombo->addItem( "Tab", "\t" );

    m_filterEdit = new QLineEdit( toolBar );
    m_filterEdit->setPlaceholderText( "Filter rows" );
    m_filterEdit->setClearButtonEnabled( true );
    m_filterEdit->setToolTip( "Show only table rows containing this text. Click a column header to sort, right-click to "
                              "show or hide columns." );
    m_filterEdit->setMaximumWidth( 200 );

    m_refreshButton = new QToolButton( toolBar );
    m_refreshButton->setText( "Refresh" );
    m_refreshButton->setToolTip( "Read the data from the plot again" );
    connect( m_refreshButton, &QToolButton::clicked, this, &RiuReportWindow::refresh );

    m_copyButton = new QToolButton( toolBar );
    m_copyButton->setText( "Copy" );
    m_copyButton->setToolTip( "Copy the data in the current tab to the clipboard" );
    m_copyButton->setPopupMode( QToolButton::MenuButtonPopup );
    auto copyMenu = new QMenu( m_copyButton );
    copyMenu->addAction( "Copy Current Tab", this, &RiuReportWindow::copyCurrentTab );
    copyMenu->addAction( "Copy All Tabs", this, &RiuReportWindow::copyAllTabs );
    m_copyButton->setMenu( copyMenu );
    connect( m_copyButton, &QToolButton::clicked, this, &RiuReportWindow::copyCurrentTab );

    m_exportButton = new QToolButton( toolBar );
    m_exportButton->setText( "Export..." );
    m_exportButton->setToolTip( "Export all tabs to a file using the selected format" );
    connect( m_exportButton, &QToolButton::clicked, this, &RiuReportWindow::exportToFile );

    layout->addWidget( new QLabel( "Format", toolBar ) );
    layout->addWidget( m_displayModeCombo );
    layout->addSpacing( 6 );
    layout->addWidget( new QLabel( "Numbers", toolBar ) );
    layout->addWidget( m_numberFormatCombo );
    layout->addWidget( new QLabel( "Precision", toolBar ) );
    layout->addWidget( m_precisionSpinBox );
    layout->addSpacing( 6 );
    layout->addWidget( new QLabel( "Delimiter", toolBar ) );
    layout->addWidget( m_delimiterCombo );
    layout->addSpacing( 6 );
    layout->addWidget( m_filterEdit );
    layout->addStretch( 1 );
    layout->addWidget( m_refreshButton );
    layout->addWidget( m_copyButton );
    layout->addWidget( m_exportButton );

    connect( m_displayModeCombo, &QComboBox::currentIndexChanged, this, &RiuReportWindow::onDisplayModeChanged );
    connect( m_numberFormatCombo, &QComboBox::currentIndexChanged, this, &RiuReportWindow::onFormatOptionsChanged );
    connect( m_precisionSpinBox, &QSpinBox::valueChanged, this, &RiuReportWindow::onFormatOptionsChanged );
    connect( m_delimiterCombo, &QComboBox::currentIndexChanged, this, &RiuReportWindow::onFormatOptionsChanged );
    connect( m_filterEdit, &QLineEdit::textChanged, m_filterTimer, qOverload<>( &QTimer::start ) );

    m_delimiterCombo->setEnabled( false );

    return toolBar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::rebuildContent()
{
    // Tabs are added and replaced below, and the sections are loaded explicitly
    QSignalBlocker tabSignalBlocker( m_tabWidget );

    const int currentTabIndex = m_tabWidget->currentIndex();

    saveTableViewStates();

    while ( m_tabWidget->count() > 0 )
    {
        QWidget* tab = m_tabWidget->widget( 0 );
        m_tabWidget->removeTab( 0 );
        tab->deleteLater();
    }

    m_titleLabel->setText( m_title );
    m_titleLabel->setVisible( !m_title.isEmpty() );
    updateSourceLabel();

    const bool hasData = !m_sections.empty();
    m_copyButton->setEnabled( hasData );
    m_exportButton->setEnabled( hasData );
    m_refreshButton->setEnabled( producer() != nullptr );

    if ( !hasData )
    {
        if ( producer() ) m_placeholderLabel->setText( "No data available for the selected plot." );
        m_stackedWidget->setCurrentWidget( m_placeholderLabel );
        return;
    }

    const int sectionCount  = static_cast<int>( m_sections.size() );
    const int shownTabIndex = ( currentTabIndex >= 0 && currentTabIndex < sectionCount ) ? currentTabIndex : 0;

    // Sections with pending tables are only loaded when their tab is shown
    m_sections[shownTabIndex].loadTables();

    for ( const auto& section : m_sections )
    {
        const QString tabTitle = section.title.isEmpty() ? QString( "Data" ) : section.title;

        QWidget* tabContent = nullptr;
        if ( section.hasPendingTables() )
        {
            tabContent = new QWidget;
            tabContent->setProperty( "isPendingSection", true );
        }
        else
        {
            tabContent = createSectionWidget( section );
        }
        m_tabWidget->addTab( tabContent, tabTitle );
    }

    m_tabWidget->setCurrentIndex( shownTabIndex );

    m_stackedWidget->setCurrentWidget( m_tabWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::onCurrentTabChanged( int index )
{
    if ( index < 0 || index >= static_cast<int>( m_sections.size() ) || index >= m_tabWidget->count() ) return;

    QWidget* tabContent = m_tabWidget->widget( index );
    if ( !tabContent || !tabContent->property( "isPendingSection" ).toBool() ) return;

    m_sections[index].loadTables();

    QSignalBlocker tabSignalBlocker( m_tabWidget );

    const QString tabTitle = m_tabWidget->tabText( index );
    m_tabWidget->removeTab( index );
    tabContent->deleteLater();
    m_tabWidget->insertTab( index, createSectionWidget( m_sections[index] ), tabTitle );
    m_tabWidget->setCurrentIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::loadAllSections()
{
    for ( auto& section : m_sections )
    {
        section.loadTables();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::updateSourceLabel()
{
    auto pdmObject = dynamic_cast<caf::PdmObject*>( m_producerObject.p() );
    if ( !pdmObject )
    {
        m_sourceLabel->clear();
        m_sourceLabel->setVisible( false );
        return;
    }

    QString objectName = pdmObject->uiCapability()->uiName();
    if ( auto descriptionField = pdmObject->userDescriptionField() )
    {
        const QString description = descriptionField->uiCapability()->uiValue().toString();
        if ( !description.isEmpty() ) objectName = QString( "%1 (%2)" ).arg( description, objectName );
    }

    m_sourceLabel->setText( QString( "Source: <a href=\"select\">%1</a>" ).arg( objectName.toHtmlEscaped() ) );
    m_sourceLabel->setVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::selectProducerObject()
{
    if ( auto pdmObject = dynamic_cast<caf::PdmObject*>( m_producerObject.p() ) )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( pdmObject );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RiuReportWindow::createSectionWidget( const RiaReportSection& section )
{
    if ( displayMode() != DisplayMode::TABLE ) return createTextWidget( section );

    if ( section.tables.empty() )
    {
        auto label = new QLabel( "No data available." );
        label->setAlignment( Qt::AlignCenter );
        return label;
    }

    auto viewStateKey = [&section]( size_t tableIndex )
    {
        const auto& table = section.tables[tableIndex];
        return QString( "%1\n%2\n%3\n%4" ).arg( section.title ).arg( tableIndex ).arg( table.title ).arg( table.columns.size() );
    };

    if ( section.tables.size() == 1 ) return createTableWidget( section.tables.front(), viewStateKey( 0 ) );

    auto splitter = new QSplitter( Qt::Vertical );
    for ( size_t tableIndex = 0; tableIndex < section.tables.size(); tableIndex++ )
    {
        splitter->addWidget( createTableWidget( section.tables[tableIndex], viewStateKey( tableIndex ) ) );
    }
    return splitter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::saveTableViewStates()
{
    const auto producerId = QVariant::fromValue( reinterpret_cast<quintptr>( m_producerObject.p() ) );

    for ( auto* tableView : m_tabWidget->findChildren<QTableView*>() )
    {
        const QString key = tableView->property( "viewStateKey" ).toString();
        if ( key.isEmpty() || tableView->property( "producerId" ) != producerId ) continue;

        m_tableViewStates[key] = tableView->horizontalHeader()->saveState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RiuReportWindow::createTableWidget( const RiaReportTable& table, const QString& viewStateKey )
{
    auto widget = new QWidget;
    auto layout = new QVBoxLayout( widget );
    layout->setContentsMargins( 2, 6, 2, 2 );
    layout->setSpacing( 2 );

    if ( !table.title.isEmpty() )
    {
        auto  titleLabel = new QLabel( table.title, widget );
        QFont font       = titleLabel->font();
        font.setBold( true );
        titleLabel->setFont( font );
        titleLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
        layout->addWidget( titleLabel );
    }

    if ( !table.description.isEmpty() )
    {
        auto descriptionLabel = new QLabel( table.description, widget );
        descriptionLabel->setWordWrap( true );
        descriptionLabel->setTextFormat( Qt::PlainText );
        descriptionLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
        descriptionLabel->setStyleSheet( QString( "color: %1;" ).arg( mutedTextColor().name() ) );
        layout->addWidget( descriptionLabel );
    }

    auto rowCountLabel = new QLabel( widget );
    rowCountLabel->setStyleSheet( QString( "color: %1;" ).arg( mutedTextColor().name() ) );
    layout->addWidget( rowCountLabel );

    auto tableView = new QTableView( widget );
    auto model     = new RiuReportTableModel( table, formatOptions(), mutedTextColor(), tableView );
    model->setFilterText( m_filterEdit->text() );

    RiuReportHeaderView::Colors headerColors{ .background = themeColor( "backgroundColor2" ),
                                              .text       = themeColor( "textColor" ),
                                              .mutedText  = mutedTextColor(),
                                              .border     = themeColor( "borderColor" ) };
    tableView->setHorizontalHeader( new RiuReportHeaderView( headerColors, tableView ) );
    tableView->setModel( model );

    tableView->setStyleSheet( themeStyleSheet() );
    tableView->verticalHeader()->setVisible( false );
    tableView->verticalHeader()->setDefaultSectionSize( tableView->fontMetrics().height() + 10 );
    tableView->setShowGrid( false );
    tableView->setAlternatingRowColors( true );
    tableView->setWordWrap( false );
    tableView->setTextElideMode( Qt::ElideRight );
    tableView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    tableView->setSelectionBehavior( QAbstractItemView::SelectItems );
    tableView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    tableView->setMouseTracking( true );
    tableView->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
    tableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    tableView->horizontalHeader()->setStretchLastSection( false );
    tableView->horizontalHeader()->setMinimumSectionSize( 40 );
    tableView->resizeColumnsToContents();

    const int maxInitialColumnWidth = 400;
    for ( int col = 0; col < model->columnCount(); col++ )
    {
        if ( tableView->columnWidth( col ) > maxInitialColumnWidth ) tableView->setColumnWidth( col, maxInitialColumnWidth );
    }

    // The omitted row and the row count change when the table is filtered or sorted
    auto onRowsChanged = [tableView, model, rowCountLabel]()
    {
        tableView->clearSpans();
        if ( model->omittedRowIndex() >= 0 && model->columnCount() > 1 )
        {
            tableView->setSpan( model->omittedRowIndex(), 0, 1, model->columnCount() );
        }

        const size_t totalRowCount = model->table().rows.size();
        rowCountLabel->setText( QString( "Showing %1 of %2 rows" ).arg( model->filteredRowCount() ).arg( totalRowCount ) );
        rowCountLabel->setVisible( model->filteredRowCount() != totalRowCount );
    };
    onRowsChanged();
    connect( model, &QAbstractItemModel::modelReset, tableView, onRowsChanged );

    tableView->setProperty( "viewStateKey", viewStateKey );
    tableView->setProperty( "producerId", QVariant::fromValue( reinterpret_cast<quintptr>( m_producerObject.p() ) ) );
    if ( auto it = m_tableViewStates.find( viewStateKey ); it != m_tableViewStates.end() )
    {
        tableView->horizontalHeader()->restoreState( it->second );
    }

    // Sorts the model on the restored sort column
    tableView->setSortingEnabled( true );

    tableView->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( tableView->horizontalHeader(),
             &QHeaderView::customContextMenuRequested,
             tableView,
             [tableView]( const QPoint& position ) { showColumnMenu( tableView, position ); } );

    auto copyAction = new QAction( "Copy", tableView );
    copyAction->setShortcut( QKeySequence::Copy );
    copyAction->setShortcutContext( Qt::WidgetShortcut );
    connect( copyAction, &QAction::triggered, tableView, [tableView]() { copySelectedCells( tableView ); } );
    tableView->addAction( copyAction );

    layout->addWidget( tableView, 1 );

    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RiuReportWindow::createTextWidget( const RiaReportSection& section )
{
    auto textEdit = new QPlainTextEdit;
    textEdit->setReadOnly( true );
    textEdit->setLineWrapMode( QPlainTextEdit::NoWrap );
    textEdit->setFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );
    textEdit->setPlainText( RifReportTableFormatter::format( section, textFormat(), formatOptions() ) );

    return textEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::copyCurrentTab()
{
    const int index = m_tabWidget->currentIndex();
    if ( index < 0 || index >= static_cast<int>( m_sections.size() ) ) return;

    copyToClipboard( QString(), { m_sections[index] } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::copyAllTabs()
{
    if ( m_sections.empty() ) return;

    loadAllSections();
    copyToClipboard( m_title, m_sections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::copyToClipboard( const QString& title, const std::vector<RiaReportSection>& sections )
{
    auto clipboard = QApplication::clipboard();
    if ( !clipboard ) return;

    auto options = formatOptions();
    if ( displayMode() == DisplayMode::TABLE )
    {
        // Rich text for documents, tab separated text for spreadsheets
        options.delimiter = "\t";

        auto mimeData = new QMimeData;
        mimeData->setHtml( RifReportTableFormatter::format( title, sections, RifReportTableFormatter::Format::HTML, options ) );
        mimeData->setText( RifReportTableFormatter::format( title, sections, RifReportTableFormatter::Format::CSV, options ) );
        clipboard->setMimeData( mimeData );
        return;
    }

    clipboard->setText( RifReportTableFormatter::format( title, sections, textFormat(), options ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::exportToFile()
{
    if ( m_sections.empty() ) return;

    QString filter;
    QString suffix;
    switch ( displayMode() )
    {
        case DisplayMode::TABLE:
            filter = "HTML File (*.html)";
            suffix = "html";
            break;
        case DisplayMode::CSV:
            filter = "CSV File (*.csv)";
            suffix = "csv";
            break;
        case DisplayMode::TEXT:
        default:
            filter = "Text File (*.txt)";
            suffix = "txt";
            break;
    }

    const QString baseName        = caf::Utils::makeValidFileBasename( m_title.isEmpty() ? QString( "PlotData" ) : m_title );
    const QString defaultFileName = RicAsciiExportSummaryPlotFeature::defaultExportDir() + "/" + baseName + "." + suffix;

    const QString fileName = RiuFileDialogTools::getSaveFileName( this, "Export Plot Data", defaultFileName, filter + ";;All files(*.*)" );
    if ( fileName.isEmpty() ) return;

    RiaApplication::instance()->setLastUsedDialogDirectory( "PLOT_ASCIIEXPORT_DIR", QFileInfo( fileName ).absolutePath() );

    loadAllSections();

    const auto format = displayMode() == DisplayMode::TABLE ? RifReportTableFormatter::Format::HTML : textFormat();
    const auto text   = RifReportTableFormatter::format( m_title, m_sections, format, formatOptions() );
    if ( !RicAsciiExportSummaryPlotFeature::exportTextToFile( fileName, text ) )
    {
        RiaLogging::error( std::format( "Could not write plot data to file {}", fileName ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::onDisplayModeChanged()
{
    m_delimiterCombo->setEnabled( displayMode() == DisplayMode::CSV );
    m_filterEdit->setEnabled( displayMode() == DisplayMode::TABLE );
    rebuildContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::onFormatOptionsChanged()
{
    rebuildContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::onFilterTextChanged()
{
    for ( auto* model : m_tabWidget->findChildren<RiuReportTableModel*>() )
    {
        model->setFilterText( m_filterEdit->text() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Menu for showing and hiding columns. The last visible column cannot be hidden.
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::showColumnMenu( QTableView* tableView, const QPoint& position )
{
    auto* model = tableView->model();
    if ( !model ) return;

    int visibleColumnCount = 0;
    for ( int col = 0; col < model->columnCount(); col++ )
    {
        if ( !tableView->isColumnHidden( col ) ) visibleColumnCount++;
    }

    QMenu menu( tableView );
    for ( int col = 0; col < model->columnCount(); col++ )
    {
        const bool isVisible = !tableView->isColumnHidden( col );

        auto action = menu.addAction( model->headerData( col, Qt::Horizontal, Qt::ToolTipRole ).toString() );
        action->setCheckable( true );
        action->setChecked( isVisible );
        action->setEnabled( !isVisible || visibleColumnCount > 1 );
        connect( action, &QAction::toggled, tableView, [tableView, col]( bool checked ) { tableView->setColumnHidden( col, !checked ); } );
    }

    menu.addSeparator();
    menu.addAction( "Show All Columns",
                    [tableView, model]()
                    {
                        for ( int col = 0; col < model->columnCount(); col++ )
                        {
                            tableView->setColumnHidden( col, false );
                        }
                    } );

    menu.exec( tableView->horizontalHeader()->mapToGlobal( position ) );
}

//--------------------------------------------------------------------------------------------------
/// Copy selected cells as tab separated text, keeping the layout of the selection
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::copySelectedCells( QTableView* tableView )
{
    if ( !tableView || !tableView->selectionModel() ) return;

    const auto selectedIndexes = tableView->selectionModel()->selectedIndexes();
    if ( selectedIndexes.isEmpty() ) return;

    std::map<int, std::map<int, QString>> cells;
    int                                   minColumn = std::numeric_limits<int>::max();
    int                                   maxColumn = 0;
    for ( const auto& index : selectedIndexes )
    {
        if ( tableView->isColumnHidden( index.column() ) ) continue;

        cells[index.row()][index.column()] = index.data( Qt::DisplayRole ).toString();
        minColumn                          = std::min( minColumn, index.column() );
        maxColumn                          = std::max( maxColumn, index.column() );
    }

    if ( cells.empty() ) return;

    QStringList lines;
    for ( const auto& [row, columns] : cells )
    {
        QStringList values;
        for ( int col = minColumn; col <= maxColumn; col++ )
        {
            if ( tableView->isColumnHidden( col ) ) continue;

            auto it = columns.find( col );
            values.push_back( it != columns.end() ? escapeClipboardCell( it->second ) : QString() );
        }
        lines.push_back( values.join( "\t" ) );
    }

    if ( auto clipboard = QApplication::clipboard() ) clipboard->setText( lines.join( "\n" ) + "\n" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuReportWindow::DisplayMode RiuReportWindow::displayMode() const
{
    return static_cast<DisplayMode>( m_displayModeCombo->currentData().toInt() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReportFormatOptions RiuReportWindow::formatOptions() const
{
    RifReportFormatOptions options;
    options.doubleFormat    = static_cast<RifReportFormatOptions::DoubleFormat>( m_numberFormatCombo->currentData().toInt() );
    options.doublePrecision = m_precisionSpinBox->value();
    options.delimiter       = m_delimiterCombo->currentData().toString();
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReportTableFormatter::Format RiuReportWindow::textFormat() const
{
    return displayMode() == DisplayMode::CSV ? RifReportTableFormatter::Format::CSV : RifReportTableFormatter::Format::TEXT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaReportDataProducer* RiuReportWindow::producer() const
{
    return dynamic_cast<RiaReportDataProducer*>( m_producerObject.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuReportWindow::themeStyleSheet() const
{
    const QColor background = themeColor( "mainBackgroundColor" );
    const QColor alternate  = themeColor( "backgroundColor1" );
    const QColor separator  = themeColor( "backgroundColor2" );
    const QColor text       = themeColor( "textColor" );
    const QColor primary    = themeColor( "primaryColor" );
    const QColor selection  = blendColors( primary, background, 0.35 );
    const QColor hover      = blendColors( primary, background, 0.15 );

    return QString( "QTableView { background-color: %1; alternate-background-color: %2; color: %3; border: none; "
                    "selection-background-color: %4; selection-color: %3; }"
                    "QTableView::item { padding: 2px 10px; border: none; border-bottom: 1px solid %5; }"
                    "QTableView::item:hover { background-color: %6; }"
                    "QTableView::item:selected { background-color: %4; color: %3; }" )
        .arg( background.name() )
        .arg( alternate.name() )
        .arg( text.name() )
        .arg( selection.name() )
        .arg( separator.name() )
        .arg( hover.name() );
}
