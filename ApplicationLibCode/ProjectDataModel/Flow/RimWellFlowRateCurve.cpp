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

#include "RiaColorTools.h"

#include "qwt_plot.h"

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
    CAF_PDM_InitObject( "Flow Rate Curve" );
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

    m_plotCurve->setTitle( createCurveAutoName() );

    if ( updateParentPlot )
    {
        RimWellLogTrack* track = nullptr;
        this->firstAncestorOrThisOfTypeAsserted( track );
        track->updateStackedCurveData();

        updateZoomInParentPlot();
    }

    if ( hasParentPlot() ) m_parentPlot->replot();
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
        std::map<int, std::vector<RimWellLogCurve*>> stackedCurveGroups = wellLogTrack->visibleStackedCurves();
        const std::vector<RimWellLogCurve*>&         curveGroup         = stackedCurveGroups[this->m_groupId];

        if ( !curveGroup.empty() )
        {
            isLastCurveInGroup = ( curveGroup.back() == this );
        }
    }

    if ( isUsingConnectionNumberDepthType() )
    {
        // TODO: handle this
        // m_plotCurve->setStyle( QwtPlotCurve::Steps );
    }

    if ( m_doFillCurve || isLastCurveInGroup ) // Fill the last curve in group with a transparent color to "tie" the
                                               // group together
    {
        QColor curveQColor = RiaColorTools::toQColor( m_curveAppearance->color() );
        QColor fillColor   = curveQColor;
        QColor lineColor   = curveQColor.darker();

        if ( !m_doFillCurve && isLastCurveInGroup )
        {
            fillColor = QColor( 24, 16, 10, 50 );
            lineColor = curveQColor;
        }

        // TODO: how to handle this????
        // QLinearGradient gradient;
        // gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
        // gradient.setColorAt( 0, fillColor.darker( 110 ) );
        // gradient.setColorAt( 0.15, fillColor );
        // gradient.setColorAt( 0.25, fillColor );
        // gradient.setColorAt( 0.4, fillColor.darker( 110 ) );
        // gradient.setColorAt( 0.6, fillColor );
        // gradient.setColorAt( 0.8, fillColor.darker( 110 ) );
        // gradient.setColorAt( 1, fillColor );
        // m_plotCurve->setBrush( gradient );

        // QPen curvePen = m_plotCurve->pen();
        // curvePen.setColor( lineColor );
        // m_plotCurve->setPen( curvePen );
        // m_plotCurve->setOrientation( Qt::Horizontal );
        // m_plotCurve->setBaseline( 0.0 );
        // m_plotCurve->setCurveAttribute( QwtPlotCurve::Inverted, true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_curveName );
    m_curveName.uiCapability()->setUiReadOnly( true );

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
bool RimWellFlowRateCurve::isUsingConnectionNumberDepthType() const
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType( wellLogPlot );
    if ( wellLogPlot && wellLogPlot->depthType() == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER )
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
    this->setValuesAndDepths( flowRates, depthValues, depthType, 0.0, RiaDefines::DepthUnitType::UNIT_NONE, false );

    m_curveAutoName = curveName;
}
