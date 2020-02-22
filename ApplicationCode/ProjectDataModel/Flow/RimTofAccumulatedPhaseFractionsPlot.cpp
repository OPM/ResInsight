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

#include "RimTofAccumulatedPhaseFractionsPlot.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimWellAllocationPlot.h"

#include "RigSimWellData.h"
#include "RigTofAccumulatedPhaseFractionsCalculator.h"

#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindow.h"
#include "RiuTofAccumulatedPhaseFractionsPlot.h"
#include "RiuWellAllocationPlot.h"

#include "cvfColor3.h"

CAF_PDM_SOURCE_INIT( RimTofAccumulatedPhaseFractionsPlot, "TofAccumulatedPhaseFractionsPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTofAccumulatedPhaseFractionsPlot::RimTofAccumulatedPhaseFractionsPlot()
{
    CAF_PDM_InitObject( "Cumulative Saturation by Time of Flight", ":/TOFAccSatPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_userName, "PlotDescription", QString( "Cumulative Saturation by Time of Flight" ), "Name", "", "", "" );
    m_userName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "" );
    m_showPlotTitle.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_maxTof, "MaxTof", 50, "Max Time of Flight [year]", "", "", "" );
    m_showWindow = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTofAccumulatedPhaseFractionsPlot::~RimTofAccumulatedPhaseFractionsPlot()
{
    removeMdiWindowFromMdiArea();

    if ( m_tofAccumulatedPhaseFractionsPlotWidget )
    {
        m_tofAccumulatedPhaseFractionsPlotWidget->hide();
        m_tofAccumulatedPhaseFractionsPlotWidget->setParent( nullptr );
        delete m_tofAccumulatedPhaseFractionsPlotWidget;
        m_tofAccumulatedPhaseFractionsPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: implement properly
//--------------------------------------------------------------------------------------------------
int RimTofAccumulatedPhaseFractionsPlot::id() const
{
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::deleteViewWidget()
{
    if ( m_tofAccumulatedPhaseFractionsPlotWidget )
    {
        m_tofAccumulatedPhaseFractionsPlotWidget->hide();
        m_tofAccumulatedPhaseFractionsPlotWidget->setParent( nullptr );
        delete m_tofAccumulatedPhaseFractionsPlotWidget;
        m_tofAccumulatedPhaseFractionsPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::reloadFromWell()
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimTofAccumulatedPhaseFractionsPlot::resultCase()
{
    RimWellAllocationPlot* allocationPlot;
    firstAncestorOrThisOfTypeAsserted( allocationPlot );

    return allocationPlot->rimCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimTofAccumulatedPhaseFractionsPlot::tracerName()
{
    RimWellAllocationPlot* allocationPlot;
    firstAncestorOrThisOfTypeAsserted( allocationPlot );

    return allocationPlot->wellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimTofAccumulatedPhaseFractionsPlot::timeStep()
{
    RimWellAllocationPlot* allocationPlot;
    firstAncestorOrThisOfTypeAsserted( allocationPlot );

    return static_cast<size_t>( allocationPlot->timeStep() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimTofAccumulatedPhaseFractionsPlot::viewWidget()
{
    return m_tofAccumulatedPhaseFractionsPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                            const QVariant&            oldValue,
                                                            const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_userName || changedField == &m_showPlotTitle )
    {
        updateMdiWindowTitle();
    }
    else if ( changedField == &m_maxTof )
    {
        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimTofAccumulatedPhaseFractionsPlot::snapshotWindowContent()
{
    QImage image;

    // TODO

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::setDescription( const QString& description )
{
    m_userName = description;
    this->updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimTofAccumulatedPhaseFractionsPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// TODO: Implement properly
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::assignIdIfNecessary()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTofAccumulatedPhaseFractionsPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_tofAccumulatedPhaseFractionsPlotWidget && m_showWindow() )
    {
        RigTofAccumulatedPhaseFractionsCalculator calc( resultCase(), tracerName(), timeStep() );

        const std::vector<double>& xValues   = calc.sortedUniqueTOFValues();
        const std::vector<double>& watValues = calc.accumulatedPhaseFractionsSwat();
        const std::vector<double>& oilValues = calc.accumulatedPhaseFractionsSoil();
        const std::vector<double>& gasValues = calc.accumulatedPhaseFractionsSgas();

        m_tofAccumulatedPhaseFractionsPlotWidget->setSamples( xValues, watValues, oilValues, gasValues, m_maxTof() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimTofAccumulatedPhaseFractionsPlot::createViewWidget( QWidget* mainWindowParent )
{
    if ( !m_tofAccumulatedPhaseFractionsPlotWidget )
    {
        m_tofAccumulatedPhaseFractionsPlotWidget = new RiuTofAccumulatedPhaseFractionsPlot( this, mainWindowParent );
    }
    return m_tofAccumulatedPhaseFractionsPlotWidget;
}
