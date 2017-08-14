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

#include "RicExportCompletionDataSettingsUi.h"

namespace caf
{
    template<>
    void RicExportCompletionDataSettingsUi::ExportSplitType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::UNIFIED_FILE,                      "UNIFIED_FILE",                      "Unified File");
        addItem(RicExportCompletionDataSettingsUi::SPLIT_ON_WELL,                     "SPLIT_ON_WELL",                     "Split on Well");
        addItem(RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE, "SPLIT_ON_WELL_AND_COMPLETION_TYPE", "Split on Well and Completion Type");
        setDefault(RicExportCompletionDataSettingsUi::UNIFIED_FILE);
    }

    template<>
    void RicExportCompletionDataSettingsUi::WellSelectionType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::ALL_WELLS,     "ALL_WELLS",     "All Wells");
        addItem(RicExportCompletionDataSettingsUi::CHECKED_WELLS, "CHECKED_WELLS", "Checked Wells");
        addItem(RicExportCompletionDataSettingsUi::SELECTED_WELLS, "SELECTED_WELLS", "Selected Wells");
        setDefault(RicExportCompletionDataSettingsUi::ALL_WELLS);
    }

    template<>
    void RicExportCompletionDataSettingsUi::CompdatExportType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES, "TRANSMISSIBILITIES", "Calculated Transmissibilities");
        addItem(RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS, "WPIMULT_AND_DEFAULT_CONNECTION_FACTORS", "Default Connection Factors and WPIMULT (Fractures Not Supported)");
        setDefault(RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES);
    }
}


CAF_PDM_SOURCE_INIT(RicExportCompletionDataSettingsUi, "RicExportCompletionDataSettingsUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi::RicExportCompletionDataSettingsUi()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi::RicExportCompletionDataSettingsUi(bool onlyWellPathCollectionSelected)
{
    CAF_PDM_InitObject("RimExportCompletionDataSettings", "", "", "");

    CAF_PDM_InitFieldNoDefault(&fileSplit, "FileSplit", "File Split", "", "", "");
    CAF_PDM_InitFieldNoDefault(&wellSelection, "WellSelection", "Well Selection", "", "", "");
    CAF_PDM_InitFieldNoDefault(&compdatExport, "compdatExport", "Export", "", " ", "");

    CAF_PDM_InitField(&timeStep, "TimeStepIndex", 0, "Time Step", "", "", "");

    CAF_PDM_InitField(&includePerforations, "IncludePerforations", true, "Include Perforations", "", "", "");
    CAF_PDM_InitField(&includeFishbones, "IncludeFishbones", true, "Include Fishbones", "", "", "");
    CAF_PDM_InitField(&includeFractures, "IncludeFractures", true, "Include Fractures", "", "", "");

    CAF_PDM_InitField(&excludeMainBoreForFishbones, "ExcludeMainBoreForFishbones", false, "Exclude Main Bore Transmissibility For Fishbones", "", "", "");
    m_onlyWellPathCollectionSelected = onlyWellPathCollectionSelected;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showForSimWells()
{
    m_displayForSimWell = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showForWellPath()
{
    m_displayForSimWell = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &compdatExport)
    {
        if (compdatExport == WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
        {
            includeFractures = false;
            includeFractures.uiCapability()->setUiReadOnly(true);
        }
        else if (compdatExport == TRANSMISSIBILITIES)
        {
            includeFractures = true;
            includeFractures.uiCapability()->setUiReadOnly(false);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportCompletionDataSettingsUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &timeStep)
    {
        QStringList timeStepNames;

        if (caseToApply)
        {
            timeStepNames = caseToApply->timeStepStrings();
        }

        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }
    else if (fieldNeedingOptions == &wellSelection)
    {
        if (m_onlyWellPathCollectionSelected)
        {
            options.push_back(caf::PdmOptionItemInfo("All Wells", ALL_WELLS));
            options.push_back(caf::PdmOptionItemInfo("Checked Wells", CHECKED_WELLS));
        }
        else
        {
            options.push_back(caf::PdmOptionItemInfo("Selected Wells", SELECTED_WELLS));
        }
    }
    else
    {
        options = RicCaseAndFileExportSettingsUi::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* generalExportSettings = uiOrdering.addNewGroup("General Export Settings");
    generalExportSettings->add(&folder);
    generalExportSettings->add(&caseToApply);
    generalExportSettings->add(&compdatExport);
    
    generalExportSettings->add(&wellSelection);
    if(!m_onlyWellPathCollectionSelected) wellSelection.setValue(SELECTED_WELLS);

    generalExportSettings->add(&fileSplit);

    if (!m_displayForSimWell)
    {
        caf::PdmUiGroup* fishboneGroup = uiOrdering.addNewGroup("Export of Fishbone Completions");
        fishboneGroup->add(&includeFishbones);
        fishboneGroup->add(&excludeMainBoreForFishbones);
        if (!includeFishbones) excludeMainBoreForFishbones.uiCapability()->setUiReadOnly(true);
        else excludeMainBoreForFishbones.uiCapability()->setUiReadOnly(false);

        caf::PdmUiGroup* perfIntervalGroup = uiOrdering.addNewGroup("Export of Perforation Completions");
        perfIntervalGroup->add(&includePerforations);
        perfIntervalGroup->add(&timeStep);
        if (!includePerforations) timeStep.uiCapability()->setUiReadOnly(true);
        else  timeStep.uiCapability()->setUiReadOnly(false);
        
        caf::PdmUiGroup* fractureGroup = uiOrdering.addNewGroup("Export of Fracture Completions");
        fractureGroup->add(&includeFractures);
        
        if (compdatExport == WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
        {
            includeFractures.uiCapability()->setUiReadOnly(true);
        }
        else if (compdatExport == TRANSMISSIBILITIES)
        {
            includeFractures.uiCapability()->setUiReadOnly(false);
        }
    }

    uiOrdering.skipRemainingFields();
}
