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

#include "RimSurfaceInView.h"

#include "RimGridView.h"
#include "RimSurface.h"

#include "RigFemPartCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RivHexGridIntersectionTools.h"
#include "RivSurfacePartMgr.h"

CAF_PDM_SOURCE_INIT( RimSurfaceInView, "SurfaceInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView::RimSurfaceInView()
{
    CAF_PDM_InitObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name", "", "", "" );
    m_name.registerGetMethod( this, &RimSurfaceInView::name );
    m_name.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_surface, "SurfaceRef", "Surface", "", "", "" );
    m_surface.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView::~RimSurfaceInView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceInView::name() const
{
    if ( m_surface ) return m_surface->userDescription();

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceInView::surface() const
{
    return m_surface();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::setSurface( RimSurface* surf )
{
    m_surface = surf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::clearGeometry()
{
    m_surfacePartMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSurfacePartMgr* RimSurfaceInView::surfacePartMgr()
{
    if ( m_surfacePartMgr.isNull() ) m_surfacePartMgr = new RivSurfacePartMgr( this );

    return m_surfacePartMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    if ( changedField == &m_isActive || changedField == &m_useSeparateDataSource || changedField == &m_separateDataSource )
    {
        RimGridView* ownerView;
        this->firstAncestorOrThisOfTypeAsserted( ownerView );
        ownerView->scheduleCreateDisplayModelAndRedraw();
    }
    else if ( changedField == &m_showInactiveCells )
    {
        m_surfacePartMgr = nullptr;
        RimGridView* ownerView;
        this->firstAncestorOrThisOfTypeAsserted( ownerView );
        ownerView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );

    uiOrdering.add( &m_showInactiveCells );

    this->defineSeparateDataSourceUi( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection* RimSurfaceInView::findSeparateResultsCollection()
{
    RimGridView* view;
    this->firstAncestorOrThisOfTypeAsserted( view );
    return view->separateSurfaceResultsCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceInView::userDescriptionField()
{
    return &m_name;
}
