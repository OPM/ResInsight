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

#include "RicfScaleFractureTemplate.h"

#include "RicfCommandForwarding.h"

#include "RimFractureTemplate.h"
#include "RimcFractureTemplate.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfScaleFractureTemplate, "scaleFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfScaleFractureTemplate::RicfScaleFractureTemplate()
{
    CAF_PDM_InitScriptableField( &m_id, "id", -1, "Id" );
    CAF_PDM_InitScriptableField( &m_halfLengthScaleFactor, "halfLength", 1.0, "HalfLengthScaleFactor" );
    CAF_PDM_InitScriptableField( &m_heightScaleFactor, "height", 1.0, "HeightScaleFactor" );
    CAF_PDM_InitScriptableField( &m_dFactorScaleFactor, "dFactor", 1.0, "DFactorScaleFactor" );
    CAF_PDM_InitScriptableField( &m_conductivityScaleFactor, "conductivity", 1.0, "ConductivityScaleFactor" );

    CAF_PDM_InitScriptableField( &m_OBSOLETE_widthScaleFactor, "width", 1.0, "WidthScaleFactor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfScaleFractureTemplate::execute()
{
    const QString commandName = classKeyword();

    if ( m_id < 0 ) return RicfForwarding::errorResponse( "Fracture template id not specified", commandName );

    auto fractureTemplate = RicfForwarding::findFractureTemplate( m_id() );
    if ( !fractureTemplate ) return RicfForwarding::errorResponse( fractureTemplate.error(), commandName );

    RimcFractureTemplate_setScaleFactors method( fractureTemplate.value() );
    method.setScaleFactors( m_halfLengthScaleFactor(), m_heightScaleFactor(), m_dFactorScaleFactor(), m_conductivityScaleFactor() );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfScaleFractureTemplate::initAfterRead()
{
    if ( m_OBSOLETE_widthScaleFactor != m_OBSOLETE_widthScaleFactor.defaultValue() )
    {
        m_halfLengthScaleFactor = m_OBSOLETE_widthScaleFactor;
    }
}
