/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimValveCollection.h"

#include "RimProject.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"
#include "RimWellPathValve.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimValveCollection, "ValveCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveCollection::RimValveCollection()
{
    CAF_PDM_InitScriptableObject( "Valves", ":/ICVValve16x16.png" );

    nameField()->uiCapability()->setUiHidden( true );
    setName( "Valves" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_valves, "Valves", "Valves" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimValveCollection::hasValves() const
{
    return !m_valves.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve* RimValveCollection::addIcvValve()
{
    RimWellPathValve* valve = new RimWellPathValve();

    auto templateCollections = RimProject::current()->allValveTemplateCollections();
    if ( templateCollections.empty() ) return nullptr;

    auto tempColl = templateCollections.front();

    auto icvTemplate = tempColl->getOrCreateIcvTemplate();
    valve->setValveTemplate( icvTemplate );
    // TOOO - get the default md from somewhere
    valve->setMeasuredDepthAndCount( 1000.0, 1.0, 1 );

    m_valves.push_back( valve );

    return valve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathValve*> RimValveCollection::valves() const
{
    std::vector<RimWellPathValve*> valvesToReturn;

    for ( const auto& v : m_valves() )
    {
        valvesToReturn.push_back( v );
    }

    return valvesToReturn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimWellPathValve*> RimValveCollection::activeValves() const
{
    std::vector<const RimWellPathValve*> valvesToReturn;

    for ( const auto& v : m_valves() )
    {
        if ( v->isChecked() )
        {
            valvesToReturn.push_back( v );
        }
    }

    return valvesToReturn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimProject* proj = RimProject::current();
    if ( changedField == &m_isChecked )
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimProject* proj = RimProject::current();
    proj->reloadCompletionTypeResultsInAllViews();

    updateConnectedEditors();
}
