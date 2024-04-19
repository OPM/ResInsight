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

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimCase.h"
#include "RimEnsembleSurface.h"
#include "RimFileSurface.h"
#include "RimGridCaseSurface.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicSectionCollection.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceResultDefinition.h"

#include "cafPdmFieldReorderCapability.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QFile>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimSurfaceCollection, "SurfaceCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection::RimSurfaceCollection()
{
    CAF_PDM_InitScriptableObject( "Surfaces", ":/ReservoirSurfaces16x16.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_collectionName, "SurfaceUserDecription", "Name" );
    m_collectionName = "Surfaces";

    CAF_PDM_InitScriptableFieldNoDefault( &m_subCollections, "SubCollections", "Surfaces" );
    auto reorderability = caf::PdmFieldReorderCapability::addToField( &m_subCollections );
    reorderability->orderChanged.connect( this, &RimSurfaceCollection::orderChanged );

    CAF_PDM_InitScriptableFieldNoDefault( &m_surfaces, "SurfacesField", "Surfaces" );

    setDeletable( true );
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
void RimSurfaceCollection::setAsTopmostFolder()
{
    m_collectionName.uiCapability()->setUiHidden( true );
    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceCollection::collectionName() const
{
    return m_collectionName.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::setCollectionName( const QString name )
{
    return m_collectionName.setValue( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceCollection::userDescriptionField()
{
    return &m_collectionName;
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
void RimSurfaceCollection::addEnsembleSurface( RimEnsembleSurface* ensembleSurface )
{
    addSubCollection( ensembleSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleSurface*> RimSurfaceCollection::ensembleSurfaces() const
{
    return descendantsIncludingThisOfType<RimEnsembleSurface>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceCollection::importSurfacesFromFiles( const QStringList& fileNames, bool showLegend /* = true */ )
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
        newSurface->setUserDescription( QFileInfo( newFileName ).fileName() );

        newSurface->setColor( newColor );

        if ( !newSurface->onLoadData() )
        {
            delete newSurface;
            errorMessages += newFileName + "\n";
        }
        else
        {
            addSurface( newSurface );
            surfacesToLoad.push_back( newSurface );
            ++newSurfCount;
        }
    }

    if ( !errorMessages.isEmpty() )
    {
        RiaLogging::warning( "Import Surfaces : Could not import the following files:\n" + errorMessages );
    }

    updateConnectedEditors();

    updateViews( surfacesToLoad, showLegend );

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
void RimSurfaceCollection::reloadSurfaces( std::vector<RimSurface*> surfaces, bool showLegend /*=true*/ )
{
    // ask the surfaces given to reload its data
    for ( RimSurface* surface : surfaces )
    {
        surface->reloadData();
    }

    updateConnectedEditors();

    updateViews( surfaces, showLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceCollection::copySurfaces( std::vector<RimSurface*> surfaces )
{
    std::vector<RimSurface*> newsurfaces;

    // create a copy of each surface given
    for ( RimSurface* surface : surfaces )
    {
        RimSurface* copy = surface->createCopy();
        if ( copy )
        {
            newsurfaces.push_back( copy );
        }
        else
        {
            RiaLogging::warning( "Create Surface Copy: Could not create a copy of the surface " + surface->fullName() );
        }
    }

    RimSurface* retsurf = nullptr;
    for ( RimSurface* surface : newsurfaces )
    {
        m_surfaces.push_back( surface );
        retsurf = surface;
    }

    updateConnectedEditors();

    return retsurf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceCollection::addGridCaseSurface( RimCase* sourceCase, int oneBasedSliceIndex )
{
    auto s = new RimGridCaseSurface;
    s->setCase( sourceCase );

    s->setOneBasedIndex( oneBasedSliceIndex );

    if ( !s->onLoadData() )
    {
        RiaLogging::warning( "Add Grid Case Surface : Could not create the grid case surface." );
        return nullptr;
    }

    m_surfaces.push_back( s );

    updateConnectedEditors();

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
    return m_surfaces.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurfaceCollection*> RimSurfaceCollection::subCollections() const
{
    return m_subCollections.childrenByType();
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

    for ( auto subColl : m_subCollections )
    {
        subColl->loadData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::updateViews( const std::vector<RimSurface*>& surfsToReload, bool showLegend /* = true */ )
{
    RimProject* proj = RimProject::current();

    // Make sure the tree items are synchronized
    for ( auto view : proj->allViews() )
    {
        view->updateViewTreeItems( RiaDefines::ItemIn3dView::SURFACE );

        if ( auto gridView = dynamic_cast<RimGridView*>( view ) )
        {
            auto seismicCollection = gridView->seismicSectionCollection();
            seismicCollection->setSurfacesVisible( surfsToReload );
        }
    }

    std::set<Rim3dView*> viewsNeedingUpdate;

    for ( auto surf : surfsToReload )
    {
        std::vector<RimSurfaceInView*> surfsInView = surf->objectsWithReferringPtrFieldsOfType<RimSurfaceInView>();
        for ( auto surfInView : surfsInView )
        {
            surfInView->clearGeometry();
            surfInView->surfaceResultDefinition()->legendConfig()->setShowLegend( showLegend );

            auto gridView = surfInView->firstAncestorOrThisOfType<Rim3dView>();
            if ( gridView ) viewsNeedingUpdate.insert( gridView );
        }
    }

    // Update the views:
    for ( auto view : viewsNeedingUpdate )
    {
        view->scheduleCreateDisplayModelAndRedraw();

        if ( view->ownerCase() )
        {
            auto views = view->ownerCase()->intersectionViewCollection()->views();
            for ( Rim2dIntersectionView* view : views )
            {
                view->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::updateViews()
{
    RimProject*             proj  = RimProject::current();
    std::vector<Rim3dView*> views = proj->allViews();

    // Make sure the tree items are synchronized

    for ( auto view : views )
    {
        view->updateViewTreeItems( RiaDefines::ItemIn3dView::SURFACE );
    }

    for ( auto view : views )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::orderChanged( const caf::SignalEmitter* emitter )
{
    updateViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::removeSurface( RimSurface* surface )
{
    m_surfaces.removeChild( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::removeMissingFileSurfaces()
{
    // get the existing surfaces
    std::list<RimFileSurface*> missingSurfaces;

    // check if a filesurface references a file no longer present
    for ( auto& surface : surfaces() )
    {
        RimFileSurface* fileSurface = dynamic_cast<RimFileSurface*>( surface );
        if ( fileSurface == nullptr ) continue;

        QString filename = fileSurface->surfaceFilePath();
        if ( !QFile::exists( filename ) )
        {
            missingSurfaces.push_back( fileSurface );
        }
    }

    // remove all surfaces with a missing input file
    for ( auto& surface : missingSurfaces )
    {
        removeSurface( surface );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceCollection::addSurfacesAtIndex( int position, std::vector<RimSurface*> surfaces )
{
    // adjust index for number of folders we have
    position = position - static_cast<int>( m_subCollections.size() );

    RimSurface* returnSurface = nullptr;
    if ( !surfaces.empty() ) returnSurface = surfaces[0];

    // insert position at end?
    if ( ( position >= static_cast<int>( m_surfaces.size() ) ) || ( position < 0 ) )
    {
        for ( auto surf : surfaces )
        {
            m_surfaces.push_back( surf );
        }
    }
    else
    {
        // build the new surface order
        std::vector<RimSurface*> orderedSurfs;

        int i = 0;

        while ( i < position )
        {
            orderedSurfs.push_back( m_surfaces[i++] );
        }

        for ( auto surf : surfaces )
        {
            orderedSurfs.push_back( surf );
        }

        int surfcount = static_cast<int>( m_surfaces.size() );
        while ( i < surfcount )
        {
            orderedSurfs.push_back( m_surfaces[i++] );
        }

        // reset the surface collection and use the new order
        m_surfaces.clearWithoutDelete();
        for ( auto surf : orderedSurfs )
        {
            m_surfaces.push_back( surf );
        }
    }

    // make sure the views are in sync with the collection order
    updateViews();

    return returnSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::addSubCollection( RimSurfaceCollection* subcoll )
{
    m_subCollections.push_back( subcoll );
    updateConnectedEditors();

    updateViews();

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection* RimSurfaceCollection::getSubCollection( const QString& name ) const
{
    for ( auto coll : m_subCollections )
    {
        if ( coll->collectionName() == name ) return coll;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::deleteSubCollection( const QString& name )
{
    auto coll = getSubCollection( name );
    if ( coll )
    {
        auto index = m_subCollections.indexOf( coll );
        m_subCollections.erase( index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurfaceCollection::containsSurface()
{
    bool containsSurface = ( !surfaces().empty() );

    for ( auto coll : m_subCollections )
    {
        containsSurface |= coll->containsSurface();
    }

    return containsSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurfaceCollection::containsFileSurface( QString filename )
{
    for ( auto& surface : surfaces() )
    {
        RimFileSurface* fileSurface = dynamic_cast<RimFileSurface*>( surface );
        if ( fileSurface == nullptr ) continue;
        if ( fileSurface->surfaceFilePath() == filename ) return true;
    }

    return false;
}
