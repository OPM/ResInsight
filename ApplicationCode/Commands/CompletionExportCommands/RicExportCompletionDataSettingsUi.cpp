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

#include "RiaApplication.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

// clang-format off
namespace caf
{
    template<>
    void RicExportCompletionDataSettingsUi::ExportSplitType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::UNIFIED_FILE,                      "UNIFIED_FILE",                      "Unified File");
        addItem(RicExportCompletionDataSettingsUi::SPLIT_ON_WELL,                     "SPLIT_ON_WELL",                     "Split on Well");
        addItem(RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE, "SPLIT_ON_WELL_AND_COMPLETION_TYPE", "Split on Well and Completion Type");
        setDefault(RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE);
    }

    template<>
    void RicExportCompletionDataSettingsUi::CompdatExportType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES, "TRANSMISSIBILITIES", "Calculated Transmissibilities");
        addItem(RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS, "WPIMULT_AND_DEFAULT_CONNECTION_FACTORS", "Default Connection Factors and WPIMULT (Fractures Not Supported)");
#ifdef _DEBUG
        addItem(RicExportCompletionDataSettingsUi::NO_COMPLETIONS, "NO_COMPLETIONS", "None");
#endif
        setDefault(RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES);
    }

    template<>
    void RicExportCompletionDataSettingsUi::CombinationModeType::setUp()
    {
        addItem(RicExportCompletionDataSettingsUi::INDIVIDUALLY,    "INDIVIDUALLY", "Individually");
        addItem(RicExportCompletionDataSettingsUi::COMBINED,        "COMBINED",     "Combined");
        setDefault(RicExportCompletionDataSettingsUi::INDIVIDUALLY);
    }

    template<>
    void RicExportCompletionDataSettingsUi::TransScalingType::setUp()
    {
        addItem(RicExportFractureCompletionsImpl::NO_SCALING, "NO_SCALING", "No scaling");
        addItem(RicExportFractureCompletionsImpl::MATRIX_TO_WELL_DP_OVER_INITIAL_DP, "MATWELL_DP_OVER_INITIALDP", "Matrix to Well dP over initial dP");
        addItem(RicExportFractureCompletionsImpl::MATRIX_TO_WELL_DP_OVER_MAX_INITIAL_DP, "MATWELL_DP_OVER_MAX_INITIALDP", "Matrix to Well dP over max initial dP");

        setDefault(RicExportFractureCompletionsImpl::NO_SCALING);
    }

    template<>
    void RicExportCompletionDataSettingsUi::TransScalingInitialWBHP::setUp()
    {
        addItem(RicExportFractureCompletionsImpl::FROM_PRODUCTION_START, "PROD_START", "Initial WBHP From Production Start");
        addItem(RicExportFractureCompletionsImpl::FROM_PRODUCTION_START_W_MIN, "PROD_START_MIN", "Initial WBHP From Production Start With Minimum WBHP");
        addItem(RicExportFractureCompletionsImpl::FIXED_INITIAL_WBHP, "FIXED_INITIAL_WBHP", "Fixed Initial WBHP");

        setDefault(RicExportFractureCompletionsImpl::FROM_PRODUCTION_START);
    }
}
// clang-format on

