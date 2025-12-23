/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RimcCornerPointCase.h"

#include "RiaApplication.h"
#include "RiaKeyValueStoreUtil.h"
#include "RiaViewRedrawScheduler.h"

#include "Rim3dView.h"
#include "RimCornerPointCase.h"
#include "RimEclipseView.h"

#include "cafPdmFieldScriptingCapability.h"

#include <expected>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCornerPointCase, RimcCornerPointCase_replaceCornerPointGridInternal, "replace_corner_point_grid_internal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcCornerPointCase_replaceCornerPointGridInternal::RimcCornerPointCase_replaceCornerPointGridInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Replace Corner Point Grid", "", "", "Replace Corner Point Grid" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_nx, "Nx", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_ny, "Ny", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_nz, "Nz", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordKey, "CoordKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_zcornKey, "ZcornKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_actnumKey, "ActnumKey", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcCornerPointCase_replaceCornerPointGridInternal::execute()
{
    auto cornerPointCase = self<RimCornerPointCase>();
    if ( !cornerPointCase )
    {
        return std::unexpected( "Unable to get corner point case." );
    }

    int nx = m_nx();
    int ny = m_ny();
    int nz = m_nz();
    if ( nx <= 0 || ny <= 0 || nz <= 0 )
    {
        return std::unexpected( "Invalid grid dimensions. nx, ny and nz must be positive." );
    }

    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    std::vector<float> coord  = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_coordKey().toStdString() ) );
    std::vector<float> zcorn  = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_zcornKey().toStdString() ) );
    std::vector<float> actnum = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_actnumKey().toStdString() ) );

    if ( coord.empty() || zcorn.empty() || actnum.empty() )
    {
        return std::unexpected( "Found unexpected empty coord, zcorn or actnum array." );
    }

    // Block view updates while the grid is being replaced.
    RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();
    RiaViewRedrawScheduler::instance()->blockUpdate( true );

    // Create a new corner point grid from the provided geometry
    auto result = RimCornerPointCase::replaceGridFromCoordinatesArray( *cornerPointCase, nx, ny, nz, coord, zcorn, actnum );
    if ( !result.has_value() )
    {
        RiaViewRedrawScheduler::instance()->blockUpdate( false );
        return std::unexpected( "Failed to replace grid" );
    }

    // Update all 3D views to reflect the new grid
    // We must completely reinitialize the views because the grid size has changed
    std::vector<Rim3dView*> views = cornerPointCase->views();
    for ( Rim3dView* view : views )
    {
        if ( RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view ) )
        {
            // Force complete regeneration by calling loadDataAndUpdate
            // This will reinitialize all geometry managers with the new grid size
            eclView->loadDataAndUpdate();
        }
    }

    RiaViewRedrawScheduler::instance()->blockUpdate( false );

    // Update connected editors
    cornerPointCase->updateConnectedEditors();

    // Clean up key-value store
    keyValueStore->remove( m_coordKey().toStdString() );
    keyValueStore->remove( m_zcornKey().toStdString() );
    keyValueStore->remove( m_actnumKey().toStdString() );

    return nullptr;
}
