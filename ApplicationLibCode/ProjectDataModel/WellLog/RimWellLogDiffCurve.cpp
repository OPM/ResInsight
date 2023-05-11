/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimWellLogDiffCurve.h"

#include "RiaDefines.h"
#include "RiaLogging.h"

#include "RigWellLogCurveData.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPathCollection.h"

#include "RiuPlotCurve.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellLogDiffCurve, "WellLogDiffCurve" );

#pragma optimize( "", off )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogDiffCurve::RimWellLogDiffCurve()
{
    CAF_PDM_InitObject( "Well Log Diff Curve", ":/WellLogCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogCurveA, "WellLogCurveA", "Well Log Curve A" );
    CAF_PDM_InitFieldNoDefault( &m_wellLogCurveB, "WellLogCurveB", "Well Log Curve B" );

    setNamingMethod( RiaDefines::ObjectNamingMethod::AUTO );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogDiffCurve::~RimWellLogDiffCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::setWellLogCurves( RimWellLogCurve* wellLogCurveA, RimWellLogCurve* wellLogCurveB )
{
    if ( wellLogCurveA ) m_wellLogCurveA = wellLogCurveA;
    if ( wellLogCurveB ) m_wellLogCurveB = wellLogCurveB;

    if ( m_namingMethod() == RiaDefines::ObjectNamingMethod::AUTO )
    {
        setAutomaticName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::createCurveAutoName()
{
    if ( !m_wellLogCurveA() || !m_wellLogCurveB() ) return QString( "Invalid well log curves" );
    return QString( "Diff (%1 - %2)" ).arg( m_wellLogCurveA->curveName() ).arg( m_wellLogCurveB->curveName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( !m_wellLogCurveA() || !m_wellLogCurveB() ) return;

    if ( m_namingMethod() == RiaDefines::ObjectNamingMethod::AUTO )
    {
        setAutomaticName();
    }

    // Use well A as reference for resampled curve data
    auto* curveDataA = m_wellLogCurveA()->curveData();
    auto* curveDataB = m_wellLogCurveB()->curveData();

    if ( !curveDataA || !curveDataB ) return;
    if ( curveDataA->depthUnit() != curveDataB->depthUnit() )
    {
        RiaLogging::warning( "Well log curve depth units are not the same" );
    }
    if ( curveDataA->propertyValueUnit() != curveDataB->propertyValueUnit() )
    {
        RiaLogging::warning( "Well log curve property value units are not the same" );
    }

    const auto depthUnit    = curveDataA->depthUnit();
    const auto depthType    = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
    const auto propertyUnit = curveDataA->propertyValueUnit();

    // Get curve A depths and property values
    const auto curveADepthValues        = curveDataA->depths( depthType );
    const auto curveDataAPropertyValues = curveDataA->propertyValues();

    // Resample curve B to curve A
    cvf::ref<RigWellLogCurveData> curveDataBResampled           = curveDataB->calculateResampledCurveData( depthType, curveADepthValues );
    auto                          curveBDepthValuesResampled    = curveDataBResampled->depths( depthType );
    auto                          curveBPropertyValuesResampled = curveDataBResampled->propertyValues();

    // Verify equal sizes
    if ( curveADepthValues.size() != curveDataAPropertyValues.size() ) return;
    if ( curveADepthValues.size() != curveBDepthValuesResampled.size() ) return;
    if ( curveADepthValues.size() != curveBPropertyValuesResampled.size() ) return;

    // Calculate diff curve
    std::vector<double> curveDiffDepthValues( curveADepthValues.size() );
    std::vector<double> curveDiffPropertyValues( curveDataAPropertyValues.size() );
    for ( size_t i = 0; i < curveDataAPropertyValues.size(); ++i )
    {
        curveDiffPropertyValues[i] = curveDataAPropertyValues[i] - curveBPropertyValuesResampled[i];
        curveDiffDepthValues[i]    = curveADepthValues[i];
    }

    const bool useLogarithmicScale = false;
    const bool isExtractionCurve   = false;

    // Set curve data
    auto depthsMap       = std::map<RiaDefines::DepthTypeEnum, std::vector<double>>();
    depthsMap[depthType] = curveDiffDepthValues;
    setPropertyValuesAndDepths( curveDiffPropertyValues, depthsMap, 0.0, depthUnit, isExtractionCurve, useLogarithmicScale, propertyUnit );

    // Set curve data to plot
    std::vector<double> xPlotValues = curveData()->propertyValuesByIntervals();
    std::vector<double> yPlotValues = curveData()->depthValuesByIntervals( depthType, depthUnit );
    m_plotCurve->setSamplesFromXValuesAndYValues( xPlotValues, yPlotValues, useLogarithmicScale );
    updateCurvePresentation( updateParentPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::setAutomaticName()
{
    m_curveName = createCurveAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::wellName() const
{
    CAF_ASSERT( "TO BE IMPLEMETNED!" );
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::wellLogChannelUiName() const
{
    return ( QString( "Diff (%1 - %2)" ).arg( m_wellLogCurveA->wellLogChannelUiName() ).arg( m_wellLogCurveB->wellLogChannelUiName() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::wellLogChannelUnits() const
{
    CAF_ASSERT( "TO BE IMPLEMETNED!" );
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Data Source" );
    group->add( &m_wellLogCurveA );
    group->add( &m_wellLogCurveB );

    RimStackablePlotCurve::defaultUiOrdering( uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellLogCurveA || changedField == &m_wellLogCurveB )
    {
        if ( !m_wellLogCurveA() || !m_wellLogCurveB() ) return;

        onLoadDataAndUpdate( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogDiffCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions );
    if ( options.size() > 0 ) return options;

    if ( fieldNeedingOptions == &m_wellLogCurveA || fieldNeedingOptions == &m_wellLogCurveB )
    {
        RimWellPathCollection*    wellPathCollection    = RimProject::current()->activeOilField()->wellPathCollection();
        RimWellLogPlotCollection* wellLogPlotCollection = RimMainPlotCollection::current()->wellLogPlotCollection();

        if ( !wellLogPlotCollection ) return {};

        // Find each well log plot in collection
        std::vector<RimWellLogCurve*> wellLogCurves;
        wellLogPlotCollection->descendantsOfType( wellLogCurves );
        for ( RimWellLogCurve* curve : wellLogCurves )
        {
            if ( !curve || curve == this ) continue;
            options.push_back( caf::PdmOptionItemInfo( curve->curveName(), curve ) );
        }
    }
    return options;
}
