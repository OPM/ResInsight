////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimSurfaceIntersectionCurve.h"

#include "RimAnnotationLineAppearance.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimSurfaceIntersectionCurve, "RimSurfaceIntersectionCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionCurve::RimSurfaceIntersectionCurve()
{
    CAF_PDM_InitObject( "SurfaceIntersectionCurve", ":/do_not_exist.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_lineAppearance, "LineAppearance", "Line Appearance", "", "", "" );
    m_lineAppearance = new RimAnnotationLineAppearance;

    CAF_PDM_InitFieldNoDefault( &m_surface1, "Surface1", "Surface 1", "", "", "" );
    m_surface1.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceIntersectionCurve::surface() const
{
    return m_surface1();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance* RimSurfaceIntersectionCurve::lineAppearance() const
{
    return m_lineAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSurfaceIntersectionCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_surface1 )
    {
        RimSurfaceCollection* surfColl = RimProject::current()->activeOilField()->surfaceCollection();

        appendOptionItemsForSources( 0, surfColl, options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCurve::appendOptionItemsForSources( int                            currentLevel,
                                                               RimSurfaceCollection*          currentCollection,
                                                               QList<caf::PdmOptionItemInfo>& options )
{
    caf::IconProvider surfaceIcon( ":/ReservoirSurface16x16.png" );

    options.push_back( caf::PdmOptionItemInfo::createHeader( currentCollection->collectionName(), true ) );

    for ( auto surf : currentCollection->surfaces() )
    {
        auto itemInfo = caf::PdmOptionItemInfo( surf->userDescription(), surf, false, surfaceIcon );
        itemInfo.setLevel( currentLevel + 1 );
        options.push_back( itemInfo );
    }

    for ( auto subColl : currentCollection->subCollections() )
    {
        appendOptionItemsForSources( currentLevel, subColl, options );
    }
}
