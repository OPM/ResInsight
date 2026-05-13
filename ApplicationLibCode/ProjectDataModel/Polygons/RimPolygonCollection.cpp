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

#include "RimPolygonCollection.h"

#include "RiaColorTables.h"

#include "Rim3dView.h"
#include "RimPolygon.h"
#include "RimPolygonFile.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimPolygonCollection, "PolygonCollection", "RimPolygonCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCollection::RimPolygonCollection()
{
    CAF_PDM_InitScriptableObject( "Polygons", ":/Folder.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_collectionName, "PolygonCollectionName", "Name" );
    m_collectionName = "Polygons";

    CAF_PDM_InitScriptableFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_items, "Polygons", "Polygons" );

    CAF_PDM_InitFieldNoDefault( &m_polygonFiles_OBSOLETE, "PolygonFiles", "Polygon Files" );
    m_polygonFiles_OBSOLETE.uiCapability()->setUiHidden( true );
    m_polygonFiles_OBSOLETE.xmlCapability()->setIOWritable( false );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCollection* RimPolygonCollection::createTopmost()
{
    auto* coll = new RimPolygonCollection();
    coll->setAsTopmostFolder();
    coll->uiCapability()->setUiIconFromResourceString( ":/PolylinesFromFile16x16.png" );
    return coll;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon* RimPolygonCollection::createUserDefinedPolygon()
{
    auto newPolygon = new RimPolygon();
    newPolygon->setName( "Polygon " + QString::number( allPolygons().size() + 1 ) );

    auto colorCandidates = RiaColorTables::summaryCurveDefaultPaletteColors();
    newPolygon->setColor( colorCandidates.cycledColor3f( allPolygons().size() ) );

    return newPolygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon* RimPolygonCollection::appendUserDefinedPolygon()
{
    auto newPolygon = createUserDefinedPolygon();
    addUserDefinedPolygon( newPolygon );

    return newPolygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::addUserDefinedPolygon( RimPolygon* polygon )
{
    m_items.push_back( polygon );

    connectPolygonSignals( polygon );

    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::deleteUserDefinedPolygons()
{
    m_items.deleteChildren();

    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::deleteAllPolygons()
{
    m_items.deleteChildren();
    m_subCollections.deleteChildren();

    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCollection::allPolygons() const
{
    return allItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::addPolygonFile( RimPolygonFile* polygonFile )
{
    if ( !polygonFile ) return;

    addSubCollection( polygonFile );
    connectPolygonFileSignals( polygonFile );

    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::appendPolygonMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder )
{
    menuBuilder << "RicCreatePolygonFeature";
    menuBuilder << "RicImportPolygonFileFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    RimPolygonCollection::appendPolygonMenuItems( menuBuilder );
    menuBuilder.addSeparator();
    menuBuilder << "RicNewNestedCollectionFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicDeleteAllPolygonsFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_collectionName );
    uiOrdering.add( &m_subCollections );
    uiOrdering.add( &m_items );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::updateViewTreeItems()
{
    RimProject* proj = RimProject::current();

    // Make sure the tree items are synchronized
    for ( auto view : proj->allViews() )
    {
        view->updateViewTreeItems( RiaDefines::ItemIn3dView::POLYGON );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::scheduleRedrawViews()
{
    RimProject* proj = RimProject::current();
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::connectPolygonSignals( RimPolygon* polygon )
{
    if ( polygon )
    {
        polygon->objectChanged.connect( this, &RimPolygonCollection::onPolygonChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::connectPolygonFileSignals( RimPolygonFile* polygonFile )
{
    if ( polygonFile )
    {
        polygonFile->objectChanged.connect( this, &RimPolygonCollection::onPolygonFileChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::onPolygonChanged( const caf::SignalEmitter* emitter )
{
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::onPolygonFileChanged( const caf::SignalEmitter* emitter )
{
    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::connectSignalsRecursively()
{
    connectSignalsForContainer( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::connectSignalsForContainer( RimPolygonContainer* container )
{
    if ( !container ) return;

    for ( auto* polygon : container->items() )
    {
        connectPolygonSignals( polygon );
    }

    for ( auto* sub : container->subCollections() )
    {
        if ( !sub ) continue;
        if ( auto* file = dynamic_cast<RimPolygonFile*>( sub ) )
        {
            connectPolygonFileSignals( file );
        }
        connectSignalsForContainer( sub );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::initAfterRead()
{
    // Migrate legacy m_polygonFiles into m_subCollections so files become polymorphic
    // children alongside folders. Old projects had files in a separate field; new
    // projects store everything in m_subCollections.
    if ( !m_polygonFiles_OBSOLETE.empty() )
    {
        std::vector<RimPolygonFile*> legacy = m_polygonFiles_OBSOLETE.childrenByType();
        m_polygonFiles_OBSOLETE.clearWithoutDelete();
        for ( auto* file : legacy )
        {
            m_subCollections.push_back( file );
        }
    }

    connectSignalsRecursively();
}
