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

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Visible", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name", "", "", "" );
    m_name.registerGetMethod( this, &RimSurfaceInView::name );

    CAF_PDM_InitFieldNoDefault( &m_surface, "SurfaceRef", "Surface", "", "", "" );
    m_surface.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView::~RimSurfaceInView() {}

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
bool RimSurfaceInView::isActive()
{
    return m_isActive();
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
caf::PdmFieldHandle* RimSurfaceInView::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceInView::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RivIntersectionHexGridInterface> RimSurfaceInView::createHexGridInterface()
{
    // RimIntersectionResultDefinition* resDef = activeSeparateResultDefinition();
    // if ( resDef && resDef->activeCase() )
    //{
    //    // Eclipse case
    //
    //    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( resDef->activeCase() );
    //    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    //    {
    //        return new RivEclipseIntersectionGrid( eclipseCase->eclipseCaseData()->mainGrid(),
    //                                              eclipseCase->eclipseCaseData()->activeCellInfo(
    //                                                  resDef->eclipseResultDefinition()->porosityModel() ),
    //                                              this->isInactiveCellsVisible() );
    //    }
    //
    //    // Geomech case
    //
    //    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>( resDef->activeCase() );
    //
    //    if ( geomCase && geomCase->geoMechData() && geomCase->geoMechData()->femParts() )
    //    {
    //        RigFemPart* femPart = geomCase->geoMechData()->femParts()->part( 0 );
    //        return new RivFemIntersectionGrid( femPart );
    //    }
    //}

    RimEclipseView* eclipseView;
    this->firstAncestorOrThisOfType( eclipseView );
    if ( eclipseView )
    {
        RigMainGrid* grid = eclipseView->mainGrid();
        return new RivEclipseIntersectionGrid( grid, eclipseView->currentActiveCellInfo(), true );
    }

    RimGeoMechView* geoView;
    this->firstAncestorOrThisOfType( geoView );
    if ( geoView && geoView->femParts() && geoView->femParts()->partCount() )
    {
        RigFemPart* femPart = geoView->femParts()->part( 0 );

        return new RivFemIntersectionGrid( femPart );
    }

    return nullptr;
}
