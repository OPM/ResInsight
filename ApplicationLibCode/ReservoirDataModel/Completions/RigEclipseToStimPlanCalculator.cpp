/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RiaThermalFractureDefines.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"
#include "RigEclipseToThermalCellTransmissibilityCalculator.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigThermalFractureDefinition.h"
#include "RigTransmissibilityCondenser.h"

#include "RimEclipseCase.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureContainmentTools.h"
#include "RimMeshFractureTemplate.h"
#include "RimThermalFractureTemplate.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseToStimPlanCalculator::RigEclipseToStimPlanCalculator( const RimEclipseCase*  caseToApply,
                                                                cvf::Mat4d             fractureTransform,
                                                                double                 skinFactor,
                                                                double                 cDarcy,
                                                                const RigFractureGrid& fractureGrid,
                                                                const RimFracture*     fracture )
    : m_case( caseToApply )
    , m_fractureTransform( fractureTransform )
    , m_fractureSkinFactor( skinFactor )
    , m_cDarcy( cDarcy )
    , m_fractureGrid( fractureGrid )
    , m_fracture( fracture )
{
    computeValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCalculator::computeValues()
{
    auto reservoirCellIndicesOpenForFlow = RimFractureContainmentTools::reservoirCellIndicesOpenForFlow( m_case, m_fracture );

    auto resultValueAtIJ = []( const std::vector<std::vector<double>>& values, const RigFractureGrid& fractureGrid, size_t i, size_t j ) {
        if ( values.empty() ) return HUGE_VAL;

        size_t adjustedI = i + 1;
        size_t adjustedJ = j + 1;

        if ( adjustedI >= fractureGrid.iCellCount() + 1 || adjustedJ >= fractureGrid.jCellCount() + 1 ) return HUGE_VAL;

        return values[adjustedJ][adjustedI];
    };

    std::vector<std::vector<double>> injectivityFactors;
    std::vector<std::vector<double>> viscosities;
    std::vector<std::vector<double>> filterCakeMobilities;

    RimThermalFractureTemplate* thermalFractureTemplate = dynamic_cast<RimThermalFractureTemplate*>( m_fracture->fractureTemplate() );

    if ( thermalFractureTemplate != nullptr )
    {
        auto   unitSystem = thermalFractureTemplate->fractureDefinition()->unitSystem();
        size_t timeStep   = thermalFractureTemplate->activeTimeStepIndex();

        injectivityFactors =
            thermalFractureTemplate->resultValues( RiaDefines::injectivityFactorResultName(),
                                                   RiaDefines::getExpectedThermalFractureUnit( RiaDefines::injectivityFactorResultName(),
                                                                                               unitSystem ),

                                                   timeStep );

        viscosities =
            thermalFractureTemplate->resultValues( RiaDefines::viscosityResultName(),
                                                   RiaDefines::getExpectedThermalFractureUnit( RiaDefines::viscosityResultName(), unitSystem ),
                                                   timeStep );

        filterCakeMobilities =
            thermalFractureTemplate->resultValues( RiaDefines::filterCakeMobilityResultName(),
                                                   RiaDefines::getExpectedThermalFractureUnit( RiaDefines::filterCakeMobilityResultName(),
                                                                                               unitSystem ),
                                                   timeStep );
    }

    for ( size_t i = 0; i < m_fractureGrid.fractureCells().size(); i++ )
    {
        const RigFractureCell& fractureCell = m_fractureGrid.fractureCells()[i];
        if ( !fractureCell.hasNonZeroConductivity() ) continue;

        std::unique_ptr<RigEclipseToStimPlanCellTransmissibilityCalculator> eclToFractureTransCalc;

        if ( thermalFractureTemplate != nullptr )
        {
            size_t cellI = fractureCell.getI();
            size_t cellJ = fractureCell.getJ();

            double injectivityFactor  = resultValueAtIJ( injectivityFactors, m_fractureGrid, cellI, cellJ );
            double viscosity          = resultValueAtIJ( viscosities, m_fractureGrid, cellI, cellJ );
            double filterCakeMobility = resultValueAtIJ( filterCakeMobilities, m_fractureGrid, cellI, cellJ );

            // Assumed value
            double relativePermeability = 1.0;

            auto filterPressureDropType = thermalFractureTemplate->filterCakePressureDropType();
            eclToFractureTransCalc      = std::make_unique<RigEclipseToThermalCellTransmissibilityCalculator>( m_case,
                                                                                                          m_fractureTransform,
                                                                                                          m_fractureSkinFactor,
                                                                                                          m_cDarcy,
                                                                                                          fractureCell,
                                                                                                          m_fracture,
                                                                                                          filterPressureDropType,
                                                                                                          injectivityFactor,
                                                                                                          filterCakeMobility,
                                                                                                          viscosity,
                                                                                                          relativePermeability );
        }
        else
        {
            eclToFractureTransCalc = std::make_unique<RigEclipseToStimPlanCellTransmissibilityCalculator>( m_case,
                                                                                                           m_fractureTransform,
                                                                                                           m_fractureSkinFactor,
                                                                                                           m_cDarcy,
                                                                                                           fractureCell,
                                                                                                           m_fracture );
        }

        eclToFractureTransCalc->computeValues( reservoirCellIndicesOpenForFlow );
        const std::vector<size_t>& fractureCellContributingEclipseCells = eclToFractureTransCalc->globalIndiciesToContributingEclipseCells();

        if ( !fractureCellContributingEclipseCells.empty() )
        {
            m_singleFractureCellCalculators.emplace( i, std::move( eclToFractureTransCalc ) );
        }
    }
}

using CellIdxSpace = RigTransmissibilityCondenser::CellAddress;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCalculator::appendDataToTransmissibilityCondenser( bool                          useFiniteConductivityInFracture,
                                                                            RigTransmissibilityCondenser* condenser ) const
{
    for ( const auto& eclToFractureTransCalc : m_singleFractureCellCalculators )
    {
        const std::vector<size_t>& fractureCellContributingEclipseCells =
            eclToFractureTransCalc.second->globalIndiciesToContributingEclipseCells();

        const std::vector<double>& fractureCellContributingEclipseCellTransmissibilities =
            eclToFractureTransCalc.second->contributingEclipseCellTransmissibilities();

        size_t stimPlanCellIndex = eclToFractureTransCalc.first;

        for ( size_t i = 0; i < fractureCellContributingEclipseCells.size(); i++ )
        {
            if ( useFiniteConductivityInFracture )
            {
                condenser->addNeighborTransmissibility( { true, CellIdxSpace::ECLIPSE, fractureCellContributingEclipseCells[i] },
                                                        { false, CellIdxSpace::STIMPLAN, stimPlanCellIndex },
                                                        fractureCellContributingEclipseCellTransmissibilities[i] );
            }
            else
            {
                condenser->addNeighborTransmissibility( { true, CellIdxSpace::ECLIPSE, fractureCellContributingEclipseCells[i] },
                                                        { true, CellIdxSpace::WELL, 1 },
                                                        fractureCellContributingEclipseCellTransmissibilities[i] );
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

    for ( const auto& singleCellCalc : m_singleFractureCellCalculators )
    {
        double cellArea = singleCellCalc.second->areaOpenForFlow();

        area += cellArea;
    }

    return area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::areaWeightedMatrixPermeability() const
{
    RiaWeightedMeanCalculator<double> calc;

    for ( const auto& singleCellCalc : m_singleFractureCellCalculators )
    {
        const std::vector<double>& areas          = singleCellCalc.second->contributingEclipseCellIntersectionAreas();
        const std::vector<double>& permeabilities = singleCellCalc.second->contributingEclipseCellPermeabilities();

        if ( areas.size() == permeabilities.size() )
        {
            for ( size_t i = 0; i < areas.size(); i++ )
            {
                calc.addValueAndWeight( permeabilities[i], areas[i] );
            }
        }
    }

    return calc.validAggregatedWeight() ? calc.weightedMean() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::areaWeightedWidth() const
{
    double width = 0.0;

    auto ellipseFractureTemplate = dynamic_cast<const RimEllipseFractureTemplate*>( m_fracture->fractureTemplate() );
    if ( ellipseFractureTemplate )
    {
        width = ellipseFractureTemplate->width();
    }

    auto stimPlanFractureTemplate = dynamic_cast<const RimMeshFractureTemplate*>( m_fracture->fractureTemplate() );
    if ( stimPlanFractureTemplate )
    {
        auto widthValues = stimPlanFractureTemplate->widthResultValues();
        if ( !widthValues.empty() )
        {
            RiaWeightedMeanCalculator<double> calc;

            for ( const auto& singleCellCalc : m_singleFractureCellCalculators )
            {
                double cellArea = singleCellCalc.second->areaOpenForFlow();

                size_t globalStimPlanCellIndex = singleCellCalc.first;
                double widthValue              = widthValues[globalStimPlanCellIndex];

                if ( !std::isinf( widthValue ) && !std::isnan( widthValue ) )
                {
                    calc.addValueAndWeight( widthValue, cellArea );
                }
            }

            width = calc.weightedMean();
        }
        else
        {
            width = stimPlanFractureTemplate->computeFractureWidth( m_fracture );
        }
    }

    return width;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::areaWeightedConductivity() const
{
    RiaWeightedMeanCalculator<double> calc;

    for ( const auto& singleCellCalc : m_singleFractureCellCalculators )
    {
        double cellArea = singleCellCalc.second->areaOpenForFlow();

        double conductivity = singleCellCalc.second->fractureCell().getConductivityValue();
        if ( !std::isinf( conductivity ) && !std::isnan( conductivity ) )
        {
            calc.addValueAndWeight( conductivity, cellArea );
        }
    }

    return calc.validAggregatedWeight() ? calc.weightedMean() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCalculator::longestYSectionOpenForFlow() const
{
    // For each I, find the longest aggregated distance along J with continuous fracture cells with conductivity above
    // zero connected to Eclipse cells open for flow

    double longestRange = 0.0;

    for ( size_t i = 0; i < m_fractureGrid.iCellCount(); i++ )
    {
        double currentAggregatedDistanceY = 0.0;
        for ( size_t j = 0; j < m_fractureGrid.jCellCount(); j++ )
        {
            size_t globalStimPlanCellIndex = m_fractureGrid.getGlobalIndexFromIJ( i, j );

            auto calculatorForCell = m_singleFractureCellCalculators.find( globalStimPlanCellIndex );
            if ( calculatorForCell != m_singleFractureCellCalculators.end() )
            {
                currentAggregatedDistanceY += calculatorForCell->second->fractureCell().cellSizeZ();
            }
            else
            {
                longestRange               = std::max( longestRange, currentAggregatedDistanceY );
                currentAggregatedDistanceY = 0.0;
            }
        }

        longestRange = std::max( longestRange, currentAggregatedDistanceY );
    }

    return longestRange;
}
