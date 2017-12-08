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

#pragma once

#include "cafPdmField.h"

#include <QList>
#include <QString>

#include <vector>

class RigWellPath;

namespace caf
{
class PdmOptionItemInfo;
class PdmUiOrdering;
} // namespace caf

//==================================================================================================
//
//==================================================================================================
class RiaSimWellBranchTools
{
public:
    static std::vector<const RigWellPath*> simulationWellBranches(const QString& simWellName, bool useAutoDetectionOfBranches);

    static QList<caf::PdmOptionItemInfo>
        valueOptionsForBranchIndexField(const std::vector<const RigWellPath*>& simulationWellPaths);

    static void appendSimWellBranchFieldsIfRequiredFromWellName(caf::PdmUiOrdering* uiOrdering, const QString& wellPathOrSimWellName,
                                                    const caf::PdmField<bool>& branchDetectionField,
                                                    const caf::PdmField<int>&  branchIndexField);

    static void appendSimWellBranchFieldsIfRequiredFromSimWellName(caf::PdmUiOrdering* uiOrdering, const QString& simWellName,
                                                    const caf::PdmField<bool>& branchDetectionField,
                                                    const caf::PdmField<int>&  branchIndexField);
};
