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

#include "RimEclipseCase.h"
#include "RimStimPlanModelTemplate.h"
#include "RimStimPlanModelTemplateCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimStimPlanModelTemplateCollection,
                                   RimcStimPlanModelTemplateCollection_appendStimPlanModelTemplate,
                                   "AppendStimPlanModelTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcStimPlanModelTemplateCollection_appendStimPlanModelTemplate::RimcStimPlanModelTemplateCollection_appendStimPlanModelTemplate(
    caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create StimPlan Model Template", "", "", "Create a new StimPlan Model Template" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_eclipseCase, "EclipseCase", "", "", "", "Eclipse Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_timeStep, "TimeStep", "", "", "", "Time Step" );

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
caf::PdmObjectHandle* RimcStimPlanModelTemplateCollection_appendStimPlanModelTemplate::execute()
{
    if ( !m_eclipseCase ) return nullptr;

    RimStimPlanModelTemplate*           appendStimPlanModelTemplate     = new RimStimPlanModelTemplate;
    RimStimPlanModelTemplateCollection* stimPlanModelTemplateCollection = self<RimStimPlanModelTemplateCollection>();

    appendStimPlanModelTemplate->setName( RicFractureNameGenerator::nameForNewStimPlanModelTemplate() );
    appendStimPlanModelTemplate->setDynamicEclipseCase( m_eclipseCase );
    appendStimPlanModelTemplate->setTimeStep( m_timeStep );
    appendStimPlanModelTemplate->setInitialPressureEclipseCase( m_eclipseCase );
    appendStimPlanModelTemplate->setStaticEclipseCase( m_eclipseCase );

    stimPlanModelTemplateCollection->addStimPlanModelTemplate( appendStimPlanModelTemplate );

    RicElasticPropertiesImportTools::importElasticPropertiesFromFile( m_elasticPropertiesFilePath,
                                                                      appendStimPlanModelTemplate );
    RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( m_faciesPropertiesFilePath,
                                                                    appendStimPlanModelTemplate,
                                                                    true );
    stimPlanModelTemplateCollection->updateAllRequiredEditors();
    return appendStimPlanModelTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcStimPlanModelTemplateCollection_appendStimPlanModelTemplate::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcStimPlanModelTemplateCollection_appendStimPlanModelTemplate::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimStimPlanModelTemplate );
}
