/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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


#include "RifReaderSettings.h"
#include "cafPdmUiCheckBoxEditor.h"


CAF_PDM_SOURCE_INIT(RifReaderSettings, "RifReaderSettings");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderSettings::RifReaderSettings()
{
    CAF_PDM_InitObject("RifReaderSettings", "", "", "");

    CAF_PDM_InitField(&importFaults, "importFaults", true, "Import Faults", "", "", "");
    importFaults.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&importNNCs, "importSimulationNNCs", true, "Import NNCs", "", "", "");
    importNNCs.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&importAdvancedMswData, "importAdvancedMswData", false, "Import Advanced MSW Data", "", "", "");
    importAdvancedMswData.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&faultIncludeFileAbsolutePathPrefix, "faultIncludeFileAbsolutePathPrefix", QString(), "Fault Include File Absolute Path Prefix", "", "Path used to prefix absolute UNIX paths in fault include statements on Windows", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderSettings::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &importFaults ||
        field == &importAdvancedMswData ||
        field == &importNNCs)
    {
        caf::PdmUiCheckBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_useNativeCheckBoxLabel = true;
        }
    }
}

void RifReaderSettings::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&importFaults);
    uiOrdering.add(&importNNCs);
    uiOrdering.add(&importAdvancedMswData);
#ifdef WIN32
    uiOrdering.add(&faultIncludeFileAbsolutePathPrefix);
#endif
}

