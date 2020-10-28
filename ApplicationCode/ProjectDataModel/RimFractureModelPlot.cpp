/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RimFractureModelPlot.h"

#include "RiaDefines.h"
#include "RiaFractureModelDefines.h"

#include "RimEclipseCase.h"
#include "RimFractureModel.h"
#include "RimFractureModelCurve.h"
#include "RimFractureModelPropertyCurve.h"
#include "RimTools.h"
#include "RimWellLogTrack.h"

#include "cafPdmBase.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiGroup.h"

CAF_PDM_SOURCE_INIT( RimFractureModelPlot, "FractureModelPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlot::RimFractureModelPlot()
{
    CAF_PDM_InitScriptableObject( "Fracture Model Plot", "", "", "A fracture model plot" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_eclipseCase, "EclipseCase", "Case", "", "", "" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "" );

    setLegendsVisible( true );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
    m_eclipseCase   = fractureModel->eclipseCase();
    m_timeStep      = fractureModel->timeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel* RimFractureModelPlot::fractureModel()
{
    return m_fractureModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_eclipseCase );
    uiOrdering.add( &m_timeStep );

    caf::PdmUiGroup* depthGroup = uiOrdering.addNewGroup( "Depth Axis" );
    RimDepthTrackPlot::uiOrderingForDepthAxis( uiConfigName, *depthGroup );

    caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup( "Plot Title" );
    RimDepthTrackPlot::uiOrderingForAutoName( uiConfigName, *titleGroup );

    caf::PdmUiGroup* plotLayoutGroup = uiOrdering.addNewGroup( "Plot Layout" );
    RimPlotWindow::uiOrderingForPlotLayout( uiConfigName, *plotLayoutGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimFractureModelPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_eclipseCase(), &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
    if ( m_fractureModel )
    {
        if ( changedField == &m_eclipseCase || changedField == &m_timeStep )
        {
            m_fractureModel->setEclipseCaseAndTimeStep( m_eclipseCase(), m_timeStep() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::onLoadDataAndUpdate()
{
    // Enable and disable detailed fluid loss curves
    if ( fractureModel() != nullptr )
    {
        std::vector<RiaDefines::CurveProperty> fluidLossCurves = {RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT,
                                                                  RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR,
                                                                  RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT,
                                                                  RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION};

        bool detailedFluidLoss = fractureModel()->useDetailedFluidLoss();

        for ( auto curveProperty : fluidLossCurves )
        {
            RimWellLogExtractionCurve* curve = findCurveByProperty( curveProperty );
            if ( curve )
            {
                RimWellLogTrack* track = nullptr;
                curve->firstAncestorOfType( track );
                if ( track )
                {
                    track->setShowWindow( detailedFluidLoss );
                }
            }
        }
    }

    RimDepthTrackPlot::onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::applyDataSource()
{
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RimFractureModelPlot::findCurveByProperty( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RimFractureModelPropertyCurve*> curves;
    descendantsIncludingThisOfType( curves );

    for ( RimFractureModelPropertyCurve* curve : curves )
    {
        if ( curve->curveProperty() == curveProperty )
        {
            return dynamic_cast<RimWellLogExtractionCurve*>( curve );
        }
    }

    return nullptr;
}
