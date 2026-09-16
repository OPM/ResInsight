/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimcGridView.h"

#include "RiaApplication.h"
#include "RiaKeyValueStoreUtil.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInViewCollection.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "Surfaces/RimSurface.h"
#include "Surfaces/RimSurfaceInViewCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmFieldScriptingCapability.h"

#include "cvfArray.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseView, RimcGridView_visibleCellsInternal, "visible_cells_internal" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( Rim3dView, RimcGridView_setPolygonVisible, "set_polygon_visible" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( Rim3dView, RimcGridView_setSurfaceVisible, "set_surface_visible" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( Rim3dView, RimcGridView_setSurfaceProperty, "set_surface_property" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcGridView_visibleCellsInternal::RimcGridView_visibleCellsInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Visible Cells Internal", "", "", "Visible Cells Internal" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_visibilityKey, "VisibilityKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_timeStep, "TimeStep", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcGridView_visibleCellsInternal::execute()
{
    auto gridView = self<RimGridView>();
    if ( !gridView )
    {
        return std::unexpected( QString( "Grid view is null." ) );
    }

    if ( m_visibilityKey().isEmpty() )
    {
        return std::unexpected( QString( "Visibility key is empty." ) );
    }

    // Get the cell visibility array
    cvf::ref<cvf::UByteArray> visibleCells = gridView->currentTotalCellVisibility();

    if ( visibleCells.isNull() )
    {
        return std::unexpected( QString( "Failed to get cell visibility data." ) );
    }

    // Convert cvf::UByteArray to std::vector<float> where 1.0 = visible, 0.0 = invisible
    std::vector<float> visibilityValues;
    visibilityValues.reserve( visibleCells->size() );
    for ( size_t i = 0; i < visibleCells->size(); ++i )
    {
        visibilityValues.push_back( ( *visibleCells )[i] ? 1.0f : 0.0f );
    }

    // Store the visibility data in the key-value store
    auto keyValueStore = RiaApplication::instance()->keyValueStore();
    keyValueStore->set( m_visibilityKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( visibilityValues ) );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcGridView_setPolygonVisible::RimcGridView_setPolygonVisible( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Polygon Visible", "", "", "Set polygon visibility in this view" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_polygon, "Polygon", "Polygon" );
    CAF_PDM_InitScriptableField( &m_visible, "Visible", true, "Visible" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcGridView_setPolygonVisible::execute()
{
    auto* gridView = self<RimGridView>();
    if ( !gridView )
    {
        return std::unexpected( QString( "Polygon visibility is only supported for grid views." ) );
    }

    if ( !m_polygon() )
    {
        return std::unexpected( QString( "Polygon is null." ) );
    }

    if ( !gridView->polygonInViewCollection()->setPolygonVisible( m_polygon(), m_visible() ) )
    {
        return std::unexpected( QString( "Polygon '%1' is not available in this view." ).arg( m_polygon()->name() ) );
    }

    gridView->scheduleCreateDisplayModelAndRedraw();
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcGridView_setSurfaceVisible::RimcGridView_setSurfaceVisible( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Surface Visible", "", "", "Set surface visibility in this view" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_surface, "Surface", "Surface" );
    CAF_PDM_InitScriptableField( &m_visible, "Visible", true, "Visible" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcGridView_setSurfaceVisible::execute()
{
    auto* gridView = self<RimGridView>();
    if ( !gridView )
    {
        return std::unexpected( QString( "Surface visibility is only supported for grid views." ) );
    }

    if ( !m_surface() )
    {
        return std::unexpected( QString( "Surface is null." ) );
    }

    auto* collection = gridView->surfaceInViewCollection();
    if ( !collection || !collection->setSurfaceVisible( m_surface(), m_visible() ) )
    {
        return std::unexpected( QString( "Surface '%1' is not available in this view." ).arg( m_surface()->fullName() ) );
    }

    gridView->scheduleCreateDisplayModelAndRedraw();
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcGridView_setSurfaceProperty::RimcGridView_setSurfaceProperty( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Surface Property", "", "", "Set the surface property shown in this view" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_surface, "Surface", "Surface" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyName, "PropertyName", "Property Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcGridView_setSurfaceProperty::execute()
{
    auto* gridView = self<RimGridView>();
    if ( !gridView )
    {
        return std::unexpected( QString( "Surface properties are only supported for grid views." ) );
    }

    if ( !m_surface() )
    {
        return std::unexpected( QString( "Surface is null." ) );
    }

    auto* collection = gridView->surfaceInViewCollection();
    if ( !collection )
    {
        return std::unexpected( QString( "Surface '%1' is not available in this view." ).arg( m_surface()->fullName() ) );
    }

    auto result = collection->setSurfaceProperty( m_surface(), m_propertyName() );
    if ( !result ) return std::unexpected( result.error() );

    gridView->scheduleCreateDisplayModelAndRedraw();
    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( Rim3dView, Rim3dView_clone, "clone" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView_clone::Rim3dView_clone( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Clone View", "", "", "Clone the view and add the copy to the same case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> Rim3dView_clone::execute()
{
    auto* view = self<Rim3dView>();
    if ( !view ) return std::unexpected( "No view is available." );

    Rim3dView* newView = nullptr;
    if ( auto* eclipseView = dynamic_cast<RimEclipseView*>( view ) )
    {
        RimEclipseCase* eclipseCase = eclipseView->eclipseCase();
        if ( !eclipseCase ) return std::unexpected( "The view has no case." );

        RimEclipseView* newEclipseView = eclipseCase->createCopyAndAddView( eclipseView );
        newEclipseView->loadDataAndUpdate();
        eclipseCase->updateConnectedEditors();
        newView = newEclipseView;
    }
    else if ( auto* geoMechView = dynamic_cast<RimGeoMechView*>( view ) )
    {
        RimGeoMechCase* geoMechCase = geoMechView->geoMechCase();
        if ( !geoMechCase ) return std::unexpected( "The view has no case." );

        RimGeoMechView* newGeoMechView = geoMechCase->createCopyAndAddView( geoMechView );
        newGeoMechView->loadDataAndUpdate();
        geoMechCase->updateConnectedEditors();
        newView = newGeoMechView;
    }

    if ( !newView ) return std::unexpected( QString( "Could not clone view '%1'" ).arg( view->name() ) );

    Riu3DMainWindowTools::setExpanded( newView );
    return newView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dView_clone::classKeywordReturnedType() const
{
    return Rim3dView::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( Rim3dView, Rim3dView_setTimeStep, "setTimeStep" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView_setTimeStep::Rim3dView_setTimeStep( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Time Step", "", "", "Set the current time step of the view" );

    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "Zero-based time step index" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView_setTimeStep::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> Rim3dView_setTimeStep::execute()
{
    auto* view = self<Rim3dView>();
    if ( !view ) return std::unexpected( "No view is available." );

    RimCase* rimCase = view->ownerCase();
    if ( !rimCase ) return std::unexpected( "The view has no case." );

    const int maxTimeStep = static_cast<int>( rimCase->timeStepStrings().size() ) - 1;
    if ( m_timeStep() < 0 || m_timeStep() > maxTimeStep )
    {
        return std::unexpected( QString( "Time step %1 is out of range [0, %2] for case '%3'" )
                                    .arg( m_timeStep() )
                                    .arg( maxTimeStep )
                                    .arg( rimCase->caseUserDescription() ) );
    }

    view->setCurrentTimeStepAndUpdate( m_timeStep() );
    view->createDisplayModelAndRedraw();

    return nullptr;
}
