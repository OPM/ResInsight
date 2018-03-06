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

#include "RiaRegressionTest.h"

#include "cafPdmSettings.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiTextEditor.h"


CAF_PDM_SOURCE_INIT(RiaRegressionTest, "RiaRegressionTest");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaRegressionTest::RiaRegressionTest(void)
{
    CAF_PDM_InitFieldNoDefault(&applicationWorkingFolder,        "workingFolder", "Folder containing <b>compare</b>", "", "Location of compare tool from Image Magic suite", "");
    applicationWorkingFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    
    CAF_PDM_InitFieldNoDefault(&regressionTestFolder, "regressionTestFolder", "Regression Test Folder", "", "", "");
    regressionTestFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&showInteractiveDiffImages, "showInteractiveDiffImages", false, "Interactive Diff Images", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&testFilter,   "testFilter", "Test Filter", "", "If empty, all tests are executed.\nTo execute a subset of tests, specify folder names separated by ;", "");
    testFilter.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaRegressionTest::~RiaRegressionTest(void)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaRegressionTest::writeSettingsToApplicationStore() const
{
    caf::PdmSettings::writeFieldsToApplicationStore(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaRegressionTest::readSettingsFromApplicationStore()
{
    caf::PdmSettings::readFieldsFromApplicationStore(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaRegressionTest::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &applicationWorkingFolder || field == &regressionTestFolder)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectDirectory = true;
        }
    }
}


