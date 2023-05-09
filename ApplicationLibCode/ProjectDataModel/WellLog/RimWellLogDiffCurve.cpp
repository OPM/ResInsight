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

    m_curveName = QString( "Diff (%1 - %2)" ).arg( m_wellLogCurveA->curveName() ).arg( m_wellLogCurveB->curveName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogDiffCurve::createCurveAutoName()
{
    CAF_ASSERT( "TO BE IMPLEMETNED!" );
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogDiffCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( !m_wellLogCurveA() || !m_wellLogCurveB() ) return;

    // Use well A as reference for resampled curve data
    auto* curveDataA = m_wellLogCurveA()->curveData();
    auto* curveDataB = m_wellLogCurveB()->curveData();

    if ( !curveDataA || !curveDataB ) return;

    if ( curveDataA->depthUnit() != curveDataB->depthUnit() )
    {
        // Return warning!
    }

    const auto depthType                = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
    const auto curveADepths             = curveDataA->depths( depthType );
    const auto curveDataAPropertyValues = curveDataA->propertyValues();

    // Resample curve B to curve A
    cvf::ref<RigWellLogCurveData> curveDataBResampled           = curveDataB->calculateResampledCurveData( depthType, curveADepths );
    auto                          curveBDepthValuesResampled    = curveDataBResampled->depths( depthType );
    auto                          curveBPropertyValuesResampled = curveDataBResampled->propertyValues();

    // curveDataA size and curveBPropertiesResampled size should be the same
    if ( curveADepths.size() != curveDataAPropertyValues.size() ) return;
    if ( curveADepths.size() != curveBDepthValuesResampled.size() ) return;
    if ( curveDataAPropertyValues.size() != curveBPropertyValuesResampled.size() ) return;

    std::vector<double> curveDiffDepthValues( curveADepths.size() );
    std::vector<double> curveDiffPropertyValues( curveDataAPropertyValues.size() );
    for ( size_t i = 0; i < curveDataAPropertyValues.size(); ++i )
    {
        curveDiffPropertyValues[i] = curveDataAPropertyValues[i] - curveBPropertyValuesResampled[i];
        curveDiffDepthValues[i]    = curveADepths[i];
    }

    const bool useLogarithmicScale = false;
    auto       depthsMap           = std::map<RiaDefines::DepthTypeEnum, std::vector<double>>();
    depthsMap[depthType]           = curveDiffDepthValues;
    setPropertyValuesAndDepths( curveDiffPropertyValues, depthsMap, 0.0, curveDataA->depthUnit(), false, false, QString( "" ) );

    // TODO: NOT VISUALISING IN PLOT!

    // m_plotCurve->setSamplesFromXValuesAndYValues( curveDiffPropertyValues, curveDiffDepthValues, useLogarithmicScale );
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
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Data Source" );
    group->add( &m_wellLogCurveA );
    group->add( &m_wellLogCurveB );

    uiOrdering.skipRemainingFields( true );
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

        onLoadDataAndUpdate( false );
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

        // for ( const auto* wellLogPlot : wellLogPlotCollection->descendantsOfType<>() )
        //{
        //     if ( !wellLogPlot ) continue;

        //    // Retrieve each track in plot
        //    // Note: How to get all RimWellLogTracks - not based on visibility?
        //    for ( auto* rimPlot : wellLogPlot->plots() )
        //    {
        //        RimWellLogTrack* plot = dynamic_cast<RimWellLogTrack*>( rimPlot );
        //        if ( !plot ) continue;

        //        // Extract each curve in track
        //        for ( RimWellLogCurve* curve : plot->curves() )
        //        {
        //            options.push_back( caf::PdmOptionItemInfo( curve->uiName(), curve ) );
        //        }
        //    }
        //}
    }
    return options;
}
