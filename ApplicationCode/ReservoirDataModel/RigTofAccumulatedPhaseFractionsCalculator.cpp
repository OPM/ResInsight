/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigTofAccumulatedPhaseFractionsCalculator.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigSingleWellResultsData.h"

#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimReservoirCellResultsStorage.h"

#include <map>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigTofAccumulatedPhaseFractionsCalculator::RigTofAccumulatedPhaseFractionsCalculator(RimEclipseResultCase* caseToApply, 
                                                                                     QString wellname, 
                                                                                     size_t timestep)
{
    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();
    RiaDefines::PorosityModelType porosityModel = RiaDefines::MATRIX_MODEL;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->resultsStorage(porosityModel);

    size_t scalarResultIndexSwat = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SWAT");
    size_t scalarResultIndexSoil = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SOIL");
    size_t scalarResultIndexSgas = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SGAS");
    size_t scalarResultIndexPorv = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORV");
    const std::vector<double>* swatResults = nullptr;
    const std::vector<double>* soilResults = nullptr;
    const std::vector<double>* sgasResults = nullptr;
    if (scalarResultIndexSwat != cvf::UNDEFINED_SIZE_T)
    {
        swatResults = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexSwat, timestep));
    }
    if (scalarResultIndexSoil != cvf::UNDEFINED_SIZE_T)
    {
        soilResults = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexSoil, timestep));
    }
    if (scalarResultIndexSgas != cvf::UNDEFINED_SIZE_T)
    {
        sgasResults = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexSgas, timestep));
    }
    const std::vector<double>* porvResults = &(eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->cellScalarResults(scalarResultIndexPorv, 0));
        
    RimFlowDiagSolution* flowDiagSolution = caseToApply->defaultFlowDiagSolution();

    std::string resultNameTof = "TOF";
    const std::vector<double>* tofData = flowDiagSolution->flowDiagResults()->resultValues(RigFlowDiagResultAddress(resultNameTof,
                                                                                                                    RigFlowDiagResultAddress::PhaseSelection::PHASE_ALL,
                                                                                                                    wellname.toStdString()),
                                                                                           timestep);

    std::string resultNameFraction = "Fraction";
    const std::vector<double>* fractionData = flowDiagSolution->flowDiagResults()->resultValues(RigFlowDiagResultAddress(resultNameFraction,
                                                                                                                         RigFlowDiagResultAddress::PhaseSelection::PHASE_ALL,
                                                                                                                         wellname.toStdString()),
                                                                                                timestep);

    sortTofAndCalculateAccPhaseFraction(tofData,
                                        fractionData,
                                        porvResults,
                                        swatResults,
                                        soilResults,
                                        sgasResults,
                                        m_tofInIncreasingOrder,
                                        m_accumulatedPhaseFractionSwat,
                                        m_accumulatedPhaseFractionSoil,
                                        m_accumulatedPhaseFractionSgas);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTofAccumulatedPhaseFractionsCalculator::sortTofAndCalculateAccPhaseFraction(const std::vector<double>* tofData,
                                                                                    const std::vector<double>* fractionData,
                                                                                    const std::vector<double>* porvResults,
                                                                                    const std::vector<double>* swatResults,
                                                                                    const std::vector<double>* soilResults,
                                                                                    const std::vector<double>* sgasResults,
                                                                                    std::vector<double>& tofInIncreasingOrder,
                                                                                    std::vector<double>& accumulatedPhaseFractionSwat,
                                                                                    std::vector<double>& accumulatedPhaseFractionSoil,
                                                                                    std::vector<double>& accumulatedPhaseFractionSgas)

{
    if (tofData == nullptr || fractionData == nullptr)
    {
        return;
    }

    std::map<double, std::vector<int> > tofAndIndexMap;

    for (int i = 0; i < static_cast<int>(tofData->size()); i++)
    {
        if ((*tofData)[i] == HUGE_VAL) continue;
        std::vector<int> vectorOfIndexes;
        vectorOfIndexes.push_back(i);

        auto iteratorBoolFromInsertToMap = tofAndIndexMap.insert(std::make_pair(tofData->at(i), vectorOfIndexes));
        if (!iteratorBoolFromInsertToMap.second)
        {
            //Element exist already, was not inserted
            iteratorBoolFromInsertToMap.first->second.push_back(i);
        }
    }

    double fractionPorvSum = 0.0;
    double fractionPorvPhaseSumSwat = 0.0;
    double fractionPorvPhaseSumSoil = 0.0;
    double fractionPorvPhaseSumSgas = 0.0;
    
    for (auto element : tofAndIndexMap) 
    {
        double tofValue = element.first;
        for (int index : element.second)
        {
            fractionPorvSum += fractionData->at(index) * porvResults->at(index);
            if (swatResults != nullptr)
            {
                fractionPorvPhaseSumSwat += fractionData->at(index) * porvResults->at(index) * swatResults->at(index);
            }
            if (soilResults != nullptr)
            {
                fractionPorvPhaseSumSoil += fractionData->at(index) * porvResults->at(index) * soilResults->at(index);
            }
            if (sgasResults != nullptr)
            {
                fractionPorvPhaseSumSgas += fractionData->at(index) * porvResults->at(index) * sgasResults->at(index);
            }
        }

        tofInIncreasingOrder.push_back(tofValue);
        accumulatedPhaseFractionSwat.push_back(fractionPorvPhaseSumSwat / fractionPorvSum);
        accumulatedPhaseFractionSoil.push_back(fractionPorvPhaseSumSoil / fractionPorvSum);
        accumulatedPhaseFractionSgas.push_back(fractionPorvPhaseSumSgas / fractionPorvSum);
    }
}