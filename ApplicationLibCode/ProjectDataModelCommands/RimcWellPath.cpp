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

#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"
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

    CAF_PDM_InitScriptableField( &m_md, "MeasuredDepth", 0.0, "Measured Depth" );
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
