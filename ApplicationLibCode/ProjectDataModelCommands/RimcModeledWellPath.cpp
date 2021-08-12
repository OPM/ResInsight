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
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
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

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimModeledWellPath,
                                   RimcModeledWellPath_appendPerforationInterval,
                                   "AppendPerforationInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcModeledWellPath_appendPerforationInterval::RimcModeledWellPath_appendPerforationInterval( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Append Perforation Interval", "", "", "Append Perforation Interval" );
    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "", "", "", "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "", "", "", "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", 0.0, "", "", "", "Diameter" );
    CAF_PDM_InitScriptableField( &m_skinFactor, "SkinFactor", 0.0, "", "", "", "Skin Factor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcModeledWellPath_appendPerforationInterval::execute()
{
    auto wellPath = self<RimModeledWellPath>();

    auto perforationInterval = new RimPerforationInterval;
    perforationInterval->setStartAndEndMD( m_startMD, m_endMD );
    perforationInterval->setSkinFactor( m_skinFactor );
    perforationInterval->setDiameter( m_diameter );

    wellPath->perforationIntervalCollection()->appendPerforation( perforationInterval );

    auto* wellPathCollection = RimTools::wellPathCollection();

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleRedrawAffectedViews();

    return perforationInterval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcModeledWellPath_appendPerforationInterval::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcModeledWellPath_appendPerforationInterval::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimPerforationInterval );
}
