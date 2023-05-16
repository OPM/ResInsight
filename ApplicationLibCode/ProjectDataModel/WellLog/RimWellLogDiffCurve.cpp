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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogDiffCurve::RimWellLogDiffCurve()
{
    CAF_PDM_InitObject( "Well Log Diff Curve", ":/WellLogCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_firstWellLogCurve, "FirstWellLogCurve", "First Well Log Curve" );
    CAF_PDM_InitFieldNoDefault( &m_secondWellLogCurve, "SecondWellLogCurve", "Second Well Log Curve" );

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
void RimWellLogDiffCurve::setWellLogCurves( RimWellLogCurve* firstWellLogCurve, RimWellLogCurve* secondWellLogCurve )
{
    if ( firstWellLogCurve )
    {
        disconnectWellLogCurveChangedFromSlots( m_firstWellLogCurve );
        m_firstWellLogCurve = firstWellLogCurve;
        connectWellLogCurveChangedToSlots( m_firstWellLogCurve );
    }
    if ( secondWellLogCurve )
    {
        disconnectWellLogCurveChangedFromSlots( m_secondWellLogCurve );
        m_secondWellLogCurve = secondWellLogCurve;
        connectWellLogCurveChangedToSlots( m_secondWellLogCurve );
    }

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
    if ( !m_firstWellLogCurve() || !m_secondWellLogCurve() ) return QString( "Invalid well log curves" );
    return QString( "Diff (%1 - %2)" ).arg( m_firstWellLogCurve->curveName() ).arg( m_secondWellLogCurve->curveName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( !m_firstWellLogCurve() || !m_secondWellLogCurve() ) return;

    if ( m_namingMethod() == RiaDefines::ObjectNamingMethod::AUTO )
    {
        setAutomaticName();
    }

    // Use well A as reference for resampled curve data
    auto* firstCurveData  = m_firstWellLogCurve()->curveData();
    auto* secondCurveData = m_secondWellLogCurve()->curveData();

    if ( !firstCurveData || !secondCurveData ) return;
    if ( firstCurveData->depthUnit() != secondCurveData->depthUnit() )
    {
        RiaLogging::warning( "Well log curve depth units are not the same" );
    }
    if ( firstCurveData->propertyValueUnit() != secondCurveData->propertyValueUnit() )
    {
        RiaLogging::warning( "Well log curve property value units are not the same" );
    }

    const auto depthUnit    = firstCurveData->depthUnit();
    const auto depthType    = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
    const auto propertyUnit = firstCurveData->propertyValueUnit();

    // Get curve A depths and property values
    const auto firstCurveDepthValues    = firstCurveData->depths( depthType );
    const auto firstCurvePropertyValues = firstCurveData->propertyValues();

    // Resample curve B to curve A
    cvf::ref<RigWellLogCurveData> secondCurveDataResampled = secondCurveData->calculateResampledCurveData( depthType, firstCurveDepthValues );
    auto secondCurveDepthValuesResampled    = secondCurveDataResampled->depths( depthType );
    auto secondCurvePropertyValuesResampled = secondCurveDataResampled->propertyValues();

    // Verify equal sizes
    if ( firstCurveDepthValues.size() != firstCurvePropertyValues.size() ) return;
    if ( firstCurveDepthValues.size() != secondCurveDepthValuesResampled.size() ) return;
    if ( firstCurveDepthValues.size() != secondCurvePropertyValuesResampled.size() ) return;

    // Calculate diff curve
    std::vector<double> curveDiffDepthValues( firstCurveDepthValues.size() );
    std::vector<double> curveDiffPropertyValues( firstCurvePropertyValues.size() );
    for ( size_t i = 0; i < firstCurvePropertyValues.size(); ++i )
    {
        curveDiffPropertyValues[i] = firstCurvePropertyValues[i] - secondCurvePropertyValuesResampled[i];
        curveDiffDepthValues[i]    = firstCurveDepthValues[i];
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
void RimWellLogDiffCurve::onWellLogCurveChanged( const SignalEmitter* emitter )
{
    onLoadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::connectWellLogCurveChangedToSlots( RimWellLogCurve* wellLogCurve )
{
    if ( !wellLogCurve ) return;

    wellLogCurve->dataChanged.connect( this, &RimWellLogDiffCurve::onWellLogCurveChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::disconnectWellLogCurveChangedFromSlots( RimWellLogCurve* wellLogCurve )
{
    if ( !wellLogCurve ) return;

    wellLogCurve->dataChanged.disconnect( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::wellName() const
{
    return m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::wellLogChannelUiName() const
{
    return m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::wellLogChannelUnits() const
{
    CAF_ASSERT( "TO BE IMPLEMETNED!" );

    if ( m_firstWellLogCurve->wellLogChannelUnits() != m_secondWellLogCurve->wellLogChannelUnits() )
    {
        return QString( "%1 - %2" ).arg( m_firstWellLogCurve->wellLogChannelUnits() ).arg( m_secondWellLogCurve->wellLogChannelUnits() );
    }
    return m_firstWellLogCurve->wellLogChannelUnits();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Data Source" );
    group->add( &m_firstWellLogCurve );
    group->add( &m_secondWellLogCurve );

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

    if ( changedField == &m_firstWellLogCurve || changedField == &m_secondWellLogCurve )
    {
        if ( !m_firstWellLogCurve() || !m_secondWellLogCurve() ) return;

        PdmObjectHandle* prevValue        = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
        auto*            prevWellLogCurve = dynamic_cast<RimWellLogCurve*>( prevValue );
        disconnectWellLogCurveChangedFromSlots( prevWellLogCurve );

        if ( changedField == &m_firstWellLogCurve ) connectWellLogCurveChangedToSlots( m_firstWellLogCurve );
        if ( changedField == &m_secondWellLogCurve ) connectWellLogCurveChangedToSlots( m_firstWellLogCurve );

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
    if ( !options.empty() ) return options;

    if ( fieldNeedingOptions == &m_firstWellLogCurve || fieldNeedingOptions == &m_secondWellLogCurve )
    {
        RimWellPathCollection*    wellPathCollection    = RimProject::current()->activeOilField()->wellPathCollection();
        RimWellLogPlotCollection* wellLogPlotCollection = RimMainPlotCollection::current()->wellLogPlotCollection();

        if ( !wellLogPlotCollection ) return {};

        // Find each well log plot in collection
        std::vector<RimWellLogCurve*> wellLogCurves = wellLogPlotCollection->descendantsOfType<RimWellLogCurve>();
        for ( RimWellLogCurve* curve : wellLogCurves )
        {
            if ( !curve || curve == this ) continue;
            options.push_back( caf::PdmOptionItemInfo( curve->curveName(), curve ) );
        }
    }
    return options;
}
