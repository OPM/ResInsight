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
#include "RimcStimPlanModelCollection.h"

#include "RicElasticPropertiesImportTools.h"

#include "FractureCommands/RicNewStimPlanModelFeature.h"

#include "RimEclipseCase.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCollection.h"
#include "RimStimPlanModelTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimStimPlanModelCollection,
                                   RimcStimPlanModelCollection_newStimPlanModel,
                                   "NewStimPlanModel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcStimPlanModelCollection_newStimPlanModel::RimcStimPlanModelCollection_newStimPlanModel( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create StimPlan Model", "", "", "Create a new StimPlan Model" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableField( &m_md, "MeasuredDepth", 0.0, "Measured Depth", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanModelTemplate, "StimPlanModelTemplate", "", "", "", "StimPlan Model Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcStimPlanModelCollection_newStimPlanModel::execute()
{
    RimStimPlanModel*           newStimPlanModel        = nullptr;
    RimStimPlanModelCollection* stimPlanModelCollection = self<RimStimPlanModelCollection>();
    if ( m_wellPath )
    {
        RimWellPathCollection* wellPathCollection = nullptr;
        stimPlanModelCollection->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

        newStimPlanModel = RicNewStimPlanModelFeature::addStimPlanModel( m_wellPath, wellPathCollection );
    }

    if ( newStimPlanModel )
    {
        newStimPlanModel->setMD( m_md() );
        newStimPlanModel->setStimPlanModelTemplate( m_stimPlanModelTemplate() );
        stimPlanModelCollection->updateAllRequiredEditors();
    }

    return newStimPlanModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcStimPlanModelCollection_newStimPlanModel::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcStimPlanModelCollection_newStimPlanModel::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimStimPlanModel );
}
