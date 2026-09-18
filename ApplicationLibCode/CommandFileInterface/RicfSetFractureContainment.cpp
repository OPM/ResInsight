#include "RicfSetExportFolder.h"
/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfSetFractureContainment.h"

#include "RicfCommandForwarding.h"

#include "RimFractureTemplate.h"
#include "RimcFractureTemplate.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfSetFractureContainment, "setFractureContainment" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetFractureContainment::RicfSetFractureContainment()
{
    CAF_PDM_InitScriptableField( &m_id, "id", -1, "Id" );
    CAF_PDM_InitScriptableField( &m_topLayer, "topLayer", -1, "TopLayer" );
    CAF_PDM_InitScriptableField( &m_baseLayer, "baseLayer", -1, "BaseLayer" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSetFractureContainment::execute()
{
    const QString commandName = classKeyword();

    if ( m_id < 0 || m_topLayer < 0 || m_baseLayer < 0 )
    {
        return RicfForwarding::errorResponse( "Required argument missing", commandName );
    }

    auto fractureTemplate = RicfForwarding::findFractureTemplate( m_id() );
    if ( !fractureTemplate ) return RicfForwarding::errorResponse( fractureTemplate.error(), commandName );

    RimFractureTemplate_setContainment method( fractureTemplate.value() );
    method.setLayers( m_topLayer(), m_baseLayer() );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
