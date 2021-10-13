/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimValveTemplateCollection.h"

#include "RimProject.h"
#include "RimValveTemplate.h"

CAF_PDM_SOURCE_INIT( RimValveTemplateCollection, "ValveTemplateCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplateCollection::RimValveTemplateCollection()
{
    CAF_PDM_InitObject( "Valve Templates", ":/ICDValve16x16.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_valveDefinitions, "ValveDefinitions", "", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_defaultUnitsForValveTemplates,
                                "ValveUnits",
                                "Default unit system for valve templates",
                                "",
                                "",
                                "" );
    m_defaultUnitsForValveTemplates = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
    m_valveDefinitions.uiCapability()->setUiTreeHidden( true );
    addDefaultValveTemplates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplateCollection::~RimValveTemplateCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimValveTemplate*> RimValveTemplateCollection::valveTemplates() const
{
    std::vector<RimValveTemplate*> templates;
    for ( auto& templ : m_valveDefinitions )
    {
        templates.push_back( templ );
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplateCollection::addValveTemplate( RimValveTemplate* valveTemplate )
{
    m_valveDefinitions.push_back( valveTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplateCollection::removeAndDeleteValveTemplate( RimValveTemplate* valveTemplate )
{
    m_valveDefinitions.removeChildObject( valveTemplate );
    delete valveTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimValveTemplateCollection::defaultUnitSystemType() const
{
    return m_defaultUnitsForValveTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplateCollection::setDefaultUnitSystemBasedOnLoadedCases()
{
    RimProject* proj = RimProject::current();

    auto commonUnitSystem = proj->commonUnitSystemForAllCases();
    if ( commonUnitSystem != RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
    {
        m_defaultUnitsForValveTemplates = commonUnitSystem;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplateCollection::addDefaultValveTemplates()
{
    RimValveTemplate* aicd = new RimValveTemplate;
    aicd->setType( RiaDefines::WellPathComponentType::AICD );
    aicd->setUserLabel( "Valve Template #1" );

    RimValveTemplate* icd = new RimValveTemplate;
    icd->setType( RiaDefines::WellPathComponentType::ICD );
    icd->setUserLabel( "Valve Template #2" );

    RimValveTemplate* icv = new RimValveTemplate;
    icv->setType( RiaDefines::WellPathComponentType::ICV );
    icv->setUserLabel( "Valve Template #3" );

    addValveTemplate( aicd );
    addValveTemplate( icd );
    addValveTemplate( icv );
}
