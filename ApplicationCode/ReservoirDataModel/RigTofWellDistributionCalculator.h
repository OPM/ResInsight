/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaDefines.h"

#include <cstddef>
#include <vector>
#include <map>

#include <QString>

class RimEclipseResultCase;
class RimFlowDiagSolution;
class QString;


//==================================================================================================
//
//
//
//==================================================================================================
class RigTofWellDistributionCalculator
{
public:
    RigTofWellDistributionCalculator(RimEclipseResultCase* caseToApply, QString targetWellname, size_t timeStepIndex);

    const std::vector<double>&  sortedUniqueTOFValues() const;

    size_t                      contributingWellCount() const;
    const QString&              contributingWellName(size_t contribWellIndex) const;
    const std::vector<double>&  accumulatedPhaseVolumeForContributingWell(RiaDefines::PhaseType phase, size_t contributingWellIndex) const;

private:
    static std::map<double, std::vector<size_t>>    buildSortedTofToCellIndicesMap(const std::vector<double>& tofData);
    static std::vector<QString>                     findCandidateContributingWellNames(const RimFlowDiagSolution& flowDiagSolution, QString targetWellname, size_t timeStepIndex);

    struct ContribWellEntry
    {
        QString             name;
        std::vector<double> accumulatedVolAlongTof_wat; // All these three have same size as m_tofInIncreasingOrder
        std::vector<double> accumulatedVolAlongTof_oil; // 
        std::vector<double> accumulatedVolAlongTof_gas; // 
    };

private:
    std::vector<double>             m_tofInIncreasingOrder;
    std::vector<ContribWellEntry>   m_contributingWells;
};

