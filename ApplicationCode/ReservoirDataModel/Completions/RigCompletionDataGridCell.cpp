/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RigCompletionDataGridCell.h"

#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionDataGridCell::RigCompletionDataGridCell()
    : m_globalCellIndex(0)
    , m_lgrName("")
    , m_localCellIndexI(0)
    , m_localCellIndexJ(0)
    , m_localCellIndexK(0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionDataGridCell::RigCompletionDataGridCell(size_t globalCellIndex, const RigMainGrid* mainGrid)
    : m_globalCellIndex(globalCellIndex)
{
    if (mainGrid)
    {
        size_t             gridLocalCellIndex;
        const RigGridBase* grid = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(globalCellIndex, &gridLocalCellIndex);

        if (grid)
        {
            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

            m_localCellIndexI = i;
            m_localCellIndexJ = j;
            m_localCellIndexK = k;

            if (grid != mainGrid)
            {
                m_lgrName = QString::fromStdString(grid->gridName());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionDataGridCell::operator==(const RigCompletionDataGridCell& other) const
{
    return m_globalCellIndex == other.m_globalCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionDataGridCell::operator<(const RigCompletionDataGridCell& other) const
{
    if (m_localCellIndexI != other.m_localCellIndexI) return m_localCellIndexI < other.m_localCellIndexI;
    if (m_localCellIndexJ != other.m_localCellIndexJ) return m_localCellIndexJ < other.m_localCellIndexJ;
    if (m_localCellIndexK != other.m_localCellIndexK) return m_localCellIndexK < other.m_localCellIndexK;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCompletionDataGridCell::globalCellIndex() const
{
    return m_globalCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCompletionDataGridCell::localCellIndexI() const
{
    return m_localCellIndexI;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCompletionDataGridCell::localCellIndexJ() const
{
    return m_localCellIndexJ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCompletionDataGridCell::localCellIndexK() const
{
    return m_localCellIndexK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigCompletionDataGridCell::oneBasedLocalCellIndexString() const
{
    QString text = QString("[%1, %2, %3]").arg(m_localCellIndexI + 1).arg(m_localCellIndexJ + 1).arg(m_localCellIndexK + 1);

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigCompletionDataGridCell::lgrName() const
{
    return m_lgrName;
}
