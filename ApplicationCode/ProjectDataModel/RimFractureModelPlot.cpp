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

#include "RimFractureModel.h"
#include "RimFractureModelCurve.h"
#include "RimFractureModelPropertyCurve.h"
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

    setLegendsVisible( true );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
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
