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

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferencesGeoMech.h"
#include "RiaPreferencesGrid.h"
#include "RiaPreferencesOpm.h"
#include "RiaPreferencesOsdu.h"
#include "RiaPreferencesSummary.h"
#include "RiaPreferencesSumo.h"
#include "RiaPreferencesSystem.h"
#include "RiaQDateTimeTools.h"
#include "RiaValidRegExpValidator.h"

#include "OsduCommands//RicDeleteOsduTokenFeature.h"
#include "Sumo/RicDeleteSumoTokenFeature.h"

#include "RiuFileDialogTools.h"
#include "RiuGuiTheme.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmSettings.h"
#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QLocale>
#include <QStandardPaths>
#include <QValidator>

namespace caf
{
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
RiaPreferences::RiaPreferences()
{
    CAF_PDM_InitField( &m_navigationPolicy,
                       "navigationPolicy",
                       caf::AppEnum<RiaDefines::RINavigationPolicy>( RiaDefines::RINavigationPolicy::NAVIGATION_POLICY_RMS ),
                       "Navigation Mode" );

    CAF_PDM_InitField( &enableGrpcServer, "enableGrpcServer", true, "Enable Python Script Server", "", "Remote Procedure Call Scripting Engine", "" );
    CAF_PDM_InitField( &defaultGrpcPortNumber, "defaultGrpcPort", 50051, "Default Python Script Server Port" );

    CAF_PDM_InitFieldNoDefault( &scriptDirectories, "scriptDirectory", "Shared Script Folder(s)" );
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

    CAF_PDM_InitField( &scriptEditorExecutable, "scriptEditorExecutable", defaultTextEditor, "Script Editor" );
    scriptEditorExecutable.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxScriptFoldersDepth, "MaxScriptFoldersDepth", 2, "Maximum Scripts Folder Search Depth" );

