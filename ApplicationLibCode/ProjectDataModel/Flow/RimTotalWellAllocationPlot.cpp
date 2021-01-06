/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimTotalWellAllocationPlot.h"

#include "RiaPreferences.h"

#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"

#include "RigSimWellData.h"

#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuNightchartsWidget.h"
#include "RiuPlotMainWindow.h"
#include "RiuWellAllocationPlot.h"

#include "cvfColor3.h"

CAF_PDM_SOURCE_INIT( RimTotalWellAllocationPlot, "TotalWellAllocationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTotalWellAllocationPlot::RimTotalWellAllocationPlot()
{
    CAF_PDM_InitObject( "Total Allocation", ":/WellAllocPie16x16.png", "", "" );

    CAF_PDM_InitField( &m_userName, "PlotDescription", QString( "Total Allocation" ), "Name", "", "", "" );
    m_userName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTotalWellAllocationPlot::~RimTotalWellAllocationPlot()
{
    removeMdiWindowFromMdiArea();

    if ( m_wellTotalAllocationPlotWidget )
    {
        m_wellTotalAllocationPlotWidget->hide();
        m_wellTotalAllocationPlotWidget->setParent( nullptr );
        delete m_wellTotalAllocationPlotWidget;
        m_wellTotalAllocationPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: implement properly
//--------------------------------------------------------------------------------------------------
int RimTotalWellAllocationPlot::id() const
{
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::deleteViewWidget()
{
    if ( m_wellTotalAllocationPlotWidget )
    {
        m_wellTotalAllocationPlotWidget->hide();
        m_wellTotalAllocationPlotWidget->setParent( nullptr );
        delete m_wellTotalAllocationPlotWidget;
        m_wellTotalAllocationPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimTotalWellAllocationPlot::fontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::updateFonts()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimTotalWellAllocationPlot::viewWidget()
{
    return m_wellTotalAllocationPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_userName || changedField == &m_showPlotTitle )
    {
        updateMdiWindowTitle();
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: Implement properly
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::assignIdIfNecessary()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimTotalWellAllocationPlot::snapshotWindowContent()
{
    QImage image;

    // TODO

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::setDescription( const QString& description )
{
    m_userName = description;
    this->updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimTotalWellAllocationPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimTotalWellAllocationPlot::totalAllocationAsText() const
{
    QString txt;

    for ( auto a : m_sliceInfo )
    {
        txt += a.first;
        txt += "\t";
        txt += QString::number( a.second );
        txt += "\n";
    }

    return txt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::addSlice( const QString& name, const cvf::Color3f& color, float value )
{
    if ( m_wellTotalAllocationPlotWidget )
    {
        QColor sliceColor( color.rByte(), color.gByte(), color.bByte() );

        m_wellTotalAllocationPlotWidget->addItem( name, sliceColor, value );
        m_wellTotalAllocationPlotWidget->update();
    }

    m_sliceInfo.push_back( std::make_pair( name, value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::clearSlices()
{
    if ( m_wellTotalAllocationPlotWidget )
    {
        m_wellTotalAllocationPlotWidget->clear();
        m_wellTotalAllocationPlotWidget->update();
    }

    m_sliceInfo.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTotalWellAllocationPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimTotalWellAllocationPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_wellTotalAllocationPlotWidget = new RiuNightchartsWidget( mainWindowParent );
    m_wellTotalAllocationPlotWidget->showLegend( false );
    return m_wellTotalAllocationPlotWidget;
}
