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

#include "RimStimPlanModelTemplateCollection.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimProject.h"
#include "RimStimPlanModelTemplate.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimStimPlanModelTemplateCollection, "StimPlanModelTemplateCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplateCollection::RimStimPlanModelTemplateCollection()
{
    CAF_PDM_InitScriptableObject( "StimPlan Model Templates", ":/FractureTemplates16x16.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanModelTemplates, "StimPlanModelTemplates", "StimPlan Model Templates" );

    CAF_PDM_InitField( &m_nextValidId, "NextValidId", 0, "" );
    m_nextValidId.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplateCollection::~RimStimPlanModelTemplateCollection()
{
    m_stimPlanModelTemplates.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplate* RimStimPlanModelTemplateCollection::stimPlanModelTemplate( int id ) const
{
    for ( const auto& templ : m_stimPlanModelTemplates )
    {
        if ( templ->id() == id ) return templ;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanModelTemplate*> RimStimPlanModelTemplateCollection::stimPlanModelTemplates() const
{
    std::vector<RimStimPlanModelTemplate*> templates;
    for ( auto& templ : m_stimPlanModelTemplates )
    {
        templates.push_back( templ );
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplateCollection::addStimPlanModelTemplate( RimStimPlanModelTemplate* templ )
{
    templ->setId( nextFractureTemplateId() );
    m_stimPlanModelTemplates.push_back( templ );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplateCollection::loadAndUpdateData()
{
    for ( RimStimPlanModelTemplate* f : m_stimPlanModelTemplates() )
    {
        f->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplateCollection::initAfterRead()
{
    // Assign template id if not already assigned
    for ( auto& templ : m_stimPlanModelTemplates )
    {
        if ( templ->id() < 0 ) templ->setId( nextFractureTemplateId() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanModelTemplateCollection::nextFractureTemplateId()
{
    int newId     = m_nextValidId;
    m_nextValidId = m_nextValidId + 1;

    return newId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplateCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                         std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimProject* proj = RimProject::current();
    if ( proj )
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }

    std::vector<Rim3dView*> views = proj->allVisibleViews();
    for ( Rim3dView* visibleView : views )
    {
        if ( dynamic_cast<RimEclipseView*>( visibleView ) )
        {
            visibleView->updateConnectedEditors();
        }
    }
}
