/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
#include "RimcFractureModelPlotCollection.h"

#include "CompletionCommands/RicNewFractureModelPlotFeature.h"

#include "RimEclipseCase.h"
#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"
#include "RimFractureModelPlotCollection.h"

#include "cafPdmFieldIOScriptability.h"
#include "cafPdmFieldScriptability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureModelPlotCollection,
                                   RimcFractureModelPlotCollection_newFractureModelPlot,
                                   "NewFractureModelPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFractureModelPlotCollection_newFractureModelPlot::RimcFractureModelPlotCollection_newFractureModelPlot(
    caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create Fracture Model", "", "", "Create a new Fracture Model" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_eclipseCase, "EclipseCase", "", "", "", "Eclipse Case" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_fractureModel, "FractureModel", "", "", "", "Fracture Model" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_timeStep, "TimeStep", "", "", "", "Time Step" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcFractureModelPlotCollection_newFractureModelPlot::execute()
{
    RimFractureModelPlot*           newFractureModelPlot        = nullptr;
    RimFractureModelPlotCollection* fractureModelPlotCollection = self<RimFractureModelPlotCollection>();

    if ( m_eclipseCase && m_fractureModel && fractureModelPlotCollection )
    {
        newFractureModelPlot = RicNewFractureModelPlotFeature::createPlot( m_eclipseCase, m_fractureModel, m_timeStep );
    }

    return newFractureModelPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFractureModelPlotCollection_newFractureModelPlot::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcFractureModelPlotCollection_newFractureModelPlot::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFractureModelPlot );
}
