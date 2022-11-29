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

#include "RiaApplication.h"
#include "RiaCurveDataTools.h"
#include "RiaPlotDefines.h"

#include "RigWellLogCurveData.h"

#include "RimDepthTrackPlot.h"
#include "RimMainPlotCollection.h"
#include "RimTools.h"
#include "RimWellLogTrack.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

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
    CAF_PDM_InitObject( "WellLogCurve", ":/WellLogCurve16x16.png" );

    m_curveData = new RigWellLogCurveData;

    m_curveDataPropertyValueRange =
        std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );

    // Ref well path as Ui element for debug purpose. If not needed: Remove use of caf::PdmPtrField,
    // and replace with regular non-ui ptr. The remove related code in calculateValueOptions() and
    // defineUiOrdering().
    CAF_PDM_InitFieldNoDefault( &m_refWellPath, "ReferenceWellPath", "Reference Well Path" );
    m_refWellPath.uiCapability()->setUiHidden( !RiaApplication::enableDevelopmentFeatures() );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::~RimWellLogCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setDepthUnit( RiaDefines::DepthUnitType depthUnit )
{
    m_curveData->setDepthUnit( depthUnit );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::propertyValueRangeInData( double* minimumValue, double* maximumValue ) const
{
    CAF_ASSERT( minimumValue && maximumValue );

    if ( !( minimumValue && maximumValue ) )
    {
        return false;
    }

    if ( m_curveDataPropertyValueRange.first == -std::numeric_limits<double>::infinity() ||
         m_curveDataPropertyValueRange.second == std::numeric_limits<double>::infinity() )
    {
        return false;
    }

    *minimumValue = m_curveDataPropertyValueRange.first;
    *maximumValue = m_curveDataPropertyValueRange.second;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::depthValueRangeInData( double* minimumValue, double* maximumValue ) const
{
    CAF_ASSERT( minimumValue && maximumValue );

    if ( !( minimumValue && maximumValue ) )
    {
        return false;
    }

    RimDepthTrackPlot* wellLogPlot = nullptr;
    firstAncestorOrThisOfTypeAsserted( wellLogPlot );
    auto depthType   = wellLogPlot->depthType();
    auto displayUnit = wellLogPlot->depthUnit();

    return m_curveData->calculateDepthRange( depthType, displayUnit, minimumValue, maximumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setPropertyValuesAndDepths( const std::vector<double>& propertyValues,
                                                  const std::vector<double>& depths,
                                                  RiaDefines::DepthTypeEnum  depthType,
                                                  double                     rkbDiff,
                                                  RiaDefines::DepthUnitType  depthUnit,
                                                  bool                       isExtractionCurve,
                                                  bool                       useLogarithmicScale,
                                                  const QString&             propertyUnit )
{
    m_curveData->setValuesAndDepths( propertyValues, depths, depthType, rkbDiff, depthUnit, isExtractionCurve, useLogarithmicScale );
    m_curveData->setPropertyValueUnit( propertyUnit );
    calculateCurveDataPropertyValueRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setPropertyValuesAndDepths( const std::vector<double>& propertyValues,
                                                  const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& depths,
                                                  double                    rkbDiff,
                                                  RiaDefines::DepthUnitType depthUnit,
                                                  bool                      isExtractionCurve,
                                                  bool                      useLogarithmicScale,

                                                  const QString& propertyUnit )
{
    m_curveData->setValuesAndDepths( propertyValues, depths, rkbDiff, depthUnit, isExtractionCurve, useLogarithmicScale );
    m_curveData->setPropertyValueUnit( propertyUnit );
    calculateCurveDataPropertyValueRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setPropertyAndDepthsAndErrors( const std::vector<double>& propertyValues,
                                                     const std::vector<double>& depthValues,
                                                     const std::vector<double>& errorValues )
{
    bool useLogarithmicScale = false;

    if ( isVerticalCurve() )
    {
        this->setSamplesFromXYErrorValues( propertyValues,
                                           depthValues,
                                           errorValues,
                                           useLogarithmicScale,
                                           RiaCurveDataTools::ErrorAxis::ERROR_ALONG_X_AXIS );
    }
    else
    {
        this->setSamplesFromXYErrorValues( depthValues,
                                           propertyValues,
                                           errorValues,
                                           useLogarithmicScale,
                                           RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setPropertyAndDepthValuesToPlotCurve( const std::vector<double>& propertyValues,
                                                            const std::vector<double>& depthValues )
{
    if ( !m_plotCurve ) return;

    if ( isVerticalCurve() )
    {
        m_plotCurve->setSamplesValues( propertyValues, depthValues );
    }
    else
    {
        m_plotCurve->setSamplesValues( depthValues, propertyValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setPropertyValuesWithMdAndTVD( const std::vector<double>& propertyValues,
                                                     const std::vector<double>& measuredDepths,
                                                     const std::vector<double>& tvdMSL,
                                                     double                     rkbDiff,
                                                     RiaDefines::DepthUnitType  depthUnit,
                                                     bool                       isExtractionCurve,
                                                     bool                       useLogarithmicScale,
                                                     const QString&             propertyUnit )
{
    std::map<RiaDefines::DepthTypeEnum, std::vector<double>> depths = { { RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                                                          measuredDepths },
                                                                        { RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH,
                                                                          tvdMSL } };
    setPropertyValuesAndDepths( propertyValues, depths, rkbDiff, depthUnit, isExtractionCurve, useLogarithmicScale, propertyUnit );
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
void RimWellLogCurve::setReferenceWellPath( RimWellPath* refWellPath )
{
    m_refWellPath = refWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateCurveAppearance()
{
    RimPlotCurve::updateCurveAppearance();

    auto orientation = RiaDefines::Orientation::VERTICAL;

    RimDepthTrackPlot* wellLogPlot = nullptr;
    firstAncestorOrThisOfType( wellLogPlot );
    if ( wellLogPlot )
    {
        orientation = wellLogPlot->depthOrientation();

        if ( m_plotCurve )
        {
            m_plotCurve->setXAxis( wellLogPlot->valueAxis() );
            m_plotCurve->setYAxis( wellLogPlot->depthAxis() );
        }
    }

    if ( fillStyle() != Qt::BrushStyle::NoBrush )
    {
        RiuQwtPlotCurve* qwtPlotCurve = dynamic_cast<RiuQwtPlotCurve*>( m_plotCurve );
        if ( qwtPlotCurve )
        {
            if ( orientation == RiaDefines::Orientation::VERTICAL )
            {
                qwtPlotCurve->setOrientation( Qt::Horizontal );
                qwtPlotCurve->setBaseline( 0.0 );
            }
            else
            {
                qwtPlotCurve->setOrientation( Qt::Vertical );
                qwtPlotCurve->setBaseline( 0.0 );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurve::wellLogChannelName() const
{
    return wellLogChannelUiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurve::wellLogCurveIconName()
{
    return ":/WellLogCurve16x16.png";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setOverrideCurveData( const std::vector<double>&               propertyValues,
                                            const std::vector<double>&               depthValues,
                                            const RiaCurveDataTools::CurveIntervals& curveIntervals )
{
    auto minmax_it = std::minmax_element( propertyValues.begin(), propertyValues.end() );
    this->setOverrideCurveDataPropertyValueRange( *( minmax_it.first ), *( minmax_it.second ) );

    if ( m_plotCurve )
    {
        setPropertyAndDepthValuesToPlotCurve( propertyValues, depthValues );

        m_plotCurve->setLineSegmentStartStopIndices( curveIntervals );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogCurve::closestYValueForX( double xValue ) const
{
    if ( m_curveData.isNull() ) return std::numeric_limits<double>::infinity();

    auto depths = m_curveData->depths( RiaDefines::DepthTypeEnum::MEASURED_DEPTH );
    auto values = m_curveData->propertyValues();

    if ( depths.empty() || values.empty() ) return std::numeric_limits<double>::infinity();

    auto it = std::upper_bound( depths.begin(), depths.end(), xValue );
    if ( it == depths.begin() ) return values.front();
    if ( it == depths.end() ) return values.back();

    auto index = std::distance( depths.begin(), it - 1 );

    double firstDistance  = std::abs( xValue - depths[index] );
    double secondDistance = std::abs( xValue - depths[index + 1] );

    if ( firstDistance < secondDistance ) return values[index];
    return values[index + 1];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateZoomInParentPlot()
{
    const double eps = 1.0e-8;

    RimWellLogTrack* wellLogTrack;
    firstAncestorOrThisOfType( wellLogTrack );

    if ( wellLogTrack )
    {
        wellLogTrack->setAutoScalePropertyValuesIfNecessary();

        RimDepthTrackPlot* wellLogPlot;
        wellLogTrack->firstAncestorOrThisOfType( wellLogPlot );

        if ( wellLogPlot )
        {
            double minPlotDepth, maxPlotDepth;
            wellLogPlot->availableDepthRange( &minPlotDepth, &maxPlotDepth );

            bool updateDepthZoom = false;
            if ( minPlotDepth == std::numeric_limits<double>::infinity() ||
                 maxPlotDepth == -std::numeric_limits<double>::infinity() )
            {
                updateDepthZoom = true;
            }
            else
            {
                double plotRange = std::abs( maxPlotDepth - minPlotDepth );
                double minCurveDepth, maxCurveDepth;
                m_curveData->calculateDepthRange( wellLogPlot->depthType(),
                                                  wellLogPlot->depthUnit(),
                                                  &minCurveDepth,
                                                  &maxCurveDepth );
                updateDepthZoom = minCurveDepth < minPlotDepth - eps * plotRange ||
                                  maxCurveDepth > maxPlotDepth + eps * plotRange;
            }
            if ( updateDepthZoom )
            {
                wellLogPlot->setAutoScaleDepthValuesEnabled( true );
            }
            wellLogPlot->updateZoom();
        }
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
        wellLogTrack->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setOverrideCurveDataPropertyValueRange( double minimumValue, double maximumValue )
{
    m_curveDataPropertyValueRange = std::make_pair( minimumValue, maximumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::calculateCurveDataPropertyValueRange()
{
    // Invalidate range first
    m_curveDataPropertyValueRange =
        std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );
    for ( double xValue : m_curveData->propertyValues() )
    {
        if ( RiaCurveDataTools::isValidValue( xValue, false ) )
        {
            m_curveDataPropertyValueRange.first  = std::min( m_curveDataPropertyValueRange.first, xValue );
            m_curveDataPropertyValueRange.second = std::max( m_curveDataPropertyValueRange.second, xValue );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    auto options = RimStackablePlotCurve::calculateValueOptions( fieldNeedingOptions );
    if ( fieldNeedingOptions == &m_refWellPath )
    {
        options.push_back( caf::PdmOptionItemInfo( QString( "None" ), nullptr ) );
        RimTools::wellPathOptionItems( &options );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimStackablePlotCurve::defineUiOrdering( uiConfigName, uiOrdering );

    auto group = uiOrdering.findGroup( "DataSource" );
    if ( group != nullptr )
    {
        group->add( &m_refWellPath );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    RimStackablePlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showCurve )
    {
        if ( m_isStacked() || m_showCurve() )
        {
            updateZoomInParentPlot();
        }
    }

    if ( changedField == &m_isStacked )
    {
        loadDataAndUpdate( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::isVerticalCurve() const
{
    auto orientation = RiaDefines::Orientation::VERTICAL;

    RimDepthTrackPlot* depthTrackPlot = nullptr;
    firstAncestorOrThisOfType( depthTrackPlot );
    if ( depthTrackPlot ) orientation = depthTrackPlot->depthOrientation();

    return orientation == RiaDefines::Orientation::VERTICAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimWellLogCurve::depthAxis() const
{
    RimDepthTrackPlot* depthTrackPlot;
    this->firstAncestorOrThisOfTypeAsserted( depthTrackPlot );

    return depthTrackPlot->depthAxis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimWellLogCurve::valueAxis() const
{
    RimDepthTrackPlot* depthTrackPlot;
    this->firstAncestorOrThisOfTypeAsserted( depthTrackPlot );

    return depthTrackPlot->valueAxis();
}
