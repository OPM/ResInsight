/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimMultiSummaryPlot.h"
#include "RimMultiPlot.h"

CAF_PDM_SOURCE_INIT( RimMultiSummaryPlot, "MultiSummaryPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiSummaryPlot::RimMultiSummaryPlot()
{
    CAF_PDM_InitObject( "Multi Summary Plot Plot", "", "", "" );
    this->setDeletable( true );

    m_multiPlot = new RimMultiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimMultiSummaryPlot::viewWidget()
{
    return m_multiPlot->viewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimMultiSummaryPlot::snapshotWindowContent()
{
    return m_multiPlot->snapshotWindowContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::zoomAll()
{
    m_multiPlot->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiSummaryPlot::description() const
{
    return "RimMultiSummaryPlot Placeholder Text";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimMultiSummaryPlot::createViewWidget( QWidget* mainWindowParent /*= nullptr*/ )
{
    return m_multiPlot->createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::deleteViewWidget()
{
    m_multiPlot->deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::onLoadDataAndUpdate()
{
    m_multiPlot->onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    m_multiPlot->doRenderWindowContent( paintDevice );
}
