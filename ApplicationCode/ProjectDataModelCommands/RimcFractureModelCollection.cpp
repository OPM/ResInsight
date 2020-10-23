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
#include "RimcFractureModelCollection.h"

#include "RicElasticPropertiesImportTools.h"

#include "FractureCommands/RicNewFractureModelFeature.h"

#include "RimEclipseCase.h"
#include "RimFractureModel.h"
#include "RimFractureModelCollection.h"
#include "RimFractureModelTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureModelCollection,
                                   RimcFractureModelCollection_newFractureModel,
                                   "NewFractureModel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFractureModelCollection_newFractureModel::RimcFractureModelCollection_newFractureModel( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create Fracture Model", "", "", "Create a new Fracture Model" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_eclipseCase, "EclipseCase", "", "", "", "Eclipse Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_timeStep, "TimeStep", "", "", "", "Time Step" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_md, "MeasuredDepth", "", "", "", "Measured Depth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_fractureModelTemplate, "FractureModelTemplate", "", "", "", "Fracture Model Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcFractureModelCollection_newFractureModel::execute()
{
    RimFractureModel*           newFractureModel        = nullptr;
    RimFractureModelCollection* fractureModelCollection = self<RimFractureModelCollection>();
    if ( m_wellPath && m_eclipseCase )
    {
        RimWellPathCollection* wellPathCollection = nullptr;
        fractureModelCollection->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

        newFractureModel =
            RicNewFractureModelFeature::addFractureModel( m_wellPath, wellPathCollection, m_eclipseCase, m_timeStep );
    }

    if ( newFractureModel )
    {
        newFractureModel->setMD( m_md() );
        newFractureModel->setFractureModelTemplate( m_fractureModelTemplate() );
        fractureModelCollection->updateAllRequiredEditors();
    }

    return newFractureModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFractureModelCollection_newFractureModel::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcFractureModelCollection_newFractureModel::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFractureModel );
}
