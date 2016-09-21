/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicExportToLasFileResampleUi.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT(RicExportToLasFileResampleUi, "RicExportToLasFileResampleUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportToLasFileResampleUi::RicExportToLasFileResampleUi(void)
{
    CAF_PDM_InitObject("Resample LAS curves for export", "", "", "");

    CAF_PDM_InitField(&exportFolder,     "ExportFolder",     QString(), "Export Folder", "", "", "");
    exportFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&activateResample, "ActivateResample", false,     "Resample Curve Data", "", "", "");
    CAF_PDM_InitField(&resampleInterval, "ResampleInterval", 1.0,       "Resample Interval [m]", "", "", "");

    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &exportFolder)
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
void RicExportToLasFileResampleUi::updateFieldVisibility()
{
    if (activateResample)
    {
        resampleInterval.uiCapability()->setUiReadOnly(false);
    }
    else
    {
        resampleInterval.uiCapability()->setUiReadOnly(true);
    }
}
