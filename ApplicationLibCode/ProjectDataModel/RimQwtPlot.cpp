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

#include "RimQwtPlot.h"

#include "RicfCommandObject.h"

#include "RimAbstractPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimPlotCurve.h"
#include "RimPlotWindow.h"
#include "RimQwtPlotCurve.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmObject.h"

#include "qwt_plot_curve.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimQwtPlot, "RimQwtPlot" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQwtPlot::RimQwtPlot()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Plot", "", "", "", "Plot", "The Abstract Base Class for all Plot Objects" );

    // CAF_PDM_InitFieldNoDefault( &m_rowSpan, "RowSpan", "Row Span", "", "", "" );
    // CAF_PDM_InitFieldNoDefault( &m_colSpan, "ColSpan", "Col Span", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQwtPlot::~RimQwtPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimQwtPlot::createViewWidget( QWidget* parent /*= nullptr */ )
{
    // TOOD: ugly!!!!
    RiuQwtPlotWidget* plotWidget = dynamic_cast<RiuQwtPlotWidget*>( doCreatePlotViewWidget( parent ) );

    RimQwtPlot::attachPlotWidgetSignals( this, plotWidget );

    updateWindowVisibility();
    plotWidget->scheduleReplot();

    return plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimQwtPlot::plotWidget()
{
    return viewer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimQwtPlot::createPlotWidget( QWidget* parent )
{
    return createViewWidget( parent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlot::attachPlotWidgetSignals( RimQwtPlot* plot, RiuQwtPlotWidget* plotWidget )
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
// void RimQwtPlot::doRenderWindowContent( QPaintDevice* paintDevice )
// {
//     if ( viewer() )
//     {
//         viewer()->renderTo( paintDevice, viewer()->frameGeometry() );
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlot::onPlotSelected( bool toggle )
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
void RimQwtPlot::onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int sampleIndex )
{
    QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>( plotItem );
    if ( curve )
    {
        RimQwtPlotCurve* selectedCurve = dynamic_cast<RimQwtPlotCurve*>( this->findPdmObjectFromQwtCurve( curve ) );
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
void RimQwtPlot::onViewerDestroyed()
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
void RimQwtPlot::onKeyPressEvent( QKeyEvent* event )
{
    handleKeyPressEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQwtPlot::onWheelEvent( QWheelEvent* event )
{
    handleWheelEvent( event );
}
