/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimIntersectionResultsDefinitionCollection.h"

#include "RimCase.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultDefinition.h"

CAF_PDM_SOURCE_INIT( RimIntersectionResultsDefinitionCollection, "RimIntersectionResultsDefinitionCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection::RimIntersectionResultsDefinitionCollection()
{
    CAF_PDM_InitObject( "Intersection Results", ":/CrossSections16x16.png" );

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_intersectionResultsDefs, "IntersectionResultDefinitions", "Data Sources" );

    m_intersectionResultsDefs.push_back( new RimIntersectionResultDefinition ); // Add the default result definition
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection::~RimIntersectionResultsDefinitionCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionResultsDefinitionCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimIntersectionResultDefinition*> RimIntersectionResultsDefinitionCollection::intersectionResultsDefinitions() const
{
    return m_intersectionResultsDefs.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultsDefinitionCollection::appendIntersectionResultDefinition( RimIntersectionResultDefinition* interResDef )
{
    m_intersectionResultsDefs.push_back( interResDef );

    if ( interResDef->activeCase() == nullptr )
    {
        auto ownerCase = firstAncestorOrThisOfType<RimCase>();
        if ( ownerCase )
        {
            interResDef->setActiveCase( ownerCase );
        }
        else if ( auto gridView = firstAncestorOrThisOfType<Rim3dView>() )
        {
            if ( auto ownerCase = gridView->ownerCase() )
            {
                interResDef->setActiveCase( ownerCase );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionResultsDefinitionCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultsDefinitionCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                                   const QVariant&            oldValue,
                                                                   const QVariant&            newValue )
{
    updateUiIconFromToggleField();

    auto gridView = firstAncestorOrThisOfType<RimGridView>();
    if ( gridView ) gridView->scheduleCreateDisplayModelAndRedraw();
    if ( !intersectionResultsDefinitions().empty() ) intersectionResultsDefinitions()[0]->update2dIntersectionViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultsDefinitionCollection::initAfterRead()
{
    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultsDefinitionCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                                 std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    auto gridView = firstAncestorOrThisOfType<RimGridView>();
    if ( gridView )
    {
        gridView->scheduleCreateDisplayModelAndRedraw();
        gridView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }
}
