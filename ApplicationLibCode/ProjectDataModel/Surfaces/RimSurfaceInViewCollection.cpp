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

#include "Rim3dView.h"
#include "RimEnsembleSurface.h"
#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceResultDefinition.h"

#include "RivSurfacePartMgr.h"

#include "cafPdmUiTreeOrdering.h"
#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimSurfaceInViewCollection, "SurfaceInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection::RimSurfaceInViewCollection()
{
    CAF_PDM_InitObject( "Surfaces", ":/ReservoirSurfaces16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_collectionName, "CollectionName", "Name" );
    m_collectionName.registerGetMethod( this, &RimSurfaceInViewCollection::name );
    m_collectionName.uiCapability()->setUiReadOnly( true );
    m_collectionName.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_collectionsInView, "SurfacesInViewFieldCollections", "SurfacesInViewFieldCollections" );

    CAF_PDM_InitFieldNoDefault( &m_surfacesInView, "SurfacesInViewField", "Surfaces" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceCollectionRef", "SurfaceCollection" );
    m_surfaceCollection.uiCapability()->setUiHidden( true );

    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection::~RimSurfaceInViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceInViewCollection::userDescriptionField()
{
    return &m_collectionName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    auto gridView     = firstAncestorOfType<RimGridView>();
    auto surfViewColl = firstAncestorOfType<RimSurfaceInViewCollection>();

    if ( gridView && !surfViewColl )
    {
        auto uiTree = gridView->separateSurfaceResultsCollection()->uiTreeOrdering();

        uiTreeOrdering.appendChild( uiTree );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceInViewCollection::name() const
{
    if ( m_surfaceCollection ) return m_surfaceCollection->collectionName();

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection* RimSurfaceInViewCollection::surfaceCollection() const
{
    return m_surfaceCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::setSurfaceCollection( RimSurfaceCollection* surfcoll )
{
    m_surfaceCollection = surfcoll;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateAllViewItems()
{
    syncCollectionsWithView();
    syncSurfacesWithView();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::syncCollectionsWithView()
{
    // check that we have surface in view collections for all sub-collections
    std::vector<RimSurfaceInViewCollection*> colls = m_collectionsInView.childrenByType();

    for ( auto surfcoll : colls )
    {
        if ( !surfcoll->surfaceCollection() )
        {
            m_collectionsInView.removeChild( surfcoll );
            delete surfcoll;
        }
    }

    // Create new collection entries and reorder
    std::vector<RimSurfaceInViewCollection*> orderedColls;
    if ( m_surfaceCollection )
    {
        // pick up the collections and the order from the surface collection
        std::vector<RimSurfaceCollection*> surfcolls = m_surfaceCollection->subCollections();
        for ( auto surfcoll : surfcolls )
        {
            // check if this is a collection we need to create

            RimSurfaceInViewCollection* viewSurfColl = getCollectionInViewForCollection( surfcoll );
            if ( viewSurfColl == nullptr )
            {
                RimSurfaceInViewCollection* newColl = new RimSurfaceInViewCollection();
                newColl->setSurfaceCollection( surfcoll );
                orderedColls.push_back( newColl );
            }
            else
            {
                orderedColls.push_back( viewSurfColl );
            }
        }

        // make sure our view surfaces have the same order as the source surface collection
        m_collectionsInView.clearWithoutDelete();
        for ( auto viewColl : orderedColls )
        {
            m_collectionsInView.push_back( viewColl );
            viewColl->updateAllViewItems();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::syncSurfacesWithView()
{
    // Delete surfaceInView without any real Surface connection

    std::vector<RimSurfaceInView*> surfsInView = m_surfacesInView.childrenByType();

    for ( auto surf : surfsInView )
    {
        if ( !surf->surface() )
        {
            m_surfacesInView.removeChild( surf );
            delete surf;
        }
    }

    // Create new surface entries and reorder
    std::vector<RimSurfaceInView*> orderedSurfs;

    if ( m_surfaceCollection )
    {
        // pick up the surfaces and the order from the surface collection
        std::vector<RimSurface*> surfs = m_surfaceCollection->surfaces();
        for ( auto surf : surfs )
        {
            // check if this is a surface we need to create
            RimSurfaceInView* viewSurf = getSurfaceInViewForSurface( surf );
            if ( viewSurf == nullptr )
            {
                RimSurfaceInView* newSurfInView = new RimSurfaceInView();
                newSurfInView->setSurface( surf );

                if ( m_surfaceCollection->collectionName() == RimEnsembleSurface::ensembleSourceFileCollectionName() )
                {
                    newSurfInView->setActive( false );
                }

                orderedSurfs.push_back( newSurfInView );
            }
            else
            {
                orderedSurfs.push_back( viewSurf );
            }
        }

        // make sure our view surfaces have the same order as the source surface collection
        m_surfacesInView.clearWithoutDelete();
        for ( auto viewSurf : orderedSurfs )
        {
            m_surfacesInView.push_back( viewSurf );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateFromSurfaceCollection()
{
    updateAllViewItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::loadData( int timeStep )
{
    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        coll->loadData( timeStep );
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->loadDataAndUpdate( timeStep );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::clearGeometry()
{
    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        coll->clearGeometry();
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        surf->clearGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::appendPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform, bool onlyNativeParts )
{
    if ( !isChecked() ) return;

    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() )
        {
            coll->appendPartsToModel( model, scaleTransform, onlyNativeParts );
        }
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            if ( onlyNativeParts )
            {
                surf->nativeSurfacePartMgr()->appendNativeGeometryPartsToModel( model, scaleTransform );
            }
            else
            {
                surf->surfacePartMgr()->appendIntersectionGeometryPartsToModel( model, scaleTransform );
            }
        }
    }

    model->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateUiIconFromToggleField();

    if ( changedField == &m_isChecked )
    {
        auto ownerView = firstAncestorOrThisOfTypeAsserted<Rim3dView>();
        ownerView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::initAfterRead()
{
    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView* RimSurfaceInViewCollection::getSurfaceInViewForSurface( const RimSurface* surf ) const
{
    for ( auto surfInView : m_surfacesInView )
    {
        if ( surfInView->surface() == surf )
        {
            return surfInView;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection* RimSurfaceInViewCollection::getCollectionInViewForCollection( const RimSurfaceCollection* coll ) const
{
    for ( auto collInView : m_collectionsInView )
    {
        if ( collInView->surfaceCollection() == coll )
        {
            return collInView;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateCellResultColor( bool hasGeneralCellResult, int timeStepIndex )
{
    if ( !isChecked() ) return;

    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() )
        {
            coll->updateCellResultColor( hasGeneralCellResult, timeStepIndex );
        }
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            bool useNativeSurfaceColors = false;

            if ( surf->isNativeSurfaceResultsActive() ) useNativeSurfaceColors = true;

            if ( !useNativeSurfaceColors )
            {
                bool showResults = surf->activeSeparateResultDefinition() ? surf->activeSeparateResultDefinition()->hasResult()
                                                                          : hasGeneralCellResult;

                if ( showResults )
                {
                    surf->surfacePartMgr()->updateCellResultColor( timeStepIndex );
                }
                else
                {
                    useNativeSurfaceColors = true;
                }
            }

            if ( useNativeSurfaceColors )
            {
                surf->surfacePartMgr()->updateNativeSurfaceColors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::applySingleColorEffect()
{
    if ( !isChecked() ) return;

    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() )
        {
            coll->applySingleColorEffect();
        }
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->surfacePartMgr()->updateNativeSurfaceColors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurfaceInViewCollection::hasAnyActiveSeparateResults()
{
    if ( !isChecked() ) return false;

    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() )
        {
            bool found = coll->hasAnyActiveSeparateResults();
            if ( found ) return true;
        }
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() && surf->activeSeparateResultDefinition() && surf->activeSeparateResultDefinition()->hasResult() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer )
{
    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() ) coll->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer, isUsingOverrideViewer );
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        surf->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer, isUsingOverrideViewer );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRegularLegendConfig*> RimSurfaceInViewCollection::legendConfigs()
{
    std::vector<RimRegularLegendConfig*> configs;

    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() )
        {
            std::vector<RimRegularLegendConfig*> collconfigs = coll->legendConfigs();
            configs.insert( configs.end(), collconfigs.begin(), collconfigs.end() );
        }
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() && surf->surfaceResultDefinition() )
        {
            configs.push_back( surf->surfaceResultDefinition()->legendConfig() );
        }
    }

    return configs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RivIntersectionGeometryGeneratorInterface*> RimSurfaceInViewCollection::intersectionGeometryGenerators() const
{
    std::vector<const RivIntersectionGeometryGeneratorInterface*> generators;

    for ( auto surf : m_surfacesInView )
    {
        if ( surf->isActive() && surf->isNativeSurfaceResultsActive() )
        {
            auto generator = surf->intersectionGeometryGenerator();

            if ( generator ) generators.push_back( generator );
        }
    }

    for ( auto child : m_collectionsInView )
    {
        auto childGenerators = child->intersectionGeometryGenerators();
        generators.insert( generators.end(), childGenerators.begin(), childGenerators.end() );
    }

    return generators;
}
