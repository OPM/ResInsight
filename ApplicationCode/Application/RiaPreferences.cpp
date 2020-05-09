/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
//  Copyright (C) 2011-2018 Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaPreferences.h"

#include "RiaColorTables.h"
#include "RiaQDateTimeTools.h"
#include "RifReaderSettings.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmSettings.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiFilePathEditor.h"

#include <QDate>
#include <QDir>
#include <QLocale>
#include <QStandardPaths>

namespace caf
{
template <>
void RiaPreferences::SummaryRestartFilesImportModeType::setUp()
{
    addItem( RiaPreferences::SummaryRestartFilesImportMode::IMPORT, "IMPORT", "Unified" );
    addItem( RiaPreferences::SummaryRestartFilesImportMode::SEPARATE_CASES, "SEPARATE_CASES", "Separate Cases" );
    addItem( RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT, "NOT_IMPORT", "Skip" );
    setDefault( RiaPreferences::SummaryRestartFilesImportMode::IMPORT );
}

template <>
void RiaPreferences::SummaryHistoryCurveStyleModeType::setUp()
{
    addItem( RiaPreferences::SummaryHistoryCurveStyleMode::SYMBOLS, "SYMBOLS", "Symbols" );
    addItem( RiaPreferences::SummaryHistoryCurveStyleMode::LINES, "LINES", "Lines" );
    addItem( RiaPreferences::SummaryHistoryCurveStyleMode::SYMBOLS_AND_LINES, "SYMBOLS_AND_LINES", "Symbols and Lines" );
    setDefault( RiaPreferences::SummaryHistoryCurveStyleMode::SYMBOLS );
}

template <>
void RiaPreferences::PageSizeEnum::setUp()
{
    addItem( QPageSize::A3, "A3", "A3" );
    addItem( QPageSize::A4, "A4", "A4" );
    addItem( QPageSize::A5, "A5", "A5" );
    addItem( QPageSize::A6, "A6", "A6" );
    addItem( QPageSize::Letter, "LETTER", "US Letter" );
    addItem( QPageSize::Legal, "LEGAL", "US Legal" );
    addItem( QPageSize::Ledger, "LEDGER", "US Ledger" );
    addItem( QPageSize::Tabloid, "TABLOID", "US Tabloid" );
    setDefault( QPageSize::A4 );
}

template <>
void RiaPreferences::PageOrientationEnum::setUp()
{
    addItem( QPageLayout::Portrait, "PORTRAIT", "Portrait" );
    addItem( QPageLayout::Landscape, "LANDSCAPE", "Landscape" );
    setDefault( QPageLayout::Portrait );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RiaPreferences, "RiaPreferences" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferences::RiaPreferences( void )
{
    CAF_PDM_InitField( &m_navigationPolicy,
                       "navigationPolicy",
                       caf::AppEnum<RiaGuiApplication::RINavigationPolicy>(
                           RiaGuiApplication::RINavigationPolicy::NAVIGATION_POLICY_RMS ),
                       "Navigation Mode",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &enableGrpcServer,
                       "enableGrpcServer",
                       true,
                       "Enable Python Script Server",
                       "",
                       "Remote Procedure Call Scripting Engine",
                       "" );
    CAF_PDM_InitField( &defaultGrpcPortNumber, "defaultGrpcPort", 50051, "Default Python Script Server Port", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &scriptDirectories, "scriptDirectory", "Shared Script Folder(s)", "", "", "" );
    scriptDirectories.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    // TODO: This only currently works for installed ResInsight.
    scriptDirectories = QCoreApplication::applicationDirPath() + "/Python/rips/PythonExamples";

    QString defaultTextEditor;
#ifdef WIN32
    defaultTextEditor = QString( "notepad.exe" );
#else
    defaultTextEditor = QStandardPaths::findExecutable( "kate" );
    if ( defaultTextEditor.isEmpty() )
    {
        defaultTextEditor = QStandardPaths::findExecutable( "gedit" );
    }
#endif

    CAF_PDM_InitField( &scriptEditorExecutable, "scriptEditorExecutable", defaultTextEditor, "Script Editor", "", "", "" );
    scriptEditorExecutable.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &octaveExecutable, "octaveExecutable", QString( "octave" ), "Octave Executable Location", "", "", "" );
    octaveExecutable.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    octaveExecutable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &octaveShowHeaderInfoWhenExecutingScripts,
                       "octaveShowHeaderInfoWhenExecutingScripts",
                       false,
                       "Show Text Header When Executing Scripts",
                       "",
                       "",
                       "" );
    octaveShowHeaderInfoWhenExecutingScripts.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &pythonExecutable, "pythonExecutable", QString( "python" ), "Python Executable Location", "", "", "" );
    pythonExecutable.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    pythonExecutable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    CAF_PDM_InitField( &showPythonDebugInfo, "pythonDebugInfo", false, "Show Python Debug Info", "", "", "" );

    CAF_PDM_InitField( &ssihubAddress, "ssihubAddress", QString( "http://" ), "SSIHUB Address", "", "", "" );
    ssihubAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_defaultMeshModeType, "defaultMeshModeType", "Show Grid Lines", "", "", "" );
    CAF_PDM_InitField( &defaultGridLineColors,
                       "defaultGridLineColors",
                       RiaColorTables::defaultGridLineColor(),
                       "Mesh Color",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &defaultFaultGridLineColors,
                       "defaultFaultGridLineColors",
                       RiaColorTables::defaultFaultLineColor(),
                       "Mesh Color Along Faults",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &defaultWellLabelColor,
                       "defaultWellLableColor",
                       RiaColorTables::defaultWellLabelColor(),
                       "Well Label Color",
                       "",
                       "The default well label color in new views",
                       "" );

    CAF_PDM_InitField( &defaultViewerBackgroundColor,
                       "defaultViewerBackgroundColor",
                       RiaColorTables::defaultViewerBackgroundColor(),
                       "Viewer Background",
                       "",
                       "The viewer background color for new views",
                       "" );

    CAF_PDM_InitField( &m_defaultScaleFactorZ, "defaultScaleFactorZ", 5, "Default Z Scale Factor", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &defaultSceneFontSize, "defaultSceneFontSizePt", "Viewer Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &defaultAnnotationFontSize, "defaultAnnotationFontSizePt", "Annotation Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &defaultWellLabelFontSize, "defaultWellLabelFontSizePt", "Well Label Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &defaultPlotFontSize, "defaultPlotFontSizePt", "Plot Font Size", "", "", "" );

    CAF_PDM_InitField( &showLasCurveWithoutTvdWarning,
                       "showLasCurveWithoutTvdWarning",
                       true,
                       "Show LAS Curve Without TVD Warning",
                       "",
                       "",
                       "" );
    showLasCurveWithoutTvdWarning.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_useShaders, "useShaders", true, "Use Shaders", "", "", "" );
    m_useShaders.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &m_showHud, "showHud", false, "Show 3D Information", "", "", "" );
    m_showHud.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &m_appendClassNameToUiText, "appendClassNameToUiText", false, "Show Class Names", "", "", "" );
    m_appendClassNameToUiText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &m_appendFieldKeywordToToolTipText,
                       "appendFieldKeywordToToolTipText",
                       false,
                       "Show Field Keyword in ToolTip",
                       "",
                       "",
                       "" );
    m_appendFieldKeywordToToolTipText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &m_showViewIdInProjectTree, "showViewIdInTree", false, "Show View Id in Project Tree", "", "", "" );
    m_showViewIdInProjectTree.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_showTestToolbar, "showTestToolbar", false, "Enable Test Toolbar", "", "", "" );
    m_showTestToolbar.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &m_includeFractureDebugInfoFile,
                       "includeFractureDebugInfoFile",
                       false,
                       "Include Fracture Debug Info for Completion Export",
                       "",
                       "",
                       "" );
    m_includeFractureDebugInfoFile.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_showLegendBackground, "showLegendBackground", true, "Show Box around Legends", "", "", "" );
    m_showLegendBackground.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_enableFaultsByDefault, "enableFaultsByDefault", true, "Enable Faults By Default", "", "", "" );
    m_enableFaultsByDefault.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &lastUsedProjectFileName, "lastUsedProjectFileName", "Last Used Project File", "", "", "" );
    lastUsedProjectFileName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &autocomputeDepthRelatedProperties,
                       "autocomputeDepth",
                       true,
                       "Compute DEPTH Related Properties",
                       "",
                       "DEPTH, DX, DY, DZ, TOP, BOTTOM",
                       "" );
    autocomputeDepthRelatedProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &loadAndShowSoil, "loadAndShowSoil", true, "Load and Show SOIL", "", "", "" );
    loadAndShowSoil.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &summaryRestartFilesShowImportDialog,
                                "summaryRestartFilesShowImportDialog",
                                "Show Import Dialog",
                                "",
                                "",
                                "" );
    CAF_PDM_InitField( &summaryImportMode,
                       "summaryImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferences::SummaryRestartFilesImportMode::IMPORT ),
                       "Default Summary Import Option",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &gridImportMode,
                       "gridImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT ),
                       "Default Grid Import Option",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &summaryEnsembleImportMode,
                       "summaryEnsembleImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferences::SummaryRestartFilesImportMode::IMPORT ),
                       "Default Ensemble Summary Import Option",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &defaultSummaryHistoryCurveStyle,
                       "defaultSummaryHistoryCurveStyle",
                       SummaryHistoryCurveStyleModeType( RiaPreferences::SummaryHistoryCurveStyleMode::SYMBOLS ),
                       "Default Curve Style for History Vectors",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &defaultSummaryCurvesTextFilter,
                       "defaultSummaryCurvesTextFilter",
                       QString( "FOPT" ),
                       "Default Summary Curves",
                       "",
                       "Semicolon separated list of filters used to create curves in new summary plots",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_holoLensExportFolder, "holoLensExportFolder", "HoloLens Export Folder", "", "", "" );
    m_holoLensExportFolder.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_holoLensExportFolder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &holoLensDisableCertificateVerification,
                       "holoLensDisableCertificateVerification",
                       false,
                       "Disable SSL Certificate Verification (HoloLens)",
                       "",
                       "",
                       "" );
    holoLensDisableCertificateVerification.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &csvTextExportFieldSeparator,
                       "csvTextExportFieldSeparator",
                       QString( "," ),
                       "CSV Text Export Field Separator",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &m_showProjectChangedDialog,
                       "showProjectChangedDialog",
                       true,
                       "Show 'Project has changed' dialog",
                       "",
                       "",
                       "" );
    m_showProjectChangedDialog.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_readerSettings, "readerSettings", "Reader Settings", "", "", "" );
    m_readerSettings = new RifReaderSettings;
    CAF_PDM_InitFieldNoDefault( &m_dateFormat, "dateFormat", "Date Format", "", "", "" );
    m_dateFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_dateFormat = RiaQDateTimeTools::supportedDateFormats().front();

    CAF_PDM_InitFieldNoDefault( &m_timeFormat, "timeFormat", "Time Format", "", "", "" );
    m_timeFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_timeFormat = RiaQDateTimeTools::supportedTimeFormats().front();

    CAF_PDM_InitField( &m_showProgressBar, "showProgressBar", true, "Show Progress Bar", "", "", "" );
    m_showProgressBar.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_plotTemplateFolders, "plotTemplateFolders", "Plot Template Folder(s)", "", "", "" );
    m_plotTemplateFolders.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_searchPlotTemplateFoldersRecursively,
                       "SearchPlotTemplateFoldersRecursively",
                       true,
                       "Search Plot Templates Recursively",
                       "",
                       "",
                       "" );
    m_searchPlotTemplateFoldersRecursively.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_defaultPlotTemplate, "defaultPlotTemplate", "Default Plot Template", "", "", "" );
    // m_plotTemplateFolders.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showSummaryTimeAsLongString,
                       "showSummaryTimeAsLongString",
                       false,
                       "Show resample time text as long time text (2010-11-21 23:15:00)",
                       "",
                       "",
                       "" );
    m_showSummaryTimeAsLongString.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_useMultipleThreadsWhenLoadingSummaryData,
                       "useMultipleThreadsWhenLoadingSummaryData",
                       false,
                       "Use multiple threads when loading summary data",
                       "",
                       "",
                       "" );
    m_useMultipleThreadsWhenLoadingSummaryData.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_pageSize, "pageSize", "Page Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_pageOrientation, "pageOrientation", "Page Orientation", "", "", "" );
    CAF_PDM_InitField( &m_pageLeftMargin, "pageLeftMargin", defaultMarginSize( m_pageSize() ), "Left Margin", "", "", "" );
    CAF_PDM_InitField( &m_pageTopMargin, "pageTopMargin", defaultMarginSize( m_pageSize() ), "Top Margin", "", "", "" );
    CAF_PDM_InitField( &m_pageRightMargin, "pageRightMargin", defaultMarginSize( m_pageSize() ), "Right Margin", "", "", "" );
    CAF_PDM_InitField( &m_pageBottomMargin, "pageBottomMargin", defaultMarginSize( m_pageSize() ), "Bottom Margin", "", "", "" );

    CAF_PDM_InitField( &m_openExportedPdfInViewer, "openExportedPdfInViewer", false, "Open Exported PDF in Viewer", "", "", "" );
    m_openExportedPdfInViewer.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferences::~RiaPreferences( void )
{
    delete m_readerSettings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferences* RiaPreferences::current()
{
    return RiaApplication::instance()->preferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                            QString                    uiConfigName,
                                            caf::PdmUiEditorAttribute* attribute )
{
    m_readerSettings->defineEditorAttribute( field, uiConfigName, attribute );

    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            if ( field == &scriptDirectories || field == &m_plotTemplateFolders )
            {
                myAttr->m_selectDirectory              = true;
                myAttr->m_appendUiSelectedFolderToText = true;
            }
        }
    }

    if ( field == &octaveShowHeaderInfoWhenExecutingScripts || field == &autocomputeDepthRelatedProperties ||
         field == &loadAndShowSoil || field == &m_useShaders || field == &m_showHud ||
         field == &m_appendClassNameToUiText || field == &m_appendFieldKeywordToToolTipText ||
         field == &m_showTestToolbar || field == &m_includeFractureDebugInfoFile ||
         field == &showLasCurveWithoutTvdWarning || field == &holoLensDisableCertificateVerification ||
         field == &m_showProjectChangedDialog || field == &m_searchPlotTemplateFoldersRecursively ||
         field == &m_showLegendBackground || field == &m_showSummaryTimeAsLongString ||
         field == &m_showViewIdInProjectTree || field == &m_useMultipleThreadsWhenLoadingSummaryData ||
         field == &m_enableFaultsByDefault || field == &m_showProgressBar || field == &m_openExportedPdfInViewer )
    {
        caf::PdmUiCheckBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_useNativeCheckBoxLabel = true;
        }
    }
    else if ( field == &m_holoLensExportFolder )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectDirectory = true;
        }
    }
    if ( field == &defaultSceneFontSize || field == &defaultWellLabelFontSize || field == &defaultAnnotationFontSize ||
         field == &defaultPlotFontSize )
    {
        caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        myAttr->minimumContentsLength             = 2;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( uiConfigName == RiaPreferences::tabNameGeneral() )
    {
        caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup( "Default Colors" );
        colorGroup->add( &defaultViewerBackgroundColor );
        colorGroup->add( &defaultGridLineColors, false );
        colorGroup->add( &defaultFaultGridLineColors );
        colorGroup->add( &defaultWellLabelColor, false );

        caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup( "Default Font Sizes" );
        fontGroup->add( &defaultSceneFontSize );
        fontGroup->add( &defaultAnnotationFontSize, false );
        fontGroup->add( &defaultWellLabelFontSize );
        fontGroup->add( &defaultPlotFontSize, false );

        caf::PdmUiGroup* viewsGroup = uiOrdering.addNewGroup( "3d Views" );
        viewsGroup->add( &m_defaultMeshModeType );
        viewsGroup->add( &m_navigationPolicy );
        viewsGroup->add( &m_defaultScaleFactorZ );
        viewsGroup->add( &m_showLegendBackground );
        viewsGroup->add( &m_enableFaultsByDefault );

        caf::PdmUiGroup* otherGroup = uiOrdering.addNewGroup( "Other" );
        otherGroup->add( &ssihubAddress );
        otherGroup->add( &showLasCurveWithoutTvdWarning );
        otherGroup->add( &holoLensDisableCertificateVerification );
    }
    else if ( uiConfigName == RiaPreferences::tabNameEclipse() )
    {
        caf::PdmUiGroup* newCaseBehaviourGroup = uiOrdering.addNewGroup( "Behavior When Loading Data" );
        newCaseBehaviourGroup->add( &autocomputeDepthRelatedProperties );
        newCaseBehaviourGroup->add( &loadAndShowSoil );

        m_readerSettings->defineUiOrdering( uiConfigName, *newCaseBehaviourGroup );

        caf::PdmUiGroup* restartBehaviourGroup = uiOrdering.addNewGroup( "Origin Files" );
        restartBehaviourGroup->add( &summaryRestartFilesShowImportDialog );

        {
            caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup( "Origin Summary Files" );
            group->add( &summaryImportMode );
        }

        {
            caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup( "Origin Grid Files" );
            group->add( &gridImportMode );
        }

        {
            caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup( "Origin Ensemble Summary Files" );
            group->add( &summaryEnsembleImportMode );
        }
    }
    else if ( uiConfigName == RiaPreferences::tabNamePlotting() )
    {
        uiOrdering.add( &defaultSummaryCurvesTextFilter );
        uiOrdering.add( &defaultSummaryHistoryCurveStyle );
        uiOrdering.add( &m_dateFormat );
        uiOrdering.add( &m_timeFormat );
        uiOrdering.add( &m_showSummaryTimeAsLongString );
        uiOrdering.add( &m_useMultipleThreadsWhenLoadingSummaryData );

        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Plot Templates" );
        group->add( &m_plotTemplateFolders );
        group->add( &m_searchPlotTemplateFoldersRecursively );

        caf::PdmUiGroup* pageSetup = uiOrdering.addNewGroup( "Page Setup" );
        pageSetup->add( &m_pageSize );
        pageSetup->add( &m_pageOrientation, false );
        pageSetup->add( &m_pageLeftMargin );
        pageSetup->add( &m_pageRightMargin, false );
        pageSetup->add( &m_pageTopMargin );
        pageSetup->add( &m_pageBottomMargin, false );

        QString unitLabel = " [mm]";
        if ( QPageSize( m_pageSize() ).definitionUnits() == QPageSize::Inch )
        {
            unitLabel = " [in]";
        }
        m_pageLeftMargin.uiCapability()->setUiName( "Left Margin" + unitLabel );
        m_pageRightMargin.uiCapability()->setUiName( "Right Margin" + unitLabel );
        m_pageTopMargin.uiCapability()->setUiName( "Top Margin" + unitLabel );
        m_pageBottomMargin.uiCapability()->setUiName( "Bottom Margin" + unitLabel );
    }

    else if ( uiConfigName == RiaPreferences::tabNameScripting() )
    {
        caf::PdmUiGroup* octaveGroup = uiOrdering.addNewGroup( "Octave" );
        octaveGroup->add( &octaveExecutable );
        octaveGroup->add( &octaveShowHeaderInfoWhenExecutingScripts );

#ifdef ENABLE_GRPC
        caf::PdmUiGroup* pythonGroup = uiOrdering.addNewGroup( "Python" );
        pythonGroup->add( &enableGrpcServer );
        pythonGroup->add( &showPythonDebugInfo );
        pythonGroup->add( &defaultGrpcPortNumber );
        pythonGroup->add( &pythonExecutable );
#endif
        caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup( "Script files" );
        scriptGroup->add( &scriptDirectories );
        scriptGroup->add( &scriptEditorExecutable );
    }
    else if ( uiConfigName == RiaPreferences::tabNameExport() )
    {
        uiOrdering.add( &csvTextExportFieldSeparator );
        uiOrdering.add( &m_openExportedPdfInViewer );
    }
    else if ( RiaApplication::enableDevelopmentFeatures() && uiConfigName == RiaPreferences::tabNameSystem() )
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

        uiOrdering.add( &m_showProgressBar );
        uiOrdering.add( &m_showProjectChangedDialog );
        uiOrdering.add( &m_showTestToolbar );
        uiOrdering.add( &m_includeFractureDebugInfoFile );
        uiOrdering.add( &m_holoLensExportFolder );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaPreferences::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                     bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if ( fieldNeedingOptions == &gridImportMode )
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip( RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT );
        SummaryRestartFilesImportModeType separate( RiaPreferences::SummaryRestartFilesImportMode::SEPARATE_CASES );

        options.push_back(
            caf::PdmOptionItemInfo( skip.uiText(), RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT ) );
        options.push_back( caf::PdmOptionItemInfo( separate.uiText(),
                                                   RiaPreferences::SummaryRestartFilesImportMode::SEPARATE_CASES ) );
    }
    else if ( fieldNeedingOptions == &summaryEnsembleImportMode )
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip( RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT );
        SummaryRestartFilesImportModeType allowImport( RiaPreferences::SummaryRestartFilesImportMode::IMPORT );

        options.push_back(
            caf::PdmOptionItemInfo( skip.uiText(), RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT ) );
        options.push_back(
            caf::PdmOptionItemInfo( allowImport.uiText(), RiaPreferences::SummaryRestartFilesImportMode::IMPORT ) );
    }
    else if ( fieldNeedingOptions == &m_dateFormat )
    {
        for ( auto dateFormat : RiaQDateTimeTools::supportedDateFormats() )
        {
            QDate   exampleDate = QDate( 2019, 8, 16 );
            QString fullDateFormat =
                RiaQDateTimeTools::dateFormatString( dateFormat, RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
            QString uiText = QString( "%1 (%2)" ).arg( fullDateFormat ).arg( exampleDate.toString( fullDateFormat ) );
            uiText.replace( "AP", "AM/PM" );
            options.push_back( caf::PdmOptionItemInfo( uiText, QVariant::fromValue( dateFormat ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeFormat )
    {
        for ( auto timeFormat : RiaQDateTimeTools::supportedTimeFormats() )
        {
            QTime   exampleTime = QTime( 15, 48, 22 );
            QString timeFormatString =
                RiaQDateTimeTools::timeFormatString( timeFormat,
                                                     RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );
            QString uiText = QString( "%1 (%2)" ).arg( timeFormatString ).arg( exampleTime.toString( timeFormatString ) );
            uiText.replace( "AP", "AM/PM" );
            options.push_back( caf::PdmOptionItemInfo( uiText, QVariant::fromValue( timeFormat ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::initAfterRead()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    if ( changedField == &m_pageSize )
    {
        m_pageLeftMargin   = defaultMarginSize( m_pageSize() );
        m_pageRightMargin  = defaultMarginSize( m_pageSize() );
        m_pageTopMargin    = defaultMarginSize( m_pageSize() );
        m_pageBottomMargin = defaultMarginSize( m_pageSize() );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameGeneral()
{
    return "General";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameEclipse()
{
    return "Eclipse";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNamePlotting()
{
    return "Plotting";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameScripting()
{
    return "Scripting";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameExport()
{
    return "Export";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameSystem()
{
    return "System";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaPreferences::defaultMarginSize( QPageSize::PageSizeId pageSizeId )
{
    QPageSize::Unit unit = QPageSize( pageSizeId ).definitionUnits();

    if ( unit == QPageSize::Inch )
    {
        return 1.0;
    }
    else
    {
        return 20.0;
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaPreferences::tabNames()
{
    QStringList names;

    names << tabNameGeneral();
    names << tabNameEclipse();
    names << tabNamePlotting();
    names << tabNameScripting();
    names << tabNameExport();

    if ( RiaApplication::enableDevelopmentFeatures() )
    {
        names << tabNameSystem();
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifReaderSettings* RiaPreferences::readerSettings() const
{
    return m_readerSettings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::appendClassNameToUiText() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_appendClassNameToUiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::appendFieldKeywordToToolTipText() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_appendFieldKeywordToToolTipText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::showViewIdInProjectTree() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_showViewIdInProjectTree();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::showTestToolbar() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_showTestToolbar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::includeFractureDebugInfoFile() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_includeFractureDebugInfoFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::showProjectChangedDialog() const
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
QString RiaPreferences::holoLensExportFolder() const
{
    return m_holoLensExportFolder();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::useShaders() const
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
bool RiaPreferences::show3dInformation() const
{
    return RiaApplication::enableDevelopmentFeatures() && m_showHud();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiaPreferences::dateFormat() const
{
    return m_dateFormat();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiaPreferences::timeFormat() const
{
    return m_timeFormat();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::dateTimeFormat() const
{
    return QString( "%1 %2" ).arg( m_dateFormat() ).arg( m_timeFormat() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::searchPlotTemplateFoldersRecursively() const
{
    return m_searchPlotTemplateFoldersRecursively();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaPreferences::plotTemplateFolders() const
{
    QStringList filteredFolders;
    QStringList pathList = m_plotTemplateFolders().split( ';' );
    for ( const auto& path : pathList )
    {
        QDir dir( path );
        if ( !path.isEmpty() && dir.exists() && dir.isReadable() )
        {
            filteredFolders.push_back( path );
        }
    }

    return filteredFolders;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::appendPlotTemplateFolders( const QString& folder )
{
    QString folders = m_plotTemplateFolders();
    if ( !folders.isEmpty() )
    {
        folders += ";";
    }

    folders += folder;

    m_plotTemplateFolders = folders;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::defaultPlotTemplateAbsolutePath() const
{
    return m_defaultPlotTemplate().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::setDefaultPlotTemplatePath( const QString& templatePath )
{
    m_defaultPlotTemplate = templatePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::showSummaryTimeAsLongString() const
{
    return m_showSummaryTimeAsLongString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::useMultipleThreadsWhenReadingSummaryData() const
{
    return m_useMultipleThreadsWhenLoadingSummaryData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::showProgressBar() const
{
    return m_showProgressBar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::openExportedPdfInViewer() const
{
    return m_openExportedPdfInViewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> RiaPreferences::defaultFontSizes() const
{
    std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> fontSizes;
    fontSizes[RiaDefines::FontSettingType::SCENE_FONT]      = defaultSceneFontSize();
    fontSizes[RiaDefines::FontSettingType::ANNOTATION_FONT] = defaultAnnotationFontSize();
    fontSizes[RiaDefines::FontSettingType::WELL_LABEL_FONT] = defaultWellLabelFontSize();
    fontSizes[RiaDefines::FontSettingType::PLOT_FONT]       = defaultPlotFontSize();
    return fontSizes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::writePreferencesToApplicationStore()
{
    caf::PdmSettings::writeFieldsToApplicationStore( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPageLayout RiaPreferences::defaultPageLayout() const
{
    QPageSize   pageSize( m_pageSize() );
    QPageLayout layout( pageSize, m_pageOrientation(), margins(), (QPageLayout::Unit)pageSize.definitionUnits() );
    layout.setMode( QPageLayout::StandardMode );
    return layout;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMarginsF RiaPreferences::margins() const
{
    return QMarginsF( m_pageLeftMargin, m_pageTopMargin, m_pageRightMargin, m_pageBottomMargin );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::MeshModeType RiaPreferences::defaultMeshModeType() const
{
    return m_defaultMeshModeType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGuiApplication::RINavigationPolicy RiaPreferences::navigationPolicy() const
{
    return m_navigationPolicy();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferences::defaultScaleFactorZ() const
{
    return m_defaultScaleFactorZ();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::showLegendBackground() const
{
    return m_showLegendBackground();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::enableFaultsByDefault() const
{
    return m_enableFaultsByDefault;
}
