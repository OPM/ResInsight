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

#include "RimWellLogCalculatedCurve.h"

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

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellLogCalculatedCurve, "WellLogCalculatedCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimWellLogCalculatedCurve::Operators>::setUp()
{
    addItem( RimWellLogCalculatedCurve::Operators::ADD, "ADD", "+" );
    addItem( RimWellLogCalculatedCurve::Operators::SUBTRACT, "SUBTRACT", "-" );
    addItem( RimWellLogCalculatedCurve::Operators::MULTIPLY, "MULTIPLY", "*" );
    addItem( RimWellLogCalculatedCurve::Operators::DIVIDE, "DIVIDE", "/" );
    setDefault( RimWellLogCalculatedCurve::Operators::SUBTRACT );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCalculatedCurve::RimWellLogCalculatedCurve()
{
    CAF_PDM_InitObject( "Well Log Calculated Curve", ":/WellLogCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_operator, "Operator", "Operator" );
    m_operator.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_firstWellLogCurve, "FirstWellLogCurve", "First Well Log Curve" );
    CAF_PDM_InitFieldNoDefault( &m_secondWellLogCurve, "SecondWellLogCurve", "Second Well Log Curve" );

    setNamingMethod( RiaDefines::ObjectNamingMethod::AUTO );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCalculatedCurve::~RimWellLogCalculatedCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::setOperator( Operators operatorValue )
{
    m_operator = operatorValue;
    if ( m_namingMethod() == RiaDefines::ObjectNamingMethod::AUTO )
    {
        setAutomaticName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::setWellLogCurves( RimWellLogCurve* firstWellLogCurve, RimWellLogCurve* secondWellLogCurve )
{
    disconnectWellLogCurveChangedFromSlots( m_firstWellLogCurve );
    disconnectWellLogCurveChangedFromSlots( m_secondWellLogCurve );
    if ( firstWellLogCurve )
    {
        m_firstWellLogCurve = firstWellLogCurve;
        connectWellLogCurveChangedToSlots( m_firstWellLogCurve );
    }
    if ( secondWellLogCurve )
    {
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
QString RimWellLogCalculatedCurve::createCurveAutoName()
{
    if ( !m_firstWellLogCurve() || !m_secondWellLogCurve() ) return QString( "Not able to find source curves for calculated curve" );

    const auto& operatorStr = m_operator().uiText();
    return QString( "Calculated (%1 %2 %3)" ).arg( m_firstWellLogCurve->curveName() ).arg( operatorStr ).arg( m_secondWellLogCurve->curveName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::onLoadDataAndUpdate( bool updateParentPlot )
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

    // Calculate curve
    std::vector<double> calculatedDepthValues( firstCurveDepthValues.size() );
    std::vector<double> calculatedPropertyValues( firstCurvePropertyValues.size() );
    for ( size_t i = 0; i < firstCurvePropertyValues.size(); ++i )
    {
        calculatedPropertyValues[i] = calculateValue( firstCurvePropertyValues[i], secondCurvePropertyValuesResampled[i], m_operator() );
        calculatedDepthValues[i]    = firstCurveDepthValues[i];
    }

    const bool useLogarithmicScale = false;
    const bool isExtractionCurve   = false;

    // Set curve data
    auto depthsMap       = std::map<RiaDefines::DepthTypeEnum, std::vector<double>>();
    depthsMap[depthType] = calculatedDepthValues;
    setPropertyValuesAndDepths( calculatedPropertyValues, depthsMap, 0.0, depthUnit, isExtractionCurve, useLogarithmicScale, propertyUnit );

    // Set curve data to plot
    std::vector<double> xPlotValues = curveData()->propertyValuesByIntervals();
    std::vector<double> yPlotValues = curveData()->depthValuesByIntervals( depthType, depthUnit );
    m_plotCurve->setSamplesFromXValuesAndYValues( xPlotValues, yPlotValues, useLogarithmicScale );
    updateCurvePresentation( updateParentPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::setAutomaticName()
{
    m_curveName = createCurveAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::onWellLogCurveChanged( const SignalEmitter* emitter )
{
    onLoadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::connectWellLogCurveChangedToSlots( RimWellLogCurve* wellLogCurve )
{
    if ( !wellLogCurve ) return;

    wellLogCurve->dataChanged.connect( this, &RimWellLogCalculatedCurve::onWellLogCurveChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::disconnectWellLogCurveChangedFromSlots( RimWellLogCurve* wellLogCurve )
{
    if ( !wellLogCurve ) return;

    wellLogCurve->dataChanged.disconnect( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogCalculatedCurve::calculateValue( double firstValue, double secondValue, Operators operatorValue )
{
    if ( operatorValue == Operators::ADD )
    {
        return firstValue + secondValue;
    }
    if ( operatorValue == Operators::SUBTRACT )
    {
        return firstValue - secondValue;
    }
    if ( operatorValue == Operators::MULTIPLY )
    {
        return firstValue * secondValue;
    }
    if ( operatorValue == Operators::DIVIDE )
    {
        return firstValue / secondValue;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCalculatedCurve::wellName() const
{
    return m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCalculatedCurve::wellLogChannelUiName() const
{
    return m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCalculatedCurve::wellLogChannelUnits() const
{
    if ( m_firstWellLogCurve->wellLogChannelUnits() != m_secondWellLogCurve->wellLogChannelUnits() )
    {
        const auto& operatorStr = m_operator().uiText();
        return QString( "%1 %2 %3" )
            .arg( m_firstWellLogCurve->wellLogChannelUnits() )
            .arg( operatorStr )
            .arg( m_secondWellLogCurve->wellLogChannelUnits() );
    }
    return m_firstWellLogCurve->wellLogChannelUnits();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Data Source" );
    group->add( &m_operator );
    group->add( &m_firstWellLogCurve );
    group->add( &m_secondWellLogCurve );

    RimStackablePlotCurve::defaultUiOrdering( uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCalculatedCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_firstWellLogCurve || changedField == &m_secondWellLogCurve )
    {
        if ( !m_firstWellLogCurve() || !m_secondWellLogCurve() ) return;

        PdmObjectHandle* prevValue        = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
        auto*            prevWellLogCurve = dynamic_cast<RimWellLogCurve*>( prevValue );
        disconnectWellLogCurveChangedFromSlots( prevWellLogCurve );

        if ( changedField == &m_firstWellLogCurve ) connectWellLogCurveChangedToSlots( m_firstWellLogCurve );
        if ( changedField == &m_secondWellLogCurve ) connectWellLogCurveChangedToSlots( m_secondWellLogCurve );

        loadDataAndUpdate( true );
    }
    if ( changedField == &m_operator )
    {
        loadDataAndUpdate( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogCalculatedCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions );
    if ( !options.empty() ) return options;

    if ( fieldNeedingOptions == &m_firstWellLogCurve || fieldNeedingOptions == &m_secondWellLogCurve )
    {
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
