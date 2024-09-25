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

#include "Rim3dView.h"
#include "RimPolygon.h"
#include "RimPolygonFile.h"
#include "RimProject.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimPolygonCollection, "RimPolygonCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCollection::RimPolygonCollection()
{
    CAF_PDM_InitObject( "Polygons", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_polygons, "Polygons", "Polygons" );
    CAF_PDM_InitFieldNoDefault( &m_polygonFiles, "PolygonFiles", "Polygon Files" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::loadData()
{
    for ( auto& p : m_polygonFiles() )
    {
        p->loadData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon* RimPolygonCollection::createUserDefinedPolygon()
{
    auto newPolygon = new RimPolygon();
    newPolygon->setName( "Polygon " + QString::number( userDefinedPolygons().size() + 1 ) );

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
    m_polygons().push_back( polygon );

    connectPolygonSignals( polygon );

    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::deleteUserDefinedPolygons()
{
    m_polygons().deleteChildren();

    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::addPolygonFile( RimPolygonFile* polygonFile )
{
    m_polygonFiles().push_back( polygonFile );

    connectPolygonFileSignals( polygonFile );

    updateViewTreeItems();
    scheduleRedrawViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCollection::userDefinedPolygons() const
{
    return m_polygons.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonFile*> RimPolygonCollection::polygonFiles() const
{
    return m_polygonFiles.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCollection::allPolygons() const
{
    std::vector<RimPolygon*> allPolygons;

    for ( auto& p : m_polygonFiles() )
    {
        for ( auto& polygon : p->polygons() )
        {
            allPolygons.push_back( polygon );
        }
    }

    for ( auto& polygon : m_polygons.childrenByType() )
    {
        allPolygons.push_back( polygon );
    }

    return allPolygons;
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
void RimPolygonCollection::initAfterRead()
{
    for ( auto& p : m_polygons() )
    {
        connectPolygonSignals( p );
    }

    for ( auto& pf : m_polygonFiles() )
    {
        connectPolygonFileSignals( pf );
    }
}
