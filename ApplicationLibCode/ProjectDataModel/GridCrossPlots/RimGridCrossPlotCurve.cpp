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

#include "RimGridCrossPlotCurve.h"

#include "RimGridCrossPlot.h"

#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"

CAF_PDM_SOURCE_INIT( RimGridCrossPlotCurve, "GridCrossPlotCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCurve::RimGridCrossPlotCurve()
    : m_dataSetIndex( 0 )
    , m_groupIndex( 0 )
{
    CAF_PDM_InitObject( "Cross Plot Points", ":/WellLogCurve16x16.png" );

    setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    setSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );
    setSymbolSize( 4 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::setGroupingInformation( int dataSetIndex, int groupIndex )
{
    m_dataSetIndex = dataSetIndex;
    m_groupIndex   = groupIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::setSamples( const std::vector<double>& xValues, const std::vector<double>& yValues )
{
    CVF_ASSERT( xValues.size() == yValues.size() );

    if ( xValues.empty() || yValues.empty() || !m_plotCurve ) return;

    m_plotCurve->setSamplesFromXValuesAndYValues( xValues, yValues, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::setCurveAutoAppearance()
{
    determineSymbol();
    updateCurveAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlotCurve::groupIndex() const
{
    return m_groupIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGridCrossPlotCurve::sampleCount() const
{
    return m_plotCurve ? m_plotCurve->numSamples() : 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::determineLegendIcon()
{
    if ( !m_plotCurve ) return;

    RimGridCrossPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );
    int fontSize = plot->legendFontSize();
    m_plotCurve->setLegendIconSize( QSize( fontSize, fontSize ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::setBlackAndWhiteLegendIcons( bool blackAndWhite )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setBlackAndWhiteLegendIcon( blackAndWhite );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::determineSymbol()
{
    RiuPlotCurveSymbol::PointSymbolEnum symbol = RiuPlotCurveSymbol::cycledSymbolStyle( m_dataSetIndex );
    setSymbol( symbol );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::updateZoomInParentPlot()
{
    RimGridCrossPlot* plot;
    this->firstAncestorOrThisOfTypeAsserted( plot );
    plot->calculateZoomRangeAndUpdateQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurve::createCurveAutoName()
{
    return m_customCurveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( updateParentPlot )
    {
        m_parentPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );
    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->add( &m_curveName );
    nameGroup->add( &m_showLegend );
    uiOrdering.skipRemainingFields( true );
}