CAF_PDM_SOURCE_INIT(RicExportCompletionDataSettingsUi, "RicExportCompletionDataSettingsUi");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi::RicExportCompletionDataSettingsUi()
{
    CAF_PDM_InitObject("RimExportCompletionDataSettings", "", "", "");

    CAF_PDM_InitFieldNoDefault(&fileSplit, "FileSplit", "File Split", "", "", "");

    CAF_PDM_InitFieldNoDefault(&compdatExport, "compdatExport", "Export", "", " ", "");

    CAF_PDM_InitField(&timeStep, "TimeStepIndex", 0, "    Time Step", "", "", "");

    CAF_PDM_InitField(&includeMsw, "IncludeMSW", false, "Include Multi Segment Well Model", "", "", "");

    CAF_PDM_InitField(&useLateralNTG, "UseLateralNTG", false, "Use NTG Horizontally", "", "", "");

    CAF_PDM_InitField(&includePerforations, "IncludePerforations", true, "Perforations", "", "", "");
    CAF_PDM_InitField(&includeFishbones, "IncludeFishbones", true, "Fishbones", "", "", "");
    CAF_PDM_InitField(&includeFractures, "IncludeFractures", true, "Fractures", "", "", "");

    CAF_PDM_InitFieldNoDefault(&transScalingType, "TransScalingType", "  Pressure Diff. Depletion Transmissibility Scaling", "", "", "");
    CAF_PDM_InitField(&transScalingTimeStep, "TransScalingTimeStep", 0, "  PDD Current WBHP Time Step", "", "", "");
    CAF_PDM_InitFieldNoDefault(&transScalingInitialWBHP, "TransScalingInitialWBHP", "  PDD Initial WBHP Selection", "", "", "");
    CAF_PDM_InitField(&transScalingWBHP, "TransScalingWBHP", 200.0, "  PDD Initial WBHP Value", "", "", "");

    CAF_PDM_InitField(&excludeMainBoreForFishbones,
                      "ExcludeMainBoreForFishbones",
                      false,
                      "    Exclude Main Bore Transmissibility",
                      "",
                      "Main bore perforation intervals are defined by start/end MD of each active fishbone sub definition",
                      "");

    CAF_PDM_InitFieldNoDefault(
        &m_reportCompletionTypesSeparately, "ReportCompletionTypesSeparately", "Export Completion Types", "", "", "");

    m_displayForSimWell = true;

    m_fracturesEnabled    = true;
    m_perforationsEnabled = true;
    m_fishbonesEnabled    = true;
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
void RicExportCompletionDataSettingsUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                         const QVariant&            oldValue,
                                                         const QVariant&            newValue)
{
    if (changedField == &compdatExport)
    {
        if (compdatExport == WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
        {
            includeFractures = false;
        }
        else if (compdatExport == TRANSMISSIBILITIES || includeMsw)
        {
            includeFractures = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicExportCompletionDataSettingsUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
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
    else if (fieldNeedingOptions == &transScalingTimeStep)
    {
        std::map<int, std::vector<std::pair<QString, QString>>> wellProductionStartStrings = generateWellProductionStartStrings();

        QStringList timeStepNames;

        if (caseToApply)
        {
            timeStepNames = caseToApply->timeStepStrings();
        }
        for (int i = 0; i < timeStepNames.size(); i++)
        {
            QString timeStepString = timeStepNames[i];
            auto it = wellProductionStartStrings.find(i);
            if (it != wellProductionStartStrings.end())
            {
                int numberOfWells = static_cast<int>(it->second.size());
                QStringList wellList;
                QStringList wellPressureList;
                const int maxStringLength = 70;
                QString startStringFormat(" [Start: %1]");

                for (int w = 0; w < numberOfWells; ++w)
                {
                    QString wellString = it->second[w].first;
                    QStringList candidateWellList = wellList; candidateWellList << wellString;

                    if (startStringFormat.arg(candidateWellList.join(", ")).length() < maxStringLength)
                    {
                        wellList = candidateWellList;
                    }

                    QString wellStringWithPressure = QString("%1 (%2)").arg(it->second[w].first).arg(it->second[w].second);
                    QStringList candidateWellPressureList = wellPressureList; candidateWellPressureList << wellStringWithPressure;
                    if (startStringFormat.arg(candidateWellPressureList.join(", ")).length() < maxStringLength)
                    {
                        wellPressureList = candidateWellPressureList;
                    }
                }
                
                if (wellList.size() < numberOfWells)
                {
                    wellList += QString("+ %1 more").arg(numberOfWells - wellList.size());
                    timeStepString += startStringFormat.arg(wellList.join(", "));
                }
                else if (wellPressureList.size() < numberOfWells)
                {
                    timeStepString += startStringFormat.arg(wellList.join(", "));
                }
                else
                {
                    timeStepString += startStringFormat.arg(wellPressureList.join(", "));
                }
            }

            options.push_back(caf::PdmOptionItemInfo(timeStepString, i));
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
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Export Settings");
        group->add(&compdatExport);
        group->add(&caseToApply);
        group->add(&useLateralNTG);
        group->add(&includeMsw);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("File Settings");
        group->add(&fileSplit);
        group->add(&m_reportCompletionTypesSeparately);
        group->add(&folder);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Completions Export Selection");
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

            if (RiaApplication::enableDevelopmentFeatures())
            {
                group->add(&transScalingType);
                group->add(&transScalingTimeStep);
                group->add(&transScalingInitialWBHP);
                group->add(&transScalingWBHP);

                if (transScalingType() == RicExportFractureCompletionsImpl::NO_SCALING)
                {
                    transScalingTimeStep.uiCapability()->setUiReadOnly(true);
                    transScalingInitialWBHP.uiCapability()->setUiReadOnly(true);
                    transScalingWBHP.uiCapability()->setUiReadOnly(true);
                }
                else
                {
                    transScalingTimeStep.uiCapability()->setUiReadOnly(false);
                    transScalingInitialWBHP.uiCapability()->setUiReadOnly(false);
                    if (transScalingInitialWBHP == RicExportFractureCompletionsImpl::FROM_PRODUCTION_START)
                    {
                        transScalingWBHP.uiCapability()->setUiReadOnly(true);
                    }
                    else
                    {
                        transScalingWBHP.uiCapability()->setUiReadOnly(false);
                        if (transScalingInitialWBHP == RicExportFractureCompletionsImpl::FROM_PRODUCTION_START_W_MIN)
                        {
                            transScalingWBHP.uiCapability()->setUiName("  PDD Minimum Initial WBHP");
                        }
                        else
                        {
                            transScalingWBHP.uiCapability()->setUiName("  PDD Fixed Initial WBHP");
                        }
                    }
                }
            }

            // Set visibility
            includeFractures.uiCapability()->setUiHidden(compdatExport == WPIMULT_AND_DEFAULT_CONNECTION_FACTORS && !includeMsw);
        }

        if (!m_displayForSimWell)
        {
            if (m_fishbonesEnabled)
            {
                group->add(&includeFishbones);
                group->add(&excludeMainBoreForFishbones);

                // Set visibility
                if (!includeFishbones)
                    excludeMainBoreForFishbones.uiCapability()->setUiReadOnly(true);
                else
                    excludeMainBoreForFishbones.uiCapability()->setUiReadOnly(false);
            }
        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, std::vector<std::pair<QString, QString>>> RicExportCompletionDataSettingsUi::generateWellProductionStartStrings()
{
    std::map<int, std::vector<std::pair<QString, QString>>> wellProductionStartStrings;

    const RimProject* project = nullptr;
    if (caseToApply)
    {
        caseToApply->firstAncestorOrThisOfTypeAsserted(project);
        for (const RimWellPath* wellPath : project->allWellPaths())
        {
            int    initialWellProductionTimeStep = -1;
            double initialWellPressure = 0.0;
            double currentWellPressure = 0.0;
            RicExportFractureCompletionsImpl::getWellPressuresAndInitialProductionTimeStepFromSummaryData(caseToApply,
                wellPath->completions()->wellNameForExport(),
                0,
                &initialWellProductionTimeStep,
                &initialWellPressure,
                &currentWellPressure);
            if (initialWellProductionTimeStep >= 0)
            {
                QString pressureUnits = RiaEclipseUnitTools::unitStringPressure(wellPath->unitSystem());
                wellProductionStartStrings[initialWellProductionTimeStep].push_back(std::make_pair(wellPath->name(), QString("%1 %2").arg(initialWellPressure, 4, 'f', 1).arg(pressureUnits)));
            }
        }
    }
    return wellProductionStartStrings;
}
