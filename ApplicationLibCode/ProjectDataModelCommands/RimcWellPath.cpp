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

#include "RimcWellPath.h"

#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"

#include "FractureCommands/RicNewWellPathFractureFeature.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_addFracture, "AddFracture" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_addFracture::RimcWellPath_addFracture( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Add StimPlan Fracture", "", "", "Add StimPlan Fracture" );

    CAF_PDM_InitScriptableField( &m_md, "MeasuredDepth", 0.0, "Measured Depth", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanFractureTemplate,
                                          "StimPlanFractureTemplate",
                                          "",
                                          "",
                                          "",
                                          "StimPlan Fracture Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcWellPath_addFracture::execute()
{
    auto wellPath = self<RimWellPath>();

    RimWellPathFracture* wellPathFracture = RicNewWellPathFractureFeature::addFracture( wellPath, m_md() );

    if ( m_stimPlanFractureTemplate ) wellPathFracture->setFractureTemplate( m_stimPlanFractureTemplate() );

    return wellPathFracture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcWellPath_addFracture::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcWellPath_addFracture::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimWellPathFracture );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_appendPerforationInterval, "AppendPerforationInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_appendPerforationInterval::RimcWellPath_appendPerforationInterval( caf::PdmObjectHandle* self )
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
caf::PdmObjectHandle* RimcWellPath_appendPerforationInterval::execute()
{
    auto wellPath = self<RimWellPath>();

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
bool RimcWellPath_appendPerforationInterval::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcWellPath_appendPerforationInterval::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimPerforationInterval );
}
