/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCustomObjectiveFunctionCollection.h"

#include "RimCustomObjectiveFunction.h"

#include <cafPdmUiTreeOrdering.h>

CAF_PDM_SOURCE_INIT( RimCustomObjectiveFunctionCollection, "RimCustomObjectiveFunctionCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionCollection::RimCustomObjectiveFunctionCollection()
    : objectiveFunctionChanged( this )
{
    CAF_PDM_InitObject( "Custom Objective Functions", ":/ObjectiveFunctionCollection.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_objectiveFunctions, "ObjectiveFunctions", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunction* RimCustomObjectiveFunctionCollection::addObjectiveFunction()
{
    auto newFunction = new RimCustomObjectiveFunction();
    m_objectiveFunctions.push_back( newFunction );
    return newFunction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionCollection::onObjectiveFunctionChanged( RimCustomObjectiveFunction* objectiveFunction )
{
    objectiveFunctionChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCustomObjectiveFunction*> RimCustomObjectiveFunctionCollection::objectiveFunctions() const
{
    return m_objectiveFunctions.childObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                                 QString                 uiConfigName /* = "" */ )
{
    for ( auto func : objectiveFunctions() )
    {
        uiTreeOrdering.add( func );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                           std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    objectiveFunctionChanged.send();
}
