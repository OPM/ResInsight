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

#include "RimSurfaceCollection.h"

#include "RiaColorTables.h"
#include "RiaLogging.h"

#include "RimFileSurface.h"
#include "RimGridCaseSurface.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"

CAF_PDM_SOURCE_INIT( RimSurfaceCollection, "SurfaceCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection::RimSurfaceCollection()
{
    CAF_PDM_InitObject( "Surfaces", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_surfaces, "SurfacesField", "Surfaces", "", "", "" );
    m_surfaces.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection::~RimSurfaceCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::addSurface( RimSurface* surface )
{
    m_surfaces.push_back( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceCollection::importSurfacesFromFiles( const QStringList& fileNames )
{
    size_t  newSurfCount      = 0;
    size_t  existingSurfCount = m_surfaces().size();
    QString errorMessages;

    std::vector<RimSurface*> surfacesToLoad;

    for ( const QString& newFileName : fileNames )
    {
        RimFileSurface* newSurface = new RimFileSurface;

        auto newColor = RiaColorTables::categoryPaletteColors().cycledColor3f( existingSurfCount + newSurfCount );

        newSurface->setSurfaceFilePath( newFileName );
        newSurface->setColor( newColor );

        if ( !newSurface->onLoadData() )
        {
            delete newSurface;
            errorMessages += newFileName + "\n";
        }
        else
        {
            this->addSurface( newSurface );
            surfacesToLoad.push_back( newSurface );
            ++newSurfCount;
        }
    }

    if ( !errorMessages.isEmpty() )
    {
        RiaLogging::warning( "Import Surfaces : Could not import the following files:\n" + errorMessages );
    }

    this->updateConnectedEditors();

    updateViews( surfacesToLoad );

    if ( newSurfCount > 0 && !m_surfaces.empty() )
    {
        return m_surfaces[m_surfaces.size() - 1];
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::reloadSurfaces( std::vector<RimSurface*> surfaces )
{
    // ask the surfaces given to reload its data
    for ( RimSurface* surface : surfaces )
    {
        surface->reloadData();
    }

    this->updateConnectedEditors();

    updateViews( surfaces );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceCollection::addGridCaseSurface( RimCase* sourceCase )
{
    auto s = new RimGridCaseSurface;
    s->setCase( sourceCase );

    int  oneBasedSliceIndex = 1;
    auto sliceType          = RiaDefines::GridCaseAxis::AXIS_K;

    s->setSliceTypeAndOneBasedIndex( sliceType, oneBasedSliceIndex );
    s->setUserDescription( "Surface" );

    if ( !s->onLoadData() )
    {
        RiaLogging::warning( "Add Grid Case Surface : Could not create the grid case surface. Don't know why." );
        return nullptr;
    }

    m_surfaces.push_back( s );

    this->updateConnectedEditors();

    std::vector<RimSurface*> surfacesToRefresh;
    surfacesToRefresh.push_back( s );
    updateViews( surfacesToRefresh );

    return s;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurface*> RimSurfaceCollection::surfaces() const
{
    return m_surfaces.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::loadData()
{
    for ( auto surf : m_surfaces )
    {
        surf->loadDataIfRequired();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::updateViews( const std::vector<RimSurface*>& surfsToReload )
{
    RimProject* proj = RimProject::current();

    std::vector<Rim3dView*> views;
    proj->allViews( views );

    // Make sure the tree items are synchronized

    for ( auto view : views )
    {
        auto gridView = dynamic_cast<RimGridView*>( view );
        if ( gridView ) gridView->updateSurfacesInViewTreeItems();
    }

    std::set<RimGridView*> viewsNeedingUpdate;

    for ( auto surf : surfsToReload )
    {
        std::vector<RimSurfaceInView*> surfsInView;
        surf->objectsWithReferringPtrFieldsOfType( surfsInView );
        for ( auto surfInView : surfsInView )
        {
            surfInView->clearGeometry();

            RimGridView* gridView;
            surfInView->firstAncestorOrThisOfType( gridView );

            if ( gridView ) viewsNeedingUpdate.insert( gridView );
        }
    }

    // Update the views:
    for ( auto view : viewsNeedingUpdate )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::updateViews()
{
    RimProject*               proj = RimProject::current();
    std::vector<RimGridView*> views;
    proj->allVisibleGridViews( views );

    // Make sure the tree items are synchronized

    for ( auto view : views )
    {
        view->updateSurfacesInViewTreeItems();
    }

    for ( auto view : views )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                           std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateViews();
}
