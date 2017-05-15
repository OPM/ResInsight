/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "cafAssert.h"

#include <map>
#include <vector>
#include <set>

class RigTransmissibilityCondenser
{
public:
    class CellAddress
    {
     public:
        enum CellType { ECLIPSE, STIMPLAN };

        CellAddress(): m_isExternal(false), 
                       m_cellType(STIMPLAN), 
                       m_globalCellIdx(-1) {}
        CellAddress(bool     isExternal, CellType cellType, size_t   globalCellIdx)
         : m_isExternal(isExternal), 
           m_cellType(cellType), 
           m_globalCellIdx(globalCellIdx) {}

        bool     m_isExternal;
        CellType m_cellType;
        size_t   m_globalCellIdx;

        bool operator==(const CellAddress& o) { return (m_isExternal == o.m_isExternal) && (m_cellType == o.m_cellType) && (m_globalCellIdx == o.m_globalCellIdx); }

        // Ordering external after internal is important for the matrix order internally
        bool operator<(const CellAddress& other) const
        {
            if (m_isExternal    != other.m_isExternal)    return !m_isExternal; // Internal cells < External cells
            if (m_cellType      != other.m_cellType)      return m_cellType < other.m_cellType; // Eclipse < StimPlan
            if (m_globalCellIdx != other.m_globalCellIdx) return m_globalCellIdx < other.m_globalCellIdx;
            return false;
        }
    };

    void addNeighborTransmissibility(CellAddress cell1, CellAddress cell2, double transmissibility);

    std::set<CellAddress> externalCells();

    double condensedTransmissibility( CellAddress externalCell1, CellAddress externalCell2);

private:
    void calculateCondensedTransmissibilitiesIfNeeded();

    std::map<CellAddress, std::map<CellAddress, double> > m_neighborTransmissibilities;
    std::map<CellAddress, std::map<CellAddress, double> > m_condensedTransmissibilities;
    std::set<CellAddress> m_externalCellAddrSet;
};  
