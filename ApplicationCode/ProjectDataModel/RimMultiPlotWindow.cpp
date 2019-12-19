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
#include "RimMultiPlotWindow.h"

#include "RimPlot.h"

#include "RiuMultiPlotWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include <QPaintDevice>
#include <QRegularExpression>

#include <cvfAssert.h>

namespace caf
{
template <>
void RimMultiPlotWindow::ColumnCountEnum::setUp()
{
    addItem( RimMultiPlotWindow::COLUMNS_1, "1", "1 Column" );
    addItem( RimMultiPlotWindow::COLUMNS_2, "2", "2 Columns" );
    addItem( RimMultiPlotWindow::COLUMNS_3, "3", "3 Columns" );
    addItem( RimMultiPlotWindow::COLUMNS_4, "4", "4 Columns" );
    addItem( RimMultiPlotWindow::COLUMNS_UNLIMITED, "UNLIMITED", "Unlimited" );
    setDefault( RimMultiPlotWindow::COLUMNS_2 );
}
template <>
void RimMultiPlotWindow::RowCountEnum::setUp()
{
    addItem( RimMultiPlotWindow::ROWS_1, "1", "1 Row" );
    addItem( RimMultiPlotWindow::ROWS_2, "2", "2 Rows" );
    addItem( RimMultiPlotWindow::ROWS_3, "3", "3 Rows" );
    addItem( RimMultiPlotWindow::ROWS_4, "4", "4 Rows" );
    setDefault( RimMultiPlotWindow::ROWS_2 );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimMultiPlotWindow, "MultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotWindow::RimMultiPlotWindow( bool hidePlotsInTreeView )
    : m_acceptDrops( true )
{
    CAF_PDM_InitObject( "Multi Plot", ":/WellLogPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_showPlotWindowTitle, "ShowTitleInPlot", true, "Show Title", "", "", "" );
    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_plots, "Tracks", "", "", "", "" );
    m_plots.uiCapability()->setUiHidden( true );
    m_plots.uiCapability()->setUiTreeChildrenHidden( hidePlotsInTreeView );

    CAF_PDM_InitFieldNoDefault( &m_columnCount, "NumberOfColumns", "Number of Columns", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_rowsPerPage, "RowsPerPage", "Rows per Page", "", "", "" );

    CAF_PDM_InitField( &m_showIndividualPlotTitles, "ShowPlotTitles", false, "Show Sub Plot Titles", "", "", "" );

    m_viewer = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotWindow::~RimMultiPlotWindow()
{
    removeMdiWindowFromMdiArea();
    m_plots.deleteAllChildObjects();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
/// Move-assignment operator. Argument has to be passed with std::move()
//--------------------------------------------------------------------------------------------------
RimMultiPlotWindow& RimMultiPlotWindow::operator=( RimMultiPlotWindow&& rhs )
{
    RimPlotWindow::operator=( std::move( rhs ) );

    // Move all tracks
    std::vector<RimPlot*> plots = rhs.m_plots.childObjects();
    rhs.m_plots.clear();
    for ( RimPlot* plot : plots )
    {
        m_plots.push_back( plot );
    }

    m_showPlotWindowTitle      = rhs.m_showPlotWindowTitle;
    m_plotWindowTitle          = rhs.m_plotWindowTitle;
    m_columnCount              = rhs.m_columnCount;
    m_rowsPerPage              = rhs.m_rowsPerPage;
    m_showIndividualPlotTitles = rhs.m_showIndividualPlotTitles;

    m_acceptDrops = rhs.m_acceptDrops;

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimMultiPlotWindow::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiPlotWindow::description() const
{
    return multiPlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlotWindow::isMultiPlotTitleVisible() const
{
    return m_showPlotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::setMultiPlotTitleVisible( bool visible )
{
    m_showPlotWindowTitle = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiPlotWindow::multiPlotTitle() const
{
    return m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::setMultiPlotTitle( const QString& title )
{
    m_plotWindowTitle = title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::addPlot( RimPlot* plot )
{
    insertPlot( plot, m_plots.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::insertPlot( RimPlot* plot, size_t index )
{
    if ( plot )
    {
        m_plots.insert( index, plot );
        plot->setShowWindow( true );

        if ( m_viewer )
        {
            plot->createPlotWidget();
            m_viewer->insertPlot( plot->viewer(), index );
        }
        plot->setLegendsVisible( false );

        onPlotAdditionOrRemoval();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::removePlot( RimPlot* plot )
{
    if ( plot )
    {
        if ( m_viewer )
        {
            m_viewer->removePlot( plot->viewer() );
        }
        m_plots.removeChildObject( plot );

        onPlotAdditionOrRemoval();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::movePlotsToThis( const std::vector<RimPlot*>& plotsToMove, RimPlot* plotToInsertAfter )
{
    for ( size_t tIdx = 0; tIdx < plotsToMove.size(); tIdx++ )
    {
        RimMultiPlotWindow* previousMultiPlotWindow = nullptr;
        plotsToMove[tIdx]->firstAncestorOrThisOfType( previousMultiPlotWindow );
        if ( previousMultiPlotWindow )
        {
            previousMultiPlotWindow->removePlot( plotsToMove[tIdx] );
        }
        else
        {
            plotsToMove[tIdx]->removeFromMdiAreaAndCollection();
        }
    }

    size_t insertionStartIndex = 0;
    if ( plotToInsertAfter ) insertionStartIndex = this->plotIndex( plotToInsertAfter ) + 1;

    for ( size_t tIdx = 0; tIdx < plotsToMove.size(); tIdx++ )
    {
        this->insertPlot( plotsToMove[tIdx], insertionStartIndex + tIdx );
    }

    this->updateLayout();
    this->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimMultiPlotWindow::plotCount() const
{
    return m_plots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimMultiPlotWindow::plotIndex( const RimPlot* plot ) const
{
    return m_plots.index( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RimMultiPlotWindow::plotByIndex( size_t index ) const
{
    return m_plots[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimMultiPlotWindow::plots() const
{
    return m_plots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimMultiPlotWindow::visiblePlots() const
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
void RimMultiPlotWindow::doUpdateLayout()
{
    if ( m_showWindow && m_viewer )
    {
        m_viewer->scheduleUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::updateSubPlotNames() {}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::updatePlotWindowTitle() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::doSetAutoScaleYEnabled( bool enabled )
{
    for ( RimPlot* plot : plots() )
    {
        plot->setAutoScaleYEnabled( enabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewer )
    {
        m_viewer->renderTo( paintDevice );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::updatePlotOrderFromGridWidget()
{
    std::sort( m_plots.begin(), m_plots.end(), [this]( RimPlot* lhs, RimPlot* rhs ) {
        auto indexLhs = m_viewer->indexOfPlotWidget( lhs->viewer() );
        auto indexRhs = m_viewer->indexOfPlotWidget( rhs->viewer() );
        return indexLhs < indexRhs;
    } );
    updateSubPlotNames();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::setAutoScaleXEnabled( bool enabled )
{
    for ( RimPlot* plot : plots() )
    {
        plot->setAutoScaleXEnabled( enabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::setAutoScaleYEnabled( bool enabled )
{
    doSetAutoScaleYEnabled( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultiPlotWindow::columnCount() const
{
    if ( m_columnCount() == COLUMNS_UNLIMITED )
    {
        return std::numeric_limits<int>::max();
    }
    return static_cast<int>( m_columnCount() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultiPlotWindow::rowsPerPage() const
{
    return static_cast<int>( m_rowsPerPage() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMultiPlotWindow::columnCountField()
{
    return &m_columnCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMultiPlotWindow::rowsPerPageField()
{
    return &m_rowsPerPage;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlotWindow::showPlotTitles() const
{
    return m_showIndividualPlotTitles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updateZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiPlotWindow::asciiDataForPlotExport() const
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
void RimMultiPlotWindow::onPlotAdditionOrRemoval()
{
    updateSubPlotNames();
    updatePlotWindowTitle();
    applyPlotWindowTitleToWidgets();
    updateConnectedEditors();
    updateLayout();
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::setAcceptDrops( bool acceptDrops )
{
    m_acceptDrops = acceptDrops;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlotWindow::acceptDrops() const
{
    return m_acceptDrops;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimMultiPlotWindow::snapshotWindowContent()
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
QWidget* RimMultiPlotWindow::createViewWidget( QWidget* mainWindowParent )
{
    if ( m_viewer.isNull() )
    {
        m_viewer = new RiuMultiPlotWindow( this, mainWindowParent );
    }
    recreatePlotWidgets();
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMultiPlotWindow::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showIndividualPlotTitles )
    {
        updateLayout();
    }
    else if ( changedField == &m_showPlotWindowTitle || changedField == &m_plotWindowTitle )
    {
        updatePlotWindowTitle();
        applyPlotWindowTitleToWidgets();
    }
    else if ( changedField == &m_columnCount || changedField == &m_rowsPerPage )
    {
        updateLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* titleAndLegendsGroup = uiOrdering.addNewGroup( "Plot Layout" );
    uiOrderingForPlotLayout( uiConfigName, *titleAndLegendsGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::uiOrderingForPlotLayout( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotWindowTitle );
    uiOrdering.add( &m_plotWindowTitle );
    uiOrdering.add( &m_showIndividualPlotTitles );
    RimPlotWindow::uiOrderingForLegendSettings( uiConfigName, uiOrdering );
    uiOrdering.add( &m_columnCount );
    uiOrdering.add( &m_rowsPerPage );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultiPlotWindow::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_columnCount )
    {
        for ( size_t i = 0; i < ColumnCountEnum::size(); ++i )
        {
            ColumnCount enumVal = ColumnCountEnum::fromIndex( i );
            if ( enumVal == COLUMNS_UNLIMITED )
            {
                QString iconPath( ":/ColumnsUnlimited.png" );
                options.push_back( caf::PdmOptionItemInfo( ColumnCountEnum::uiText( enumVal ),
                                                           enumVal,
                                                           false,
                                                           caf::QIconProvider( iconPath ) ) );
            }
            else
            {
                QString iconPath = QString( ":/Columns%1.png" ).arg( static_cast<int>( enumVal ) );
                options.push_back( caf::PdmOptionItemInfo( ColumnCountEnum::uiText( enumVal ),
                                                           enumVal,
                                                           false,
                                                           caf::QIconProvider( iconPath ) ) );
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    updatePlotWindowTitle();
    applyPlotWindowTitleToWidgets();
    updatePlots();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::initAfterRead()
{
    RimPlotWindow::initAfterRead();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::applyPlotWindowTitleToWidgets()
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
void RimMultiPlotWindow::updatePlots()
{
    if ( m_showWindow )
    {
        for ( RimPlot* plot : plots() )
        {
            plot->loadDataAndUpdate();
        }
        this->updateZoom();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::updateZoom()
{
    for ( RimPlot* plot : plots() )
    {
        plot->updateZoomInQwt();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::recreatePlotWidgets()
{
    CVF_ASSERT( m_viewer );

    auto plotVector = plots();

    for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    {
        plotVector[tIdx]->createPlotWidget();
        m_viewer->addPlot( plotVector[tIdx]->viewer() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlotWindow::hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
{
    if ( fontSettingType == RiaDefines::PLOT_FONT && m_viewer )
    {
        if ( m_viewer->fontSize() != defaultFontSize )
        {
            return true;
        }
        if ( m_legendFontSize() != defaultFontSize )
        {
            return true;
        }
        for ( const RimPlot* plot : plots() )
        {
            if ( plot->hasCustomFontSizes( fontSettingType, defaultFontSize ) )
            {
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlotWindow::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                        int                         oldFontSize,
                                        int                         fontSize,
                                        bool                        forceChange /*= false */ )
{
    bool somethingChanged = false;
    if ( fontSettingType == RiaDefines::PLOT_FONT && m_viewer )
    {
        if ( oldFontSize == m_viewer->fontSize() || forceChange )
        {
            m_viewer->setFontSize( fontSize );
            somethingChanged = true;
        }

        if ( oldFontSize == m_legendFontSize() || forceChange )
        {
            m_legendFontSize = fontSize;
            somethingChanged = true;
        }

        for ( RimPlot* plot : plots() )
        {
            if ( plot->applyFontSize( fontSettingType, oldFontSize, fontSize, forceChange ) )
            {
                somethingChanged = true;
            }
        }
        if ( somethingChanged )
        {
            m_viewer->scheduleUpdate();
        }
    }
    return somethingChanged;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlotWindow::cleanupBeforeClose()
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
