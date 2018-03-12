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

#include "RigVirtualPerforationTransmissibilities.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CompletionDataFrame::CompletionDataFrame() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CompletionDataFrame::setCompletionData(const std::vector<RigCompletionData>& completions)
{
    for (auto& completion : completions)
    {
        auto it = m_multipleCompletionsPerEclipseCell.find(completion.completionDataGridCell());
        if (it != m_multipleCompletionsPerEclipseCell.end())
        {
            it->second.push_back(completion);
        }
        else
        {
            m_multipleCompletionsPerEclipseCell.insert(std::pair<RigCompletionDataGridCell, std::vector<RigCompletionData>>(
                completion.completionDataGridCell(), std::vector<RigCompletionData>{completion}));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& CompletionDataFrame::multipleCompletionsPerEclipseCell() const
{
    return m_multipleCompletionsPerEclipseCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigVirtualPerforationTransmissibilities::RigVirtualPerforationTransmissibilities() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigVirtualPerforationTransmissibilities::~RigVirtualPerforationTransmissibilities() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVirtualPerforationTransmissibilities::appendCompletionDataForWellPath(
    RimWellPath*                          wellPath,
    const std::vector<RigCompletionData>& completions)
{
/*
    auto item = m_mapFromWellToCompletionData.find(wellPath);
    if (item != m_mapFromWellToCompletionData.end())
    {
        item->second.setCompletionData
    }

    auto it = m_multipleCompletionsPerEclipseCell.find(completion.completionDataGridCell());
    if (it != m_multipleCompletionsPerEclipseCell.end())
    {
        it->second.push_back(completion);
    }
    else
    {
        m_multipleCompletionsPerEclipseCell.insert(std::pair<RigCompletionDataGridCell, std::vector<RigCompletionData>>(
            completion.completionDataGridCell(), std::vector<RigCompletionData>{completion}));
    }
*/

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>&
    RigVirtualPerforationTransmissibilities::multipleCompletionsPerEclipseCell(RimWellPath* wellPath, size_t timeStepIndex) const
{
    static std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> dummy;

    auto item = m_mapFromWellToCompletionData.find(wellPath);
    if (item != m_mapFromWellToCompletionData.end())
    {
        return item->second[timeStepIndex].multipleCompletionsPerEclipseCell();
    }
    
    return dummy;
}
