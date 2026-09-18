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

#include "RicfCreateMultipleFractures.h"

#include "RicfApplicationTools.h"
#include "RicfCommandForwarding.h"

#include "RimEclipseCase.h"
#include "RimFractureTemplate.h"
#include "RimWellPath.h"
#include "RimcEclipseCase.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCreateMultipleFractures, "createMultipleFractures" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateMultipleFractures::RicfCreateMultipleFractures()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names" );
    CAF_PDM_InitScriptableField( &m_minDistFromWellTd, "minDistFromWellTd", 100.0, "Min Distance From Well TD" );
    CAF_PDM_InitScriptableField( &m_maxFracturesPerWell, "maxFracturesPerWell", 100, "Max Fractures per Well" );
    CAF_PDM_InitScriptableField( &m_templateId, "templateId", -1, "Template ID" );
    CAF_PDM_InitScriptableField( &m_topLayer, "topLayer", -1, "Top Layer" );
    CAF_PDM_InitScriptableField( &m_baseLayer, "baseLayer", -1, "Base Layer" );
    CAF_PDM_InitScriptableField( &m_spacing, "spacing", 300.0, "Spacing" );
    CAF_PDM_InitScriptableField( &m_action,
                                 "action",
                                 caf::AppEnum<MultipleFractures::Action>( MultipleFractures::Action::APPEND_FRACTURES ),
                                 "Action" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateMultipleFractures::execute()
{
    const QString commandName = classKeyword();

    if ( m_caseId() < 0 || m_templateId() < 0 )
    {
        return RicfForwarding::errorResponse( "Mandatory argument(s) missing: caseId and templateId are required", commandName );
    }

    QStringList               wellsNotFound;
    std::vector<RimWellPath*> wellPaths =
        RicfApplicationTools::wellPathsFromNames( RicfApplicationTools::toQStringList( m_wellPathNames ), &wellsNotFound );
    if ( !wellsNotFound.empty() )
    {
        return RicfForwarding::errorResponse( "These well paths were not found: " + wellsNotFound.join( ", " ), commandName );
    }
    if ( wellPaths.empty() ) return RicfForwarding::errorResponse( "No wellpaths found", commandName );

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase.value() );
    if ( !eclipseCase )
    {
        return RicfForwarding::errorResponse( QString( "Case with ID %1 is not an Eclipse case" ).arg( m_caseId() ), commandName );
    }

    auto fractureTemplate = RicfForwarding::findFractureTemplate( m_templateId() );
    if ( !fractureTemplate ) return RicfForwarding::errorResponse( fractureTemplate.error(), commandName );

    RimEclipseCase_createMultipleFractures method( eclipseCase );
    method.setWellPaths( wellPaths );
    method.setFractureTemplate( fractureTemplate.value() );
    method.setMinDistFromWellTd( m_minDistFromWellTd() );
    method.setMaxFracturesPerWell( m_maxFracturesPerWell() );
    method.setTopLayer( m_topLayer() );
    method.setBaseLayer( m_baseLayer() );
    method.setSpacing( m_spacing() );
    method.setAction( m_action() );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
