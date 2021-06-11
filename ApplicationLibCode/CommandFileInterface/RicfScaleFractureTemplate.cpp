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

#include "RiaLogging.h"

#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfScaleFractureTemplate, "scaleFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfScaleFractureTemplate::RicfScaleFractureTemplate()
{
    CAF_PDM_InitScriptableField( &m_id, "id", -1, "Id", "", "", "" );
    CAF_PDM_InitScriptableField( &m_halfLengthScaleFactor, "halfLength", 1.0, "HalfLengthScaleFactor", "", "", "" );
    CAF_PDM_InitScriptableField( &m_heightScaleFactor, "height", 1.0, "HeightScaleFactor", "", "", "" );
    CAF_PDM_InitScriptableField( &m_dFactorScaleFactor, "dFactor", 1.0, "DFactorScaleFactor", "", "", "" );
    CAF_PDM_InitScriptableField( &m_conductivityScaleFactor, "conductivity", 1.0, "ConductivityScaleFactor", "", "", "" );

    CAF_PDM_InitScriptableField( &m_OBSOLETE_widthScaleFactor, "width", 1.0, "WidthScaleFactor", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfScaleFractureTemplate::execute()
{
    if ( m_id < 0 )
    {
        QString error( "scaleFractureTemplate: Fracture template id not specified" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RimProject* project = RimProject::current();

    if ( !project )
    {
        QString error( "scaleFractureTemplate: Project not found" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RimFractureTemplateCollection* templColl =
        !project->allFractureTemplateCollections().empty() ? project->allFractureTemplateCollections()[0] : nullptr;
    RimFractureTemplate* templ = templColl ? templColl->fractureTemplate( m_id ) : nullptr;

    if ( !templ )
    {
        QString error = QString( "scaleFractureTemplate: Fracture template not found. Id=%1" ).arg( m_id );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    templ->setScaleFactors( m_halfLengthScaleFactor, m_heightScaleFactor, m_dFactorScaleFactor, m_conductivityScaleFactor );
    templ->loadDataAndUpdateGeometryHasChanged();
    return caf::PdmScriptResponse();
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