    CAF_PDM_InitField( &m_octaveExecutable, "octaveExecutable", QString( "octave" ), "Octave Executable Location" );
    m_octaveExecutable.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_octaveExecutable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_octaveShowHeaderInfoWhenExecutingScripts,
                       "octaveShowHeaderInfoWhenExecutingScripts",
                       false,
                       "Show Text Header When Executing Scripts" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_octaveShowHeaderInfoWhenExecutingScripts );

    CAF_PDM_InitFieldNoDefault( &m_octavePortNumber, "octavePortNumber", "Octave Port Number" );
    m_octavePortNumber.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_pythonExecutable, "pythonExecutable", QString( "python" ), "Python Executable Location" );
    m_pythonExecutable.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_pythonExecutable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    CAF_PDM_InitField( &showPythonDebugInfo, "pythonDebugInfo", false, "Show Python Debug Info" );

    auto defaultFilename = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation );
    if ( defaultFilename.isEmpty() )
    {
        defaultFilename = QStandardPaths::writableLocation( QStandardPaths::HomeLocation );
    }
    defaultFilename += "/ResInsight.log";

    CAF_PDM_InitField( &m_storeBackupOfProjectFile, "storeBackupOfProjectFile", true, "Store Backup of Project Files" );

    CAF_PDM_InitFieldNoDefault( &m_defaultMeshModeType, "defaultMeshModeType", "Show Grid Lines" );
    CAF_PDM_InitField( &defaultGridLineColors, "defaultGridLineColors", RiaColorTables::defaultGridLineColor(), "Mesh Color" );
    CAF_PDM_InitField( &defaultFaultGridLineColors,
                       "defaultFaultGridLineColors",
                       RiaColorTables::defaultFaultLineColor(),
                       "Mesh Color Along Faults" );
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

    CAF_PDM_InitField( &m_defaultScaleFactorZ, "defaultScaleFactorZ", 5.0, "Default Z Scale Factor" );

    CAF_PDM_InitFieldNoDefault( &defaultSceneFontSize, "defaultSceneFontSizePt", "Viewer Font Size" );
    CAF_PDM_InitFieldNoDefault( &defaultAnnotationFontSize, "defaultAnnotationFontSizePt", "Annotation Font Size" );
    CAF_PDM_InitFieldNoDefault( &defaultWellLabelFontSize, "defaultWellLabelFontSizePt", "Well Label Font Size" );

    auto defaultValue = FontSizeEnum( RiaFontCache::FontSize::FONT_SIZE_10 );
    CAF_PDM_InitField( &defaultPlotFontSize, "defaultPlotFontSizePt", defaultValue, "Plot Font Size" );

    CAF_PDM_InitField( &m_showLegendBackground, "showLegendBackground", true, "Show Box around Legends" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showLegendBackground );

    CAF_PDM_InitField( &m_enableFaultsByDefault, "enableFaultsByDefault", true, "Enable Faults By Default" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_enableFaultsByDefault );

    CAF_PDM_InitField( &m_showInfoBox, "showInfoBox", true, "Show Info Box in New Projects" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showInfoBox );

    CAF_PDM_InitField( &m_showGridBox, "showGridBox", true, "Show Grid Box in New Projects" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showGridBox );

    CAF_PDM_InitFieldNoDefault( &lastUsedProjectFileName, "lastUsedProjectFileName", "Last Used Project File" );
    lastUsedProjectFileName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &holoLensDisableCertificateVerification,
                       "holoLensDisableCertificateVerification",
                       false,
                       "Disable SSL Certificate Verification (HoloLens)" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &holoLensDisableCertificateVerification );

    CAF_PDM_InitField( &csvTextExportFieldSeparator, "csvTextExportFieldSeparator", QString( "," ), "CSV Text Export Field Separator" );

    CAF_PDM_InitFieldNoDefault( &m_dateFormat, "dateFormat", "Date Format" );
    m_dateFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_dateFormat = RiaQDateTimeTools::supportedDateFormats().front();

    CAF_PDM_InitFieldNoDefault( &m_timeFormat, "timeFormat", "Time Format" );
    m_timeFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_timeFormat = RiaQDateTimeTools::supportedTimeFormats().front();

    CAF_PDM_InitField( &m_useUndoRedo, "useUndoRedo", false, "Enable Undo/Redo for Property Editor changes" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useUndoRedo );

    CAF_PDM_InitFieldNoDefault( &m_plotTemplateFolders, "plotTemplateFolders", "Plot Template Folder(s)" );
    m_plotTemplateFolders.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_maxPlotTemplateFoldersDepth, "MaxPlotTemplateFoldersDepth", 2, "Maximum Plot Template Folder Search Depth" );

    CAF_PDM_InitFieldNoDefault( &m_lastUsedPlotTemplate, "defaultPlotTemplate", "Default Plot Template" );

    CAF_PDM_InitFieldNoDefault( &m_pageSize, "pageSize", "Page Size" );
    CAF_PDM_InitFieldNoDefault( &m_pageOrientation, "pageOrientation", "Page Orientation" );
    CAF_PDM_InitField( &m_pageLeftMargin, "pageLeftMargin", defaultMarginSize( m_pageSize() ), "Left Margin" );
    CAF_PDM_InitField( &m_pageTopMargin, "pageTopMargin", defaultMarginSize( m_pageSize() ), "Top Margin" );
    CAF_PDM_InitField( &m_pageRightMargin, "pageRightMargin", defaultMarginSize( m_pageSize() ), "Right Margin" );
    CAF_PDM_InitField( &m_pageBottomMargin, "pageBottomMargin", defaultMarginSize( m_pageSize() ), "Bottom Margin" );

    CAF_PDM_InitField( &m_openExportedPdfInViewer, "openExportedPdfInViewer", false, "Open Exported PDF in Viewer" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_openExportedPdfInViewer );

    CAF_PDM_InitField( &m_writeEchoInGrdeclFiles, "writeEchoInGrdeclFiles", false, "Write NOECHO and ECHO in GRDECL files" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_writeEchoInGrdeclFiles );

    CAF_PDM_InitFieldNoDefault( &m_gridCalculationExpressionFolder, "gridCalculationExpressionFolder", "Grid Calculation Expression Folder" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCalculationExpressionFolder,
                                "summaryCalculationExpressionFolder",
                                "Summary Calculation Expression Folder" );

    CAF_PDM_InitField( &m_surfaceImportResamplingDistance,
                       "SurfaceImportResamplingDistance",
                       100.0,
                       "Surface Import Coarsening",
                       "",
                       "Defines preferred minimum distance between surface points in XY-plane",
                       "" );

    CAF_PDM_InitField( &m_multiLateralWellPattern,
                       "MultiLateralWellPattern",
                       defaultMultiLateralWellNamePattern(),
                       "Multi Lateral Well Path Name Pattern",
                       "",
                       "Pattern to be used to decide if an imported well is part of a multi-lateral well. Allows use "
                       "of ? and * as wildcards.",
                       "" );
    m_multiLateralWellPattern.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_guiTheme, "guiTheme", "GUI theme" );

    CAF_PDM_InitFieldNoDefault( &m_summaryPreferences, "summaryPreferences", "summaryPreferences" );
    m_summaryPreferences = new RiaPreferencesSummary;

    CAF_PDM_InitFieldNoDefault( &m_gridPreferences, "gridPreferences", "gridPreferences" );
    m_gridPreferences = new RiaPreferencesGrid();

    CAF_PDM_InitFieldNoDefault( &m_geoMechPreferences, "geoMechPreferences", "geoMechPreferences" );
    m_geoMechPreferences = new RiaPreferencesGeoMech;

    CAF_PDM_InitFieldNoDefault( &m_opmPreferences, "opmPreferences", "opmPreferences" );
    m_opmPreferences = new RiaPreferencesOpm();

    CAF_PDM_InitFieldNoDefault( &m_systemPreferences, "systemPreferences", "systemPreferences" );
    m_systemPreferences = new RiaPreferencesSystem;

    CAF_PDM_InitFieldNoDefault( &m_osduPreferences, "osduPreferences", "osduPreferences" );
    m_osduPreferences = new RiaPreferencesOsdu;
    CAF_PDM_InitField( &m_deleteOsduToken, "deleteOsduToken", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_deleteOsduToken );
    m_deleteOsduToken.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_sumoPreferences, "sumoPreferences", "sumoPreferences" );
    m_sumoPreferences = new RiaPreferencesSumo;
    CAF_PDM_InitField( &m_deleteSumoToken, "deleteSumoToken", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_deleteSumoToken );
    m_deleteSumoToken.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_importPreferences, "importPreferences", "Import From File" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_importPreferences );

    CAF_PDM_InitFieldNoDefault( &m_exportPreferences, "exportPreferences", "Export To File" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_exportPreferences );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferences::~RiaPreferences()
{
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
void RiaPreferences::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    m_summaryPreferences->defineEditorAttribute( field, uiConfigName, attribute );

    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            if ( field == &scriptDirectories || field == &m_plotTemplateFolders )
            {
                myAttr->m_selectDirectory              = true;
                myAttr->m_appendUiSelectedFolderToText = true;
            }
            else if ( field == &m_gridCalculationExpressionFolder || field == &m_summaryCalculationExpressionFolder )
            {
                myAttr->m_selectDirectory = true;
            }
        }
    }

    if ( field == &defaultSceneFontSize || field == &defaultWellLabelFontSize || field == &defaultAnnotationFontSize ||
         field == &defaultPlotFontSize )
    {
        caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->minimumContentsLength = 2;
        }
    }
    else if ( field == &m_multiLateralWellPattern )
    {
        caf::PdmUiLineEditorAttribute* myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->validator = new RiaValidRegExpValidator( RiaPreferences::defaultMultiLateralWellNamePattern() );
        }
    }
    else if ( field == &m_defaultScaleFactorZ )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->validator = new QDoubleValidator( 0.000001, 100000.0, 6 );
        }
    }
    else if ( ( field == &m_deleteOsduToken ) || ( field == &m_deleteSumoToken ) )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            pbAttribute->m_buttonText = "Delete Token";
        }
    }
    else if ( field == &m_importPreferences )
    {
        if ( auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            pbAttribute->m_buttonText = "Import Preferences";
        }
    }
    else if ( field == &m_exportPreferences )
    {
        if ( auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            pbAttribute->m_buttonText = "Export Preferences";
        }
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
        colorGroup->appendToRow( &defaultGridLineColors );
        colorGroup->add( &defaultFaultGridLineColors );
        colorGroup->appendToRow( &defaultWellLabelColor );
        colorGroup->add( &m_guiTheme, { .newRow = true, .totalColumnSpan = 2 } );

        caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup( "Default Font Sizes" );
        fontGroup->add( &defaultSceneFontSize );
        fontGroup->appendToRow( &defaultAnnotationFontSize );
        fontGroup->add( &defaultWellLabelFontSize );
        fontGroup->appendToRow( &defaultPlotFontSize );

        caf::PdmUiGroup* viewsGroup = uiOrdering.addNewGroup( "3d Views" );
        viewsGroup->add( &m_defaultMeshModeType );
        viewsGroup->add( &m_navigationPolicy );
        viewsGroup->add( &m_defaultScaleFactorZ );

        viewsGroup->add( &m_showLegendBackground );
        viewsGroup->add( &m_enableFaultsByDefault, { .newRow = false, .totalColumnSpan = 1 } );
        viewsGroup->add( &m_showInfoBox );
        viewsGroup->add( &m_showGridBox, { .newRow = false, .totalColumnSpan = 1 } );

        caf::PdmUiGroup* loggingGroup = uiOrdering.addNewGroup( "Logging and Backup" );
        loggingGroup->add( &m_storeBackupOfProjectFile );

        caf::PdmUiGroup* otherGroup = uiOrdering.addNewGroup( "Other" );
        otherGroup->setCollapsedByDefault();
        otherGroup->add( &holoLensDisableCertificateVerification );
        otherGroup->add( &m_useUndoRedo );

        caf::PdmUiGroup* importExportGroup = uiOrdering.addNewGroup( "Import and Export" );
        importExportGroup->setCollapsedByDefault();
        importExportGroup->add( &m_importPreferences, { .newRow = false, .totalColumnSpan = 1 } );
        importExportGroup->add( &m_exportPreferences, { .newRow = false, .totalColumnSpan = 1 } );
    }
    else if ( uiConfigName == RiaPreferences::tabNameGrid() )
    {
        m_gridPreferences()->appendItems( uiOrdering );
    }
    else if ( uiConfigName == RiaPreferences::tabNameSummary() )
    {
        m_summaryPreferences->appendRestartFileGroup( uiOrdering );

        {
            caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Summary Data Import" );

            m_summaryPreferences()->uiOrdering( uiConfigName, *group );
        }
    }
    else if ( uiConfigName == RiaPreferences::tabNamePlotting() )
    {
        caf::PdmUiGroup* summaryGrp = uiOrdering.addNewGroup( "Summary Plots" );

        summaryPreferences()->appendItemsToPlottingGroup( *summaryGrp );

        caf::PdmUiGroup* group = summaryGrp->addNewGroup( "Plot Templates" );
        group->add( &m_plotTemplateFolders );
        group->add( &m_maxPlotTemplateFoldersDepth );

        caf::PdmUiGroup* generalGrp = uiOrdering.addNewGroup( "General" );

        generalGrp->add( &m_dateFormat );
        generalGrp->add( &m_timeFormat );

        caf::PdmUiGroup* pageSetup = generalGrp->addNewGroup( "Page Setup" );
        pageSetup->add( &m_pageSize );
        pageSetup->appendToRow( &m_pageOrientation );
        pageSetup->add( &m_pageLeftMargin );
        pageSetup->appendToRow( &m_pageRightMargin );
        pageSetup->add( &m_pageTopMargin );
        pageSetup->appendToRow( &m_pageBottomMargin );

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
        octaveGroup->add( &m_octaveExecutable );
        octaveGroup->add( &m_octaveShowHeaderInfoWhenExecutingScripts );
        octaveGroup->add( &m_octavePortNumber );

#ifdef ENABLE_GRPC
        caf::PdmUiGroup* pythonGroup = uiOrdering.addNewGroup( "Python" );
        pythonGroup->add( &enableGrpcServer );
        pythonGroup->add( &showPythonDebugInfo );
        pythonGroup->add( &defaultGrpcPortNumber );
        pythonGroup->add( &m_pythonExecutable );
#endif
        caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup( "Script files" );
        scriptGroup->add( &scriptDirectories );
        scriptGroup->add( &m_maxScriptFoldersDepth );
        scriptGroup->add( &scriptEditorExecutable );

        m_opmPreferences()->appendItems( uiOrdering );
    }
