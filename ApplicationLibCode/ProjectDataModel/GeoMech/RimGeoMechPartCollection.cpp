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
    , m_displacementsUsed( false )
    , m_currentScaleFactor( 1.0 )
    , m_noDisplacements()
{
    CAF_PDM_InitScriptableObject( "Parts", ":/GeoMechCase24x24.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_parts, "Parts", "Parts" );
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

        m_parts.deleteChildren();

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
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechPart*> RimGeoMechPartCollection::parts() const
{
    return m_parts.children();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPart* RimGeoMechPartCollection::part( int partId ) const
{
    for ( const auto& part : m_parts )
    {
        if ( part->partId() == partId ) return part;
    }
    return nullptr;
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
    RimGeoMechPart* thepart = part( partId );
    if ( thepart ) return thepart->isChecked();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPartCollection::setCurrentDisplacementSettings( int currentTimeStep, bool showDisplacement, double scaleFactor )
{
    m_currentDisplacementTimeStep = currentTimeStep;
    m_displacementsUsed           = showDisplacement;
    m_currentScaleFactor          = scaleFactor;
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
double RimGeoMechPartCollection::currentDisplacementScaleFactor() const
{
    return m_currentScaleFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3f>& RimGeoMechPartCollection::displacements( int partId ) const
{
    RimGeoMechPart* thepart = part( partId );
    if ( thepart ) return thepart->displacements();

    return m_noDisplacements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPartCollection::isDisplacementsUsed() const
{
    return m_displacementsUsed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, bool>
    RimGeoMechPartCollection::needsReloadOrRebuildUpdate( int currentTimeStep, bool showDisplacement, double scaleFactor )
{
    bool rebuild = m_displacementsUsed != showDisplacement || ( m_currentDisplacementTimeStep != currentTimeStep ) ||
                   ( std::abs( m_currentScaleFactor - scaleFactor ) > 0.0001 );

    bool reload = false;

    if ( showDisplacement )
    {
        bool missingDisplacement = false;
        for ( const auto& part : m_parts )
        {
            missingDisplacement = missingDisplacement || ( part->displacements().size() == 0 );
        }

        rebuild = rebuild || missingDisplacement;
        reload  = ( m_currentDisplacementTimeStep != currentTimeStep ) || missingDisplacement;
    }

    return std::make_pair( reload, rebuild );
}
