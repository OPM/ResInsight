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
#include "RiuQwtPlotWidget.h"

#include "cafPdmObject.h"

#include "qwt_plot_curve.h"

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
    RiuQwtPlotWidget* plotWidget = doCreatePlotViewWidget( parent );

    RimPlot::attachPlotWidgetSignals( this, plotWidget );

    updateWindowVisibility();
    plotWidget->scheduleReplot();

    return plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::updateFonts()
{
    if ( viewer() )
    {
        viewer()->setPlotTitleFontSize( titleFontSize() );
        viewer()->setLegendFontSize( legendFontSize() );
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

    RimAbstractPlotCollection* plotCollection = nullptr;
    this->firstAncestorOfType( plotCollection );
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
    /*    updateLegend();
        updateAxes();
        updateLayout(); */
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
void RimPlot::attachPlotWidgetSignals( RimPlot* plot, RiuQwtPlotWidget* plotWidget )
{
    CAF_ASSERT( plot && plotWidget );
    plot->connect( plotWidget, SIGNAL( plotSelected( bool ) ), SLOT( onPlotSelected( bool ) ) );
    plot->connect( plotWidget, SIGNAL( axisSelected( int, bool ) ), SLOT( onAxisSelected( int, bool ) ) );
    plot->connect( plotWidget,
                   SIGNAL( plotItemSelected( QwtPlotItem*, bool, int ) ),
                   SLOT( onPlotItemSelected( QwtPlotItem*, bool, int ) ) );
    plot->connect( plotWidget, SIGNAL( onKeyPressEvent( QKeyEvent* ) ), SLOT( onKeyPressEvent( QKeyEvent* ) ) );
    plot->connect( plotWidget, SIGNAL( onWheelEvent( QWheelEvent* ) ), SLOT( onWheelEvent( QWheelEvent* ) ) );
    plot->connect( plotWidget, SIGNAL( destroyed() ), SLOT( onViewerDestroyed() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( viewer() )
    {
        viewer()->renderTo( paintDevice, viewer()->frameGeometry() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onPlotSelected( bool toggle )
{
    if ( toggle )
    {
        RiuPlotMainWindowTools::toggleItemInSelection( this );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int sampleIndex )
{
    QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>( plotItem );
    if ( curve )
    {
        RimPlotCurve* selectedCurve = dynamic_cast<RimPlotCurve*>( this->findPdmObjectFromQwtCurve( curve ) );
        if ( selectedCurve )
        {
            if ( toggle )
            {
                RiuPlotMainWindowTools::toggleItemInSelection( selectedCurve );
            }
            else
            {
                RiuPlotMainWindowTools::selectAsCurrentItem( selectedCurve );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlot::onViewerDestroyed()
{
    RimPlotWindow* parent = nullptr;
    this->firstAncestorOfType( parent );

    bool isIndependentPlot = parent == nullptr;
    bool hasVisibleParent  = parent && parent->showWindow();
    if ( isIndependentPlot || hasVisibleParent )
    {
        m_showWindow = false;
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
void RimPlot::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                              std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    loadDataAndUpdate();
}