#ifdef USE_ODB_API
    else if ( uiConfigName == RiaPreferences::tabNameGeomech() )
    {
        m_geoMechPreferences()->appendItems( uiOrdering );
    }
#endif
    else if ( uiConfigName == RiaPreferences::tabNameImportExport() )
    {
        caf::PdmUiGroup* importGroup = uiOrdering.addNewGroup( "Import" );
        importGroup->add( &m_surfaceImportResamplingDistance );
        importGroup->add( &m_multiLateralWellPattern );

        caf::PdmUiGroup* exportGroup = uiOrdering.addNewGroup( "Export" );
        exportGroup->add( &csvTextExportFieldSeparator );
        exportGroup->add( &m_openExportedPdfInViewer );
        exportGroup->add( &m_writeEchoInGrdeclFiles );

        caf::PdmUiGroup* otherGroup = uiOrdering.addNewGroup( "Other" );
        otherGroup->add( &m_gridCalculationExpressionFolder );
        otherGroup->add( &m_summaryCalculationExpressionFolder );

        caf::PdmUiGroup* osduGroup = uiOrdering.addNewGroup( "OSDU" );
        osduGroup->setCollapsedByDefault();
        m_osduPreferences()->uiOrdering( uiConfigName, *osduGroup );
        osduGroup->add( &m_deleteOsduToken );

        caf::PdmUiGroup* sumoGroup = uiOrdering.addNewGroup( "SUMO" );
        sumoGroup->setCollapsedByDefault();
        m_sumoPreferences()->uiOrdering( uiConfigName, *sumoGroup );
        sumoGroup->add( &m_deleteSumoToken );
    }
    else if ( RiaApplication::enableDevelopmentFeatures() && uiConfigName == RiaPreferences::tabNameSystem() )
    {
        m_systemPreferences()->uiOrdering( uiConfigName, uiOrdering );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaPreferences::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_dateFormat )
    {
        for ( auto dateFormat : RiaQDateTimeTools::supportedDateFormats() )
        {
            QDate   exampleDate = QDate( 2019, 8, 16 );
            QString fullDateFormat =
                RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
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
                RiaQDateTimeTools::timeFormatString( timeFormat, RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );
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
void RiaPreferences::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_pageSize )
    {
        m_pageLeftMargin   = defaultMarginSize( m_pageSize() );
        m_pageRightMargin  = defaultMarginSize( m_pageSize() );
        m_pageTopMargin    = defaultMarginSize( m_pageSize() );
        m_pageBottomMargin = defaultMarginSize( m_pageSize() );
    }
    else if ( changedField == &m_guiTheme )
    {
        RiuGuiTheme::updateGuiTheme( m_guiTheme() );
    }
    else if ( changedField == &m_deleteOsduToken )
    {
        RicDeleteOsduTokenFeature::deleteUserToken();
        m_deleteOsduToken = false;
    }
    else if ( changedField == &m_deleteSumoToken )
    {
        RicDeleteSumoTokenFeature::deleteUserToken();
        m_deleteSumoToken = false;
    }
    else if ( changedField == &m_exportPreferences )
    {
        QString filePath = RiuFileDialogTools::getSaveFileName( nullptr, "Export Preferences", "", "XML files (*.xml);;All files(*.*)" );
        exportPreferenceValuesToFile( filePath );

        m_exportPreferences = false;
    }
    else if ( changedField == &m_importPreferences )
    {
        QString filePath = RiuFileDialogTools::getOpenFileName( nullptr, "Import Preferences", "", "XML files (*.xml);;All files(*.*)" );
        importPreferenceValuesFromFile( filePath );

        m_importPreferences = false;
    }
    else
    {
        m_summaryPreferences->fieldChangedByUi( changedField, oldValue, newValue );
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
QString RiaPreferences::tabNameGrid()
{
    return "Grid";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameSummary()
{
    return "Summary";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameGeomech()
{
    return "GeoMechanical";
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
QString RiaPreferences::tabNameSystem()
{
    return "System";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::tabNameImportExport()
{
    return "Import/Export";
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
    names << tabNameGrid();
    names << tabNameSummary();
    names << tabNamePlotting();
    names << tabNameScripting();
#ifdef USE_ODB_API
    names << tabNameGeomech();
#endif
    names << tabNameImportExport();

    if ( RiaApplication::enableDevelopmentFeatures() )
    {
        names << tabNameSystem();
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::useUndoRedo() const
{
    return m_useUndoRedo();
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
QString RiaPreferences::dateTimeFormat( RiaDefines::DateFormatComponents dateComponents, RiaDefines::TimeFormatComponents timeComponents ) const
{
    return QString( "%1 %2" )
        .arg( RiaQDateTimeTools::dateFormatString( m_dateFormat(), dateComponents ) )
        .arg( RiaQDateTimeTools::timeFormatString( m_timeFormat(), timeComponents ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferences::maxScriptFoldersDepth() const
{
    return m_maxScriptFoldersDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferences::maxPlotTemplateFoldersDepth() const
{
    return m_maxPlotTemplateFoldersDepth();
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
QString RiaPreferences::lastUsedPlotTemplateAbsolutePath() const
{
    return m_lastUsedPlotTemplate().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::setLastUsedPlotTemplatePath( const QString& templatePath )
{
    m_lastUsedPlotTemplate = templatePath;
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
bool RiaPreferences::writeEchoInGrdeclFiles() const
{
    return m_writeEchoInGrdeclFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ThemeEnum RiaPreferences::guiTheme() const
{
    return m_guiTheme();
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
    if ( !RiaGuiApplication::isRunning() ) return;

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
double RiaPreferences::surfaceImportResamplingDistance() const
{
    return m_surfaceImportResamplingDistance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::multiLateralWellNamePattern() const
{
    return m_multiLateralWellPattern;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::defaultMultiLateralWellNamePattern()
{
    return "?*Y*";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::gridCalculationExpressionFolder() const
{
    return m_gridCalculationExpressionFolder().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::summaryCalculationExpressionFolder() const
{
    return m_summaryCalculationExpressionFolder().path();
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
RiaDefines::RINavigationPolicy RiaPreferences::navigationPolicy() const
{
    return m_navigationPolicy();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaPreferences::defaultScaleFactorZ() const
{
    if ( m_defaultScaleFactorZ < 0.000001 ) return 0.000001;
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
bool RiaPreferences::showInfoBox() const
{
    return m_showInfoBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::showGridBox() const
{
    return m_showGridBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::pythonExecutable() const
{
    return m_pythonExecutable().trimmed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::octaveExecutable() const
{
    return m_octaveExecutable().trimmed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::octaveShowHeaderInfoWhenExecutingScripts() const
{
    return m_octaveShowHeaderInfoWhenExecutingScripts();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferences::octavePortNumber() const
{
    if ( m_octavePortNumber().first ) return m_octavePortNumber().second;

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::storeBackupOfProjectFiles() const
{
    return m_storeBackupOfProjectFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary* RiaPreferences::summaryPreferences() const
{
    return m_summaryPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSystem* RiaPreferences::systemPreferences() const
{
    return m_systemPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesGeoMech* RiaPreferences::geoMechPreferences() const
{
    return m_geoMechPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOpm* RiaPreferences::opmPreferences() const
{
    return m_opmPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOsdu* RiaPreferences::osduPreferences() const
{
    return m_osduPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSumo* RiaPreferences::sumoPreferences() const
{
    return m_sumoPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferences::enableFaultsByDefault() const
{
    return m_enableFaultsByDefault;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesGrid* RiaPreferences::gridPreferences() const
{
    return m_gridPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::importPreferenceValuesFromFile( const QString& fileName )
{
    if ( fileName.isEmpty() ) return;

    QFile file( fileName );
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QTextStream in( &file );
        QString     fileContent = in.readAll();
        file.close();
        if ( !fileContent.isEmpty() )
        {
            this->readObjectFromXmlString( fileContent, caf::PdmDefaultObjectFactory::instance() );
        }
        auto txt = "Imported preferences from " + fileName;
        RiaLogging::info( txt );
    }
    else
    {
        auto txt = QString( "Unable to open file: " ) + fileName;
        RiaLogging::error( txt );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::exportPreferenceValuesToFile( const QString& fileName )
{
    QFile file( fileName );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &file );

        auto content = this->writeObjectToXmlString();
        out << content;
        file.close();

        QString txt = "Exported preferences to " + fileName;
        RiaLogging::info( txt );
    }
    else
    {
        auto txt = QString( "Unable to open file: " ) + fileName;
        RiaLogging::error( txt );
    }
}
