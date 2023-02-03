/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimSeismicSectionCollection.h"

#include "RimSeismicSection.h"

CAF_PDM_SOURCE_INIT( RimSeismicSectionCollection, "SeismicSectionCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSectionCollection::RimSeismicSectionCollection()
{
    CAF_PDM_InitObject( "Seismic Sections", ":/Seismic16x16.png" );

    CAF_PDM_InitField( &m_userDescription, "UserDecription", QString( "Seismic Sections" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_seismicSections, "SeismicSections", "SeismicSections" );
    m_seismicSections.uiCapability()->setUiTreeHidden( true );

    setName( "Seismic Sections" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSectionCollection::~RimSeismicSectionCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSection* RimSeismicSectionCollection::addNewSection()
{
    RimSeismicSection* newSection = new RimSeismicSection();
    m_seismicSections.push_back( newSection );
    updateConnectedEditors();
    return newSection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicSectionCollection::userDescription()
{
    return m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSectionCollection::setUserDescription( QString description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSeismicSectionCollection::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicSectionCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}
