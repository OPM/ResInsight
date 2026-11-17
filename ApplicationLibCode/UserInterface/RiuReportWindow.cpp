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
#include "RiaPreferences.h"
#include "RiaQStringFormatter.h"
#include "RiaReportDataProducer.h"

#include "ApplicationCommands/RicEditPreferencesFeature.h"
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
#include <QContextMenuEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <format>
#include <functional>

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

QString defaultPlaceholderText()
{
    return "Select a plot and use <b>Show Plot Data</b> to see the plot data here.";
}

//--------------------------------------------------------------------------------------------------
/// Handles Ctrl+C and the context menu of a table view. An event filter is used instead of a widget
/// shortcut, because the application wide Ctrl+C shortcut would make a widget shortcut ambiguous.
//--------------------------------------------------------------------------------------------------
class TableCopyHandler : public QObject
{
public:
    TableCopyHandler( QTableView* tableView, std::function<void()> copyFunction, std::function<void( const QPoint& )> contextMenuFunction )
        : QObject( tableView )
        , m_tableView( tableView )
        , m_copyFunction( std::move( copyFunction ) )
        , m_contextMenuFunction( std::move( contextMenuFunction ) )
    {
        tableView->installEventFilter( this );
        tableView->viewport()->installEventFilter( this );
    }

protected:
    bool eventFilter( QObject* watched, QEvent* event ) override
    {
        if ( watched == m_tableView && ( event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress ) )
        {
            auto keyEvent = static_cast<QKeyEvent*>( event );
            if ( !keyEvent->matches( QKeySequence::Copy ) ) return false;

            // Accepting the override makes sure the key press is delivered to the table, and not to a global shortcut
            if ( event->type() == QEvent::ShortcutOverride )
                event->accept();
            else
                m_copyFunction();
            return true;
        }

        if ( watched == m_tableView->viewport() && event->type() == QEvent::ContextMenu )
        {
            m_contextMenuFunction( static_cast<QContextMenuEvent*>( event )->globalPos() );
            return true;
        }

        return false;
    }

private:
    QTableView*                          m_tableView;
    std::function<void()>                m_copyFunction;
    std::function<void( const QPoint& )> m_contextMenuFunction;
};
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuReportWindow::RiuReportWindow( QWidget* parent )
    : QWidget( parent )
    , m_isRefreshPending( false )
    , m_sections( std::make_shared<std::vector<RiaReportSection>>() )
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

    // Title to the left and source to the right, on the same row below the tables
    auto headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins( 0, 0, 0, 0 );
    headerLayout->addWidget( m_titleLabel );
    headerLayout->addStretch( 1 );
    headerLayout->addWidget( m_sourceLabel );

    m_stackedWidget = new QStackedWidget( this );

    m_placeholderLabel = new QLabel( defaultPlaceholderText(), this );
    m_placeholderLabel->setAlignment( Qt::AlignCenter );
    m_placeholderLabel->setWordWrap( true );
    m_stackedWidget->addWidget( m_placeholderLabel );

    m_tabWidget = new QTabWidget( this );
    m_tabWidget->setDocumentMode( true );
    m_tabWidget->setTabBarAutoHide( true );
    m_tabWidget->setTabPosition( QTabWidget::South );
    connect( m_tabWidget, &QTabWidget::currentChanged, this, &RiuReportWindow::onCurrentTabChanged );
    m_stackedWidget->addWidget( m_tabWidget );

    mainLayout->addWidget( m_stackedWidget, 1 );
    mainLayout->addLayout( headerLayout );
    mainLayout->addWidget( createBottomBar() );

    m_refreshTimer = new QTimer( this );
    m_refreshTimer->setSingleShot( true );
    m_refreshTimer->setInterval( 200 );
    connect( m_refreshTimer, &QTimer::timeout, this, &RiuReportWindow::refresh );

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

    if ( producerObject != m_producerObject.p() ) m_tableViewStates.clear();

    m_producerObject = producerObject;
    refresh();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::refresh()
{
    // The table views are rebuilt on refresh, so wait until a menu opened from a table is closed
    if ( m_isMenuOpen )
    {
        m_refreshTimer->start();
        return;
    }

    m_refreshTimer->stop();
    m_isRefreshPending = false;

    m_title.clear();
    auto sections = std::make_shared<std::vector<RiaReportSection>>();

    if ( auto dataProducer = producer() )
    {
        m_title   = dataProducer->reportTitle();
        *sections = dataProducer->reportSections();
    }
    m_sections = std::move( sections );

    rebuildContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::clearIfProducerDeleted()
{
    const bool hasContent = !m_title.isEmpty() || !m_sections->empty();
    if ( m_producerObject.isNull() && hasContent ) refresh();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::scheduleRefreshIfShowing( const caf::PdmObjectHandle* object )
{
    clearIfProducerDeleted();

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
QWidget* RiuReportWindow::createBottomBar()
{
    auto toolBar = new QWidget( this );
    auto layout  = new QHBoxLayout( toolBar );
    layout->setContentsMargins( 8, 6, 8, 6 );
    layout->setSpacing( 8 );

    m_numberFormatCombo = new QComboBox( toolBar );
    for ( auto format : { RimPlotAxisProperties::NUMBER_FORMAT_AUTO,
                          RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL,
                          RimPlotAxisProperties::NUMBER_FORMAT_SCIENTIFIC } )
    {
        m_numberFormatCombo->addItem( caf::AppEnum<RimPlotAxisProperties::NumberFormatType>::uiText( format ), static_cast<int>( format ) );
    }
    m_numberFormatCombo->setToolTip( "Auto uses the default format of each column" );

    m_precisionSpinBox = new QSpinBox( toolBar );
    m_precisionSpinBox->setRange( 0, 16 );
    m_precisionSpinBox->setValue( 2 );

    layout->addWidget( new QLabel( "Number Format", toolBar ) );
    layout->addWidget( m_numberFormatCombo );
    layout->addSpacing( 16 );
    layout->addWidget( new QLabel( "Number of Decimals", toolBar ) );
    layout->addWidget( m_precisionSpinBox );
    layout->addStretch( 1 );
    layout->addSpacing( 16 );

    auto csvSettingsLink = new QLabel( "<a href=\"preferences\">CSV Settings...</a>", toolBar );
    csvSettingsLink->setToolTip( "Open the Import/Export page in Preferences" );
    connect( csvSettingsLink,
             &QLabel::linkActivated,
             this,
             []() { RicEditPreferencesFeature::showPreferences( RiaPreferences::tabNameImportExport() ); } );
    layout->addWidget( csvSettingsLink );

    loadSettings();

    connect( m_numberFormatCombo, &QComboBox::currentIndexChanged, this, &RiuReportWindow::onFormatOptionsChanged );
    connect( m_precisionSpinBox, &QSpinBox::valueChanged, this, &RiuReportWindow::onFormatOptionsChanged );

    m_precisionSpinBox->setEnabled( numberFormat() != RimPlotAxisProperties::NUMBER_FORMAT_AUTO );
    // The toolbar scrolls horizontally when the window is narrow, so that it does not force a minimum width on the window
    auto scrollArea = new QScrollArea( this );
    scrollArea->setWidget( toolBar );
    scrollArea->setWidgetResizable( true );
    scrollArea->setFrameShape( QFrame::NoFrame );
    scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    scrollArea->setMinimumWidth( 0 );

    auto updateScrollAreaHeight = [scrollArea, toolBar]()
    {
        auto*     scrollBar = scrollArea->horizontalScrollBar();
        const int extra     = scrollBar->maximum() > 0 ? scrollBar->sizeHint().height() : 0;
        scrollArea->setFixedHeight( toolBar->sizeHint().height() + extra );
    };
    connect( scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged, scrollArea, updateScrollAreaHeight );
    updateScrollAreaHeight();

    return scrollArea;
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

    const bool hasData = !m_sections->empty();

    if ( !hasData )
    {
        m_placeholderLabel->setText( producer() ? QString( "No data available for the selected plot." ) : defaultPlaceholderText() );
        m_stackedWidget->setCurrentWidget( m_placeholderLabel );
        return;
    }

    auto&     sections      = *m_sections;
    const int sectionCount  = static_cast<int>( sections.size() );
    const int shownTabIndex = ( currentTabIndex >= 0 && currentTabIndex < sectionCount ) ? currentTabIndex : 0;

    // Sections with pending tables are only loaded when their tab is shown
    sections[shownTabIndex].loadTables();

    for ( size_t sectionIndex = 0; sectionIndex < sections.size(); sectionIndex++ )
    {
        const auto&   section  = sections[sectionIndex];
        const QString tabTitle = section.title.isEmpty() ? QString( "Data" ) : section.title;

        QWidget* tabContent = nullptr;
        if ( section.hasPendingTables() )
        {
            tabContent = new QWidget;
            tabContent->setProperty( "isPendingSection", true );
        }
        else
        {
            tabContent = createSectionWidget( sectionIndex );
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
    if ( index < 0 || index >= static_cast<int>( m_sections->size() ) || index >= m_tabWidget->count() ) return;

    QWidget* tabContent = m_tabWidget->widget( index );
    if ( !tabContent || !tabContent->property( "isPendingSection" ).toBool() ) return;

    ( *m_sections )[index].loadTables();

    QSignalBlocker tabSignalBlocker( m_tabWidget );

    const QString tabTitle = m_tabWidget->tabText( index );
    m_tabWidget->removeTab( index );
    tabContent->deleteLater();
    m_tabWidget->insertTab( index, createSectionWidget( static_cast<size_t>( index ) ), tabTitle );
    m_tabWidget->setCurrentIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::loadAllSections()
{
    for ( auto& section : *m_sections )
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
QWidget* RiuReportWindow::createSectionWidget( size_t sectionIndex )
{
    const auto& section = ( *m_sections )[sectionIndex];
    if ( section.tables.empty() )
    {
        auto label = new QLabel( "No data available." );
        label->setAlignment( Qt::AlignCenter );
        return label;
    }

    auto viewStateKey = [&section]( size_t tableIndex )
    {
        const auto& table = section.tables[tableIndex];
        return QString( "%1\n%2\n%3\n%4" ).arg( section.title, QString::number( tableIndex ), table.title, QString::number( table.columns.size() ) );
    };

    // Shares ownership of the section vector, while pointing to the table
    auto sharedTable = [this, &section]( size_t tableIndex )
    { return std::shared_ptr<const RiaReportTable>( m_sections, &section.tables[tableIndex] ); };

    if ( section.tables.size() == 1 ) return createTableWidget( sharedTable( 0 ), viewStateKey( 0 ) );

    auto splitter = new QSplitter( Qt::Vertical );
    for ( size_t tableIndex = 0; tableIndex < section.tables.size(); tableIndex++ )
    {
        splitter->addWidget( createTableWidget( sharedTable( tableIndex ), viewStateKey( tableIndex ) ) );
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
QWidget* RiuReportWindow::createTableWidget( std::shared_ptr<const RiaReportTable> table, const QString& viewStateKey )
{
    auto widget = new QWidget;
    auto layout = new QVBoxLayout( widget );
    layout->setContentsMargins( 2, 2, 2, 6 );
    layout->setSpacing( 2 );

    if ( !table->title.isEmpty() )
    {
        auto  titleLabel = new QLabel( table->title, widget );
        QFont font       = titleLabel->font();
        font.setBold( true );
        titleLabel->setFont( font );
        titleLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
        layout->addWidget( titleLabel );
    }

    if ( !table->description.isEmpty() )
    {
        auto descriptionLabel = new QLabel( table->description, widget );
        descriptionLabel->setWordWrap( true );
        descriptionLabel->setTextFormat( Qt::PlainText );
        descriptionLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
        descriptionLabel->setStyleSheet( QString( "color: %1;" ).arg( mutedTextColor().name() ) );
        layout->addWidget( descriptionLabel );
    }

    auto tableView = new QTableView( widget );
    auto model     = new RiuReportTableModel( std::move( table ), formatOptions(), mutedTextColor(), tableView );

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
    tableView->setSelectionMode( QAbstractItemView::NoSelection );
    tableView->setFocusPolicy( Qt::ClickFocus );
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

    // The omitted row changes when the table is sorted
    auto onRowsChanged = [tableView, model]()
    {
        tableView->clearSpans();
        if ( model->omittedRowIndex() >= 0 && model->columnCount() > 1 )
        {
            tableView->setSpan( model->omittedRowIndex(), 0, 1, model->columnCount() );
        }
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
             [this, tableView]( const QPoint& position ) { showColumnMenu( tableView, position ); } );

    new TableCopyHandler(
        tableView,
        [this]() { copyCurrentTab( RifReportTableFormatter::Format::TEXT ); },
        [this]( const QPoint& globalPosition ) { showTableContextMenu( globalPosition ); } );

    // The table is placed above its title and description
    layout->insertWidget( 0, tableView, 1 );

    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::showTableContextMenu( const QPoint& globalPosition )
{
    using Format = RifReportTableFormatter::Format;

    // Not parented to the table view, which can be deleted while the menu is open
    QMenu menu;

    auto copyTextAction = menu.addAction( QIcon( ":/Copy.svg" ), "Copy Text" );
    copyTextAction->setShortcut( QKeySequence::Copy );
    connect( copyTextAction, &QAction::triggered, this, [this]() { copyCurrentTab( Format::TEXT ); } );

    auto copyCsvAction = menu.addAction( QIcon( ":/Copy.svg" ), "Copy CSV" );
    copyCsvAction->setToolTip( "Copy the data in the current tab. The CSV separator is defined in Preferences." );
    connect( copyCsvAction, &QAction::triggered, this, [this]() { copyCurrentTab( Format::CSV ); } );

    menu.addSeparator();

    auto exportTextAction = menu.addAction( QIcon( ":/export.svg" ), "Export Text..." );
    connect( exportTextAction, &QAction::triggered, this, [this]() { exportToFile( Format::TEXT ); } );

    auto exportCsvAction = menu.addAction( QIcon( ":/export.svg" ), "Export CSV..." );
    connect( exportCsvAction, &QAction::triggered, this, [this]() { exportToFile( Format::CSV ); } );

    const bool hasData = !m_sections->empty();
    for ( auto action : menu.actions() )
    {
        action->setEnabled( hasData );
    }

    m_isMenuOpen = true;
    menu.exec( globalPosition );
    m_isMenuOpen = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::copyCurrentTab( RifReportTableFormatter::Format format )
{
    const int index = m_tabWidget->currentIndex();
    if ( index < 0 || index >= static_cast<int>( m_sections->size() ) ) return;

    auto clipboard = QApplication::clipboard();
    if ( !clipboard ) return;

    clipboard->setText( RifReportTableFormatter::format( ( *m_sections )[index], format, formatOptions() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::exportToFile( RifReportTableFormatter::Format format )
{
    if ( m_sections->empty() ) return;

    const bool    isCsv  = format == RifReportTableFormatter::Format::CSV;
    const QString filter = isCsv ? "CSV File (*.csv)" : "Text File (*.txt)";
    const QString suffix = isCsv ? "csv" : "txt";

    const QString baseName        = caf::Utils::makeValidFileBasename( m_title.isEmpty() ? QString( "PlotData" ) : m_title );
    const QString defaultFileName = RicAsciiExportSummaryPlotFeature::defaultExportDir() + "/" + baseName + "." + suffix;

    const QString fileName = RiuFileDialogTools::getSaveFileName( this, "Export Plot Data", defaultFileName, filter + ";;All files(*.*)" );
    if ( fileName.isEmpty() ) return;

    RiaApplication::instance()->setLastUsedDialogDirectory( "PLOT_ASCIIEXPORT_DIR", QFileInfo( fileName ).absolutePath() );

    loadAllSections();

    const auto text = RifReportTableFormatter::format( m_title, *m_sections, format, formatOptions() );
    if ( !RicAsciiExportSummaryPlotFeature::exportTextToFile( fileName, text ) )
    {
        RiaLogging::error( std::format( "Could not write plot data to file {}", fileName ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::onFormatOptionsChanged()
{
    m_precisionSpinBox->setEnabled( numberFormat() != RimPlotAxisProperties::NUMBER_FORMAT_AUTO );
    saveSettings();
    rebuildContent();
}

//--------------------------------------------------------------------------------------------------
/// Restore the output and number format settings from the previous session
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup( "reportWindow" );

    auto selectData = []( QComboBox* combo, const QVariant& data )
    {
        const int index = combo->findData( data );
        if ( index >= 0 ) combo->setCurrentIndex( index );
    };

    selectData( m_numberFormatCombo, settings.value( "numberFormat", m_numberFormatCombo->currentData() ) );

    const int precision = settings.value( "decimals", m_precisionSpinBox->value() ).toInt();
    m_precisionSpinBox->setValue( std::clamp( precision, m_precisionSpinBox->minimum(), m_precisionSpinBox->maximum() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup( "reportWindow" );
    settings.setValue( "numberFormat", m_numberFormatCombo->currentData() );
    settings.setValue( "decimals", m_precisionSpinBox->value() );
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

    QMenu menu;
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
                    tableView,
                    [tableView]()
                    {
                        for ( int col = 0; col < tableView->model()->columnCount(); col++ )
                        {
                            tableView->setColumnHidden( col, false );
                        }
                    } );

    const QPoint globalPosition = tableView->horizontalHeader()->mapToGlobal( position );

    m_isMenuOpen = true;
    menu.exec( globalPosition );
    m_isMenuOpen = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReportFormatOptions RiuReportWindow::formatOptions() const
{
    RifReportFormatOptions options;
    switch ( numberFormat() )
    {
        case RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL:
            options.doubleFormat    = RifReportFormatOptions::DoubleFormat::FLOAT;
            options.doublePrecision = m_precisionSpinBox->value();
            break;
        case RimPlotAxisProperties::NUMBER_FORMAT_SCIENTIFIC:
            options.doubleFormat    = RifReportFormatOptions::DoubleFormat::SCIENTIFIC;
            options.doublePrecision = m_precisionSpinBox->value();
            break;
        case RimPlotAxisProperties::NUMBER_FORMAT_AUTO:
        default:
            options.doubleFormat    = RifReportFormatOptions::DoubleFormat::COLUMN_DEFAULT;
            options.doublePrecision = -1;
            break;
    }
    options.delimiter = RiaPreferences::current()->csvTextExportFieldSeparator();
    if ( options.delimiter.isEmpty() ) options.delimiter = ",";
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties::NumberFormatType RiuReportWindow::numberFormat() const
{
    return static_cast<RimPlotAxisProperties::NumberFormatType>( m_numberFormatCombo->currentData().toInt() );
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
