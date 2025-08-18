/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RimcValveTemplateCollection.h"

#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimValveTemplateCollection, RimcValveTemplateCollection_add_template, "AddTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcValveTemplateCollection_add_template::RimcValveTemplateCollection_add_template( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Valve Template", "", "", "Add a new valve template" );

    CAF_PDM_InitScriptableField( &m_completionType,
                                 "CompletionType",
                                 caf::AppEnum<RiaDefines::WellPathComponentType>( RiaDefines::WellPathComponentType::ICD ),
                                 "",
                                 "",
                                 "",
                                 "Completion type (ICD, ICV, or AICD)" );
    CAF_PDM_InitScriptableField( &m_orificeDiameter, "OrificeDiameter", RimValveTemplate::defaultOrificeDiameter(), "", "", "", "Orifice diameter" );
    CAF_PDM_InitScriptableField( &m_flowCoefficient, "FlowCoefficient", RimValveTemplate::defaultFlowCoefficient(), "", "", "", "Flow coefficient" );
    CAF_PDM_InitScriptableField( &m_userLabel, "UserLabel", QString( "" ), "", "", "", "User-defined label for the template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcValveTemplateCollection_add_template::execute()
{
    RimValveTemplateCollection* valveTemplateCollection = self<RimValveTemplateCollection>();
    if ( !valveTemplateCollection )
    {
        return std::unexpected( QString( "Invalid valve template collection" ) );
    }

    // Validate completion type
    RiaDefines::WellPathComponentType completionType = m_completionType();
    if ( completionType != RiaDefines::WellPathComponentType::ICD && completionType != RiaDefines::WellPathComponentType::ICV &&
         completionType != RiaDefines::WellPathComponentType::AICD )
    {
        return std::unexpected( QString( "Invalid completion type. Must be ICD, ICV, or AICD" ) );
    }

    // Create new valve template
    RimValveTemplate* newTemplate = new RimValveTemplate();
    newTemplate->setType( completionType );
    newTemplate->setUnitSystem( valveTemplateCollection->defaultUnitSystemType() );

    // Set user label or generate default
    QString userLabel = m_userLabel();
    if ( userLabel.isEmpty() )
    {
        // Generate default label based on type and count
        auto templates = valveTemplateCollection->valveTemplates();
        int  count     = static_cast<int>( templates.size() ) + 1;
        userLabel      = QString( "Template %1" ).arg( count );
    }
    newTemplate->setUserLabel( userLabel );

    // Update the name to reflect the type and user label
    newTemplate->setName( newTemplate->fullLabel() );

    // Set unit-specific defaults first, then override with user values
    newTemplate->setDefaultValuesFromUnits();
    newTemplate->setOrificeDiameter( m_orificeDiameter() );
    newTemplate->setFlowCoefficient( m_flowCoefficient() );

    // Add to collection
    valveTemplateCollection->addValveTemplate( newTemplate );
    valveTemplateCollection->updateAllRequiredEditors();

    return newTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcValveTemplateCollection_add_template::classKeywordReturnedType() const
{
    return RimValveTemplate::classKeywordStatic();
}