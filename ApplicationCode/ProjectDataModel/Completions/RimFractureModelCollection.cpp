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

#include "RimFractureModelCollection.h"

#include "RimFractureModel.h"
#include "RimProject.h"

#include "cafPdmObject.h"

CAF_PDM_SOURCE_INIT( RimFractureModelCollection, "FractureModelCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCollection::RimFractureModelCollection( void )
{
    CAF_PDM_InitObject( "Fracture Models", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModels, "FractureModels", "Fracture Models", "", "", "" );
    m_fractureModels.uiCapability()->setUiHidden( true );

    setName( "Fracture Models" );
    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCollection::~RimFractureModelCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCollection::addFractureModel( RimFractureModel* fracture )
{
    m_fractureModels.push_back( fracture );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCollection::deleteFractureModels()
{
    m_fractureModels.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureModel*> RimFractureModelCollection::allFractureModels() const
{
    return m_fractureModels.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureModel*> RimFractureModelCollection::activeFractureModels() const
{
    std::vector<RimFractureModel*> active;

    if ( isChecked() )
    {
        for ( const auto& f : allFractureModels() )
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
void RimFractureModelCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
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
