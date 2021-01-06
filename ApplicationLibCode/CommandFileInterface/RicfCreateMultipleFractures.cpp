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
#include "RicfCommandFileExecutor.h"

#include "FractureCommands/RicCreateMultipleFracturesFeature.h"
#include "FractureCommands/RicCreateMultipleFracturesOptionItemUi.h"
#include "FractureCommands/RicCreateMultipleFracturesUi.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "RiaLogging.h"
#include "RiaWellNameComparer.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfCreateMultipleFractures, "createMultipleFractures" );

namespace caf
{
template <>
void AppEnum<MultipleFractures::Action>::setUp()
{
    addItem( MultipleFractures::Action::APPEND_FRACTURES, "APPEND_FRACTURES", "Append Fractures" );
    addItem( MultipleFractures::Action::REPLACE_FRACTURES, "REPLACE_FRACTURES", "Replace Fractures" );

    setDefault( MultipleFractures::Action::NONE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCreateMultipleFractures::RicfCreateMultipleFractures()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names", "", "", "" );
    CAF_PDM_InitScriptableField( &m_minDistFromWellTd, "minDistFromWellTd", 100.0, "Min Distance From Well TD", "", "", "" );
    CAF_PDM_InitScriptableField( &m_maxFracturesPerWell, "maxFracturesPerWell", 100, "Max Fractures per Well", "", "", "" );
    CAF_PDM_InitScriptableField( &m_templateId, "templateId", -1, "Template ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_topLayer, "topLayer", -1, "Top Layer", "", "", "" );
    CAF_PDM_InitScriptableField( &m_baseLayer, "baseLayer", -1, "Base Layer", "", "", "" );
    CAF_PDM_InitScriptableField( &m_spacing, "spacing", 300.0, "Spacing", "", "", "" );
    CAF_PDM_InitScriptableField( &m_action,
                                 "action",
                                 caf::AppEnum<MultipleFractures::Action>( MultipleFractures::Action::APPEND_FRACTURES ),
                                 "Action",
                                 "",
                                 "",
                                 "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfCreateMultipleFractures::execute()
{
    using TOOLS = RicfApplicationTools;

    RimProject*                   project  = RimProject::current();
    RiuCreateMultipleFractionsUi* settings = project->dialogData()->multipleFractionsData();

    // Get case and fracture template
    auto                      gridCase         = TOOLS::caseFromId( m_caseId );
    auto                      fractureTemplate = fractureTemplateFromId( m_templateId );
    std::vector<RimWellPath*> wellPaths;

    // Find well paths
    {
        QStringList wellsNotFound;
        wellPaths = TOOLS::wellPathsFromNames( TOOLS::toQStringList( m_wellPathNames ), &wellsNotFound );
        if ( !wellsNotFound.empty() )
        {
            QString error =
                QString( "createMultipleFractures: These well paths were not found: %1" ).arg( wellsNotFound.join( ", " ) );
            RiaLogging::error( error );
            return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
        }
    }

    if ( !gridCase )
    {
        QString error = QString( "createMultipleFractures: Could not find case with ID %1" ).arg( m_caseId );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    if ( !fractureTemplate )
    {
        QString error =
            QString( "createMultipleFractures: Could not find fracture template with ID %1" ).arg( m_templateId );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    if ( wellPaths.empty() )
    {
        QString error( "createMultipleFractures: No wellpaths found" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    if ( !validateArguments() )
    {
        QString error( "createMultipleFractures: Mandatory argument(s) missing" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RicCreateMultipleFracturesOptionItemUi* options        = new RicCreateMultipleFracturesOptionItemUi();
    caf::CmdFeatureManager*                 commandManager = caf::CmdFeatureManager::instance();
    auto                                    feature        = dynamic_cast<RicCreateMultipleFracturesFeature*>(
        commandManager->getCommandFeature( "RicCreateMultipleFracturesFeature" ) );

    // Default layers
    int topLayer  = m_topLayer;
    int baseLayer = m_baseLayer;
    if ( feature && ( topLayer < 0 || baseLayer < 0 ) )
    {
        auto ijkRange = feature->ijkRangeForGrid( gridCase );
        if ( topLayer < 0 ) topLayer = static_cast<int>( ijkRange.first.z() );
        if ( baseLayer < 0 ) baseLayer = static_cast<int>( ijkRange.second.z() );
    }
    options->setValues( topLayer, baseLayer, fractureTemplate, m_spacing );

    settings->clearWellPaths();
    for ( auto wellPath : wellPaths )
    {
        settings->addWellPath( wellPath );
    }

    settings->setValues( gridCase, m_minDistFromWellTd, m_maxFracturesPerWell );
    settings->clearOptions();
    settings->insertOptionItem( nullptr, options );

    if ( feature )
    {
        if ( m_action == MultipleFractures::Action::APPEND_FRACTURES ) feature->appendFractures();
        if ( m_action == MultipleFractures::Action::REPLACE_FRACTURES ) feature->replaceFractures();
    }
    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicfCreateMultipleFractures::validateArguments() const
{
    bool valid = m_caseId >= 0 && m_templateId >= 0;

    if ( valid ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RicfCreateMultipleFractures::fractureTemplateFromId( int templateId ) const
{
    for ( RimFractureTemplate* t : RimProject::current()->allFractureTemplates() )
    {
        if ( t->id() == templateId ) return t;
    }

    RiaLogging::error( QString( "createMultipleFractures: Could not find fracture template with ID %1" ).arg( templateId ) );
    return nullptr;
}
