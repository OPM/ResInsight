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

#pragma once

#include <QString>

class RigMainGrid;

//==================================================================================================
///
//==================================================================================================
class RigCompletionDataGridCell
{
public:
    RigCompletionDataGridCell();

    RigCompletionDataGridCell(size_t globalCellIndex, const RigMainGrid* mainGrid);

    bool operator==(const RigCompletionDataGridCell& other) const;

    bool operator<(const RigCompletionDataGridCell& other) const;

    size_t globalCellIndex() const;

    size_t localCellIndexI() const;
    size_t localCellIndexJ() const;
    size_t localCellIndexK() const;

    QString oneBasedLocalCellIndexString() const;

    QString lgrName() const;

    bool isMainGridCell() const;

private:
    size_t  m_globalCellIndex;
    QString m_lgrName;

    size_t m_localCellIndexI;
    size_t m_localCellIndexJ;
    size_t m_localCellIndexK;
};
