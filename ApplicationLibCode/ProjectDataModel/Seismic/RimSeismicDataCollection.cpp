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
#include "RimSeismicDifferenceData.h"
#include "RimSeismicSectionCollection.h"

#include <QFile>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimSeismicDataCollection, "SeismicDataCollection", "SeismicCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataCollection::RimSeismicDataCollection()
{
    CAF_PDM_InitObject( "Data", ":/SeismicData24x24.png" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );

    CAF_PDM_InitFieldNoDefault( &m_differenceData, "DifferenceData", "Seismic Difference Data" );

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
RimSeismicDataInterface* RimSeismicDataCollection::importSeismicFromFile( const QString fileName )
{
    RimSeismicData* seisData = new RimSeismicData();
    seisData->setFileName( fileName );

    QFileInfo fi( fileName );
    seisData->setUserDescription( fi.baseName() );
    m_seismicData.push_back( seisData );
    updateAllRequiredEditors();

    if ( m_seismicData.size() == 1 ) updateTreeForAllViews();

    return seisData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSeismicData*> RimSeismicDataCollection::seismicData() const
{
    return m_seismicData.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSeismicDifferenceData*> RimSeismicDataCollection::differenceData() const
{
    return m_differenceData.childrenByType();
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
    if ( ( m_seismicData.size() + m_differenceData.size() ) == 0 ) updateTreeForAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDataCollection::updateViews()
{
    RimProject*               proj  = RimProject::current();
    std::vector<RimGridView*> views = proj->allVisibleGridViews();
    for ( auto view : views )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDataCollection::updateTreeForAllViews()
{
    RimProject* proj = RimProject::current();
    if ( proj != nullptr )
    {
        std::vector<RimGridView*> views = proj->allVisibleGridViews();
        for ( auto view : views )
        {
            view->updateAllRequiredEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataInterface* RimSeismicDataCollection::createDifferenceSeismicData( RimSeismicData* data1, RimSeismicData* data2 )
{
    if ( ( data1 == nullptr ) || ( data2 == nullptr ) ) return nullptr;

    if ( !data1->gridIsEqual( data2 ) ) return nullptr;

    RimSeismicDifferenceData* retdata = new RimSeismicDifferenceData();
    retdata->setInputData( data1, data2 );
    retdata->setUserDescription( "Difference" );

    m_differenceData.push_back( retdata );

    updateAllRequiredEditors();
    if ( m_differenceData.size() == 1 ) updateTreeForAllViews();

    return retdata;
}
