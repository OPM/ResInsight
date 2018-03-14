/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RigCompletionData.h"

#include "cvfBase.h"
#include "cvfObject.h"

#include <map>
#include <vector>

class RigCompletionData;
class RigCompletionDataGridCell;
class RimWellPath;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class CompletionDataFrame
{
public:
    CompletionDataFrame();

    void setCompletionData(const std::vector<RigCompletionData>& completions);

    const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& multipleCompletionsPerEclipseCell() const;

private:
    std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> m_multipleCompletionsPerEclipseCell;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigVirtualPerforationTransmissibilities : public cvf::Object
{
public:
    RigVirtualPerforationTransmissibilities();
    ~RigVirtualPerforationTransmissibilities();

    void setCompletionDataForWellPath(RimWellPath* wellPath, std::vector<std::vector<RigCompletionData>>& completionsPerTimeStep);

    const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>&
        multipleCompletionsPerEclipseCell(RimWellPath* wellPath, size_t timeStepIndex) const;

    void computeMinMax(double* minValue, double* maxValue, double* posClosestToZero, double* negClosestToZero) const;
private:
    std::map<RimWellPath*, std::vector<CompletionDataFrame>> m_mapFromWellToCompletionData;
};
