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
#include "RimcStimPlanModelTemplateCollection.h"

#include "RicElasticPropertiesImportTools.h"
#include "RicFaciesPropertiesImportTools.h"

#include "FractureCommands/RicFractureNameGenerator.h"
#include "FractureCommands/RicNewStimPlanModelFeature.h"

#include "RimStimPlanModelTemplate.h"
#include "RimStimPlanModelTemplateCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimStimPlanModelTemplateCollection,
                                   RimcStimPlanModelTemplateCollection_newStimPlanModelTemplate,
                                   "NewStimPlanModelTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcStimPlanModelTemplateCollection_newStimPlanModelTemplate::RimcStimPlanModelTemplateCollection_newStimPlanModelTemplate(
    caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create StimPlan Model Template", "", "", "Create a new StimPlan Model Template" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_faciesPropertiesFilePath,
                                          "FaciesPropertiesFilePath",
                                          "",
                                          "",
                                          "",
                                          "Facies Properties File Path" );
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
caf::PdmObjectHandle* RimcStimPlanModelTemplateCollection_newStimPlanModelTemplate::execute()
{
    RimStimPlanModelTemplate*           newStimPlanModelTemplate        = new RimStimPlanModelTemplate;
    RimStimPlanModelTemplateCollection* stimPlanModelTemplateCollection = self<RimStimPlanModelTemplateCollection>();
    newStimPlanModelTemplate->setName( RicFractureNameGenerator::nameForNewStimPlanModelTemplate() );
    stimPlanModelTemplateCollection->addStimPlanModelTemplate( newStimPlanModelTemplate );

    RicElasticPropertiesImportTools::importElasticPropertiesFromFile( m_elasticPropertiesFilePath,
                                                                      newStimPlanModelTemplate );
    RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( m_faciesPropertiesFilePath,
                                                                    newStimPlanModelTemplate,
                                                                    true );
    stimPlanModelTemplateCollection->updateAllRequiredEditors();
    return newStimPlanModelTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcStimPlanModelTemplateCollection_newStimPlanModelTemplate::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcStimPlanModelTemplateCollection_newStimPlanModelTemplate::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimStimPlanModelTemplate );
}
