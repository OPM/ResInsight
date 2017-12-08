/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaSimWellBranchTools.h"

#include "RiaApplication.h"
#include "RigEclipseCaseData.h"
#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellPlotTools.h"
#include "cafPdmUiOrdering.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<const RigWellPath*> RiaSimWellBranchTools::simulationWellBranches(const QString& simWellName, bool useAutoDetectionOfBranches)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    // Find first case containing the specified simulation well
    auto simCases = proj->eclipseCases();
    auto caseItr = std::find_if(simCases.begin(), simCases.end(), [&simWellName](const RimEclipseCase* eclCase) {
        const auto& eclData = eclCase->eclipseCaseData();
        return eclData != nullptr && eclData->hasSimulationWell(simWellName);
    });
    RimEclipseCase* eclipseCase = caseItr != simCases.end() ? *caseItr : nullptr;
    RigEclipseCaseData* eclCaseData = eclipseCase != nullptr ? eclipseCase->eclipseCaseData() : nullptr;
    return eclCaseData != nullptr ?
        eclCaseData->simulationWellBranches(simWellName, false, useAutoDetectionOfBranches) :
        std::vector<const RigWellPath*>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaSimWellBranchTools::valueOptionsForBranchIndexField(const std::vector<const RigWellPath*>& simulationWellPaths)
{
    QList<caf::PdmOptionItemInfo> options;

    size_t branchCount = simulationWellPaths.size();
    if (simulationWellPaths.size() == 0)
    {
        options.push_front(caf::PdmOptionItemInfo("None", -1));
    }
    else
    {
        for (int bIdx = 0; bIdx < static_cast<int>(branchCount); ++bIdx)
        {
            options.push_back(caf::PdmOptionItemInfo("Branch " + QString::number(bIdx + 1), QVariant::fromValue(bIdx)));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromWellName(caf::PdmUiOrdering*        uiOrdering,
                                                                            const QString&             wellPathOrSimWellName,
                                                                            const caf::PdmField<bool>& branchDetectionField,
                                                                            const caf::PdmField<int>&  branchIndexField)
{
    if (!RimWellPlotTools::hasAssociatedWellPath(wellPathOrSimWellName))
    {
        const QString simWellName = RimWellPlotTools::simWellName(wellPathOrSimWellName);

        RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromSimWellName(uiOrdering, 
                                                                                  simWellName, 
                                                                                  branchDetectionField,
                                                                                  branchIndexField);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromSimWellName(caf::PdmUiOrdering*        uiOrdering,
                                                                               const QString&             simWellName,
                                                                               const caf::PdmField<bool>& branchDetectionField,
                                                                               const caf::PdmField<int>&  branchIndexField)
{
    if (RiaSimWellBranchTools::simulationWellBranches(simWellName, true).size() > 1)
    {
        uiOrdering->add(&branchDetectionField);

        if (RiaSimWellBranchTools::simulationWellBranches(simWellName, branchDetectionField).size() > 1)
        {
            uiOrdering->add(&branchIndexField);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiaSimWellBranchTools::clampBranchIndex(const QString& simWellName, int branchIndexValue, bool branchDetection)
{
    auto branches = RiaSimWellBranchTools::simulationWellBranches(simWellName, branchDetection);

    if (branches.size() == 0)
    {
        return -1;
    }

    if (branchIndexValue >= branches.size())
    {
        branchIndexValue = static_cast<int>(branches.size()) - 1;
    }

    return branchIndexValue;
}
