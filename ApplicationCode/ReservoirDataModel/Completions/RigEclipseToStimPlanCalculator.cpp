/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RigEclipseToStimPlanCalculator.h"

#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigTransmissibilityCondenser.h"

#include "RimEclipseCase.h"
#include "RimFracture.h"
#include "RimFractureContainmentTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseToStimPlanCalculator::RigEclipseToStimPlanCalculator(RimEclipseCase*        caseToApply,
                                                               cvf::Mat4d             fractureTransform,
                                                               double                 skinFactor,
                                                               double                 cDarcy,
                                                               const RigFractureGrid& fractureGrid)
    : m_case(caseToApply)
    , m_fractureTransform(fractureTransform)
    , m_fractureSkinFactor(skinFactor)
    , m_cDarcy(cDarcy)
    , m_fractureGrid(fractureGrid)
{
    computeValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCalculator::computeValues()
{
    for (const RigFractureCell& fractureCell : m_fractureGrid.fractureCells())
    {
        if (!fractureCell.hasNonZeroConductivity()) continue;

        RigEclipseToStimPlanCellTransmissibilityCalculator eclToFractureTransCalc(
            m_case, m_fractureTransform, m_fractureSkinFactor, m_cDarcy, fractureCell);

        const std::vector<size_t>& fractureCellContributingEclipseCells =
            eclToFractureTransCalc.globalIndiciesToContributingEclipseCells();

        if (!fractureCellContributingEclipseCells.empty())
        {
            m_singleFractureCellCalculators.emplace_back(eclToFractureTransCalc);
        }
    }
}

using CellIdxSpace = RigTransmissibilityCondenser::CellAddress;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCalculator::appendDataToTransmissibilityCondenser(const RimFracture* fracture,
                                                                           bool               useFiniteConductivityInFracture,
                                                                           RigTransmissibilityCondenser* condenser) const
{
    for (const auto& eclToFractureTransCalc : m_singleFractureCellCalculators)
    {
        const std::vector<size_t>& fractureCellContributingEclipseCells =
            eclToFractureTransCalc.globalIndiciesToContributingEclipseCells();

        const std::vector<double>& fractureCellContributingEclipseCellTransmissibilities =
            eclToFractureTransCalc.contributingEclipseCellTransmissibilities();

        const auto& fractureCell      = eclToFractureTransCalc.fractureCell();
        size_t      stimPlanCellIndex = m_fractureGrid.getGlobalIndexFromIJ(fractureCell.getI(), fractureCell.getJ());

        auto truncatedFractureCellIndices = RimFractureContainmentTools::fracturedCellsTruncatedByFaults(m_case, fracture);

        for (size_t i = 0; i < fractureCellContributingEclipseCells.size(); i++)
        {
            if (fracture->isEclipseCellWithinContainment(
                    m_case->eclipseCaseData()->mainGrid(), truncatedFractureCellIndices, fractureCellContributingEclipseCells[i]))
            {
                if (useFiniteConductivityInFracture)
                {
                    condenser->addNeighborTransmissibility({true, CellIdxSpace::ECLIPSE, fractureCellContributingEclipseCells[i]},
                                                           {false, CellIdxSpace::STIMPLAN, stimPlanCellIndex},
                                                           fractureCellContributingEclipseCellTransmissibilities[i]);
                }
                else
                {
                    condenser->addNeighborTransmissibility({true, CellIdxSpace::ECLIPSE, fractureCellContributingEclipseCells[i]},
                                                           {true, CellIdxSpace::WELL, 1},
                                                           fractureCellContributingEclipseCellTransmissibilities[i]);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, double> RigEclipseToStimPlanCalculator::eclipseCellAreas() const
{
    std::map<size_t, double> areaForEclipseReservoirCells;

    for (const auto& singleCellCalc : m_singleFractureCellCalculators)
    {
        const auto& cellIndices = singleCellCalc.globalIndiciesToContributingEclipseCells();
        const auto& cellAreas   = singleCellCalc.contributingEclipseCellAreas();

        for (size_t i = 0; i < cellIndices.size(); i++)
        {
            areaForEclipseReservoirCells[cellIndices[i]] += cellAreas[i];
        }
    }

    return areaForEclipseReservoirCells;
}
