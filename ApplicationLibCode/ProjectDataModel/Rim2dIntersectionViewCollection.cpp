/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "Rim2dIntersectionViewCollection.h"

#include "Rim2dIntersectionView.h"
#include "RimCase.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimReservoirGridEnsemble.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( Rim2dIntersectionViewCollection, "Intersection2dViewCollection" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// Views displaying a case are not necessarily children of the case. Views can be located in the project level
/// view collection or in the view collection of a grid ensemble. Collect all grid views displaying the case.
//--------------------------------------------------------------------------------------------------
std::vector<RimGridView*> gridViewsForCase( RimCase* rimCase )
{
    std::vector<RimGridView*> gridViews = rimCase->gridViews();

    // Views in a view collection are not children of the case, and are not reported by gridViews(). These views refer to
    // the case using a ptr field, and can be found by inspecting the objects referring to the case.
    for ( auto view : rimCase->objectsWithReferringPtrFieldsOfType<RimGridView>() )
    {
        if ( !view || view->ownerCase() != rimCase ) continue;
        if ( std::find( gridViews.begin(), gridViews.end(), view ) != gridViews.end() ) continue;

        gridViews.push_back( view );
    }

    return gridViews;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionViewCollection::Rim2dIntersectionViewCollection()
{
    CAF_PDM_InitObject( "2D Intersection Views", ":/CrossSection16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_intersectionViews, "IntersectionViews", "Intersection Views", ":/CrossSection16x16.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionViewCollection::~Rim2dIntersectionViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim2dIntersectionView*> Rim2dIntersectionViewCollection::views()
{
    return m_intersectionViews.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionViewCollection::syncFromExistingIntersections( bool doUpdate )
{
    auto parentCase = firstAncestorOrThisOfTypeAsserted<RimCase>();

    std::vector<RimExtrudedCurveIntersection*> allOrderedIntersectionsInCase;
    for ( auto gridView : gridViewsForCase( parentCase ) )
    {
        if ( !gridView || !gridView->intersectionCollection() ) continue;

        for ( auto intersection : gridView->intersectionCollection()->intersections() )
        {
            if ( intersection ) allOrderedIntersectionsInCase.push_back( intersection );
        }
    }

    // Delete views without a valid intersection

    for ( Rim2dIntersectionView* iv : m_intersectionViews )
    {
        if ( iv && !iv->intersection() )
        {
            delete iv;
        }
    }

    // Clean up the container by removing nullptr's

    m_intersectionViews.removeChild( nullptr );

    // Build map from intersection to view

    std::map<RimExtrudedCurveIntersection*, Rim2dIntersectionView*> intersectionToViewMap;
    for ( Rim2dIntersectionView* iv : m_intersectionViews )
    {
        CVF_ASSERT( iv && iv->intersection() );
        intersectionToViewMap[iv->intersection()] = iv;
    }

    m_intersectionViews.clearWithoutDelete(); // Not deleting the views. The are managed by the map

    // Insert the old views in correct order, and create new views as we go

    for ( RimExtrudedCurveIntersection* intersection : allOrderedIntersectionsInCase )
    {
        auto view = intersection->firstAncestorOrThisOfType<RimGridView>();

        auto it = intersectionToViewMap.find( intersection );
        if ( it == intersectionToViewMap.end() )
        {
            Rim2dIntersectionView* newView = new Rim2dIntersectionView();

            if ( view )
            {
                newView->setCurrentTimeStep( view->currentTimeStep() );
                view->cellVisibilityChanged.connect( newView, &Rim2dIntersectionView::onCellVisibilityChanged );
            }

            newView->setIntersection( intersection );

            m_intersectionViews.push_back( newView );
        }
        else
        {
            if ( view )
            {
                view->cellVisibilityChanged.disconnect( it->second );
                view->cellVisibilityChanged.connect( it->second, &Rim2dIntersectionView::onCellVisibilityChanged );
            }
            m_intersectionViews.push_back( it->second );
        }
    }

    if ( doUpdate ) updateConnectedEditors();

    parentCase->updateConnectedEditors();

    // For a case in a grid ensemble, this collection is displayed as a child of the ensemble
    if ( auto ensemble = parentCase->firstAncestorOfType<RimReservoirGridEnsemble>() )
    {
        ensemble->updateConnectedEditors();
    }
}
