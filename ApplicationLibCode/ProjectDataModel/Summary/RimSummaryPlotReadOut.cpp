/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimSummaryPlotReadOut.h"

#include "RiaPreferencesSummary.h"

#include "RimAnnotationLineAppearance.h"

#include "cafPdmUiCheckBoxEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryPlotReadOut, "RimSummaryPlotReadOut" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotReadOut::RimSummaryPlotReadOut()
{
    CAF_PDM_InitObject( "Summary Plot Read Out", "" );

    CAF_PDM_InitFieldNoDefault( &m_readOutType, "ReadOutType", "Readout Mode" );
    m_readOutType = RiaPreferencesSummary::current()->defaultSummaryReadoutMode();

    CAF_PDM_InitFieldNoDefault( &m_lineAppearance, "LineAppearance", "" );
    m_lineAppearance = new RimAnnotationLineAppearance;
    m_lineAppearance->configureForSummaryAnnotations();
    m_lineAppearance.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_verticalLineLabelAlignment, "VerticalLineLabelAlignment", "Vertical Label Alignment" );
    m_verticalLineLabelAlignment = RiaDefines::TextAlignment::LEFT;

    CAF_PDM_InitFieldNoDefault( &m_horizontalLineLabelAlignment, "HorizontalLineLabelAlignment", "Horizontal Label Alignment" );
    m_horizontalLineLabelAlignment = RiaDefines::TextAlignment::LEFT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotReadOut::enableCurvePointTracking() const
{
    return m_readOutType() == RiaDefines::ReadOutType::SNAP_TO_POINT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotReadOut::enableHorizontalLine() const
{
    return m_readOutType() == RiaDefines::ReadOutType::TIME_VALUE_TRACKING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotReadOut::enableVerticalLine() const
{
    return ( ( m_readOutType() == RiaDefines::ReadOutType::TIME_TRACKING ) ||
             ( m_readOutType() == RiaDefines::ReadOutType::TIME_VALUE_TRACKING ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance* RimSummaryPlotReadOut::lineAppearance() const
{
    return m_lineAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::TextAlignment RimSummaryPlotReadOut::verticalLineLabelAlignment() const
{
    return m_verticalLineLabelAlignment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::TextAlignment RimSummaryPlotReadOut::horizontalLineLabelAlignment() const
{
    return m_horizontalLineLabelAlignment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotReadOut::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_readOutType );

    if ( enableVerticalLine() ) uiOrdering.add( &m_verticalLineLabelAlignment );
    if ( enableHorizontalLine() ) uiOrdering.add( &m_horizontalLineLabelAlignment );

    if ( enableVerticalLine() || enableHorizontalLine() )
    {
        m_lineAppearance->uiOrdering( uiConfigName, uiOrdering );
    }

    uiOrdering.skipRemainingFields();
}
