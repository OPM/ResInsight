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

#include "RimStimPlanModelCollection.h"

#include "RimProject.h"
#include "RimStimPlanModel.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimStimPlanModelCollection, "StimPlanModelCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCollection::RimStimPlanModelCollection( void )
{
    CAF_PDM_InitScriptableObject( "StimPlan Models", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanModels, "StimPlanModels", "", "", "", "" );
    m_stimPlanModels.uiCapability()->setUiHidden( true );

    setName( "StimPlan Models" );
    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCollection::~RimStimPlanModelCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelCollection::hasStimPlanModels() const
{
    return !m_stimPlanModels.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCollection::addStimPlanModel( RimStimPlanModel* fracture )
{
    m_stimPlanModels.push_back( fracture );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCollection::deleteStimPlanModels()
{
    m_stimPlanModels.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanModel*> RimStimPlanModelCollection::allStimPlanModels() const
{
    return m_stimPlanModels.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanModel*> RimStimPlanModelCollection::activeStimPlanModels() const
{
    std::vector<RimStimPlanModel*> active;

    if ( isChecked() )
    {
        for ( const auto& f : allStimPlanModels() )
        {
            if ( f->isChecked() )
            {
                active.push_back( f );
            }
        }
    }

    return active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    if ( changedField == &m_isChecked )
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}
