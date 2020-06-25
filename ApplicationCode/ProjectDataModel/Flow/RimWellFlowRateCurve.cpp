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

#include "RimWellFlowRateCurve.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RigWellLogCurveData.h"

#include "RimWellAllocationPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuQwtPlotCurve.h"

#include "qwt_plot.h"

#include "cvfMath.h"

#include <cmath>

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimWellFlowRateCurve, "WellFlowRateCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellFlowRateCurve::RimWellFlowRateCurve()
{
    CAF_PDM_InitObject( "Flow Rate Curve", "", "", "" );
    m_groupId     = 0;
    m_doFillCurve = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellFlowRateCurve::~RimWellFlowRateCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimWellFlowRateCurve::rimCase()
{
    RimWellAllocationPlot* wap = wellAllocationPlot();
    if ( wap )
    {
        return wap->rimCase();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellFlowRateCurve::timeStep()
{
    RimWellAllocationPlot* wap = wellAllocationPlot();

    if ( wap )
    {
        return wap->timeStep();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::wellName() const
{
    QString name;

    RimWellAllocationPlot* wap = wellAllocationPlot();

    if ( wap )
    {
        name = wap->wellName();
    }
    else
    {
        name = "Undefined";
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::wellLogChannelUiName() const
{
    return "AccumulatedFlowRate";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::wellLogChannelUnits() const
{
    return RiaWellLogUnitTools<double>::noUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::setGroupId( int groupId )
{
    m_groupId = groupId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellFlowRateCurve::groupId() const
{
    return m_groupId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::setDoFillCurve( bool doFill )
{
    m_doFillCurve = doFill;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::createCurveAutoName()
{
    return m_curveAutoName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    this->RimPlotCurve::updateCurvePresentation( updateParentPlot );

    m_qwtPlotCurve->setTitle( createCurveAutoName() );

    updateStackedPlotData();

    updateZoomInParentPlot();

    if ( m_parentQwtPlot ) m_parentQwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::updateCurveAppearance()
{
    RimWellLogCurve::updateCurveAppearance();

    bool isLastCurveInGroup = false;
    {
        RimWellLogTrack* wellLogTrack;
        firstAncestorOrThisOfTypeAsserted( wellLogTrack );
        std::map<int, std::vector<RimWellFlowRateCurve*>> stackedCurveGroups = wellLogTrack->visibleStackedCurves();
        const std::vector<RimWellFlowRateCurve*>&         curveGroup         = stackedCurveGroups[this->m_groupId];

        if ( !curveGroup.empty() )
        {
            isLastCurveInGroup = ( curveGroup.back() == this );
        }
    }

    if ( isUsingConnectionNumberDepthType() )
    {
        m_qwtPlotCurve->setStyle( QwtPlotCurve::Steps );
    }

    if ( m_doFillCurve || isLastCurveInGroup ) // Fill the last curve in group with a transparent color to "tie" the
                                               // group together
    {
        QColor curveQColor =
            QColor( m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte() );
        QColor fillColor = curveQColor;
        QColor lineColor = curveQColor.darker();

        if ( !m_doFillCurve && isLastCurveInGroup )
        {
            fillColor = QColor( 24, 16, 10, 50 );
            lineColor = curveQColor;
        }

        QLinearGradient gradient;
        gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
        gradient.setColorAt( 0, fillColor.darker( 110 ) );
        gradient.setColorAt( 0.15, fillColor );
        gradient.setColorAt( 0.25, fillColor );
        gradient.setColorAt( 0.4, fillColor.darker( 110 ) );
        gradient.setColorAt( 0.6, fillColor );
        gradient.setColorAt( 0.8, fillColor.darker( 110 ) );
        gradient.setColorAt( 1, fillColor );
        m_qwtPlotCurve->setBrush( gradient );

        QPen curvePen = m_qwtPlotCurve->pen();
        curvePen.setColor( lineColor );
        m_qwtPlotCurve->setPen( curvePen );
        m_qwtPlotCurve->setOrientation( Qt::Horizontal );
        m_qwtPlotCurve->setBaseline( 0.0 );
        m_qwtPlotCurve->setCurveAttribute( QwtPlotCurve::Inverted, true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_curveName );
    m_curveName.uiCapability()->setUiReadOnly( true );
    uiOrdering.add( &m_curveColor );
    m_curveColor.uiCapability()->setUiReadOnly( true );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
    if ( changedField == &m_showCurve )
    {
        loadDataAndUpdate( true );
    }

    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::updateStackedPlotData()
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfTypeAsserted( wellLogPlot );

    RimWellLogTrack* wellLogTrack;
    firstAncestorOrThisOfTypeAsserted( wellLogTrack );

    RimWellLogPlot::DepthTypeEnum depthType   = wellLogPlot->depthType();
    RiaDefines::DepthUnitType     displayUnit = wellLogPlot->depthUnit();
    if ( depthType == RiaDefines::CONNECTION_NUMBER )
    {
        displayUnit = RiaDefines::UNIT_NONE;
    }

    std::vector<double>                    depthValues;
    std::vector<double>                    stackedValues;
    std::vector<std::pair<size_t, size_t>> polyLineStartStopIndices;

    // Z-position of curve, to draw them in correct order
    double zPos = -10000.0 + 100.0 * static_cast<double>( groupId() );

    // Starting way behind the grid (z == 0) at -10000 giving room for 100 groups with 100 curves each before getting
    // above the grid
    {
        std::map<int, std::vector<RimWellFlowRateCurve*>> stackedCurveGroups = wellLogTrack->visibleStackedCurves();

        std::vector<RimWellFlowRateCurve*> stackedCurves;

        if ( stackedCurveGroups.count( groupId() ) > 0 )
        {
            stackedCurves = stackedCurveGroups[groupId()];
        }

        std::vector<double> allDepthValues = curveData()->depths( depthType );
        if ( allDepthValues.empty() ) return;

        std::vector<double> allStackedValues( allDepthValues.size() );

        for ( RimWellFlowRateCurve* stCurve : stackedCurves )
        {
            std::vector<double> allValues = stCurve->curveData()->xValues();

            for ( size_t i = 0; i < allValues.size(); ++i )
            {
                if ( allValues[i] != HUGE_VAL )
                {
                    allStackedValues[i] += allValues[i];
                }
            }

            if ( stCurve == this ) break;
            zPos -= 1.0;
        }

        RigWellLogCurveData tempCurveData;
        tempCurveData.setValuesAndDepths( allStackedValues, allDepthValues, depthType, 0.0, displayUnit, false );

        depthValues              = tempCurveData.depthPlotValues( depthType, displayUnit );
        stackedValues            = tempCurveData.xPlotValues();
        polyLineStartStopIndices = tempCurveData.polylineStartStopIndices();
    }

    auto minmax_it = std::minmax_element( stackedValues.begin(), stackedValues.end() );

    this->setOverrideCurveDataXRange( *( minmax_it.first ), *( minmax_it.second ) );
    m_qwtPlotCurve->setSamples( stackedValues.data(), depthValues.data(), static_cast<int>( depthValues.size() ) );
    m_qwtPlotCurve->setLineSegmentStartStopIndices( polyLineStartStopIndices );

    m_qwtPlotCurve->setZ( zPos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellFlowRateCurve::isUsingConnectionNumberDepthType() const
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType( wellLogPlot );
    if ( wellLogPlot && wellLogPlot->depthType() == RiaDefines::CONNECTION_NUMBER )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot* RimWellFlowRateCurve::wellAllocationPlot() const
{
    RimWellAllocationPlot* wap = nullptr;
    this->firstAncestorOrThisOfType( wap );

    return wap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::setFlowValuesPrDepthValue( const QString&             curveName,
                                                      RiaDefines::DepthTypeEnum  depthType,
                                                      const std::vector<double>& depthValues,
                                                      const std::vector<double>& flowRates )
{
    this->setValuesAndDepths( flowRates, depthValues, depthType, 0.0, RiaDefines::UNIT_NONE, false );

    m_curveAutoName = curveName;
}
