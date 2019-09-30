/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogCurve.h"

#include "RiaCurveDataTools.h"
#include "RigWellLogCurveData.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuQwtPlotCurve.h"
#include "RiuWellLogTrack.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_symbol.h"

#include <algorithm>

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimWellLogCurve, "WellLogPlotCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::RimWellLogCurve()
{
    CAF_PDM_InitObject( "WellLogCurve", ":/WellLogCurve16x16.png", "", "" );

    m_qwtPlotCurve->setXAxis( QwtPlot::xTop );
    m_qwtPlotCurve->setErrorBarsXAxis( QwtPlot::xTop );
    m_qwtPlotCurve->setYAxis( QwtPlot::yLeft );

    m_curveData       = new RigWellLogCurveData;
    m_curveDataXRange = std::make_pair( std::numeric_limits<double>::infinity(),
                                        -std::numeric_limits<double>::infinity() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::~RimWellLogCurve() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::xValueRangeInData( double* minimumValue, double* maximumValue ) const
{
    CAF_ASSERT( minimumValue && maximumValue );

    if ( !( minimumValue && maximumValue ) )
    {
        return false;
    }

    if ( m_curveDataXRange.first == -std::numeric_limits<double>::infinity() ||
         m_curveDataXRange.second == std::numeric_limits<double>::infinity() )
    {
        return false;
    }

    *minimumValue = m_curveDataXRange.first;
    *maximumValue = m_curveDataXRange.second;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setValuesAndMD( const std::vector<double>& xValues,
                                      const std::vector<double>& measuredDepths,
                                      RiaDefines::DepthUnitType  depthUnit,
                                      bool                       isExtractionCurve )
{
    m_curveData->setValuesAndMD( xValues, measuredDepths, depthUnit, isExtractionCurve );
    calculateCurveDataXRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setValuesWithTVD( const std::vector<double>& xValues,
                                        const std::vector<double>& measuredDepths,
                                        const std::vector<double>& tvDepths,
                                        RiaDefines::DepthUnitType  depthUnit,
                                        bool                       isExtractionCurve )
{
    m_curveData->setValuesWithTVD( xValues, measuredDepths, tvDepths, depthUnit, isExtractionCurve );
    calculateCurveDataXRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellLogCurveData* RimWellLogCurve::curveData() const
{
    return m_curveData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateZoomInParentPlot()
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType( wellLogPlot );
    if ( wellLogPlot )
    {
        wellLogPlot->calculateAvailableDepthRange();
        wellLogPlot->updateDepthZoom();
    }

    RimWellLogTrack* plotTrack;
    firstAncestorOrThisOfType( plotTrack );
    if ( plotTrack )
    {
        plotTrack->calculateXZoomRangeAndUpdateQwt();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateLegendsInPlot()
{
    RimWellLogTrack* wellLogTrack;
    firstAncestorOrThisOfType( wellLogTrack );
    if ( wellLogTrack )
    {
        wellLogTrack->updateAllLegendItems();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setOverrideCurveDataXRange( double minimumValue, double maximumValue )
{
    m_curveDataXRange = std::make_pair( minimumValue, maximumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::calculateCurveDataXRange()
{
    // Invalidate range first
    m_curveDataXRange = std::make_pair( std::numeric_limits<double>::infinity(),
                                        -std::numeric_limits<double>::infinity() );

    for ( double xValue : m_curveData->xValues() )
    {
        if ( RiaCurveDataTools::isValidValue( xValue, false ) )
        {
            m_curveDataXRange.first  = std::min( m_curveDataXRange.first, xValue );
            m_curveDataXRange.second = std::max( m_curveDataXRange.second, xValue );
        }
    }
}
