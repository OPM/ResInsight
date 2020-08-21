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

#include "RimFractureModel.h"
#include "RimFractureModelCollection.h"
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
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_md, "MeasuredDepth", "", "", "", "Measured Depth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_elasticPropertiesFilePath,
                                          "ElasticPropertiesFilePath",
                                          "",
                                          "",
                                          "",
                                          "Elastic Properties File Path" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcFractureModelCollection_newFractureModel::execute()
{
    RimFractureModel* newFractureModel = nullptr;
    if ( m_wellPath )
    {
        RimFractureModelCollection* fractureModelCollection = self<RimFractureModelCollection>();
        RimWellPathCollection*      wellPathCollection      = nullptr;
        fractureModelCollection->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

        newFractureModel = RicNewFractureModelFeature::addFractureModel( m_wellPath, wellPathCollection );
    }

    if ( newFractureModel )
    {
        newFractureModel->setMD( m_md() );

        RicElasticPropertiesImportTools::importElasticPropertiesFromFile( m_elasticPropertiesFilePath, newFractureModel );

        self<RimFractureModelCollection>()->updateAllRequiredEditors();
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
