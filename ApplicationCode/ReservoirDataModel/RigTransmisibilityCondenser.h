#pragma once

#include <map>
#include <vector>
#include "cafAssert.h"

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

    void addNeighborTransmisibility(CellAddress cell1, CellAddress cell2, double transmisibility)
    {
        m_condensedTransmisibilities.clear();
        if ( cell1 < cell2 ) 
            m_neighborTransmisibilities[cell1][cell2] = transmisibility;
        else  
            m_neighborTransmisibilities[cell2][cell1] = transmisibility;
    }

    std::vector<CellAddress> externalCells()
    {
        calculateCondensedTransmisibilitiesIfNeeded(); std::vector<CellAddress> extCells; 
        for ( const auto& adrToAdrTransMapPair : m_condensedTransmisibilities ) extCells.push_back(adrToAdrTransMapPair.first);
    }

    double condensedTransmisibility( CellAddress externalCell1, CellAddress externalCell2) 
    { 
        CAF_ASSERT(!(externalCell1 == externalCell2));

        calculateCondensedTransmisibilitiesIfNeeded();

        if (externalCell2 < externalCell1) std::swap(externalCell1, externalCell2);

        const auto& adrToAdrTransMapPair = m_condensedTransmisibilities.find(externalCell1);
        if ( adrToAdrTransMapPair != m_condensedTransmisibilities.end() )
        {
            const auto& adrTransPair = adrToAdrTransMapPair->second.find(externalCell2);
            if ( adrTransPair != adrToAdrTransMapPair->second.end() )
            {
                return adrTransPair->second;
            }
        }
        return 0.0;
    }

private:
    void calculateCondensedTransmisibilitiesIfNeeded();

    std::map<CellAddress, std::map<CellAddress, double> > m_neighborTransmisibilities;
    std::map<CellAddress, std::map<CellAddress, double> > m_condensedTransmisibilities;
};  

#include <Eigen/Core>
#include <Eigen/LU>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTransmissibilityCondenser::calculateCondensedTransmisibilitiesIfNeeded()
{
    if (m_condensedTransmisibilities.size()) return;

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
        for ( const auto& adrEqIdxPair : m_neighborTransmisibilities )
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

    for (const auto& adrToAdrTransMapPair : m_neighborTransmisibilities)
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
                if (cell1 < cell2) m_condensedTransmisibilities[cell1][cell2] = T;
                else m_condensedTransmisibilities[cell2][cell1] = T;
            }
        }                
    }
}

