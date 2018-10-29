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

#include "RigActiveCellInfo.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RiaLogging.h"
#include "RiaWeightedMeanCalculator.h"

#include "cvfAssert.h"
#include "cvfBase.h"
#include "cvfMath.h"

#include <Eigen/Core>
#include <Eigen/LU>
#include <iomanip>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTransmissibilityCondenser::RigTransmissibilityCondenser()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTransmissibilityCondenser::RigTransmissibilityCondenser(const RigTransmissibilityCondenser& copyFrom)
    : m_neighborTransmissibilities(copyFrom.m_neighborTransmissibilities)
    , m_condensedTransmissibilities(copyFrom.m_condensedTransmissibilities)
    , m_externalCellAddrSet(copyFrom.m_externalCellAddrSet)
    , m_TiiInv(copyFrom.m_TiiInv)
    , m_Tie(copyFrom.m_Tie)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigTransmissibilityCondenser& RigTransmissibilityCondenser::operator=(const RigTransmissibilityCondenser& rhs)
{
    m_neighborTransmissibilities  = rhs.m_neighborTransmissibilities;
    m_condensedTransmissibilities = rhs.m_condensedTransmissibilities;
    m_externalCellAddrSet         = rhs.m_externalCellAddrSet;
    m_TiiInv                      = rhs.m_TiiInv;
    m_Tie                         = rhs.m_Tie;
    return *this;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTransmissibilityCondenser::addNeighborTransmissibility(CellAddress cell1, CellAddress cell2, double transmissibility)
{
    if (transmissibility < 1e-9) return;

    m_condensedTransmissibilities.clear();
    m_externalCellAddrSet.clear();
    if ( cell1 < cell2 )
        m_neighborTransmissibilities[cell1][cell2] += transmissibility;
    else
        m_neighborTransmissibilities[cell2][cell1] += transmissibility;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RigTransmissibilityCondenser::CellAddress> RigTransmissibilityCondenser::externalCells()
{
    if (m_externalCellAddrSet.empty())
    {
        calculateCondensedTransmissibilities();
    }

    return m_externalCellAddrSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigTransmissibilityCondenser::condensedTransmissibility(CellAddress externalCell1, CellAddress externalCell2)
{
    CAF_ASSERT(!(externalCell1 == externalCell2));

    if (m_condensedTransmissibilities.empty())
    {
        calculateCondensedTransmissibilities();
    }

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
std::map<size_t, double>
    RigTransmissibilityCondenser::scaleMatrixToFracTransByMatrixWellDP(const RigActiveCellInfo*   actCellInfo,
                                                                       double                     initialWellPressure,
                                                                       double                     currentWellPressure,
                                                                       const std::vector<double>& initialMatrixPressures,
                                                                       const std::vector<double>& currentMatrixPressures,
                                                                       bool                       normalizeByMax)
{
    CVF_ASSERT(initialMatrixPressures.size() == currentMatrixPressures.size());

    std::map<size_t, double> originalLumpedMatrixToFractureTrans; // Sum(T_mf)

    double maxInitialDeltaPressure = 0.0;
    if (normalizeByMax)
    {
        for (auto it = m_neighborTransmissibilities.begin(); it != m_neighborTransmissibilities.end(); ++it)
        {
            if (it->first.m_cellIndexSpace == CellAddress::STIMPLAN)
            {
                for (auto jt = it->second.begin(); jt != it->second.end(); ++jt)
                {
                    if (jt->first.m_cellIndexSpace == CellAddress::ECLIPSE)
                    {
                        size_t globalMatrixCellIdx = jt->first.m_globalCellIdx;
                        size_t eclipseResultIndex  = actCellInfo->cellResultIndex(globalMatrixCellIdx);
                        CVF_ASSERT(eclipseResultIndex < currentMatrixPressures.size());

                        double initialDeltaPressure = initialMatrixPressures[eclipseResultIndex] - initialWellPressure;
                        maxInitialDeltaPressure     = std::max(maxInitialDeltaPressure, initialDeltaPressure);
                    }
                }
            }
        }
    }
    for (auto it = m_neighborTransmissibilities.begin(); it != m_neighborTransmissibilities.end(); ++it)
    {
        if (it->first.m_cellIndexSpace == CellAddress::STIMPLAN)
        {
            for (auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            {
                if (jt->first.m_cellIndexSpace == CellAddress::ECLIPSE)
                {
                    size_t globalMatrixCellIdx = jt->first.m_globalCellIdx;
                    size_t eclipseResultIndex  = actCellInfo->cellResultIndex(globalMatrixCellIdx);
                    CVF_ASSERT(eclipseResultIndex < currentMatrixPressures.size());

                    originalLumpedMatrixToFractureTrans[globalMatrixCellIdx] += jt->second;

                    double initialDeltaPressure = initialMatrixPressures[eclipseResultIndex] - initialWellPressure;
                    double currentDeltaPressure = currentMatrixPressures[eclipseResultIndex] - currentWellPressure;
                    if (normalizeByMax)
                    {
                        jt->second *= currentDeltaPressure / maxInitialDeltaPressure;
                    }
                    else
                    {
                        jt->second *= currentDeltaPressure / initialDeltaPressure;
                    }
                }
            }
        }
    }
    return originalLumpedMatrixToFractureTrans;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, double> RigTransmissibilityCondenser::calculateFicticiousFractureToWellTransmissibilities()
{
    std::map<size_t, double> matrixToAllFracturesTrans;
    for (auto it = m_neighborTransmissibilities.begin(); it != m_neighborTransmissibilities.end(); ++it)
    {
        if (it->first.m_cellIndexSpace == CellAddress::STIMPLAN)
        {
            for (auto jt = it->second.begin(); jt != it->second.end(); ++jt)
            {
                if (jt->first.m_cellIndexSpace == CellAddress::ECLIPSE)
                {
                    size_t globalMatrixCellIdx = jt->first.m_globalCellIdx;
                    // T'_mf
                    double matrixToFractureTrans = jt->second;
                    // Sum(T'_mf)
                    matrixToAllFracturesTrans[globalMatrixCellIdx] += matrixToFractureTrans;
                }
            }
        }
    }

    std::map<size_t, double> fictitiousFractureToWellTrans; // T'_fjw
    for (const CellAddress& externalCell : m_externalCellAddrSet)
    {
        if (externalCell.m_cellIndexSpace == CellAddress::ECLIPSE)
        {
            size_t globalMatrixCellIdx = externalCell.m_globalCellIdx;
            // Sum(T'_mf)
            double scaledMatrixToFractureTrans = matrixToAllFracturesTrans[globalMatrixCellIdx];
            // T'mw
            double scaledMatrixToWellTrans =
                condensedTransmissibility(externalCell, {true, RigTransmissibilityCondenser::CellAddress::WELL, 1});
            // T'_fjw
            fictitiousFractureToWellTrans[globalMatrixCellIdx] =
                RigFractureTransmissibilityEquations::effectiveInternalFractureToWellTransPDDHC(scaledMatrixToFractureTrans,
                                                                                                scaledMatrixToWellTrans);
        }
    }
    return fictitiousFractureToWellTrans;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, double> RigTransmissibilityCondenser::calculateEffectiveMatrixToWellTransmissibilities(
    const std::map<size_t, double>& originalLumpedMatrixToFractureTrans,
    const std::map<size_t, double>& ficticuousFractureToWellTransMap)
{
    std::map<size_t, double> effectiveMatrixToWellTrans;
    for (const CellAddress& externalCell : m_externalCellAddrSet)    
    {
        if (externalCell.m_cellIndexSpace == CellAddress::ECLIPSE)
        {
            size_t globalMatrixCellIdx = externalCell.m_globalCellIdx;

            auto matrixToFractureIt = originalLumpedMatrixToFractureTrans.find(globalMatrixCellIdx);
            CVF_ASSERT(matrixToFractureIt != originalLumpedMatrixToFractureTrans.end());
            // Sum(T_mf)
            double lumpedOriginalMatrixToFractureT = matrixToFractureIt->second;
            // T'_fjw
            auto fictitiousFractureToWellIt = ficticuousFractureToWellTransMap.find(globalMatrixCellIdx);
            CVF_ASSERT(fictitiousFractureToWellIt != ficticuousFractureToWellTransMap.end());
            double fictitiousFractureToWellTrans = fictitiousFractureToWellIt->second;
            // T^dp_mw
            effectiveMatrixToWellTrans[globalMatrixCellIdx] =
                RigFractureTransmissibilityEquations::effectiveMatrixToWellTransPDDHC(lumpedOriginalMatrixToFractureT, fictitiousFractureToWellTrans);
        }
    }
    return effectiveMatrixToWellTrans;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTransmissibilityCondenser::calculateCondensedTransmissibilities()
{
    if (m_neighborTransmissibilities.empty()) return;

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

    MatrixXd condensedSystem;
    MatrixXd Tee = totalSystem.bottomRightCorner(externalEquationCount, externalEquationCount);

    if (internalEquationCount == 0)
    {
        condensedSystem = Tee;
    }
    else
    {
        MatrixXd Tei = totalSystem.bottomLeftCorner(externalEquationCount, internalEquationCount);
        m_TiiInv = totalSystem.topLeftCorner(internalEquationCount, internalEquationCount).inverse();
        m_Tie = totalSystem.topRightCorner(internalEquationCount, externalEquationCount);
        condensedSystem = Tee - Tei * m_TiiInv * m_Tie;
    }
     
    
    // std::cout  << "Te = " << std::endl <<  condensedSystem << std::endl << std::endl;

    for (int exEqIdx = 0; exEqIdx < externalEquationCount; ++exEqIdx)
    {
        for (int exColIdx = exEqIdx +1; exColIdx < externalEquationCount; ++exColIdx)
        {
            double T = condensedSystem(exEqIdx, exColIdx);
            //if (T != 0.0)
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

#include "RimStimPlanFractureTemplate.h"
#include "RigMainGrid.h"
#include "RigFractureCell.h"

void printCellAddress(std::stringstream& str, 
                      const RigMainGrid* mainGrid, 
                      const RigFractureGrid* fractureGrid,
                      RigTransmissibilityCondenser::CellAddress cellAddr)
{
    using CellAddress =  RigTransmissibilityCondenser::CellAddress;

    str << (cellAddr.m_isExternal ? "E " : "I ");

    switch (cellAddr.m_cellIndexSpace) {
        case CellAddress::ECLIPSE: 
        {
            if (cellAddr.m_globalCellIdx > mainGrid->cellCount())
            {
                str << "ECL - LGR CELL     ";
            }
            else
            {
                str << "ECL ";
                size_t i, j, k;
                mainGrid->ijkFromCellIndex(cellAddr.m_globalCellIdx, &i, &j, &k);
                str << std::setw(5) << i+1 << std::setw(5) << j+1 << std::setw(5) << k+1;
            }
        }
        break;
        case CellAddress::STIMPLAN: 
        {
            str << "STP ";
            const RigFractureCell& stpCell = fractureGrid->cellFromIndex(cellAddr.m_globalCellIdx);
            str << std::setw(5) << stpCell.getI()+1 << std::setw(5) << stpCell.getJ()+1  << std::setw(5) << " ";
        }
        break;

        case CellAddress::WELL: 
        {
            str << "WEL ";
            str << std::setw(5) << cellAddr.m_globalCellIdx << std::setw(5) << " "  << std::setw(5) << " ";
        }
        break;
    }

    str << " ";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RigTransmissibilityCondenser::neighborTransDebugOutput(const RigMainGrid* mainGrid, const RigFractureGrid* fractureGrid)
{
    std::stringstream debugText;
    for ( const auto& adrEqIdxPair : m_neighborTransmissibilities )
    {
        for (const auto& adrTransPair :adrEqIdxPair.second)
        {          
            debugText << "-- ";
            printCellAddress(debugText, mainGrid, fractureGrid, adrEqIdxPair.first);
            printCellAddress(debugText, mainGrid, fractureGrid, adrTransPair.first);

            debugText << " Trans: " << std::setprecision(10) << std::fixed << adrTransPair.second;
            debugText << std::endl;
        }
    }

    return debugText.str();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RigTransmissibilityCondenser::condensedTransDebugOutput(const RigMainGrid* mainGrid, const RigFractureGrid* fractureGrid)
{
    std::stringstream debugText;
    for ( const auto& adrEqIdxPair : m_condensedTransmissibilities )
    {
        for (const auto& adrTransPair :adrEqIdxPair.second)
        {
            debugText << "-- ";
            printCellAddress(debugText, mainGrid, fractureGrid, adrEqIdxPair.first);
            printCellAddress(debugText, mainGrid, fractureGrid, adrTransPair.first);

            debugText << " Trans: " << std::setprecision(10) << std::fixed << adrTransPair.second;
            debugText << std::endl;
        }
    }

    return debugText.str();
}
