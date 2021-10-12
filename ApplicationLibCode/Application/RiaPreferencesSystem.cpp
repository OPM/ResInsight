/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaApplication.h"

#include "RiaPreferences.h"
#include "RiaPreferencesSystem.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RiaPreferencesSystem, "RiaPreferencesSystem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSystem::RiaPreferencesSystem()
{
    CAF_PDM_InitField( &m_useShaders, "useShaders", true, "Use Shaders", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useShaders );
    CAF_PDM_InitField( &m_showHud, "showHud", false, "Show 3D Information", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showHud );
    CAF_PDM_InitField( &m_appendClassNameToUiText, "appendClassNameToUiText", false, "Show Class Names", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_appendClassNameToUiText );

    CAF_PDM_InitField( &m_appendFieldKeywordToToolTipText,
                       "appendFieldKeywordToToolTipText",
                       false,
                       "Show Field Keyword in ToolTip",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_appendFieldKeywordToToolTipText );

    CAF_PDM_InitField( &m_showViewIdInProjectTree, "showViewIdInTree", false, "Show View Id in Project Tree", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showViewIdInProjectTree );

    CAF_PDM_InitField( &m_showTestToolbar, "showTestToolbar", false, "Enable Test Toolbar", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showTestToolbar );

    CAF_PDM_InitField( &m_includeFractureDebugInfoFile,
                       "includeFractureDebugInfoFile",
                       false,
                       "Include Fracture Debug Info for Completion Export",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeFractureDebugInfoFile );

    CAF_PDM_InitFieldNoDefault( &m_holoLensExportFolder, "holoLensExportFolder", "HoloLens Export Folder", "", "", "" );
    m_holoLensExportFolder.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_holoLensExportFolder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showProjectChangedDialog,
                       "showProjectChangedDialog",
                       true,
                       "Show 'Project has changed' dialog",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showProjectChangedDialog );

    CAF_PDM_InitField( &m_showProgressBar, "showProgressBar", true, "Show Progress Bar", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showProgressBar );

    CAF_PDM_InitField( &m_gtestFilter, "gtestFilter", QString(), "Unit Test Filter (gtest)", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSystem* RiaPreferencesSystem::current()
{
    return RiaApplication::instance()->preferences()->systemPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::appendClassNameToUiText() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_appendClassNameToUiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::appendFieldKeywordToToolTipText() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_appendFieldKeywordToToolTipText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::showViewIdInProjectTree() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_showViewIdInProjectTree();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::showTestToolbar() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_showTestToolbar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::includeFractureDebugInfoFile() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_includeFractureDebugInfoFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::showProjectChangedDialog() const
{
    if ( !RiaApplication::enableDevelopmentFeatures() )
    {
        return true;
    }

    return m_showProjectChangedDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesSystem::holoLensExportFolder() const
{
    return m_holoLensExportFolder();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::useShaders() const
{
    if ( !RiaApplication::enableDevelopmentFeatures() )
    {
        return true;
    }

    return m_useShaders();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::show3dInformation() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_showHud();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesSystem::gtestFilter() const
{
    return m_gtestFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::showProgressBar() const
{
    return m_showProgressBar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSystem::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Project Tree" );
        group->add( &m_appendClassNameToUiText );
        group->add( &m_appendFieldKeywordToToolTipText );
        group->add( &m_showViewIdInProjectTree );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "3D View" );
        group->add( &m_useShaders );
        group->add( &m_showHud );
    }

    uiOrdering.add( &m_gtestFilter );
    uiOrdering.add( &m_showProjectChangedDialog );
    uiOrdering.add( &m_showTestToolbar );
    uiOrdering.add( &m_includeFractureDebugInfoFile );
    uiOrdering.add( &m_holoLensExportFolder );
    uiOrdering.add( &m_showProgressBar );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RiaPreferencesSystem::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    /*
        if ( fieldNeedingOptions == &m_summaryReader )
        {
            std::vector<SummaryReaderMode> availableModes;

    #ifdef USE_HDF5
            availableModes.push_back( SummaryReaderMode::HDF5_OPM_COMMON );
    #endif // USE_HDF5
            availableModes.push_back( SummaryReaderMode::LIBECL );
            availableModes.push_back( SummaryReaderMode::OPM_COMMON );

            for ( auto enumValue : availableModes )
            {
                options.push_back( caf::PdmOptionItemInfo( SummaryReaderModeType::uiText( enumValue ), enumValue ) );
            }
        }
        else if ( fieldNeedingOptions == &m_gridImportMode )
        {
            // Manual option handling in order to one only a subset of the enum values
            SummaryRestartFilesImportModeType skip( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT );
            SummaryRestartFilesImportModeType separate(
    RiaPreferencesSummary::SummaryRestartFilesImportMode::SEPARATE_CASES );

            options.push_back(
                caf::PdmOptionItemInfo( skip.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT
    ) ); options.push_back( caf::PdmOptionItemInfo( separate.uiText(),
                                                       RiaPreferencesSummary::SummaryRestartFilesImportMode::SEPARATE_CASES
    ) );
        }
        else if ( fieldNeedingOptions == &m_summaryEnsembleImportMode )
        {
            // Manual option handling in order to one only a subset of the enum values
            SummaryRestartFilesImportModeType skip( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT );
            SummaryRestartFilesImportModeType allowImport( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT
    );

            options.push_back(
                caf::PdmOptionItemInfo( skip.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT
    ) ); options.push_back( caf::PdmOptionItemInfo( allowImport.uiText(),
                                                       RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT ) );
        }
    */

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSystem::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                  QString                    uiConfigName,
                                                  caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_holoLensExportFolder )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectDirectory = true;
        }
    }
}
