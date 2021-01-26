/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimWellPathFractureCollection.h"

#include "RimProject.h"
#include "RimWellPathFracture.h"

#include "cafPdmObject.h"

CAF_PDM_SOURCE_INIT( RimWellPathFractureCollection, "WellPathFractureCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection::RimWellPathFractureCollection( void )
{
    CAF_PDM_InitObject( "Fractures", ":/FractureLayout16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractures, "Fractures", "", "", "", "" );
    m_fractures.uiCapability()->setUiHidden( true );

    setName( "Fractures" );
    nameField()->uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_mswParameters_OBSOLETE, "MswParameters", "Multi Segment Well Parameters", "", "", "" );
    m_mswParameters_OBSOLETE = new RimMswCompletionParameters;
    m_mswParameters_OBSOLETE.uiCapability()->setUiTreeHidden( true );
    m_mswParameters_OBSOLETE.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_refMDType_OBSOLETE, "RefMDType", std::numeric_limits<int>::max(), "Reference MD", "", "", "" );
    CAF_PDM_InitField( &m_refMD_OBSOLETE, "RefMD", std::numeric_limits<double>::infinity(), "", "", "", "" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection::~RimWellPathFractureCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathFractureCollection::hasFractures() const
{
    return !m_fractures.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::addFracture( RimWellPathFracture* fracture )
{
    m_fractures.push_back( fracture );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::deleteFractures()
{
    m_fractures.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathFracture*> RimWellPathFractureCollection::allFractures() const
{
    return m_fractures.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathFracture*> RimWellPathFractureCollection::activeFractures() const
{
    std::vector<RimWellPathFracture*> active;

    if ( isChecked() )
    {
        for ( const auto& f : allFractures() )
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
void RimWellPathFractureCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    if ( changedField == &m_isChecked )
    {
        RimProject::current()->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::initAfterRead()
{
    if ( m_refMDType_OBSOLETE() != std::numeric_limits<int>::max() )
    {
        m_mswParameters_OBSOLETE->setReferenceMDType( (RimMswCompletionParameters::ReferenceMDType)m_refMDType_OBSOLETE() );
    }

    if ( m_refMD_OBSOLETE() != std::numeric_limits<double>::infinity() )
    {
        m_mswParameters_OBSOLETE->setManualReferenceMD( m_refMD_OBSOLETE() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                    std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
}
