/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimFractureModelTemplateCollection.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureModelTemplate.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimFractureModelTemplateCollection, "FractureModelTemplateCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelTemplateCollection::RimFractureModelTemplateCollection()
{
    CAF_PDM_InitScriptableObject( "Fracture Model Templates", ":/FractureTemplates16x16.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fractureModelTemplates,
                                          "FractureModelTemplates",
                                          "Fracture Model Templates",
                                          "",
                                          "",
                                          "" );
    m_fractureModelTemplates.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_nextValidId, "NextValidId", 0, "", "", "", "" );
    m_nextValidId.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelTemplateCollection::~RimFractureModelTemplateCollection()
{
    m_fractureModelTemplates.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelTemplate* RimFractureModelTemplateCollection::fractureModelTemplate( int id ) const
{
    for ( const auto& templ : m_fractureModelTemplates )
    {
        if ( templ->id() == id ) return templ;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureModelTemplate*> RimFractureModelTemplateCollection::fractureModelTemplates() const
{
    std::vector<RimFractureModelTemplate*> templates;
    for ( auto& templ : m_fractureModelTemplates )
    {
        templates.push_back( templ );
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelTemplateCollection::addFractureModelTemplate( RimFractureModelTemplate* templ )
{
    templ->setId( nextFractureTemplateId() );
    m_fractureModelTemplates.push_back( templ );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelTemplateCollection::loadAndUpdateData()
{
    for ( RimFractureModelTemplate* f : m_fractureModelTemplates() )
    {
        f->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelTemplateCollection::initAfterRead()
{
    // Assign template id if not already assigned
    for ( auto& templ : m_fractureModelTemplates )
    {
        if ( templ->id() < 0 ) templ->setId( nextFractureTemplateId() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFractureModelTemplateCollection::nextFractureTemplateId()
{
    int newId     = m_nextValidId;
    m_nextValidId = m_nextValidId + 1;

    return newId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelTemplateCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                         std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimProject* proj = nullptr;
    firstAncestorOrThisOfType( proj );
    if ( proj )
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }

    std::vector<Rim3dView*> views;
    proj->allVisibleViews( views );
    for ( Rim3dView* visibleView : views )
    {
        if ( dynamic_cast<RimEclipseView*>( visibleView ) )
        {
            visibleView->updateConnectedEditors();
        }
    }
}
