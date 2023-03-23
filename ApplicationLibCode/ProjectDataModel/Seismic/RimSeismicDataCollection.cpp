/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimSeismicDataCollection.h"

#include "RiaLogging.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimSeismicData.h"

#include <QFile>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimSeismicDataCollection, "SeismicDataCollection", "SeismicCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataCollection::RimSeismicDataCollection()
{
    CAF_PDM_InitObject( "Seismic", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );
    m_seismicData.uiCapability()->setUiTreeHidden( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataCollection::~RimSeismicDataCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicData* RimSeismicDataCollection::importSeismicFromFile( const QString fileName )
{
    RimSeismicData* seisData = new RimSeismicData();
    seisData->setFileName( fileName );

    QFileInfo fi( fileName );
    seisData->setUserDescription( fi.baseName() );
    m_seismicData.push_back( seisData );
    updateConnectedEditors();

    return seisData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSeismicData*> RimSeismicDataCollection::seismicData() const
{
    return m_seismicData.children();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicDataCollection::isEmpty()
{
    return !m_seismicData.hasChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDataCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDataCollection::updateViews()
{
    RimProject*               proj = RimProject::current();
    std::vector<RimGridView*> views;
    proj->allVisibleGridViews( views );

    for ( auto view : views )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}
