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

#include "RiaApplication.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuMultiPlotBook.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include <QPaintDevice>
#include <QRegularExpression>

#include <cvfAssert.h>

namespace caf
{
template <>
void RimMultiPlot::ColumnCountEnum::setUp()
{
    addItem( RimMultiPlot::ColumnCount::COLUMNS_1, "1", "1 Column" );
    addItem( RimMultiPlot::ColumnCount::COLUMNS_2, "2", "2 Columns" );
    addItem( RimMultiPlot::ColumnCount::COLUMNS_3, "3", "3 Columns" );
    addItem( RimMultiPlot::ColumnCount::COLUMNS_4, "4", "4 Columns" );
    addItem( RimMultiPlot::ColumnCount::COLUMNS_UNLIMITED, "UNLIMITED", "Unlimited" );
    setDefault( RimMultiPlot::ColumnCount::COLUMNS_2 );
}
template <>
void RimMultiPlot::RowCountEnum::setUp()
{
    addItem( RimMultiPlot::ROWS_1, "1", "1 Row" );
    addItem( RimMultiPlot::ROWS_2, "2", "2 Rows" );
    addItem( RimMultiPlot::ROWS_3, "3", "3 Rows" );
    addItem( RimMultiPlot::ROWS_4, "4", "4 Rows" );
    setDefault( RimMultiPlot::ROWS_2 );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimMultiPlot, "MultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot::RimMultiPlot()
{
    CAF_PDM_InitObject( "Multi Plot", ":/MultiPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_showPlotWindowTitle, "ShowTitleInPlot", true, "Show Title", "", "", "" );
    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_plots, "Plots", "", "", "", "" );
    m_plots.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_columnCount, "NumberOfColumns", "Number of Columns", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_rowsPerPage, "RowsPerPage", "Rows per Page", "", "", "" );

    CAF_PDM_InitField( &m_showIndividualPlotTitles, "ShowPlotTitles", true, "Show Sub Plot Titles", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_majorTickmarkCount, "MajorTickmarkCount", "Major Tickmark Count", "", "", "" );

    m_viewer = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot::~RimMultiPlot()
{
    removeMdiWindowFromMdiArea();
    m_plots.deleteAllChildObjects();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
/// Move-assignment operator. Argument has to be passed with std::move()
//--------------------------------------------------------------------------------------------------
RimMultiPlot& RimMultiPlot::operator=( RimMultiPlot&& rhs )
{
    RimPlotWindow::operator=( std::move( rhs ) );

    // Move all tracks
    std::vector<RimPlot*> plots = rhs.m_plots.childObjects();
    rhs.m_plots.clear();
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::addPlot( RimPlot* plot )
{
    insertPlot( plot, m_plots.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::insertPlot( RimPlot* plot, size_t index )
{
    if ( plot )
    {
        m_plots.insert( index, plot );

        if ( m_viewer )
        {
            plot->createPlotWidget();
            m_viewer->insertPlot( plot->viewer(), index );
        }
        plot->setShowWindow( true );
        plot->updateAfterInsertingIntoMultiPlot();

        onPlotAdditionOrRemoval();
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
            m_viewer->removePlot( plot->viewer() );
        }
        m_plots.removeChildObject( plot );

        onPlotAdditionOrRemoval();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::movePlotsToThis( const std::vector<RimPlot*>& plotsToMove, RimPlot* plotToInsertAfter )
{
    for ( size_t tIdx = 0; tIdx < plotsToMove.size(); tIdx++ )
    {
        RimMultiPlot* previousMultiPlotWindow = nullptr;
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
size_t RimMultiPlot::plotCount() const
{
    return m_plots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimMultiPlot::plotIndex( const RimPlot* plot ) const
{
    return m_plots.index( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RimMultiPlot::plotByIndex( size_t index ) const
{
    return m_plots[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimMultiPlot::plots() const
{
    return m_plots.childObjects();
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
void RimMultiPlot::updatePlotWindowTitle()
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
int RimMultiPlot::columnCount() const
{
    if ( m_columnCount() == ColumnCount::COLUMNS_UNLIMITED )
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
    return static_cast<int>( m_rowsPerPage() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMultiPlot::columnCountField()
{
    return &m_columnCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMultiPlot::rowsPerPageField()
{
    return &m_rowsPerPage;
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
    updatePlotWindowTitle();
    applyPlotWindowTitleToWidgets();
    updateConnectedEditors();
    updateLayout();
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiPlot::previewModeEnabled() const
{
    if ( m_viewer )
    {
        return m_viewer->previewModeEnabled();
    }
    return false;
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
        updatePlotWindowTitle();
        applyPlotWindowTitleToWidgets();
    }
    else if ( changedField == &m_columnCount || changedField == &m_rowsPerPage )
    {
        updateLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField = &m_majorTickmarkCount )
    {
        for ( RimPlot* plot : plots() )
        {
            std::vector<RimSummaryTimeAxisProperties*> timeAxisProps;
            plot->descendantsIncludingThisOfType( timeAxisProps );
            for ( auto tap : timeAxisProps )
            {
                tap->setMajorTickmarkCount( m_majorTickmarkCount() );
            }
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::uiOrderingForMultiPlotLayout( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotWindowTitle );
    uiOrdering.add( &m_plotWindowTitle );
    uiOrdering.add( &m_showIndividualPlotTitles );
    RimPlotWindow::uiOrderingForPlotLayout( uiConfigName, uiOrdering );
    uiOrdering.add( &m_columnCount );
    uiOrdering.add( &m_rowsPerPage );
    uiOrdering.add( &m_majorTickmarkCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultiPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                   bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_columnCount )
    {
        for ( size_t i = 0; i < ColumnCountEnum::size(); ++i )
        {
            ColumnCount enumVal = ColumnCountEnum::fromIndex( i );
            if ( enumVal == ColumnCount::COLUMNS_UNLIMITED )
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
void RimMultiPlot::onLoadDataAndUpdate()
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
        this->updateZoom();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::updateZoom()
{
    for ( RimPlot* plot : plots() )
    {
        plot->updateZoomInQwt();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiPlot::recreatePlotWidgets()
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
bool RimMultiPlot::hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
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
bool RimMultiPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
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
