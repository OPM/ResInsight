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

#include "RimSeismicCollection.h"

#include "RiaLogging.h"

#include "RimSeismicData.h"

#include <QFile>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimSeismicCollection, "SeismicCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicCollection::RimSeismicCollection()
{
    CAF_PDM_InitObject( "Seismic", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );
    m_seismicData.uiCapability()->setUiTreeHidden( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicCollection::~RimSeismicCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicCollection::addSeismicData( RimSeismicData* data )
{
    m_seismicData.push_back( data );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicData* RimSeismicCollection::importSeismicFromFile( const QString fileName )
{
    RimSeismicData* seisData = new RimSeismicData();
    seisData->setFileName( fileName );

    QFileInfo fi( fileName );

    seisData->setUserDescription( fi.baseName() );

    addSeismicData( seisData );

    this->updateConnectedEditors();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSeismicData*> RimSeismicCollection::seismicData() const
{
    return m_seismicData.children();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicCollection::isEmpty()
{
    return !m_seismicData.hasChildren();
}
