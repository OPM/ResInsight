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

#include "RimPlot.h"

#include "RicfCommandObject.h"

#include "RimAbstractPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimPlotCurve.h"
#include "RimPlotWindow.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuPlotWidget.h"

#include "cafPdmObject.h"

#include "qwt_legend_data.h"

namespace caf
{
template <>
void RimPlot::RowOrColSpanEnum::setUp()
{
    addItem( RimPlot::UNLIMITED, "UNLIMITED", "Unlimited" );
    addItem( RimPlot::ONE, "ONE", "1" );
    addItem( RimPlot::TWO, "TWO", "2" );
    addItem( RimPlot::THREE, "THREE", "3" );
    addItem( RimPlot::FOUR, "FOUR", "4" );
    addItem( RimPlot::FIVE, "FIVE", "5" );
    addItem( RimPlot::SIX, "SIX", "6" );
    setDefault( RimPlot::ONE );
}
} // namespace caf

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlot, "RimPlot" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::RimPlot()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Plot", "", "", "", "Plot", "The Abstract Base Class for all Plot Objects" );

    CAF_PDM_InitFieldNoDefault( &m_rowSpan, "RowSpan", "Row Span" );
    CAF_PDM_InitFieldNoDefault( &m_colSpan, "ColSpan", "Col Span" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::~RimPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimPlot::createViewWidget( QWidget* parent /*= nullptr */ )
{
    RiuPlotWidget* plotWidget = doCreatePlotViewWidget( parent );

    updateWindowVisibility();
    if ( showWindow() ) plotWidget->scheduleReplot();

    return plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::updateFonts()
{
    if ( plotWidget() )
    {
        plotWidget()->setPlotTitleFontSize( titleFontSize() );
        plotWidget()->setLegendFontSize( legendFontSize() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimPlot::createPlotWidget( QWidget* parent )
{
    return createViewWidget( parent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::RowOrColSpan RimPlot::rowSpan() const
{
    return m_rowSpan();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::RowOrColSpan RimPlot::colSpan() const
{
    return m_colSpan();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::setRowSpan( RowOrColSpan rowSpan )
{
    m_rowSpan = rowSpan;
    updateParentLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::setColSpan( RowOrColSpan colSpan )
{
    m_colSpan = colSpan;
    updateParentLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::removeFromMdiAreaAndCollection()
{
    if ( isMdiWindow() )
    {
        revokeMdiWindowStatus();
    }

    auto plotCollection = firstAncestorOfType<RimAbstractPlotCollection>();
    if ( plotCollection )
    {
        plotCollection->removeRimPlot( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::updateAfterInsertingIntoMultiPlot()
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !isMdiWindow() )
    {
        uiOrdering.add( &m_rowSpan );
        uiOrdering.add( &m_colSpan );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showWindow )
    {
        updateParentLayout();
    }
    else if ( changedField == &m_colSpan || changedField == &m_rowSpan )
    {
        updateParentLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( plotWidget() )
    {
        plotWidget()->renderTo( paintDevice, plotWidget()->frameGeometry() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onPlotSelected( bool toggle )
{
    RiuPlotMainWindowTools::selectOrToggleObject( this, toggle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onViewerDestroyed()
{
    auto parent = firstAncestorOfType<RimPlotWindow>();

    bool isIndependentPlot = parent == nullptr;
    bool hasVisibleParent  = parent && parent->showWindow();
    if ( isIndependentPlot || hasVisibleParent )
    {
        updateConnectedEditors();
        updateUiIconFromToggleField();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onKeyPressEvent( QKeyEvent* event )
{
    handleKeyPressEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onWheelEvent( QWheelEvent* event )
{
    handleWheelEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::updatePlotWidgetFromAxisRanges()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::updateAxisRangesFromPlotWidget()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RimPlot::visibleCurvesForLegend()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::handleKeyPressEvent( QKeyEvent* event )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::handleWheelEvent( QWheelEvent* event )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlot::isCurveHighlightSupported() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::any RimPlot::valueForKey( std::string key ) const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::zoomAllForMultiPlot()
{
    // Default behavior is to call zoomAll() on the current plot. Override this function to find the parent multi plot and do zoomAll(). See
    // RimSummaryPlot::zoomAllForMultiPlot()
    zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onPlotItemSelected( std::shared_ptr<RiuPlotItem>, bool, int )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onAxisSelected( RiuPlotAxis axis, bool toggle )
{
}
