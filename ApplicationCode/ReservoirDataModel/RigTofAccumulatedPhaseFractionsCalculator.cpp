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
    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(porosityModel);

    size_t scalarResultIndexSwat = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SWAT");
    size_t scalarResultIndexSoil = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SOIL");
    size_t scalarResultIndexSgas = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SGAS");
    size_t scalarResultIndexPorv = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORV");
    const std::vector<double>* swatResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexSwat, timestep));
    const std::vector<double>* soilResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexSoil, timestep));
    const std::vector<double>* sgasResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexSgas, timestep));
    const std::vector<double>* porvResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexPorv, timestep));
        
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
                                        m_accumulatedPhaseFractionSwat,
                                        m_accumulatedPhaseFractionSoil,
                                        m_accumulatedPhaseFractionSgas,
                                        m_tofInIncreasingOrder);
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
    std::map<double, std::vector<int> > tofAndIndexMap;

    for (int i = 0; i < tofData->size(); i++)
    {
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
            fractionPorvPhaseSumSwat += fractionData->at(index) * porvResults->at(index) * swatResults->at(index);
            fractionPorvPhaseSumSoil += fractionData->at(index) * porvResults->at(index) * soilResults->at(index);
            fractionPorvPhaseSumSgas += fractionData->at(index) * porvResults->at(index) * sgasResults->at(index);
        }

        tofInIncreasingOrder.push_back(tofValue);
        accumulatedPhaseFractionSwat.push_back(fractionPorvPhaseSumSwat / fractionPorvSum);
        accumulatedPhaseFractionSoil.push_back(fractionPorvPhaseSumSoil / fractionPorvSum);
        accumulatedPhaseFractionSgas.push_back(fractionPorvPhaseSumSgas / fractionPorvSum);
    }
}