/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimGeoMechPartCollection.h"

#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechPart.h"

#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimGeoMechPartCollection, "GeoMechPartCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPartCollection::RimGeoMechPartCollection()
    : m_case( nullptr )
    , m_currentDisplacementTimeStep( -1 )
    , m_diplacementsUsed( false )
{
    CAF_PDM_InitScriptableObject( "Parts", ":/GeoMechCase24x24.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_parts, "Parts", "Parts", "", "", "" );
    m_parts.uiCapability()->setUiTreeHidden( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPartCollection::~RimGeoMechPartCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPartCollection::syncWithCase( RimGeoMechCase* geoCase )
{
    m_case = geoCase;

    if ( geoCase && geoCase->geoMechData() && geoCase->geoMechData()->femParts() )
    {
        const int count = geoCase->geoMechData()->femParts()->partCount();

        if ( count != (int)m_parts.size() )
        {
            m_parts.clear();

            for ( int i = 0; i < count; i++ )
            {
                const auto& femPart = geoCase->geoMechData()->femParts()->part( i );

                RimGeoMechPart* part = new RimGeoMechPart();
                part->setPartId( i );
                part->setName( QString( femPart->name().c_str() ) );
                part->setCheckState( femPart->enabled() );
                m_parts.push_back( part );
            }
        }
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechPart*> RimGeoMechPartCollection::parts() const
{
    return m_parts.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPartCollection::shouldBeVisibleInTree() const
{
    return m_parts.size() > 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPartCollection::isPartEnabled( int partId ) const
{
    for ( const auto& part : m_parts )
    {
        if ( part->partId() == partId ) return part->isChecked();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPartCollection::setCurrentDisplacementTimeStep( int timeStep )
{
    m_currentDisplacementTimeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGeoMechPartCollection::currentDisplacementTimeStep() const
{
    return m_currentDisplacementTimeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPartCollection::setDisplacementsForPart( int partId, std::vector<cvf::Vec3f> displacements )
{
    for ( const auto& part : m_parts )
    {
        if ( part->partId() == partId )
        {
            part->setDisplacements( displacements );
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3f> RimGeoMechPartCollection::displacements( int partId ) const
{
    for ( const auto& part : m_parts )
    {
        if ( part->partId() == partId )
        {
            return part->displacements();
        }
    }
    return std::vector<cvf::Vec3f>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPartCollection::setDisplacementsUsed( bool isUsed )
{
    m_diplacementsUsed = isUsed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPartCollection::isDisplacementsUsed() const
{
    return m_diplacementsUsed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPartCollection::shouldRebuildPartVisualization( int currentTimeStep, bool showDisplacement )
{
    return ( ( m_currentDisplacementTimeStep != currentTimeStep ) || ( m_diplacementsUsed != showDisplacement ) );
}
