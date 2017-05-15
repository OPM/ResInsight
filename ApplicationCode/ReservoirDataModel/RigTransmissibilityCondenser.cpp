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

#include "RigTransmissibilityCondenser.h"


#include <Eigen/Core>
#include <Eigen/LU>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTransmissibilityCondenser::addNeighborTransmissibility(CellAddress cell1, CellAddress cell2, double transmissibility)
{
    m_condensedTransmissibilities.clear();
    m_externalCellAddrSet.clear();

    if ( cell1 < cell2 )
        m_neighborTransmissibilities[cell1][cell2] = transmissibility;
    else
        m_neighborTransmissibilities[cell2][cell1] = transmissibility;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RigTransmissibilityCondenser::CellAddress> RigTransmissibilityCondenser::externalCells()
{
    calculateCondensedTransmissibilitiesIfNeeded(); 

    return m_externalCellAddrSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityCondenser::condensedTransmissibility(CellAddress externalCell1, CellAddress externalCell2)
{
    CAF_ASSERT(!(externalCell1 == externalCell2));

    calculateCondensedTransmissibilitiesIfNeeded();

    if ( externalCell2 < externalCell1 ) std::swap(externalCell1, externalCell2);

    const auto& adrToAdrTransMapPair = m_condensedTransmissibilities.find(externalCell1);
    if ( adrToAdrTransMapPair != m_condensedTransmissibilities.end() )
    {
        const auto& adrTransPair = adrToAdrTransMapPair->second.find(externalCell2);
        if ( adrTransPair != adrToAdrTransMapPair->second.end() )
        {
            return adrTransPair->second;
        }
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTransmissibilityCondenser::calculateCondensedTransmissibilitiesIfNeeded()
{
    if (m_condensedTransmissibilities.size()) return;

    // Find all equations, and their total ordering

    union
    {
        int idxToFirstExternalEquation;
        int internalEquationCount;
    };
    idxToFirstExternalEquation = -1;
    int totalEquationCount = -1;

    std::map<CellAddress, int> cellAddressToEqIdxMap;
    std::vector<CellAddress>   eqIdxToCellAddressMapping;
    {
        for ( const auto& adrEqIdxPair : m_neighborTransmissibilities )
        {
            cellAddressToEqIdxMap.insert({ adrEqIdxPair.first, -1 });
            for ( const auto& adrTranspair : adrEqIdxPair.second )
            {
                cellAddressToEqIdxMap.insert({ adrTranspair.first, -1 });
            }
        }
       
        int currentEqIdx = 0;
        
        for ( auto& adrEqIdxPair : cellAddressToEqIdxMap)
        {
            adrEqIdxPair.second = currentEqIdx;
            eqIdxToCellAddressMapping.push_back(adrEqIdxPair.first);

            if ( idxToFirstExternalEquation == -1 && adrEqIdxPair.first.m_isExternal )
            {
                idxToFirstExternalEquation = currentEqIdx;
            }
            ++currentEqIdx;
        }
        totalEquationCount = currentEqIdx;
    }
    
    CAF_ASSERT(idxToFirstExternalEquation != -1);

    using namespace Eigen;
    
    MatrixXd totalSystem = MatrixXd::Zero(totalEquationCount, totalEquationCount);

    for (const auto& adrToAdrTransMapPair : m_neighborTransmissibilities)
    {
        CAF_ASSERT(cellAddressToEqIdxMap.count(adrToAdrTransMapPair.first)); // Remove when stabilized
        int c1EquationIdx = cellAddressToEqIdxMap[adrToAdrTransMapPair.first];
        for (const auto& adrTranspair : adrToAdrTransMapPair.second)
        {
            CAF_ASSERT(cellAddressToEqIdxMap.count(adrTranspair.first)); // Remove when stabilized

            int c2EquationIdx = cellAddressToEqIdxMap[adrTranspair.first];

            totalSystem(c1EquationIdx, c2EquationIdx) += adrTranspair.second;
            totalSystem(c2EquationIdx, c1EquationIdx) += adrTranspair.second;
            totalSystem(c1EquationIdx, c1EquationIdx) -= adrTranspair.second;
            totalSystem(c2EquationIdx, c2EquationIdx) -= adrTranspair.second;
        }

        ++c1EquationIdx;
    }

    // std::cout  << "T = " << std::endl <<  totalSystem << std::endl;

    int externalEquationCount =  totalEquationCount - internalEquationCount;
    MatrixXd condensedSystem = totalSystem.bottomRightCorner(externalEquationCount, externalEquationCount) 
                               - totalSystem.bottomLeftCorner(externalEquationCount, internalEquationCount) 
                               * totalSystem.topLeftCorner(internalEquationCount, internalEquationCount).inverse()
                               * totalSystem.topRightCorner(internalEquationCount, externalEquationCount );
    
    // std::cout  << "Te = " << std::endl <<  condensedSystem << std::endl << std::endl;

    for (int exEqIdx = 0; exEqIdx < externalEquationCount; ++exEqIdx)
    {
        for (int exColIdx = exEqIdx +1; exColIdx < externalEquationCount; ++exColIdx)
        {
            double T = condensedSystem(exEqIdx, exColIdx);
            if (T != 0.0)
            {
                CellAddress cell1 = eqIdxToCellAddressMapping[exEqIdx + internalEquationCount];
                CellAddress cell2 = eqIdxToCellAddressMapping[exColIdx + internalEquationCount];
                if (cell1 < cell2) m_condensedTransmissibilities[cell1][cell2] = T;
                else m_condensedTransmissibilities[cell2][cell1] = T;

                m_externalCellAddrSet.insert(cell1);
                m_externalCellAddrSet.insert(cell2);

            }
        }                
    }
}



