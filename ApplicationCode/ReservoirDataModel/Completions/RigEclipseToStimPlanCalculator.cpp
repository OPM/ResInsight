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

#include "RiaWeightedAverageCalculator.h"
#include "RimEclipseCase.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureContainmentTools.h"
#include "RimStimPlanFractureTemplate.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseToStimPlanCalculator::RigEclipseToStimPlanCalculator(const RimEclipseCase*  caseToApply,
                                                               cvf::Mat4d             fractureTransform,
                                                               double                 skinFactor,
                                                               double                 cDarcy,
                                                               const RigFractureGrid& fractureGrid,
                                                               const RimFracture*     fracture)
    : m_case(caseToApply)
    , m_fractureTransform(fractureTransform)
    , m_fractureSkinFactor(skinFactor)
    , m_cDarcy(cDarcy)
    , m_fractureGrid(fractureGrid)
    , m_fracture(fracture)
{

    computeValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCalculator::computeValues()
{
    auto reservoirCellIndicesOpenForFlow = RimFractureContainmentTools::reservoirCellIndicesOpenForFlow(m_case, m_fracture);

    for (size_t i = 0; i < m_fractureGrid.fractureCells().size(); i++)
    {
        const RigFractureCell& fractureCell = m_fractureGrid.fractureCells()[i];
        if (!fractureCell.hasNonZeroConductivity()) continue;

        RigEclipseToStimPlanCellTransmissibilityCalculator eclToFractureTransCalc(
            m_case, m_fractureTransform, m_fractureSkinFactor, m_cDarcy, fractureCell, reservoirCellIndicesOpenForFlow, m_fracture);

        const std::vector<size_t>& fractureCellContributingEclipseCells =
            eclToFractureTransCalc.globalIndiciesToContributingEclipseCells();

        if (!fractureCellContributingEclipseCells.empty())
        {
            m_singleFractureCellCalculators.emplace(i, eclToFractureTransCalc);
        }
    }
}

using CellIdxSpace = RigTransmissibilityCondenser::CellAddress;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCalculator::appendDataToTransmissibilityCondenser(bool useFiniteConductivityInFracture,
                                                                           RigTransmissibilityCondenser* condenser) const
{
    for (const auto& eclToFractureTransCalc : m_singleFractureCellCalculators)
    {
        const std::vector<size_t>& fractureCellContributingEclipseCells =
            eclToFractureTransCalc.second.globalIndiciesToContributingEclipseCells();

        const std::vector<double>& fractureCellContributingEclipseCellTransmissibilities =
            eclToFractureTransCalc.second.contributingEclipseCellTransmissibilities();

        size_t stimPlanCellIndex = eclToFractureTransCalc.first;

        for (size_t i = 0; i < fractureCellContributingEclipseCells.size(); i++)
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::totalEclipseAreaOpenForFlow() const
{
    double area = 0.0;

    for (const auto& singleCellCalc : m_singleFractureCellCalculators)
    {
        const auto& cellAreas = singleCellCalc.second.contributingEclipseCellAreas();

        for (const auto& cellArea : cellAreas)
        {
            area += cellArea;
        }
    }

    return area;

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::areaWeightedMatrixTransmissibility() const
{
    RiaWeightedAverageCalculator<double> calc;

    for (const auto& singleCellCalc : m_singleFractureCellCalculators)
    {
        const RigEclipseToStimPlanCellTransmissibilityCalculator& calulator = singleCellCalc.second;

        calc.addValueAndWeight(calulator.aggregatedMatrixTransmissibility(), calulator.areaOpenForFlow());
    }

    return calc.weightedAverage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::areaWeightedWidth() const
{
    double width = 0.0;

    auto ellipseFractureTemplate = dynamic_cast<const RimEllipseFractureTemplate*>(m_fracture->fractureTemplate());
    if (ellipseFractureTemplate)
    {
        width = ellipseFractureTemplate->width();
    }

    auto stimPlanFractureTemplate = dynamic_cast<const RimStimPlanFractureTemplate*>(m_fracture->fractureTemplate());
    if (stimPlanFractureTemplate)
    {
        RiaWeightedAverageCalculator<double> calc;

        auto widthValues = stimPlanFractureTemplate->widthResultValues();

        for (const auto& singleCellCalc : m_singleFractureCellCalculators)
        {
            double cellArea = singleCellCalc.second.areaOpenForFlow();

            size_t globalStimPlanCellIndex = singleCellCalc.first;
            double widthValue              = widthValues[globalStimPlanCellIndex];

            calc.addValueAndWeight(widthValue, cellArea);
        }

        width = calc.weightedAverage();
    }

    return width;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::areaWeightedConductivity() const
{
    RiaWeightedAverageCalculator<double> calc;

    for (const auto& singleCellCalc : m_singleFractureCellCalculators)
    {
        double cellArea = singleCellCalc.second.areaOpenForFlow();

        calc.addValueAndWeight(singleCellCalc.second.fractureCell().getConductivityValue(), cellArea);
    }

    return calc.weightedAverage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::longestYSectionOpenForFlow() const
{
    // For each I, find the longest aggregated distance along J with continuous fracture cells with conductivity above zero
    // connected to Eclipse cells open for flow

    double longestRange = 0.0;

    for (size_t i = 0; i < m_fractureGrid.iCellCount(); i++)
    {
        double currentAggregatedDistanceY = 0.0;
        for (size_t j = 0; j < m_fractureGrid.jCellCount(); j++)
        {
            size_t globalStimPlanCellIndex = m_fractureGrid.getGlobalIndexFromIJ(i, j);

            auto calculatorForCell = m_singleFractureCellCalculators.find(globalStimPlanCellIndex);
            if (calculatorForCell != m_singleFractureCellCalculators.end())
            {
                currentAggregatedDistanceY += calculatorForCell->second.fractureCell().cellSizeZ();
            }
            else
            {
                longestRange               = std::max(longestRange, currentAggregatedDistanceY);
                currentAggregatedDistanceY = 0.0;
            }
        }

        longestRange = std::max(longestRange, currentAggregatedDistanceY);
    }

    return longestRange;
}
