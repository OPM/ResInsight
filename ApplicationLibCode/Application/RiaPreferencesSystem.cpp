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

namespace caf
{
template <>
void RiaPreferencesSystem::EclipseTextFileReaderModeType::setUp()
{
    addItem( RiaPreferencesSystem::EclipseTextFileReaderMode::MEMORY_MAPPED_FILE, "MEMORY_MAPPED_FILE", "Memory Mapped File Import" );
    addItem( RiaPreferencesSystem::EclipseTextFileReaderMode::FILE, "FILE", "Default File Import" );

    setDefault( RiaPreferencesSystem::EclipseTextFileReaderMode::FILE );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RiaPreferencesSystem, "RiaPreferencesSystem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSystem::RiaPreferencesSystem()
{
    CAF_PDM_InitField( &m_useShaders, "useShaders", true, "Use Shaders" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useShaders );
    CAF_PDM_InitField( &m_showHud, "showHud", false, "Show 3D Information" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showHud );
    CAF_PDM_InitField( &m_appendClassNameToUiText, "appendClassNameToUiText", false, "Show Class Names" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_appendClassNameToUiText );

    CAF_PDM_InitField( &m_appendFieldKeywordToToolTipText, "appendFieldKeywordToToolTipText", false, "Show Field Keyword in ToolTip" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_appendFieldKeywordToToolTipText );

    CAF_PDM_InitField( &m_showViewIdInProjectTree, "showViewIdInTree", false, "Show View Id in Project Tree" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showViewIdInProjectTree );

    CAF_PDM_InitField( &m_showTestToolbar, "showTestToolbar", false, "Enable Test Toolbar" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showTestToolbar );

    CAF_PDM_InitField( &m_includeFractureDebugInfoFile, "includeFractureDebugInfoFile", false, "Include Fracture Debug Info for Completion Export" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeFractureDebugInfoFile );

    CAF_PDM_InitFieldNoDefault( &m_holoLensExportFolder, "holoLensExportFolder", "HoloLens Export Folder" );
    m_holoLensExportFolder.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_holoLensExportFolder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showProjectChangedDialog, "showProjectChangedDialog", true, "Show 'Project has changed' dialog" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showProjectChangedDialog );

    CAF_PDM_InitField( &m_logToFile, "logToFile", true, "Write Log Files in Home Folder" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_logToFile );

    CAF_PDM_InitField( &m_showProgressBar, "showProgressBar", true, "Show Progress Bar" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showProgressBar );

    CAF_PDM_InitField( &m_showPdfExportDialog, "showPdfExportDialog", true, "Show PDF Export Dialog" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showPdfExportDialog );

    CAF_PDM_InitField( &m_useImprovedSummaryImport, "useImprovedSummaryImport", false, "Use Improved Summary Import" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useImprovedSummaryImport );

    CAF_PDM_InitField( &m_useMultiThreadingForSummary_TEMPORARY,
                       "useMultiThreadingForSummary",
                       true,
                       "Use Multithreading for Summary Import (TEMPORARY)" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useMultiThreadingForSummary_TEMPORARY );

    CAF_PDM_InitField( &m_gtestFilter, "gtestFilter", QString(), "Unit Test Filter (gtest)" );
    CAF_PDM_InitField( &m_exportScalingFactor, "exportScalingFactor", -1.0, "Export Scaling Factor (<0 disable)" );

    CAF_PDM_InitField( &m_eclipseReaderMode,
                       "eclipseReaderMode",
                       EclipseTextFileReaderModeType( RiaPreferencesSystem::EclipseTextFileReaderMode::FILE ),
                       "Eclipse Text File Import mode (GRDECL)" );

    CAF_PDM_InitField( &m_keywordsForLogging,
                       "KeywordsForLogging",
                       QString(),
                       "Keywords to enable debug logging, separated by semicolon.\nType 'enable-all' to enable logging for all objects." );
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
void RiaPreferencesSystem::setAppendClassNameToUiText( bool enable )
{
    m_appendClassNameToUiText = enable;
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
bool RiaPreferencesSystem::showPdfExportDialog() const
{
    return m_showPdfExportDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaPreferencesSystem::exportPdfScalingFactor() const
{
    return m_exportScalingFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::useImprovedSummaryImport() const
{
    return m_useImprovedSummaryImport();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::useMultiThreadingForSummaryImport() const
{
    return m_useMultiThreadingForSummary_TEMPORARY();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::logToFile() const
{
    return m_logToFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSystem::EclipseTextFileReaderMode RiaPreferencesSystem::eclipseTextFileReaderMode() const
{
    return m_eclipseReaderMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSystem::isLoggingActivatedForKeyword( const QString& keyword ) const
{
    if ( keyword.isEmpty() ) return true;

    QStringList keywords = m_keywordsForLogging().split( ";" );

    if ( keywords.contains( "enable-all" ) ) return true;

    return keywords.contains( keyword );
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

    uiOrdering.add( &m_logToFile );
    uiOrdering.add( &m_showProjectChangedDialog );
    uiOrdering.add( &m_showTestToolbar );
    uiOrdering.add( &m_includeFractureDebugInfoFile );
    uiOrdering.add( &m_holoLensExportFolder );
    uiOrdering.add( &m_showProgressBar );

    uiOrdering.add( &m_showPdfExportDialog );
    uiOrdering.add( &m_exportScalingFactor );
    uiOrdering.add( &m_eclipseReaderMode );

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Developer Settings" );
        group->add( &m_keywordsForLogging );
        group->add( &m_gtestFilter );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Summary Settings - July 2025" );
        group->add( &m_useImprovedSummaryImport );
        group->add( &m_useMultiThreadingForSummary_TEMPORARY );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaPreferencesSystem::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSystem::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
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
