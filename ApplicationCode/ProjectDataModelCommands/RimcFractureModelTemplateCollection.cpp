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
#include "RimcFractureModelTemplateCollection.h"

#include "RicElasticPropertiesImportTools.h"
#include "RicFaciesPropertiesImportTools.h"

#include "FractureCommands/RicFractureNameGenerator.h"
#include "FractureCommands/RicNewFractureModelFeature.h"

#include "RimFractureModelTemplate.h"
#include "RimFractureModelTemplateCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureModelTemplateCollection,
                                   RimcFractureModelTemplateCollection_newFractureModelTemplate,
                                   "NewFractureModelTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFractureModelTemplateCollection_newFractureModelTemplate::RimcFractureModelTemplateCollection_newFractureModelTemplate(
    caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create Fracture Model Template", "", "", "Create a new Fracture Model Template" );
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
caf::PdmObjectHandle* RimcFractureModelTemplateCollection_newFractureModelTemplate::execute()
{
    RimFractureModelTemplate*           newFractureModelTemplate        = new RimFractureModelTemplate;
    RimFractureModelTemplateCollection* fractureModelTemplateCollection = self<RimFractureModelTemplateCollection>();
    newFractureModelTemplate->setName( RicFractureNameGenerator::nameForNewFractureModelTemplate() );
    fractureModelTemplateCollection->addFractureModelTemplate( newFractureModelTemplate );

    RicElasticPropertiesImportTools::importElasticPropertiesFromFile( m_elasticPropertiesFilePath,
                                                                      newFractureModelTemplate );
    RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( m_faciesPropertiesFilePath, newFractureModelTemplate, true );
    fractureModelTemplateCollection->updateAllRequiredEditors();
    return newFractureModelTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFractureModelTemplateCollection_newFractureModelTemplate::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcFractureModelTemplateCollection_newFractureModelTemplate::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFractureModelTemplate );
}
