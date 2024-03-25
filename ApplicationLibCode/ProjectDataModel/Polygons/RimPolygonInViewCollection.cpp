/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygonInViewCollection.h"

#include "Rim3dView.h"
#include "RimPolygon.h"
#include "RimPolygonCollection.h"
#include "RimPolygonFile.h"
#include "RimPolygonInView.h"
#include "RimTools.h"

CAF_PDM_SOURCE_INIT( RimPolygonInViewCollection, "RimPolygonInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection::RimPolygonInViewCollection()
{
    CAF_PDM_InitObject( "Polygons", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_polygonsInView, "Polygons", "Polygons" );
    CAF_PDM_InitFieldNoDefault( &m_collectionsInView, "Collections", "Collections" );

    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::updateFromPolygonCollection()
{
    updateAllViewItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonInView*> RimPolygonInViewCollection::visiblePolygonsInView() const
{
    if ( !m_isChecked ) return {};

    std::vector<RimPolygonInView*> polys = m_polygonsInView.childrenByType();

    for ( auto coll : m_collectionsInView )
    {
        if ( !coll->isChecked() ) continue;

        auto other = coll->visiblePolygonsInView();
        polys.insert( polys.end(), other.begin(), other.end() );
    }

    return polys;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonInView*> RimPolygonInViewCollection::allPolygonsInView() const
{
    std::vector<RimPolygonInView*> polys = m_polygonsInView.childrenByType();

    for ( auto coll : m_collectionsInView )
    {
        auto other = coll->visiblePolygonsInView();
        polys.insert( polys.end(), other.begin(), other.end() );
    }

    return polys;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::setPolygonFile( RimPolygonFile* polygonFile )
{
    m_polygonFile = polygonFile;

    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFile* RimPolygonInViewCollection::polygonFile() const
{
    return m_polygonFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimCheckableNamedObject::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_isChecked )
    {
        for ( auto poly : visiblePolygonsInView() )
        {
            poly->updateConnectedEditors();
        }

        if ( auto view = firstAncestorOfType<Rim3dView>() )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    RimPolygonCollection::appendPolygonMenuItems( menuBuilder );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::updateAllViewItems()
{
    // Based on the same concept as RimSurfaceInViewCollection

    syncCollectionsWithView();
    syncPolygonsWithView();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::syncCollectionsWithView()
{
    // Based on the same concept as RimSurfaceInViewCollection

    auto colls = m_collectionsInView.childrenByType();

    for ( auto coll : colls )
    {
        if ( !coll->polygonFile() )
        {
            m_collectionsInView.removeChild( coll );
            delete coll;
        }
    }

    if ( !m_polygonFile )
    {
        std::vector<RimPolygonInViewCollection*> orderedColls;

        if ( auto polygonCollection = RimTools::polygonCollection() )
        {
            std::vector<RimPolygonInView*> newPolygonsInView;

            for ( auto polygonFile : polygonCollection->polygonFiles() )
            {
                if ( polygonFile->polygons().empty() ) continue;

                auto viewPolygonFile = getCollectionInViewForPolygonFile( polygonFile );
                if ( viewPolygonFile == nullptr )
                {
                    auto newColl = new RimPolygonInViewCollection();
                    newColl->setPolygonFile( polygonFile );
                    orderedColls.push_back( newColl );
                }
                else
                {
                    viewPolygonFile->updateName();
                    orderedColls.push_back( viewPolygonFile );
                }
            }
        }

        m_collectionsInView.clearWithoutDelete();
        for ( auto viewColl : orderedColls )
        {
            m_collectionsInView.push_back( viewColl );
            viewColl->updateAllViewItems();
        }
    }

    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::syncPolygonsWithView()
{
    std::vector<RimPolygonInView*> existingPolygonsInView = m_polygonsInView.childrenByType();
    m_polygonsInView.clearWithoutDelete();

    std::vector<RimPolygon*> polygons;

    if ( m_polygonFile )
    {
        polygons = m_polygonFile->polygons();
    }
    else
    {
        auto polygonCollection = RimTools::polygonCollection();
        polygons               = polygonCollection->userDefinedPolygons();
    }

    std::vector<RimPolygonInView*> newPolygonsInView;

    for ( auto polygon : polygons )
    {
        auto it = std::find_if( existingPolygonsInView.begin(),
                                existingPolygonsInView.end(),
                                [polygon]( auto* polygonInView ) { return polygonInView->polygon() == polygon; } );

        if ( it != existingPolygonsInView.end() )
        {
            newPolygonsInView.push_back( *it );
            ( *it )->updateTargetsFromPolygon();
            existingPolygonsInView.erase( it );
        }
        else
        {
            auto polygonInView = new RimPolygonInView();
            polygonInView->setPolygon( polygon );
            newPolygonsInView.push_back( polygonInView );
        }
    }

    m_polygonsInView.setValue( newPolygonsInView );

    for ( auto polyInView : existingPolygonsInView )
    {
        delete polyInView;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::updateName()
{
    QString name = "Polygons";

    if ( m_polygonFile )
    {
        name = m_polygonFile->name();
    }

    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection* RimPolygonInViewCollection::getCollectionInViewForPolygonFile( const RimPolygonFile* polygonFile ) const
{
    for ( auto collInView : m_collectionsInView )
    {
        if ( collInView->polygonFile() == polygonFile )
        {
            return collInView;
        }
    }

    return nullptr;
}
