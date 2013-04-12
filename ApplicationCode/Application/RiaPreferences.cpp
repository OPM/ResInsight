/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT(RiaPreferences, "RiaPreferences");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaPreferences::RiaPreferences(void)
{
    CAF_PDM_InitField(&navigationPolicy,                "navigationPolicy", caf::AppEnum<RiaApplication::RINavigationPolicy>(RiaApplication::NAVIGATION_POLICY_CAD), "Navigation mode", "", "", "");

    CAF_PDM_InitFieldNoDefault(&scriptDirectory,        "scriptDirectory", "Shared Script Folder", "", "", "");
    scriptDirectory.setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&scriptEditorExecutable,          "scriptEditorExecutable", QString("kate"), "Script Editor", "", "", "");
    scriptEditorExecutable.setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&octaveExecutable,                "octaveExecutable", QString("octave"), "Octave", "", "", "");
    octaveExecutable.setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&defaultGridLines,                "defaultGridLines", true, "Gridlines", "", "", "");
    CAF_PDM_InitField(&defaultScaleFactorZ,             "defaultScaleFactorZ", 5, "Z scale factor", "", "", "");

    CAF_PDM_InitField(&useShaders,                      "useShaders", true, "Use Shaders", "", "", "");
    CAF_PDM_InitField(&showHud,                         "showHud", false, "Show 3D Information", "", "", "");

    CAF_PDM_InitFieldNoDefault(&lastUsedProjectFileName,"lastUsedProjectFileName", "Last Used Project File", "", "", "");
    lastUsedProjectFileName.setUiHidden(true);

    CAF_PDM_InitField(&autocomputeSOIL,                 "autocomputeSOIL", true, "SOIL", "", "SOIL = 1.0 - SGAS - SWAT", "");
    CAF_PDM_InitField(&autocomputeDepthRelatedProperties,"autocomputeDepth", true, "DEPTH related properties", "", "DEPTH, DX, DY, DZ, TOP, BOTTOM", "");
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
    if (field == &scriptDirectory)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = static_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectDirectory = true;
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
    scriptGroup->add(&scriptDirectory);
    scriptGroup->add(&scriptEditorExecutable);
    scriptGroup->add(&octaveExecutable);

    caf::PdmUiGroup* defaultSettingsGroup = uiOrdering.addNewGroup("Default settings");
    defaultSettingsGroup->add(&defaultScaleFactorZ);
    defaultSettingsGroup->add(&defaultGridLines);

    caf::PdmUiGroup* autoComputeGroup = uiOrdering.addNewGroup("Compute when loading new case");
    autoComputeGroup->add(&autocomputeSOIL);
    autoComputeGroup->add(&autocomputeDepthRelatedProperties);
}

