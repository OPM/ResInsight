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

#include <Eigen/Core>

#include <map>
#include <set>
#include <vector>

class RigActiveCellInfo;
class RigMainGrid;
class RimStimPlanFractureTemplate;
class RigFractureGrid;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigTransmissibilityCondenser
{
public:
    RigTransmissibilityCondenser();
    RigTransmissibilityCondenser(const RigTransmissibilityCondenser& copyFrom);
    RigTransmissibilityCondenser& operator=(const RigTransmissibilityCondenser& rhs);

    class CellAddress
    {
    public:
        enum CellIndexSpace
        {
            ECLIPSE,
            STIMPLAN,
            WELL
        };

        CellAddress()
            : m_isExternal(false)
            , m_cellIndexSpace(STIMPLAN)
            , m_globalCellIdx(-1)
        {
        }
        CellAddress(bool isExternal, CellIndexSpace cellType, size_t globalCellIdx)
            : m_isExternal(isExternal)
            , m_cellIndexSpace(cellType)
            , m_globalCellIdx(globalCellIdx)
        {
        }

        bool           m_isExternal;
        CellIndexSpace m_cellIndexSpace;
        size_t         m_globalCellIdx;

        bool operator==(const CellAddress& o)
        {
            return (m_isExternal == o.m_isExternal) && (m_cellIndexSpace == o.m_cellIndexSpace) &&
                   (m_globalCellIdx == o.m_globalCellIdx);
        }

        // Ordering external after internal is important for the matrix order internally

        bool operator<(const CellAddress& other) const
        {
            if (m_isExternal != other.m_isExternal) return !m_isExternal; // Internal cells < External cells
            if (m_cellIndexSpace != other.m_cellIndexSpace)
                return m_cellIndexSpace < other.m_cellIndexSpace; // Eclipse < StimPlan
            if (m_globalCellIdx != other.m_globalCellIdx) return m_globalCellIdx < other.m_globalCellIdx;
            return false;
        }
    };

    void addNeighborTransmissibility(CellAddress cell1, CellAddress cell2, double transmissibility);

    std::set<CellAddress> externalCells();

    double condensedTransmissibility(CellAddress externalCell1, CellAddress externalCell2);

    std::string neighborTransDebugOutput(const RigMainGrid* mainGrid, const RigFractureGrid* fractureGrid);
    std::string condensedTransDebugOutput(const RigMainGrid* mainGrid, const RigFractureGrid* fractureGrid);

    std::map<size_t, double> scaleMatrixToFracTransByMatrixWellDP(const RigActiveCellInfo* actCellInfo, double currentWellPressure, const std::vector<double>& currentMatrixPressures, double* minPressureDrop, double* maxPressureDrop);

    std::map<size_t, double> calculateFicticiousFractureToWellTransmissibilities();
    std::map<size_t, double>
        calculateEffectiveMatrixToWellTransmissibilities(const std::map<size_t, double>& originalLumpedMatrixToFractureTrans,
                                                         const std::map<size_t, double>& ficticuousFractureToWellTransMap);

    void calculateCondensedTransmissibilities();

protected:
    typedef std::pair<CellAddress, std::map<CellAddress, double>> ConnectionTransmissibility;
    typedef std::map<CellAddress, std::map<CellAddress, double>>  ConnectionTransmissibilities;

    ConnectionTransmissibilities m_neighborTransmissibilities;
    ConnectionTransmissibilities m_condensedTransmissibilities;

    std::set<CellAddress> m_externalCellAddrSet;

    Eigen::MatrixXd m_TiiInv;
    Eigen::MatrixXd m_Tie;
};
