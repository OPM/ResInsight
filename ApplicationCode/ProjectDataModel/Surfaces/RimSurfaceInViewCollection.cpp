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

#include "RimSurfaceInViewCollection.h"

#include "RiaApplication.h"
#include "RimGridView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"

#include "RivSurfacePartMgr.h"

#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimSurfaceInViewCollection, "SurfaceInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection::RimSurfaceInViewCollection()
{
    CAF_PDM_InitObject( "Surfaces", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitField( &m_isActive, "isActive", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_surfacesInView, "SurfacesInViewField", "SurfacesInViewField", "", "", "" );
    m_surfacesInView.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection::~RimSurfaceInViewCollection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateFromSurfaceCollection()
{
    // Delete surfaceInView without any real Surface connection

    std::vector<RimSurfaceInView*> surfsInView = m_surfacesInView.childObjects();

    for ( auto surf : surfsInView )
    {
        if ( !surf->surface() )
        {
            m_surfacesInView.removeChildObject( surf );
            delete surf;
        }
    }

    // Create new entries

    RimProject*           proj     = RiaApplication::instance()->project();
    RimSurfaceCollection* surfColl = proj->activeOilField()->surfaceCollection();

    if ( surfColl )
    {
        std::vector<RimSurface*> surfs = surfColl->surfaces();

        for ( auto surf : surfs )
        {
            if ( !this->hasSurfaceInViewForSurface( surf ) )
            {
                RimSurfaceInView* newSurfInView = new RimSurfaceInView();
                newSurfInView->setSurface( surf );
                m_surfacesInView.push_back( newSurfInView );
            }
        }
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::appendPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( !m_isActive() ) return;

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->surfacePartMgr()->appendNativeGeometryPartsToModel( model, scaleTransform );
        }
    }

    model->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    if ( changedField == &m_isActive )
    {
        RimGridView* ownerView;
        this->firstAncestorOrThisOfTypeAsserted( ownerView );
        ownerView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurfaceInViewCollection::hasSurfaceInViewForSurface( const RimSurface* surf ) const
{
    for ( auto surfInView : m_surfacesInView )
    {
        if ( surfInView->surface() == surf )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceInViewCollection::objectToggleField()
{
    return &m_isActive;
}
