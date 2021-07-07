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

#include "RimEnsembleSurfaceInView.h"
#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceResultDefinition.h"

#include "RivSurfacePartMgr.h"

#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimSurfaceInViewCollection, "SurfaceInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection::RimSurfaceInViewCollection()
{
    CAF_PDM_InitObject( "Surfaces", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_collectionName, "CollectionName", "Name", "", "", "" );
    m_collectionName.registerGetMethod( this, &RimSurfaceInViewCollection::name );
    m_collectionName.uiCapability()->setUiReadOnly( true );
    m_collectionName.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_collectionsInView,
                                "SurfacesInViewFieldCollections",
                                "SurfacesInViewFieldCollections",
                                "",
                                "",
                                "" );
    m_collectionsInView.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_surfacesInView, "SurfacesInViewField", "SurfacesInViewField", "", "", "" );
    m_surfacesInView.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensembleSurfacesInView, "EnsemblesSurfacesInView", "EnsembleSurfacesInView", "", "", "" );
    m_ensembleSurfacesInView.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceCollectionRef", "SurfaceCollection", "", "", "" );
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
    syncEnsembleSurfacesWithView();
    syncSurfacesWithView();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::syncCollectionsWithView()
{
    // check that we have surface in view collections for all sub-collections
    std::vector<RimSurfaceInViewCollection*> colls = m_collectionsInView.childObjects();

    for ( auto surfcoll : colls )
    {
        if ( !surfcoll->surfaceCollection() )
        {
            m_collectionsInView.removeChildObject( surfcoll );
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

            RimSurfaceInViewCollection* viewSurfColl = this->getCollectionInViewForCollection( surfcoll );
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
        m_collectionsInView.clear();
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
void RimSurfaceInViewCollection::syncEnsembleSurfacesWithView()
{
    // check that we have ensemble in view collectinons
    std::vector<RimEnsembleSurfaceInView*> ensembleSurfaces = m_ensembleSurfacesInView.childObjects();

    for ( auto ensembleSurface : ensembleSurfaces )
    {
        if ( !ensembleSurface->ensembleSurface() )
        {
            m_ensembleSurfacesInView.removeChildObject( ensembleSurface );
            delete ensembleSurface;
        }
    }

    // Create new collection entries and reorder
    std::vector<RimEnsembleSurfaceInView*> orderedEnsembleSurfaces;
    if ( m_surfaceCollection )
    {
        // pick up the collections and the order from the surface collection
        std::vector<RimEnsembleSurface*> ensSurfs = m_surfaceCollection->ensembleSurfaces();
        for ( auto ensSurf : ensSurfs )
        {
            // check if this is a collection we need to create

            RimEnsembleSurfaceInView* ensembleSurfaceInView = this->getEnsembleSurfaceInViewForEnsembleSurface( ensSurf );
            if ( ensembleSurfaceInView == nullptr )
            {
                RimEnsembleSurfaceInView* newColl = new RimEnsembleSurfaceInView();
                newColl->setEnsembleSurface( ensSurf );
                orderedEnsembleSurfaces.push_back( newColl );
            }
            else
            {
                orderedEnsembleSurfaces.push_back( ensembleSurfaceInView );
            }
        }

        // make sure our view surfaces have the same order as the source surface collection
        m_ensembleSurfacesInView.clear();
        for ( auto ensSurf : orderedEnsembleSurfaces )
        {
            m_ensembleSurfacesInView.push_back( ensSurf );
            ensSurf->updateAllViewItems();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::syncSurfacesWithView()
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

    // Create new surfade entries and reorder
    std::vector<RimSurfaceInView*> orderedSurfs;

    if ( m_surfaceCollection )
    {
        // pick up the surfaces and the order from the surface collection
        std::vector<RimSurface*> surfs = m_surfaceCollection->surfaces();
        for ( auto surf : surfs )
        {
            // check if this is a surface we need to create
            RimSurfaceInView* viewSurf = this->getSurfaceInViewForSurface( surf );
            if ( viewSurf == nullptr )
            {
                RimSurfaceInView* newSurfInView = new RimSurfaceInView();
                newSurfInView->setSurface( surf );
                orderedSurfs.push_back( newSurfInView );
            }
            else
            {
                orderedSurfs.push_back( viewSurf );
            }
        }

        // make sure our view surfaces have the same order as the source surface collection
        m_surfacesInView.clear();
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
void RimSurfaceInViewCollection::loadData()
{
    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        coll->loadData();
    }

    for ( RimEnsembleSurfaceInView* ensSurf : m_ensembleSurfacesInView )
    {
        ensSurf->loadData();
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->loadDataAndUpdate();
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

    for ( RimEnsembleSurfaceInView* ensSurf : m_ensembleSurfacesInView )
    {
        ensSurf->clearGeometry();
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        surf->clearGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::appendPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( !isChecked() ) return;

    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() )
        {
            coll->appendPartsToModel( model, scaleTransform );
        }
    }

    for ( RimEnsembleSurfaceInView* ensSurf : m_ensembleSurfacesInView )
    {
        if ( ensSurf->isChecked() )
        {
            ensSurf->appendPartsToModel( model, scaleTransform );
        }
    }

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->surfacePartMgr()->appendIntersectionGeometryPartsToModel( model, scaleTransform );
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
    this->updateUiIconFromToggleField();

    if ( changedField == &m_isChecked )
    {
        RimGridView* ownerView;
        this->firstAncestorOrThisOfTypeAsserted( ownerView );
        ownerView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::initAfterRead()
{
    this->updateUiIconFromToggleField();
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
RimSurfaceInViewCollection*
    RimSurfaceInViewCollection::getCollectionInViewForCollection( const RimSurfaceCollection* coll ) const
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
RimEnsembleSurfaceInView*
    RimSurfaceInViewCollection::getEnsembleSurfaceInViewForEnsembleSurface( const RimEnsembleSurface* coll ) const
{
    for ( auto collInView : m_ensembleSurfacesInView )
    {
        if ( collInView->ensembleSurface() == coll )
        {
            return collInView;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex )
{
    if ( !this->isChecked() ) return;

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
                bool showResults = surf->activeSeparateResultDefinition()
                                       ? surf->activeSeparateResultDefinition()->hasResult()
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
    if ( !this->isChecked() ) return;

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
    if ( !this->isChecked() ) return false;

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
        if ( surf->isActive() && surf->activeSeparateResultDefinition() &&
             surf->activeSeparateResultDefinition()->hasResult() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
                                                                      bool       isUsingOverrideViewer )
{
    for ( RimSurfaceInViewCollection* coll : m_collectionsInView )
    {
        if ( coll->isChecked() )
            coll->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer, isUsingOverrideViewer );
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
std::vector<const RivIntersectionGeometryGeneratorIF*> RimSurfaceInViewCollection::intersectionGeometryGenerators() const
{
    std::vector<const RivIntersectionGeometryGeneratorIF*> generators;

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
