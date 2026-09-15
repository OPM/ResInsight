/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimWellPathInViewCollection.h"

#include "Rim3dView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

CAF_PDM_SOURCE_INIT( RimWellPathInViewCollection, "RimWellPathInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathInViewCollection::RimWellPathInViewCollection()
{
    CAF_PDM_InitObject( "Well Paths", ":/WellCollection.png" );

    CAF_PDM_InitFieldNoDefault( &m_itemsInView, "WellPaths", "Well Paths" );
    CAF_PDM_InitFieldNoDefault( &m_collectionsInView, "Collections", "Collections" );
    CAF_PDM_InitFieldNoDefault( &m_sourceCollection, "SourceCollection", "Source Collection" );
    m_sourceCollection.uiCapability()->setUiHidden( true );

    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathInViewCollection::updateFromWellPathCollection()
{
    if ( !sourceCollection() )
    {
        setSourceCollection( RimWellPathCollection::instance() );
    }
    updateFromSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathInView*> RimWellPathInViewCollection::visibleWellPathsInView() const
{
    if ( !m_isChecked ) return {};

    std::vector<RimWellPathInView*> wellPathsInView;
    for ( auto wellPathInView : m_itemsInView.childrenByType() )
    {
        if ( wellPathInView->isChecked() ) wellPathsInView.push_back( wellPathInView );
    }

    return wellPathsInView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathInView*> RimWellPathInViewCollection::allWellPathsInView() const
{
    return m_itemsInView.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathInViewCollection::isWellPathVisible( const RimWellPath* wellPath ) const
{
    if ( !m_isChecked ) return false;

    auto* wellPathInView = findWellPathInView( wellPath );

    // A well path not yet mirrored into the view (e.g. sync has not run) is not hidden by default.
    if ( !wellPathInView ) return true;

    return wellPathInView->isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathInViewCollection::setWellPathVisible( RimWellPath* wellPath, bool visible )
{
    updateFromWellPathCollection();

    auto* wellPathInView = findWellPathInView( wellPath );
    if ( !wellPathInView ) return false;

    wellPathInView->setCheckState( visible );
    wellPathInView->updateConnectedEditors();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathInView* RimWellPathInViewCollection::findWellPathInView( const RimWellPath* wellPath ) const
{
    for ( auto wellPathInView : m_itemsInView )
    {
        if ( wellPathInView && wellPathInView->wellPath() == wellPath ) return wellPathInView;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimCheckableNamedObject::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_isChecked )
    {
        for ( auto wellPathInView : allWellPathsInView() )
        {
            wellPathInView->updateConnectedEditors();
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
std::vector<RimWellPathCollection*> RimWellPathInViewCollection::sourceSubCollections() const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathInViewCollection::sourceItems() const
{
    if ( auto* src = sourceCollection() ) return src->allWellPaths();
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathInView* RimWellPathInViewCollection::createItemInView( RimWellPath* source )
{
    auto* viewItem = new RimWellPathInView();
    viewItem->setWellPath( source );
    return viewItem;
}
