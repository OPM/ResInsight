/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimStreamline.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimStreamline, "Streamline" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamline::RimStreamline( QString simWellName )
{
    CAF_PDM_InitScriptableObject( "Streamline", ":/Erase.png", "", "" );

    CAF_PDM_InitScriptableField( &m_simWellName, "Name", simWellName, "Name" );
    m_simWellName.uiCapability()->setUiReadOnly( true );
    m_simWellName.uiCapability()->setUiHidden( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamline::~RimStreamline()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimStreamline::userDescriptionField()
{
    return &m_simWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigTracer& RimStreamline::tracer() const
{
    return m_tracer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimStreamline::simWellName() const
{
    return m_simWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamline::addTracerPoint( cvf::Vec3d position, cvf::Vec3d direction, RiaDefines::PhaseType dominantPhase )
{
    m_tracer.appendPoint( position, direction, dominantPhase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamline::reverse()
{
    m_tracer.reverse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimStreamline::size() const
{
    return m_tracer.size();
}
