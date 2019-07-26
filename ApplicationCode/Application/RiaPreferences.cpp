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
#include "RifReaderSettings.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiFilePathEditor.h"

namespace caf 
{
    template<>
    void RiaPreferences::SummaryRestartFilesImportModeType::setUp()
    {
        addItem(RiaPreferences::IMPORT, "IMPORT", "Unified");
        addItem(RiaPreferences::SEPARATE_CASES, "SEPARATE_CASES", "Separate Cases");
        addItem(RiaPreferences::NOT_IMPORT, "NOT_IMPORT", "Skip");
        setDefault(RiaPreferences::IMPORT);
    }
}


CAF_PDM_SOURCE_INIT(RiaPreferences, "RiaPreferences");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaPreferences::RiaPreferences(void)
{
    CAF_PDM_InitField(&navigationPolicy,                "navigationPolicy", caf::AppEnum<RiaGuiApplication::RINavigationPolicy>(RiaGuiApplication::NAVIGATION_POLICY_RMS), "Navigation Mode", "", "", "");

    CAF_PDM_InitField(&enableGrpcServer, "enableGrpcServer", true, "Enable gRPC script server", "", "Remote Procedure Call Scripting Engine", "");
    CAF_PDM_InitField(&defaultGrpcPortNumber, "defaultGrpcPort", 50051, "Default gRPC port", "", "", "");

    CAF_PDM_InitFieldNoDefault(&scriptDirectories,        "scriptDirectory", "Shared Script Folder(s)", "", "", "");
    scriptDirectories.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&scriptEditorExecutable,          "scriptEditorExecutable", QString("kate"), "Script Editor", "", "", "");
    scriptEditorExecutable.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&octaveExecutable,                "octaveExecutable", QString("octave"), "Octave Executable Location", "", "", "");
    octaveExecutable.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    octaveExecutable.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitField(&octaveShowHeaderInfoWhenExecutingScripts, "octaveShowHeaderInfoWhenExecutingScripts", false, "Show Text Header When Executing Scripts", "", "", "");
    octaveShowHeaderInfoWhenExecutingScripts.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&pythonExecutable, "pythonExecutable", QString("python"), "Python Executable Location", "", "", "");
    pythonExecutable.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    pythonExecutable.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitField(&ssihubAddress,                   "ssihubAddress", QString("http://"), "SSIHUB Address", "", "", "");
    ssihubAddress.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitFieldNoDefault(&defaultMeshModeType,    "defaultMeshModeType", "Show Grid Lines", "", "", "");
    CAF_PDM_InitField(&defaultGridLineColors,           "defaultGridLineColors", RiaColorTables::defaultGridLineColor(), "Mesh Color", "", "", "");
    CAF_PDM_InitField(&defaultFaultGridLineColors,      "defaultFaultGridLineColors", RiaColorTables::defaultFaultLineColor(), "Mesh Color Along Faults", "", "", "");
    CAF_PDM_InitField(&defaultWellLabelColor,           "defaultWellLableColor", RiaColorTables::defaultWellLabelColor(), "Well Label Color", "", "The default well label color in new views", "");

    CAF_PDM_InitField(&defaultViewerBackgroundColor,      "defaultViewerBackgroundColor", RiaColorTables::defaultViewerBackgroundColor(), "Viewer Background", "", "The viewer background color for new views", "");

    CAF_PDM_InitField(&defaultScaleFactorZ,                "defaultScaleFactorZ", 5, "Default Z Scale Factor", "", "", "");

    caf::AppEnum<RiaFontCache::FontSize> fontSize = RiaFontCache::FONT_SIZE_8;
    CAF_PDM_InitField(&defaultSceneFontSize,        "fontSizeInScene", fontSize,  "Viewer Font Size", "", "", "");
    CAF_PDM_InitField(&defaultAnnotationFontSize,  "defaultAnnotationFontSize", fontSize, "Annotation Font Size", "", "", "");
    CAF_PDM_InitField(&defaultWellLabelFontSize,   "wellLabelFontSize", fontSize, "Well Label Font Size", "", "", "");
    CAF_PDM_InitField(&defaultPlotFontSize,        "defaultPlotFontSize", fontSize, "Plot Font Size", "", "", "");

    CAF_PDM_InitField(&showLasCurveWithoutTvdWarning,   "showLasCurveWithoutTvdWarning", true, "Show LAS Curve Without TVD Warning", "", "", "");
    showLasCurveWithoutTvdWarning.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&useShaders,                      "useShaders", true, "Use Shaders", "", "", "");
    useShaders.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    CAF_PDM_InitField(&showHud,                         "showHud", false, "Show 3D Information", "", "", "");
    showHud.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    CAF_PDM_InitField(&m_appendClassNameToUiText,         "appendClassNameToUiText", false, "Show Class Names", "", "", "");
    m_appendClassNameToUiText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    CAF_PDM_InitField(&m_appendFieldKeywordToToolTipText, "appendFieldKeywordToToolTipText", false, "Show Field Keyword in ToolTip", "", "", "");
    m_appendFieldKeywordToToolTipText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    CAF_PDM_InitField(&m_showTestToolbar, "showTestToolbar", false, "Enable Test Toolbar", "", "", "");
    m_showTestToolbar.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    CAF_PDM_InitField(&m_includeFractureDebugInfoFile, "includeFractureDebugInfoFile", false, "Include Fracture Debug Info for Completion Export", "", "", "");
    m_includeFractureDebugInfoFile.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&showLegendBackground, "showLegendBackground", true, "Show Box around Legends", "", "", "");
    showLegendBackground.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&lastUsedProjectFileName,"lastUsedProjectFileName", "Last Used Project File", "", "", "");
    lastUsedProjectFileName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&autocomputeDepthRelatedProperties, "autocomputeDepth", true, "Compute DEPTH Related Properties", "", "DEPTH, DX, DY, DZ, TOP, BOTTOM", "");
    autocomputeDepthRelatedProperties.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    
    CAF_PDM_InitField(&loadAndShowSoil, "loadAndShowSoil", true, "Load and Show SOIL", "", "", "");
    loadAndShowSoil.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&summaryRestartFilesShowImportDialog, "summaryRestartFilesShowImportDialog", "Show Import Dialog", "", "", "");
    CAF_PDM_InitField(&summaryImportMode, "summaryImportMode", SummaryRestartFilesImportModeType(RiaPreferences::IMPORT), "Default Summary Import Option", "", "", "");
    CAF_PDM_InitField(&gridImportMode, "gridImportMode", SummaryRestartFilesImportModeType(RiaPreferences::NOT_IMPORT), "Default Grid Import Option", "", "", "");
    CAF_PDM_InitField(&summaryEnsembleImportMode, "summaryEnsembleImportMode", SummaryRestartFilesImportModeType(RiaPreferences::IMPORT), "Default Ensemble Summary Import Option", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_holoLensExportFolder, "holoLensExportFolder", "HoloLens Export Folder", "", "", "");
    m_holoLensExportFolder.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_holoLensExportFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&holoLensDisableCertificateVerification, "holoLensDisableCertificateVerification", false, "Disable SSL Certificate Verification (HoloLens)", "", "", "");
    holoLensDisableCertificateVerification.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&csvTextExportFieldSeparator, "csvTextExportFieldSeparator", QString(","), "CSV Text Export Field Separator", "", "", "");

    CAF_PDM_InitField(&m_showProjectChangedDialog, "showProjectChangedDialog", true, "Show 'Project has changed' dialog", "", "", "");
    m_showProjectChangedDialog.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_readerSettings,        "readerSettings", "Reader Settings", "", "", "");
    m_readerSettings = new RifReaderSettings;

    m_tabNames << "General";
    m_tabNames << "Eclipse";
    m_tabNames << "Scripting";
    m_tabNames << "Export";
    if (RiaApplication::enableDevelopmentFeatures())
    {
        m_tabNames << "System";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaPreferences::~RiaPreferences(void)
{
    delete m_readerSettings;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaPreferences::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    m_readerSettings->defineEditorAttribute(field, uiConfigName, attribute);

    if (field == &scriptDirectories)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectDirectory = true;
            myAttr->m_appendUiSelectedFolderToText = true;
        }
    }
    else if (field == &octaveShowHeaderInfoWhenExecutingScripts ||
            field == &autocomputeDepthRelatedProperties ||
            field == &loadAndShowSoil ||
            field == &useShaders ||
            field == &showHud ||
            field == &m_appendClassNameToUiText ||
            field == &m_appendFieldKeywordToToolTipText ||
            field == &m_showTestToolbar ||
            field == &m_includeFractureDebugInfoFile ||
            field == &showLasCurveWithoutTvdWarning ||
            field == &holoLensDisableCertificateVerification ||
            field == &m_showProjectChangedDialog ||
            field == &showLegendBackground)
    {
        caf::PdmUiCheckBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_useNativeCheckBoxLabel = true;
        }
    }
    else if (field == &m_holoLensExportFolder)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectDirectory = true;
        }
    }
    if (field == &defaultSceneFontSize || field == &defaultWellLabelFontSize ||
        field == &defaultAnnotationFontSize || field == &defaultPlotFontSize)
    {
        caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>(attribute);
        myAttr->minimumContentsLength = 2;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaPreferences::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
{
    if (uiConfigName == m_tabNames[0])
    {
        caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup("Default Colors");
        colorGroup->add(&defaultViewerBackgroundColor);
        colorGroup->add(&defaultGridLineColors, false);
        colorGroup->add(&defaultFaultGridLineColors);
        colorGroup->add(&defaultWellLabelColor, false);
        
        caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup("Default Font Sizes");
        fontGroup->add(&defaultSceneFontSize);
        fontGroup->add(&defaultAnnotationFontSize, false);
        fontGroup->add(&defaultWellLabelFontSize);
        fontGroup->add(&defaultPlotFontSize, false);
        
        caf::PdmUiGroup* viewsGroup = uiOrdering.addNewGroup("3d Views");
        viewsGroup->add(&defaultMeshModeType);
        viewsGroup->add(&navigationPolicy);
        viewsGroup->add(&defaultScaleFactorZ);
        viewsGroup->add(&showLegendBackground);
        viewsGroup->add(&useShaders);
        viewsGroup->add(&showHud);
        
        caf::PdmUiGroup* otherGroup = uiOrdering.addNewGroup("Other");
        otherGroup->add(&ssihubAddress);
        otherGroup->add(&showLasCurveWithoutTvdWarning);
        otherGroup->add(&holoLensDisableCertificateVerification);
    }
    else if (uiConfigName == m_tabNames[1])
    {
        caf::PdmUiGroup* newCaseBehaviourGroup = uiOrdering.addNewGroup("Behavior When Loading Data");
        newCaseBehaviourGroup->add(&autocomputeDepthRelatedProperties);
        newCaseBehaviourGroup->add(&loadAndShowSoil);
    
        m_readerSettings->defineUiOrdering(uiConfigName, *newCaseBehaviourGroup);
        
        caf::PdmUiGroup* restartBehaviourGroup = uiOrdering.addNewGroup("Origin Files");
        restartBehaviourGroup->add(&summaryRestartFilesShowImportDialog);

        {
            caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup("Origin Summary Files");
            group->add(&summaryImportMode);
        }

        {
            caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup("Origin Grid Files");
            group->add(&gridImportMode);
        }

        {
            caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup("Origin Ensemble Summary Files");
            group->add(&summaryEnsembleImportMode);
        }
    }
    else if (uiConfigName == m_tabNames[2])
    {
#ifdef ENABLE_GRPC
        caf::PdmUiGroup* grpcGroup = uiOrdering.addNewGroup("gRPC Server");
        grpcGroup->add(&enableGrpcServer);
        grpcGroup->add(&defaultGrpcPortNumber);
#endif

        caf::PdmUiGroup* octaveGroup = uiOrdering.addNewGroup("Octave");
        octaveGroup->add(&octaveExecutable);
        octaveGroup->add(&octaveShowHeaderInfoWhenExecutingScripts);

        caf::PdmUiGroup* pythonGroup = uiOrdering.addNewGroup("Python");
        pythonGroup->add(&pythonExecutable);

        caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup("Script files");
        scriptGroup->add(&scriptDirectories);
        scriptGroup->add(&scriptEditorExecutable);
    }
    else if (uiConfigName == m_tabNames[3])
    {
        uiOrdering.add(&csvTextExportFieldSeparator);
    }
    else if (RiaApplication::enableDevelopmentFeatures() && uiConfigName == m_tabNames[4])
    {
        uiOrdering.add(&m_appendClassNameToUiText);
        uiOrdering.add(&m_appendFieldKeywordToToolTipText);

        uiOrdering.add(&m_showProjectChangedDialog);

        uiOrdering.add(&m_showTestToolbar);
        uiOrdering.add(&m_includeFractureDebugInfoFile);
        uiOrdering.add(&m_holoLensExportFolder);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaPreferences::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if (fieldNeedingOptions == &gridImportMode)
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip(RiaPreferences::NOT_IMPORT);
        SummaryRestartFilesImportModeType separate(RiaPreferences::SEPARATE_CASES);

        options.push_back(caf::PdmOptionItemInfo(skip.uiText(), RiaPreferences::NOT_IMPORT));
        options.push_back(caf::PdmOptionItemInfo(separate.uiText(), RiaPreferences::SEPARATE_CASES));
    }
    else if (fieldNeedingOptions == &summaryEnsembleImportMode)
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip(RiaPreferences::NOT_IMPORT);
        SummaryRestartFilesImportModeType allowImport(RiaPreferences::IMPORT);

        options.push_back(caf::PdmOptionItemInfo(skip.uiText(), RiaPreferences::NOT_IMPORT));
        options.push_back(caf::PdmOptionItemInfo(allowImport.uiText(), RiaPreferences::IMPORT));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferences::initAfterRead()
{
    // If the stored font size is larger than the maximum enum value, the stored font size is actually point size
    int defaultSceneFontEnumValue = static_cast<int>(defaultSceneFontSize.v());
    if (defaultSceneFontEnumValue > (int) RiaFontCache::MAX_FONT_SIZE)
    {
        defaultSceneFontSize = RiaFontCache::fontSizeEnumFromPointSize(defaultSceneFontEnumValue);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RiaPreferences::tabNames()
{
    return m_tabNames;
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
    if (!RiaApplication::enableDevelopmentFeatures())
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
std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> RiaPreferences::defaultFontSizes() const
{
    std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> fontSizes;
    fontSizes[RiaDefines::SCENE_FONT]      = defaultSceneFontSize();
    fontSizes[RiaDefines::ANNOTATION_FONT] = defaultAnnotationFontSize();
    fontSizes[RiaDefines::WELL_LABEL_FONT] = defaultWellLabelFontSize();
    fontSizes[RiaDefines::PLOT_FONT]       = defaultPlotFontSize();
    return fontSizes;
}
