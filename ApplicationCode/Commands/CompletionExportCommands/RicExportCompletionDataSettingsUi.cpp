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
    void RicExportCompletionDataSettingsUi::CompdatExportType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES, "TRANSMISSIBILITIES", "Calculated Transmissibilities");
        addItem(RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS, "WPIMULT_AND_DEFAULT_CONNECTION_FACTORS", "Default Connection Factors and WPIMULT (Fractures Not Supported)");
        setDefault(RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES);
    }

    template<>
    void RicExportCompletionDataSettingsUi::CombinationModeType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::INDIVIDUALLY,    "INDIVIDUALLY", "Individually");
        addItem(RicExportCompletionDataSettingsUi::COMBINED,        "COMBINED",     "Combined");
        setDefault(RicExportCompletionDataSettingsUi::INDIVIDUALLY);
    }
}


CAF_PDM_SOURCE_INIT(RicExportCompletionDataSettingsUi, "RicExportCompletionDataSettingsUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi::RicExportCompletionDataSettingsUi()
{
    CAF_PDM_InitObject("RimExportCompletionDataSettings", "", "", "");

    CAF_PDM_InitFieldNoDefault(&fileSplit, "FileSplit", "File Split", "", "", "");

    CAF_PDM_InitFieldNoDefault(&compdatExport, "compdatExport", "Export", "", " ", "");

    CAF_PDM_InitField(&timeStep, "TimeStepIndex", 0, "  Time Step", "", "", "");

    CAF_PDM_InitField(&useLateralNTG, "UseLateralNTG", false, "Use NTG Horizontally", "", "", "");

    CAF_PDM_InitField(&includePerforations, "IncludePerforations", true, "Perforations", "", "", "");
    CAF_PDM_InitField(&includeFishbones, "IncludeFishbones", true, "Fishbones", "", "", "");
    CAF_PDM_InitField(&includeFractures, "IncludeFractures", true, "Fractures", "", "", "");

    CAF_PDM_InitField(&excludeMainBoreForFishbones, "ExcludeMainBoreForFishbones", false, "  Exclude Main Bore Transmissibility", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_reportCompletionTypesSeparately, "ReportCompletionTypesSeparately", "Export Completion Types", "", "", "");

    m_displayForSimWell = true;
    
    m_fracturesEnabled = true;
    m_perforationsEnabled = true;
    m_fishbonesEnabled = true;
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
void RicExportCompletionDataSettingsUi::setCombinationMode(CombinationMode combinationMode)
{
    m_reportCompletionTypesSeparately = combinationMode;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showFractureInUi(bool enable)
{
    m_fracturesEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showPerforationsInUi(bool enable)
{
    m_perforationsEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showFishbonesInUi(bool enable)
{
    m_fishbonesEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionDataSettingsUi::reportCompletionsTypesIndividually() const
{
    return m_reportCompletionTypesSeparately() == INDIVIDUALLY;
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
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("File Settings");
    
        group->add(&folder);
        group->add(&fileSplit);
        group->add(&m_reportCompletionTypesSeparately);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Settings");

        group->add(&caseToApply);
        group->add(&compdatExport);
        group->add(&useLateralNTG);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Visible Completions");
        if (!m_displayForSimWell)
        {
            if (m_perforationsEnabled)
            {
                group->add(&includePerforations);
                group->add(&timeStep);
                if (!includePerforations)
                    timeStep.uiCapability()->setUiReadOnly(true);
                else
                    timeStep.uiCapability()->setUiReadOnly(false);
            }
        }

        if (m_fracturesEnabled)
        {
            group->add(&includeFractures);

            if (compdatExport == WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
            {
                includeFractures.uiCapability()->setUiReadOnly(true);
            }
            else if (compdatExport == TRANSMISSIBILITIES)
            {
                includeFractures.uiCapability()->setUiReadOnly(false);
            }
        }

        if (!m_displayForSimWell)
        {
            if (m_fishbonesEnabled)
            {
                group->add(&includeFishbones);
                group->add(&excludeMainBoreForFishbones);
                if (!includeFishbones)
                    excludeMainBoreForFishbones.uiCapability()->setUiReadOnly(true);
                else
                    excludeMainBoreForFishbones.uiCapability()->setUiReadOnly(false);
            }
        }
    }

    uiOrdering.skipRemainingFields();
}
