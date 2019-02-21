/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RigEclipseCrossPlotDataExtractor.h"

#include "RigEclipseCaseData.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigActiveCellInfo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RigEclipseCrossPlotDataExtractor::extract(RigEclipseCaseData*            caseData,
                                                                                 int                            timeStep,
                                                                                 const RigEclipseResultAddress& xAxisProperty,
                                                                                 const RigEclipseResultAddress& yAxisProperty)
{
    RigCaseCellResultsData* resultData = caseData->results(RiaDefines::MATRIX_MODEL);
    std::vector<double> xValues = resultData->cellScalarResults(xAxisProperty)[timeStep];
    std::vector<double> yValues = resultData->cellScalarResults(xAxisProperty)[timeStep];
    size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
    size_t resultValueIndex   = m_activeCellInfo->cellResultIndex(reservoirCellIndex);
    if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;

    if (resultValueIndex < m_reservoirResultValues->size()) return m_reservoirResultValues->at(resultValueIndex);
}
