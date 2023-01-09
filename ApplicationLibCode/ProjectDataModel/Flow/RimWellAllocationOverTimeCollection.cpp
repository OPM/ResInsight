/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimWellAllocationOverTimeCollection.h"

#include "cafAssert.h"

#include "RigAccWellFlowCalculator.h"
#include "RigFlowDiagResultAddress.h"
#include "RigWellResultPoint.h"

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationOverTimeCollection::RimWellAllocationOverTimeCollection(
    const std::vector<QDateTime>&                        timeStepDates,
    const std::map<QDateTime, RigAccWellFlowCalculator>& timeStepAndCalculatorPairs )
    : m_timeStepDates( timeStepDates )
    , m_timeStepAndCalculatorPairs( timeStepAndCalculatorPairs )
{
    for ( const auto& [date, calculator] : m_timeStepAndCalculatorPairs )
    {
        std::string err = "Calculator for time step date " + date.toString().toStdString() +
                          " does not exist in time step dates vector ";
        CAF_ASSERT( std::find( m_timeStepDates.begin(), m_timeStepDates.end(), date ) != m_timeStepDates.end() &&
                    err.data() );
    }

    std::sort( m_timeStepDates.begin(), m_timeStepDates.end() );

    // Retrieve union of well names across all calculators
    std::set<QString> allWellNames = {};
    for ( const auto& [date, calculator] : m_timeStepAndCalculatorPairs )
    {
        allWellNames.insert( calculator.tracerNames().begin(), calculator.tracerNames().end() );
    }

    // Fill default well values into map
    const double defaultValue = 0.0;
    for ( const auto& well : allWellNames )
    {
        for ( const auto& date : m_timeStepDates )
        {
            std::pair<QDateTime, double> defaultPair( date, defaultValue );
            m_defaultWellValuesMap[well].insert( defaultPair );
        }
    }
    m_wellValuesMap = m_defaultWellValuesMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::fillWithPercentageValues()
{
    m_wellValuesMap = m_defaultWellValuesMap;
    for ( auto& [timeStep, calculator] : m_timeStepAndCalculatorPairs )
    {
        const auto totalTracerFractions = calculator.totalTracerFractions();
        for ( const auto& [wellName, value] : totalTracerFractions )
        {
            double valuePercent                 = 100.0 * value;
            m_wellValuesMap[wellName][timeStep] = valuePercent;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::fillWithFlowRateValues()
{
    m_wellValuesMap        = m_defaultWellValuesMap;
    const size_t branchIdx = 0;
    for ( auto& [timeStep, calculator] : m_timeStepAndCalculatorPairs )
    {
        for ( const auto& wellName : calculator.tracerNames() )
        {
            const auto& accumulatedConnectionFlows = calculator.accumulatedTracerFlowPrConnection( wellName, branchIdx );
            const double topConnectionFlow = accumulatedConnectionFlows.empty() ? 0.0 : accumulatedConnectionFlows.back();
            m_wellValuesMap[wellName][timeStep] = topConnectionFlow;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::fillWithFlowVolumeValues()
{
    fillWithFlowRateValues();

    // Scale with number of days for volume
    for ( auto& [well, timeStepsAndValues] : m_wellValuesMap )
    {
        QDateTime prevTimeStep;
        for ( auto& [timeStep, value] : timeStepsAndValues )
        {
            if ( !prevTimeStep.isValid() )
            {
                prevTimeStep = timeStep;
                continue;
            }

            const auto numDays = static_cast<double>( prevTimeStep.daysTo( timeStep ) );
            value              = value * numDays;
            prevTimeStep       = timeStep;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::fillWithAccumulatedFlowVolumeValues( double smallContributionsThreshold )
{
    fillWithFlowVolumeValues();

    for ( auto& [well, timeStepsAndValues] : m_wellValuesMap )
    {
        double accumulatedValue = 0.0;
        for ( auto& [timeStep, value] : timeStepsAndValues )
        {
            accumulatedValue += value;
            value = accumulatedValue;
        }
    }

    if ( smallContributionsThreshold > 0.0 )
    {
        groupAccumulatedFlowVolumes( m_wellValuesMap, smallContributionsThreshold );
    }
}

//--------------------------------------------------------------------------------------------------
/// Handle grouping of small contributors in accumulated data based on threshold.
/// Group small contributors in "Others" based on last time sample values.
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::groupAccumulatedFlowVolumes( std::map<QString, std::map<QDateTime, double>>& rWellValuesMap,
                                                                       double threshold )
{
    if ( m_timeStepDates.size() == 0 )
    {
        return;
    }

    std::map<QString, std::map<QDateTime, double>> groupedWellValuesMap;
    const QDateTime                                lastTimeStep                 = m_timeStepDates.back();
    std::map<QString, double>                      lastAccumulatedWellValues    = {};
    double                                         sumLastAccumulatedWellValues = 0.0;
    for ( auto& [well, values] : rWellValuesMap )
    {
        const double lastWellValue      = values[lastTimeStep];
        lastAccumulatedWellValues[well] = lastWellValue;
        sumLastAccumulatedWellValues += lastWellValue;
    }

    // Filter out wells with accumulated flow less than threshold and place among "others"
    std::vector<QString> groupedWells   = {};
    std::vector<QString> ungroupedWells = {};
    for ( const auto& [well, value] : lastAccumulatedWellValues )
    {
        if ( ( value / sumLastAccumulatedWellValues ) < threshold )
        {
            groupedWells.push_back( well );
        }
        else
        {
            ungroupedWells.push_back( well );
        }
    }

    for ( const auto& well : ungroupedWells )
    {
        groupedWellValuesMap[well] = rWellValuesMap[well];
    }
    for ( const auto& well : groupedWells )
    {
        if ( groupedWellValuesMap.count( RIG_TINY_TRACER_GROUP_NAME ) == 0 )
        {
            groupedWellValuesMap[RIG_TINY_TRACER_GROUP_NAME] = rWellValuesMap[well];
        }
        else
        {
            for ( const auto& [date, value] : rWellValuesMap[well] )
            {
                groupedWellValuesMap[RIG_TINY_TRACER_GROUP_NAME][date] += value;
            }
        }
    }

    rWellValuesMap = groupedWellValuesMap;
}
