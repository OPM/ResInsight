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

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimStimPlanModelCollection, RimcStimPlanModelCollection_appendStimPlanModel, "AppendStimPlanModel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcStimPlanModelCollection_appendStimPlanModel::RimcStimPlanModelCollection_appendStimPlanModel( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create StimPlan Model", "", "", "Create a new StimPlan Model" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "", "", "", "Well Path" );
    CAF_PDM_InitScriptableField( &m_md, "MeasuredDepth", 0.0, "Measured Depth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanModelTemplate, "StimPlanModelTemplate", "", "", "", "StimPlan Model Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcStimPlanModelCollection_appendStimPlanModel::execute()
{
    RimStimPlanModel*           stimPlanModel           = nullptr;
    RimStimPlanModelCollection* stimPlanModelCollection = self<RimStimPlanModelCollection>();
    if ( m_wellPath )
    {
        auto wellPathCollection = stimPlanModelCollection->firstAncestorOrThisOfTypeAsserted<RimWellPathCollection>();

        stimPlanModel = RicNewStimPlanModelFeature::addStimPlanModel( m_wellPath, wellPathCollection );
    }

    if ( stimPlanModel )
    {
        stimPlanModel->setMD( m_md() );
        stimPlanModel->setStimPlanModelTemplate( m_stimPlanModelTemplate() );
        stimPlanModelCollection->updateAllRequiredEditors();
        return stimPlanModel;
    }
    else
    {
        return std::unexpected( "Unable to add StimPlan model." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcStimPlanModelCollection_appendStimPlanModel::classKeywordReturnedType() const
{
    return RimStimPlanModel::classKeywordStatic();
}
