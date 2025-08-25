/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimMultiPlot.h"

#include "RiaPreferences.h"
#include "RiaPreferencesSummary.h"
#include "RiaVersionInfo.h"

#include "RimPlot.h"
#include "RimProject.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuMultiPlotBook.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiToolButtonEditor.h"

#include <QPaintDevice>
#include <QRegularExpression>

#include <cvfAssert.h>

CAF_PDM_SOURCE_INIT( RimMultiPlot, "MultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot::RimMultiPlot()
    : m_isValid( true )
    , m_delayPlotUpdatesDuringBatchAdd( false )
{
    CAF_PDM_InitObject( "Multi Plot", ":/MultiPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_projectFileVersionString, "ProjectFileVersionString", "" );
    m_projectFileVersionString.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showPlotWindowTitle, "ShowTitleInPlot", true, "Show Title" );
    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_plots, "Plots", "" );
    auto reorderability = caf::PdmFieldReorderCapability::addToField( &m_plots );
    reorderability->orderChanged.connect( this, &RimMultiPlot::onPlotsReordered );

    RiaPreferencesSummary* sumPrefs = RiaPreferencesSummary::current();
    CAF_PDM_InitFieldNoDefault( &m_columnCount, "NumberOfColumns", "Number of Columns" );
    m_columnCount = sumPrefs->defaultMultiPlotColumnCount();
    CAF_PDM_InitFieldNoDefault( &m_rowsPerPage, "RowsPerPage", "Rows per Page" );
    m_rowsPerPage = sumPrefs->defaultMultiPlotRowCount();

    CAF_PDM_InitField( &m_showIndividualPlotTitles, "ShowPlotTitles", true, "Show Sub Plot Titles" );
    CAF_PDM_InitFieldNoDefault( &m_majorTickmarkCount, "MajorTickmarkCount", "Major Tickmark Count" );

    CAF_PDM_InitFieldNoDefault( &m_subTitleFontSize, "SubTitleFontSize", "Sub Plot Title Font Size" );
    m_subTitleFontSize = caf::FontTools::RelativeSize::Large;

    CAF_PDM_InitField( &m_pagePreviewMode, "PagePreviewMode", false, "Page Preview Mode", "", "Page Preview" );
    m_pagePreviewMode.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_pagePreviewMode.uiCapability()->setUiIconFromResourceString( ":/PagePreview16x16.png" );
    m_viewer = nullptr;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot::~RimMultiPlot()
{
    m_isValid = false;

    removeMdiWindowFromMdiArea();
    m_plots.deleteChildren();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
/// Move-assignment operator. Argument has to be passed with std::move()
//--------------------------------------------------------------------------------------------------
RimMultiPlot& RimMultiPlot::operator=( RimMultiPlot&& rhs )
{
    RimPlotWindow::operator=( std::move( rhs ) );

    // Move all tracks
    std::vector<RimPlot*> plots = rhs.m_plots.childrenByType();
    rhs.m_plots.clearWithoutDelete();
    for ( RimPlot* plot : plots )
    {
        m_plots.push_back( plot );
    }

    // Deliberately don't set m_plotWindowTitle. This operator is used for copying parameters from children.
    // This only happens for some plots that used to own a plot but now inherits the plot.
    // These all had their own description at top level which we don't want to overwrite.

    m_showPlotWindowTitle      = rhs.m_showPlotWindowTitle;
    m_columnCount              = rhs.m_columnCount;
    m_rowsPerPage              = rhs.m_rowsPerPage;
    m_showIndividualPlotTitles = rhs.m_showIndividualPlotTitles;
    m_subTitleFontSize         = rhs.m_subTitleFontSize;
    m_pagePreviewMode          = rhs.m_pagePreviewMode;

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimMultiPlot::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiPlot::description() const
{
    return multiPlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiPlot::projectFileVersionString() const
{
    return m_projectFileVersionString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlot::isMultiPlotTitleVisible() const
{
    return m_showPlotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setMultiPlotTitleVisible( bool visible )
{
    m_showPlotWindowTitle = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiPlot::multiPlotTitle() const
{
    return m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setMultiPlotTitle( const QString& title )
{
    m_plotWindowTitle = title;
    if ( !m_viewer.isNull() ) m_viewer->setPlotTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::insertPlot( RimPlot* plot, size_t index )
{
    if ( plot )
    {
        setTickmarkCount( plot, m_majorTickmarkCount() );

        if ( index > m_plots.size() ) index = m_plots.size();

        m_plots.insert( index, plot );

        if ( m_viewer )
        {
            plot->setShowWindow( true );
            plot->createPlotWidget( m_viewer );
            m_viewer->insertPlot( plot->plotWidget(), index );
        }

        if ( !m_delayPlotUpdatesDuringBatchAdd )
        {
            plot->updateAfterInsertingIntoMultiPlot();
            onPlotAdditionOrRemoval();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::removePlot( RimPlot* plot )
{
    if ( plot )
    {
        if ( m_viewer )
        {
            m_viewer->removePlot( plot->plotWidget() );
        }
        m_plots.removeChild( plot );

        onPlotAdditionOrRemoval();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::removePlotNoUpdate( RimPlot* plot )
{
    if ( plot )
    {
        if ( m_viewer )
        {
            m_viewer->removePlotNoUpdate( plot->plotWidget() );
        }
        m_plots.removeChild( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::updateAfterPlotRemove()
{
    onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::movePlotsToThis( const std::vector<RimPlot*>& plotsToMove, int insertAtPosition )
{
    for ( size_t tIdx = 0; tIdx < plotsToMove.size(); tIdx++ )
    {
        RimMultiPlot* previousMultiPlotWindow = plotsToMove[tIdx]->firstAncestorOrThisOfType<RimMultiPlot>();
        if ( previousMultiPlotWindow )
        {
            previousMultiPlotWindow->removePlot( plotsToMove[tIdx] );
        }
        else
        {
            plotsToMove[tIdx]->removeFromMdiAreaAndCollection();
        }
    }

    for ( size_t tIdx = 0; tIdx < plotsToMove.size(); tIdx++ )
    {
        if ( insertAtPosition >= 0 )
        {
            insertPlot( plotsToMove[tIdx], (size_t)insertAtPosition + tIdx );
        }
        else
        {
            addPlot( plotsToMove[tIdx] );
        }
    }

    updateLayout();
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::deleteAllPlots()
{
    for ( auto plot : m_plots() )
    {
        if ( plot && m_viewer )
        {
            m_viewer->removePlot( plot->plotWidget() );
        }
    }

    m_plots.deleteChildren();
    onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimMultiPlot::plotCount() const
{
    return m_plots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimMultiPlot::plotIndex( const RimPlot* plot ) const
{
    return m_plots.indexOf( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimMultiPlot::plots() const
{
    return m_plots.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimMultiPlot::visiblePlots() const
{
    std::vector<RimPlot*> allVisiblePlots;
    for ( RimPlot* plot : m_plots() )
    {
        if ( plot->showWindow() )
        {
            allVisiblePlots.push_back( plot );
        }
    }
    return allVisiblePlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::doUpdateLayout()
{
    if ( m_showWindow && m_viewer )
    {
        m_viewer->setPlotTitle( description() );
        m_viewer->setTitleVisible( m_showPlotWindowTitle );
        m_viewer->setSubTitlesVisible( m_showIndividualPlotTitles );

        m_viewer->setTitleFontSizes( titleFontSize(), subTitleFontSize() );
        m_viewer->setLegendFontSize( legendFontSize() );
        m_viewer->setAxisFontSizes( axisTitleFontSize(), axisValueFontSize() );
        m_viewer->setPagePreviewModeEnabled( m_pagePreviewMode() );

        m_viewer->scheduleUpdate();
        m_viewer->adjustSize();
    }
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::updateSubPlotNames()
{
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::updatePlotTitles()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewer )
    {
        m_viewer->renderTo( paintDevice );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::updatePlotOrderFromGridWidget()
{
    std::sort( m_plots.begin(),
               m_plots.end(),
               [this]( RimPlot* lhs, RimPlot* rhs )
               {
                   auto indexLhs = m_viewer->indexOfPlotWidget( lhs->plotWidget() );
                   auto indexRhs = m_viewer->indexOfPlotWidget( rhs->plotWidget() );
                   return indexLhs < indexRhs;
               } );
    updateSubPlotNames();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setAutoScaleXEnabled( bool enabled )
{
    for ( RimPlot* plot : plots() )
    {
        plot->setAutoScaleXEnabled( enabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setAutoScaleYEnabled( bool enabled )
{
    for ( RimPlot* plot : plots() )
    {
        plot->setAutoScaleYEnabled( enabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setColumnCount( RiaDefines::ColumnCount columnCount )
{
    m_columnCount = columnCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setRowCount( RiaDefines::RowCount rowCount )
{
    m_rowsPerPage = rowCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setTickmarkCount( RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum tickmarkCount )
{
    m_majorTickmarkCount = tickmarkCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setTickmarkCount( RimPlot* plot, RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum tickmarkCount )
{
    std::vector<RimSummaryTimeAxisProperties*> timeAxisProps = plot->descendantsIncludingThisOfType<RimSummaryTimeAxisProperties>();
    for ( auto tap : timeAxisProps )
    {
        tap->setMajorTickmarkCount( tickmarkCount );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultiPlot::columnCount() const
{
    if ( m_columnCount() == RiaDefines::ColumnCount::COLUMNS_UNLIMITED )
    {
        return std::numeric_limits<int>::max();
    }
    return static_cast<int>( m_columnCount().value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultiPlot::rowsPerPage() const
{
    RiaDefines::RowCount rowEnum = m_rowsPerPage().value();

    int rowCount = 2;

    switch ( rowEnum )
    {
        case RiaDefines::RowCount::ROWS_1:
            rowCount = 1;
            break;
        case RiaDefines::RowCount::ROWS_2:
            rowCount = 2;
            break;
        case RiaDefines::RowCount::ROWS_3:
            rowCount = 3;
            break;
        case RiaDefines::RowCount::ROWS_4:
            rowCount = 4;
            break;
        default:
            break;
    }

    return rowCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setShowPlotTitles( bool enable )
{
    m_showIndividualPlotTitles = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlot::showPlotTitles() const
{
    return m_showIndividualPlotTitles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updateZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::zoomAllYAxes()
{
    setAutoScaleYEnabled( true );
    updateZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiPlot::asciiDataForPlotExport() const
{
    QString out = multiPlotTitle() + "\n";

    for ( RimPlot* plot : plots() )
    {
        if ( plot->showWindow() )
        {
            out += plot->asciiDataForPlotExport();
        }
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::onPlotAdditionOrRemoval()
{
    updateSubPlotNames();
    updatePlotTitles();
    applyPlotWindowTitleToWidgets();
    updateAllRequiredEditors();
    updateLayout();
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::onPlotsReordered( const caf::SignalEmitter* emitter )
{
    updateSubPlotNames();
    recreatePlotWidgets();
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateSubPlotNames();
    recreatePlotWidgets();
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlot::previewModeEnabled() const
{
    if ( m_viewer )
    {
        return m_viewer->pagePreviewModeEnabled();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultiPlot::subTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_subTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultiPlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultiPlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimMultiPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        QPixmap pix( m_viewer->size() );
        m_viewer->renderTo( &pix );
        image = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimMultiPlot::createViewWidget( QWidget* mainWindowParent )
{
    if ( m_viewer.isNull() )
    {
        m_viewer = new RiuMultiPlotBook( this, mainWindowParent );
    }
    recreatePlotWidgets();

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMultiPlot::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showIndividualPlotTitles )
    {
        updateLayout();
    }
    else if ( changedField == &m_showPlotWindowTitle || changedField == &m_plotWindowTitle )
    {
        updatePlotTitles();
        applyPlotWindowTitleToWidgets();
    }
    else if ( changedField == &m_subTitleFontSize )
    {
        updateFonts();
    }
    else if ( changedField == &m_columnCount || changedField == &m_rowsPerPage || changedField == &m_pagePreviewMode )
    {
        updateLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_majorTickmarkCount )
    {
        for ( RimPlot* plot : plots() )
        {
            setTickmarkCount( plot, m_majorTickmarkCount() );
        }

        updatePlots();
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* titleAndLegendsGroup = uiOrdering.addNewGroup( "Plot Layout" );
    uiOrderingForMultiPlotLayout( uiConfigName, *titleAndLegendsGroup );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_rowsPerPage || field == &m_columnCount )
    {
        auto myattr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( myattr )
        {
            myattr->iconSize = QSize( 24, 16 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::uiOrderingForMultiPlotLayout( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotWindowTitle );
    uiOrdering.add( &m_plotWindowTitle );
    uiOrdering.add( &m_showIndividualPlotTitles );
    uiOrdering.add( &m_subTitleFontSize );

    RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );
    uiOrdering.add( &m_columnCount );
    uiOrdering.add( &m_rowsPerPage );
    uiOrdering.add( &m_majorTickmarkCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultiPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_columnCount )
    {
        for ( size_t i = 0; i < ColumnCountEnum::size(); ++i )
        {
            RiaDefines::ColumnCount enumVal = ColumnCountEnum::fromIndex( i );
            QString                 columnCountString =
                ( enumVal == RiaDefines::ColumnCount::COLUMNS_UNLIMITED ) ? "Unlimited" : QString( "%1" ).arg( static_cast<int>( enumVal ) );
            QString iconPath = QString( ":/Columns%1.png" ).arg( columnCountString );
            options.push_back(
                caf::PdmOptionItemInfo( ColumnCountEnum::uiText( enumVal ), enumVal, false, caf::IconProvider( iconPath, QSize( 24, 16 ) ) ) );
        }
    }
    if ( fieldNeedingOptions == &m_rowsPerPage )
    {
        for ( size_t i = 0; i < RowCountEnum::size(); ++i )
        {
            RiaDefines::RowCount enumVal  = RowCountEnum::fromIndex( i );
            QString              iconPath = QString( ":/Rows%1.png" ).arg( static_cast<int>( enumVal ) );
            options.push_back(
                caf::PdmOptionItemInfo( RowCountEnum::uiText( enumVal ), enumVal, false, caf::IconProvider( iconPath, QSize( 24, 16 ) ) ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::onLoadDataAndUpdate()
{
    // PERFORMANCE NOTE
    // Creation and update of the legend widgets is expensive. Disable display of legends during construction  of the
    // multi plot and creation of widgets. The legends will be made visible due to redraw operations always scheduled
    // after this method.
    bool originalShowState = m_showPlotLegends();
    m_showPlotLegends      = false;

    updateMdiWindowVisibility();
    updatePlotTitles();
    applyPlotWindowTitleToWidgets();
    updatePlots();
    updateLayout();
    RiuPlotMainWindowTools::refreshToolbars();

    m_showPlotLegends = originalShowState;

    if ( m_viewer ) m_viewer->forcePerformUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::initAfterRead()
{
    RimPlotWindow::initAfterRead();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::applyPlotWindowTitleToWidgets()
{
    if ( m_viewer )
    {
        m_viewer->setTitleVisible( m_showPlotWindowTitle() );
        m_viewer->setPlotTitle( multiPlotTitle() );
    }
    updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::updatePlots()
{
    if ( m_showWindow )
    {
        for ( RimPlot* plot : plots() )
        {
            plot->loadDataAndUpdate();
        }
        updateZoom();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::updateZoom()
{
    for ( RimPlot* plot : plots() )
    {
        plot->updatePlotWidgetFromAxisRanges();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::recreatePlotWidgets()
{
    CVF_ASSERT( m_viewer );

    m_viewer->removeAllPlots();

    auto plotVector = plots();

    for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    {
        plotVector[tIdx]->createPlotWidget( m_viewer );
        m_viewer->addPlot( plotVector[tIdx]->plotWidget() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::cleanupBeforeClose()
{
    auto plotVector = plots();
    for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    {
        plotVector[tIdx]->detachAllCurves();
    }

    if ( m_viewer )
    {
        m_viewer->setParent( nullptr );
        delete m_viewer;
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::setupBeforeSave()
{
    m_projectFileVersionString = STRPRODUCTVER;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlot::isMouseCursorInsidePlot()
{
    if ( !m_viewer ) return false;
    return m_viewer->underMouse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimMultiPlot::fieldsToShowInToolbar()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimMultiPlot::fieldsToShowInLayoutToolbar()
{
    return { &m_columnCount, &m_rowsPerPage, &m_pagePreviewMode };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlot::isValid() const
{
    return m_isValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::startBatchAddOperation()
{
    m_delayPlotUpdatesDuringBatchAdd = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::endBatchAddOperation()
{
    m_delayPlotUpdatesDuringBatchAdd = false;
    onPlotAdditionOrRemoval();
}
