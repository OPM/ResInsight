/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RimSummaryPlotExportSettings.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmPtrField.h"

CAF_PDM_SOURCE_INIT(RimSummaryPlotExportSettings, "RimSummaryPlotExportSettings");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotExportSettings::RimSummaryPlotExportSettings()
{
    CAF_PDM_InitObject("RimFractureExportSettings", "", "", "");

    CAF_PDM_InitFieldNoDefault(&fileName, "Filename", "Export filename", "", "", "");
    fileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotExportSettings::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &fileName)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = static_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}

