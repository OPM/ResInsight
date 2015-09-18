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

#include "RiaStdInclude.h"
#include "RiaPreferences.h"

#include "RifReaderSettings.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT(RiaPreferences, "RiaPreferences");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaPreferences::RiaPreferences(void)
{
    CAF_PDM_InitField(&navigationPolicy,                "navigationPolicy", caf::AppEnum<RiaApplication::RINavigationPolicy>(RiaApplication::NAVIGATION_POLICY_CEETRON), "Navigation mode", "", "", "");

    CAF_PDM_InitFieldNoDefault(&scriptDirectories,        "scriptDirectory", "Shared Script Folder(s)", "", "", "");
    scriptDirectories.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&scriptEditorExecutable,          "scriptEditorExecutable", QString("kate"), "Script Editor", "", "", "");
    scriptEditorExecutable.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&octaveExecutable,                "octaveExecutable", QString("octave"), "Octave executable location", "", "", "");
    octaveExecutable.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    octaveExecutable.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitField(&octaveShowHeaderInfoWhenExecutingScripts, "octaveShowHeaderInfoWhenExecutingScripts", false, "Show text header when executing scripts", "", "", "");
    octaveShowHeaderInfoWhenExecutingScripts.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&ssihubAddress,                   "ssihubAddress", QString("http://"), "ssihub Address", "", "", "");

    CAF_PDM_InitField(&defaultGridLines,                "defaultGridLines", true, "Gridlines", "", "", "");
    CAF_PDM_InitField(&defaultGridLineColors,           "defaultGridLineColors", cvf::Color3f(0.92f, 0.92f, 0.92f), "Mesh color", "", "", "");
    CAF_PDM_InitField(&defaultFaultGridLineColors,      "defaultFaultGridLineColors", cvf::Color3f(0.08f, 0.08f, 0.08f), "Mesh color along faults", "", "", "");
    CAF_PDM_InitField(&defaultWellLabelColor,           "defaultWellLableColor", cvf::Color3f(0.92f, 0.92f, 0.92f), "Well label color", "", "The default well label color in new views", "");

    CAF_PDM_InitField(&defaultViewerBackgroundColor,      "defaultViewerBackgroundColor", cvf::Color3f(0.69f, 0.77f, 0.87f), "Viewer background", "", "The viewer background color for new views", "");

    CAF_PDM_InitField(&defaultScaleFactorZ,             "defaultScaleFactorZ", 5, "Z scale factor", "", "", "");

    CAF_PDM_InitField(&useShaders,                      "useShaders", true, "Use Shaders", "", "", "");
    CAF_PDM_InitField(&showHud,                         "showHud", false, "Show 3D Information", "", "", "");
    CAF_PDM_InitField(&appendClassNameToUiText,         "appendClassNameToUiText", false, "[System] Show Class Names", "", "", "");

    CAF_PDM_InitFieldNoDefault(&lastUsedProjectFileName,"lastUsedProjectFileName", "Last Used Project File", "", "", "");
    lastUsedProjectFileName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&autocomputeDepthRelatedProperties, "autocomputeDepth", true, "Compute DEPTH related properties", "", "DEPTH, DX, DY, DZ, TOP, BOTTOM", "");
    autocomputeDepthRelatedProperties.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    
    CAF_PDM_InitField(&loadAndShowSoil, "loadAndShowSoil", true, "Load and show SOIL", "", "", "");
    loadAndShowSoil.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    readerSettings = new RifReaderSettings;
    CAF_PDM_InitFieldNoDefault(&readerSettings,        "readerSettings", "Reader settings", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaPreferences::~RiaPreferences(void)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaPreferences::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    readerSettings->defineEditorAttribute(field, uiConfigName, attribute);

    if (field == &scriptDirectories)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = static_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectDirectory = true;
            myAttr->m_appendUiSelectedFolderToText = true;
        }
    }
    else if (field == &octaveShowHeaderInfoWhenExecutingScripts ||
            field == &autocomputeDepthRelatedProperties ||
            field == &loadAndShowSoil )
    {
        caf::PdmUiCheckBoxEditorAttribute* myAttr = static_cast<caf::PdmUiCheckBoxEditorAttribute*>(attribute);
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
    uiOrdering.add(&navigationPolicy);

    caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup("Script configuration");
    scriptGroup->add(&scriptDirectories);
    scriptGroup->add(&scriptEditorExecutable);

    caf::PdmUiGroup* octaveGroup = uiOrdering.addNewGroup("Octave");
    octaveGroup->add(&octaveExecutable);
    octaveGroup->add(&octaveShowHeaderInfoWhenExecutingScripts);

    caf::PdmUiGroup* defaultSettingsGroup = uiOrdering.addNewGroup("Default settings");
    defaultSettingsGroup->add(&defaultScaleFactorZ);
    defaultSettingsGroup->add(&defaultViewerBackgroundColor);
    defaultSettingsGroup->add(&defaultGridLines);
    defaultSettingsGroup->add(&defaultGridLineColors);
    defaultSettingsGroup->add(&defaultFaultGridLineColors);
    defaultSettingsGroup->add(&defaultWellLabelColor);

    caf::PdmUiGroup* autoComputeGroup = uiOrdering.addNewGroup("Behavior when loading new case");
    autoComputeGroup->add(&autocomputeDepthRelatedProperties);
    autoComputeGroup->add(&loadAndShowSoil);
    
    caf::PdmUiGroup* readerSettingsGroup = uiOrdering.addNewGroup("Reader settings");
    std::vector<caf::PdmFieldHandle*> readerSettingsFields;
    readerSettings->fields(readerSettingsFields);
    for (size_t i = 0; i < readerSettingsFields.size(); i++)
    {
        readerSettingsGroup->add(readerSettingsFields[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// This function is called as part of the regression test system to make sure the configuration
/// for regression tests is consistent
//--------------------------------------------------------------------------------------------------
void RiaPreferences::configureForRegressionTests()
{
    useShaders = true;
    showHud = false;

    autocomputeDepthRelatedProperties = true;
    loadAndShowSoil = true;

    CVF_ASSERT(readerSettings);
    readerSettings->importFaults = false;
}

