/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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
#include "RimcFractureTemplateCollection.h"

#include "FractureCommands/RicFractureNameGenerator.h"
#include "FractureCommands/RicNewStimPlanFractureTemplateFeature.h"
#include "FractureCommands/RicNewStimPlanModelFeature.h"
#include "FractureCommands/RicNewThermalFractureTemplateFeature.h"

#include "RimEclipseCase.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimThermalFractureTemplate.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureTemplateCollection,
                                   RimcFractureTemplateCollection_appendFractureTemplate,
                                   "AppendFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFractureTemplateCollection_appendFractureTemplate::RimcFractureTemplateCollection_appendFractureTemplate( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Fracture Template", "", "", "Create a new StimPlan Fracture Template" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_filePath, "FilePath", "", "", "", "File Path to StimPlan Countour File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFractureTemplateCollection_appendFractureTemplate::execute()
{
    RimFractureTemplateCollection* stimPlanModelTemplateCollection = self<RimFractureTemplateCollection>();

    bool reuseExistingTemplatesWithMatchingNames = false;
    auto newTemplates =
        RicNewStimPlanFractureTemplateFeature::createNewTemplatesFromFiles( { m_filePath }, reuseExistingTemplatesWithMatchingNames );

    if ( newTemplates.empty() ) return nullptr;

    stimPlanModelTemplateCollection->updateAllRequiredEditors();
    return newTemplates[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcFractureTemplateCollection_appendFractureTemplate::classKeywordReturnedType() const
{
    return RimStimPlanFractureTemplate::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureTemplateCollection,
                                   RimcFractureTemplateCollection_appendThermalFractureTemplate,
                                   "AppendThermalFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFractureTemplateCollection_appendThermalFractureTemplate::RimcFractureTemplateCollection_appendThermalFractureTemplate( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Fracture Template", "", "", "Create a new Thermal Fracture Template" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_filePath, "FilePath", "", "", "", "File Path to Thermal Fracture CSV File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFractureTemplateCollection_appendThermalFractureTemplate::execute()
{
    RimFractureTemplateCollection* fractureTemplateCollection = self<RimFractureTemplateCollection>();

    bool reuseExistingTemplatesWithMatchingNames = false;
    auto newTemplates =
        RicNewThermalFractureTemplateFeature::createNewTemplatesFromFiles( { m_filePath }, reuseExistingTemplatesWithMatchingNames );

    if ( newTemplates.empty() ) return nullptr;

    fractureTemplateCollection->updateAllRequiredEditors();
    return newTemplates[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcFractureTemplateCollection_appendThermalFractureTemplate::classKeywordReturnedType() const
{
    return RimThermalFractureTemplate::classKeywordStatic();
}
