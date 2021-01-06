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

#include "RiaLogging.h"

#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfSetFractureContainment, "setFractureContainment" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetFractureContainment::RicfSetFractureContainment()
{
    CAF_PDM_InitScriptableField( &m_id, "id", -1, "Id", "", "", "" );
    CAF_PDM_InitScriptableField( &m_topLayer, "topLayer", -1, "TopLayer", "", "", "" );
    CAF_PDM_InitScriptableField( &m_baseLayer, "baseLayer", -1, "BaseLayer", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSetFractureContainment::execute()
{
    if ( m_id < 0 || m_topLayer < 0 || m_baseLayer < 0 )
    {
        QString error( "setFractureContainment: Required argument missing" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RimProject* project = RimProject::current();

    if ( !project )
    {
        QString error( "setFractureContainment: Project not found" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RimFractureTemplateCollection* templColl =
        !project->allFractureTemplateCollections().empty() ? project->allFractureTemplateCollections()[0] : nullptr;
    RimFractureTemplate* templ = templColl ? templColl->fractureTemplate( m_id ) : nullptr;

    if ( !templ )
    {
        QString error = QString( "setFractureContainment: Fracture template not found. Id=%1" ).arg( m_id );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    templ->setContainmentTopKLayer( m_topLayer );
    templ->setContainmentBaseKLayer( m_baseLayer );
    templ->loadDataAndUpdateGeometryHasChanged();
    return caf::PdmScriptResponse();
}
