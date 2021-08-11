/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimcModeledWellPath.h"

#include "RimModeledWellPath.h"
#include "RimTools.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "WellPathCommands/RicNewWellPathLateralAtDepthFeature.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimModeledWellPath, RimcModeledWellPath_appendLateral, "AppendLateral" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcModeledWellPath_appendLateral::RimcModeledWellPath_appendLateral( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Append Well Path Lateral", "", "", "Append Well Path Lateral" );
    CAF_PDM_InitScriptableField( &m_tieInDepth, "TieInDepth", 0.0, "", "", "", "Measured Depth on the Parent Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_lateralName, "LateralName", "", "", "", "Lateral Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcModeledWellPath_appendLateral::execute()
{
    auto parentWellPath = self<RimModeledWellPath>();

    auto lateral = RicNewWellPathLateralAtDepthFeature::createLateralAtMeasuredDepth( parentWellPath, m_tieInDepth );
    if ( !m_lateralName().isEmpty() )
    {
        lateral->setName( m_lateralName );
    }
    lateral->geometryDefinition()->enableTargetPointPicking( false );

    return lateral;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcModeledWellPath_appendLateral::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcModeledWellPath_appendLateral::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimModeledWellPath );
}
