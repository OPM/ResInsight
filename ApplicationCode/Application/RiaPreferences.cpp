/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
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

#include "RifReaderSettings.h"

#include "cafPdmFieldCvfColor.h"
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
    CAF_PDM_InitField(&navigationPolicy,                "navigationPolicy", caf::AppEnum<RiaApplication::RINavigationPolicy>(RiaApplication::NAVIGATION_POLICY_RMS), "Navigation Mode", "", "", "");

    CAF_PDM_InitFieldNoDefault(&scriptDirectories,        "scriptDirectory", "Shared Script Folder(s)", "", "", "");
    scriptDirectories.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&scriptEditorExecutable,          "scriptEditorExecutable", QString("kate"), "Script Editor", "", "", "");
    scriptEditorExecutable.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&octaveExecutable,                "octaveExecutable", QString("octave"), "Octave Executable Location", "", "", "");
    octaveExecutable.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    octaveExecutable.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitField(&octaveShowHeaderInfoWhenExecutingScripts, "octaveShowHeaderInfoWhenExecutingScripts", false, "Show Text Header When Executing Scripts", "", "", "");
    octaveShowHeaderInfoWhenExecutingScripts.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&ssihubAddress,                   "ssihubAddress", QString("http://"), "SSIHUB Address", "", "", "");
    ssihubAddress.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitField(&defaultGridLines,                "defaultGridLines", true, "Gridlines", "", "", "");
    CAF_PDM_InitField(&defaultGridLineColors,           "defaultGridLineColors", cvf::Color3f(0.92f, 0.92f, 0.92f), "Mesh Color", "", "", "");
    CAF_PDM_InitField(&defaultFaultGridLineColors,      "defaultFaultGridLineColors", cvf::Color3f(0.08f, 0.08f, 0.08f), "Mesh Color Along Faults", "", "", "");
    CAF_PDM_InitField(&defaultWellLabelColor,           "defaultWellLableColor", cvf::Color3f(0.92f, 0.92f, 0.92f), "Well Label Color", "", "The default well label color in new views", "");

    CAF_PDM_InitField(&defaultViewerBackgroundColor,      "defaultViewerBackgroundColor", cvf::Color3f(0.69f, 0.77f, 0.87f), "Viewer Background", "", "The viewer background color for new views", "");

    CAF_PDM_InitField(&defaultScaleFactorZ,             "defaultScaleFactorZ", 5, "Default Z Scale Factor", "", "", "");
    CAF_PDM_InitField(&fontSizeInScene,                 "fontSizeInScene", QString("8"), "Font Size", "", "", "");

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

    CAF_PDM_InitField(&showLegendBackground, "showLegendBackground", true, "Enable Legend Background", "", "", "");

    CAF_PDM_InitFieldNoDefault(&lastUsedProjectFileName,"lastUsedProjectFileName", "Last Used Project File", "", "", "");
    lastUsedProjectFileName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&autocomputeDepthRelatedProperties, "autocomputeDepth", true, "Compute DEPTH Related Properties", "", "DEPTH, DX, DY, DZ, TOP, BOTTOM", "");
    autocomputeDepthRelatedProperties.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    
    CAF_PDM_InitField(&loadAndShowSoil, "loadAndShowSoil", true, "Load and Show SOIL", "", "", "");
    loadAndShowSoil.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&summaryRestartFilesShowImportDialog, "summaryRestartFilesShowImportDialog", "Show Import Dialog", "", "", "");
    CAF_PDM_InitField(&summaryImportMode, "summaryImportMode", SummaryRestartFilesImportModeType(RiaPreferences::IMPORT), "Default Summary Import Option", "", "", "");
    CAF_PDM_InitField(&gridImportMode, "gridImportMode", SummaryRestartFilesImportModeType(RiaPreferences::NOT_IMPORT), "Default Grid Import Option", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_readerSettings,        "readerSettings", "Reader Settings", "", "", "");
    m_readerSettings = new RifReaderSettings;

    m_tabNames << "General";
    m_tabNames << "Eclipse";
    m_tabNames << "Octave";
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
            field == &showLasCurveWithoutTvdWarning)
    {
        caf::PdmUiCheckBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_useNativeCheckBoxLabel = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaPreferences::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
{
    if (uiConfigName == m_tabNames[0])
    {
        caf::PdmUiGroup* defaultSettingsGroup = uiOrdering.addNewGroup("Default Settings");
        defaultSettingsGroup->add(&defaultViewerBackgroundColor);
        defaultSettingsGroup->add(&defaultGridLines);
        defaultSettingsGroup->add(&defaultGridLineColors);
        defaultSettingsGroup->add(&defaultFaultGridLineColors);
        defaultSettingsGroup->add(&defaultWellLabelColor);
        defaultSettingsGroup->add(&fontSizeInScene);
        defaultSettingsGroup->add(&defaultScaleFactorZ);
        defaultSettingsGroup->add(&showLegendBackground);

        caf::PdmUiGroup* viewsGroup = uiOrdering.addNewGroup("3D Views");
        viewsGroup->add(&navigationPolicy);
        viewsGroup->add(&useShaders);
        viewsGroup->add(&showHud);


        caf::PdmUiGroup* otherGroup = uiOrdering.addNewGroup("Other");
        otherGroup->add(&ssihubAddress);
        otherGroup->add(&showLasCurveWithoutTvdWarning);

    }
    else if (uiConfigName == m_tabNames[1])
    {
        caf::PdmUiGroup* newCaseBehaviourGroup = uiOrdering.addNewGroup("Behavior When Loading Data");
        newCaseBehaviourGroup->add(&autocomputeDepthRelatedProperties);
        newCaseBehaviourGroup->add(&loadAndShowSoil);
    
        m_readerSettings->defineUiOrdering(uiConfigName, *newCaseBehaviourGroup);

        caf::PdmUiGroup* restartBehaviourGroup = uiOrdering.addNewGroup("Origin Files");
        restartBehaviourGroup->add(&summaryRestartFilesShowImportDialog);
        caf::PdmUiGroup* summaryImportOptionGroup = restartBehaviourGroup->addNewGroup("Origin Summary Files");
        summaryImportOptionGroup->add(&summaryImportMode);
        caf::PdmUiGroup* gridImportOptionGroup = restartBehaviourGroup->addNewGroup("Origin Grid Files");
        gridImportOptionGroup->add(&gridImportMode);
    }
    else if (uiConfigName == m_tabNames[2])
    {
        caf::PdmUiGroup* octaveGroup = uiOrdering.addNewGroup("Octave");
        octaveGroup->add(&octaveExecutable);
        octaveGroup->add(&octaveShowHeaderInfoWhenExecutingScripts);

        caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup("Script files");
        scriptGroup->add(&scriptDirectories);
        scriptGroup->add(&scriptEditorExecutable);
    }
    else if (RiaApplication::enableDevelopmentFeatures() && uiConfigName == m_tabNames[3])
    {
        uiOrdering.add(&m_appendClassNameToUiText);
        uiOrdering.add(&m_appendFieldKeywordToToolTipText);
        uiOrdering.add(&m_showTestToolbar);
        uiOrdering.add(&m_includeFractureDebugInfoFile);
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

    if (&fontSizeInScene == fieldNeedingOptions)
    {
        QStringList fontSizes;
        fontSizes <<  "8";
        fontSizes << "12";
        fontSizes << "16";
        fontSizes << "24";
        fontSizes << "32";

        for (int oIdx = 0; oIdx < fontSizes.size(); ++oIdx)
        {
            options.push_back(caf::PdmOptionItemInfo(fontSizes[oIdx], fontSizes[oIdx]));
        }
    }
    else if (fieldNeedingOptions == &gridImportMode)
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip(RiaPreferences::NOT_IMPORT);
        SummaryRestartFilesImportModeType separate(RiaPreferences::SEPARATE_CASES);

        options.push_back(caf::PdmOptionItemInfo(skip.uiText(), RiaPreferences::NOT_IMPORT));
        options.push_back(caf::PdmOptionItemInfo(separate.uiText(), RiaPreferences::SEPARATE_CASES));
    }

    return options;
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

