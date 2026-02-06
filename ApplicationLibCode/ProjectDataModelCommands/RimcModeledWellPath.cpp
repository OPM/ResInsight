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
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "WellPathCommands/RicDuplicateWellPathFeature.h"
#include "WellPathCommands/RicNewWellPathLateralAtDepthFeature.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimModeledWellPath, RimcModeledWellPath_appendLateral, "AppendLateral" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcModeledWellPath_appendLateral::RimcModeledWellPath_appendLateral( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Append Well Path Lateral", "", "", "Append Well Path Lateral" );

    CAF_PDM_InitScriptableField( &m_tieInDepth, "TieInDepth", 0.0, "", "", "", "Measured Depth on the Parent Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_lateralName, "LateralName", "", "", "", "Lateral Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcModeledWellPath_appendLateral::execute()
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
QString RimcModeledWellPath_appendLateral::classKeywordReturnedType() const
{
    return RimModeledWellPath::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimModeledWellPath, RimcModeledWellPath_duplicate, "Duplicate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcModeledWellPath_duplicate::RimcModeledWellPath_duplicate( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_VALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
{
    CAF_PDM_InitObject( "Duplicate", "", "", "Duplicate" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcModeledWellPath_duplicate::execute()
{
    auto wellPath = self<RimWellPath>();

    return RicDuplicateWellPathFeature::duplicateWellPath( wellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcModeledWellPath_duplicate::classKeywordReturnedType() const
{
    return RimModeledWellPath::classKeywordStatic();
}
