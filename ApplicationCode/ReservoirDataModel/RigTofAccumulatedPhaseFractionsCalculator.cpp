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

#include "RigEclipseCaseData.h"
#include "RimEclipseCase.h"
#include "RimReservoirCellResultsStorage.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigSingleWellResultsData.h"
#include "RigFlowDiagResultAddress.h"
#include "RimFlowDiagSolution.h"
#include "RigFlowDiagResults.h"
#include "RigCaseCellResultsData.h"

#include <map>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigTofAccumulatedPhaseFractionsCalculator::computeTOFaccumulations()
{
    RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    

    RimReservoirCellResultsStorage* gridCellResults = m_case->results(porosityModel);

    size_t scalarResultIndexSwat = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SWAT");
    size_t scalarResultIndexSoil = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SOIL");
    size_t scalarResultIndexSgas = gridCellResults->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SGAS");
    size_t scalarResultIndexPorv = gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORV");

    const std::vector<double>* swatResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexSwat, m_timeStep));
    const std::vector<double>* soilResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexSoil, m_timeStep));
    const std::vector<double>* sgasResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexSgas, m_timeStep));
    const std::vector<double>* porvResults = &(eclipseCaseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(scalarResultIndexPorv, m_timeStep));
    
    const RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);

    std::string resultNameTof = "TOF";
    const std::vector<double>* tofData = m_flowDiagSolution->flowDiagResults()->resultValues(RigFlowDiagResultAddress(resultNameTof, 
                                                                                                                      m_wellName.toStdString()),
                                                                                             m_timeStep);
    
    std::string resultNameFraction = "Fraction";
    const std::vector<double>* fractionData = m_flowDiagSolution->flowDiagResults()->resultValues(RigFlowDiagResultAddress(resultNameFraction, 
                                                                                                                           m_wellName.toStdString()),
                                                                                                  m_timeStep);


    std::vector<double> accumulatedPhaseFractionSwat;
    std::vector<double> accumulatedPhaseFractionSoil;
    std::vector<double> accumulatedPhaseFractionSgas;
    std::vector<double> tofInIncreasingOrder;


    sortTofAndCalculateAccPhaseFraction(tofData, 
                                        fractionData, 
                                        porvResults, 
                                        swatResults, 
                                        soilResults, 
                                        sgasResults, 
                                        accumulatedPhaseFractionSwat, 
                                        accumulatedPhaseFractionSoil, 
                                        accumulatedPhaseFractionSgas, 
                                        tofInIncreasingOrder);



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
        auto it = tofAndIndexMap.find(tofData->at(i));
        if (it == tofAndIndexMap.end())
        {
            //Key does not exist
            std::vector<int> vectorOfIndexes;
            vectorOfIndexes.push_back(i);
            tofAndIndexMap[tofData->at(i)] = vectorOfIndexes;
        }
        else
        {
            //Key does exisit
            std::vector<int> vectorOfIndexes = it->second;
            vectorOfIndexes.push_back(i);
            tofAndIndexMap[tofData->at(i)] = vectorOfIndexes;
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