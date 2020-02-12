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
    CAF_PDM_InitObject( "Separate Intersection Results", ":/CrossSections16x16.png", "", "" );

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_intersectionResultsDefs, "IntersectionResultDefinitions", "Data Sources", "", "", "" );
    m_intersectionResultsDefs.uiCapability()->setUiTreeHidden( true );

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
std::vector<RimIntersectionResultDefinition*>
    RimIntersectionResultsDefinitionCollection::intersectionResultsDefinitions() const
{
    return m_intersectionResultsDefs.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultsDefinitionCollection::appendIntersectionResultDefinition(
    RimIntersectionResultDefinition* interResDef )
{
    m_intersectionResultsDefs.push_back( interResDef );

    if ( interResDef->activeCase() == nullptr )
    {
        RimCase* ownerCase = nullptr;
        this->firstAncestorOrThisOfType( ownerCase );
        interResDef->setActiveCase( ownerCase );
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
    this->updateUiIconFromToggleField();

    RimGridView* gridView = nullptr;
    this->firstAncestorOrThisOfType( gridView );
    if ( gridView )
    {
        gridView->scheduleCreateDisplayModelAndRedraw();
        gridView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionResultsDefinitionCollection::initAfterRead()
{
    this->updateUiIconFromToggleField();
}
