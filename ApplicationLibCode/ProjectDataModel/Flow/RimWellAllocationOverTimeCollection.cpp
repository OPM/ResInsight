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
{
    for ( const auto& [date, calculator] : timeStepAndCalculatorPairs )
    {
        std::string err = "Calculator for time step date " + date.toString().toStdString() +
                          " does not exist in time step dates vector ";
        CAF_ASSERT( std::find( m_timeStepDates.begin(), m_timeStepDates.end(), date ) != m_timeStepDates.end() &&
                    err.data() );
    }

    // Time steps not present in input map is considered "excluded" time steps
    // Build new time step and calculator map using calculator for "next" valid time step for
    // "excluded" time steps
    QDateTime prevValidTimeStep;
    for ( auto it = m_timeStepDates.rbegin(); it != m_timeStepDates.rend(); ++it )
    {
        const QDateTime& timeStep             = *it;
        auto             timeStepCalculatorIt = timeStepAndCalculatorPairs.find( timeStep );
        if ( timeStepCalculatorIt != timeStepAndCalculatorPairs.end() )
        {
            m_timeStepAndCalculatorPairs.emplace( timeStep, timeStepCalculatorIt->second );
            prevValidTimeStep = timeStep;
        }
        else if ( prevValidTimeStep.isValid() )
        {
            // If no calculator for this time step, use the previous valid time step calculator
            m_timeStepAndCalculatorPairs.emplace( timeStep, timeStepAndCalculatorPairs.at( prevValidTimeStep ) );
        }
    }

    std::sort( m_timeStepDates.begin(), m_timeStepDates.end() );

    // Retrieve union of well names across all calculators
    std::set<QString> allWellNames;
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
void RimWellAllocationOverTimeCollection::fillWithFlowRatePercentageValues()
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
/// Fill with flow volume at time step.
///
/// Create volume by multiplying with number of days since last time step.
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::fillWithFlowVolumeValues()
{
    fillWithFlowRateValues();

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
/// Fill with accumulated flow volume over a range of time steps. Create volume by multiplying with
/// number of days since last time step.
///
/// Group small contributors in "Others" if accumulated volume value at last time step is below
/// threshold value.
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::fillWithAccumulatedFlowVolumeValues( double smallContributionsThreshold )
{
    fillWithFlowRateValues();

    for ( auto& [well, timeStepsAndValues] : m_wellValuesMap )
    {
        QDateTime prevTimeStep;
        double    accumulatedVolume = 0.0;
        for ( auto& [timeStep, value] : timeStepsAndValues )
        {
            if ( !prevTimeStep.isValid() )
            {
                prevTimeStep = timeStep;
                continue;
            }

            const auto   numDays = static_cast<double>( prevTimeStep.daysTo( timeStep ) );
            const double volume  = value * numDays;
            accumulatedVolume += volume;
            value        = accumulatedVolume;
            prevTimeStep = timeStep;
        }
    }

    if ( smallContributionsThreshold > 0.0 )
    {
        groupAccumulatedFlowVolumes( m_wellValuesMap, smallContributionsThreshold );
    }
}

//--------------------------------------------------------------------------------------------------
/// Fill with accumulated well flow volumes in percent of total accumulated flow volume at each
/// time step.
///
///
/// Group small contributors in "Others" if percentage value for well is below threshold at every
/// time step.
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::fillWithAccumulatedFlowVolumePercentageValues( double smallContributionsThreshold )
{
    // Handle threshold filtering afterwards
    const double nonFilteringThreshold = 0.0;
    fillWithAccumulatedFlowVolumeValues( nonFilteringThreshold );

    for ( const auto& timeStep : m_timeStepDates )
    {
        double                    totalAccumulatedVolume = 0.0;
        std::map<QString, double> timeStepWellValues;

        // Sum accumulated volumes at time step
        for ( auto& [well, values] : m_wellValuesMap )
        {
            const auto accumulatedVolume = values[timeStep];
            totalAccumulatedVolume += accumulatedVolume;
            timeStepWellValues[well] = accumulatedVolume;
        }

        // If no accumulated volume exist at time step
        if ( totalAccumulatedVolume == 0.0 ) continue;

        // Create percentage value
        for ( auto& [well, value] : timeStepWellValues )
        {
            m_wellValuesMap[well][timeStep] = 100.0 * value / totalAccumulatedVolume;
        }
    }

    if ( smallContributionsThreshold > 0.0 )
    {
        const auto percentageThreshold = 100.0 * smallContributionsThreshold;
        groupAccumulatedFlowVolumePercentages( m_wellValuesMap, percentageThreshold );
    }
}

//--------------------------------------------------------------------------------------------------
/// Handle grouping of small contributors in accumulated volume data based on threshold.
/// Group small contributors in "Others" if accumulated volume value at last time step is below
/// threshold value.
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::groupAccumulatedFlowVolumes( std::map<QString, std::map<QDateTime, double>>& rWellValuesMap,
                                                                       double threshold )
{
    if ( m_timeStepDates.empty() ) return;

    std::map<QString, std::map<QDateTime, double>> groupedWellValuesMap;
    std::map<QString, double>                      lastAccumulatedWellValues;
    double                                         sumLastAccumulatedWellValues = 0.0;
    const QDateTime                                lastTimeStep                 = m_timeStepDates.back();
    for ( auto& [well, values] : rWellValuesMap )
    {
        const double lastWellValue      = values[lastTimeStep];
        lastAccumulatedWellValues[well] = lastWellValue;
        sumLastAccumulatedWellValues += lastWellValue;
    }

    // Filter out wells with accumulated flow less than threshold and place among "others"
    std::vector<QString> contributingWells;
    std::vector<QString> groupedWells;
    for ( const auto& [well, value] : lastAccumulatedWellValues )
    {
        if ( sumLastAccumulatedWellValues > 0.0 && ( value / sumLastAccumulatedWellValues ) < threshold )
        {
            groupedWells.push_back( well );
        }
        else
        {
            contributingWells.push_back( well );
        }
    }

    for ( const auto& well : contributingWells )
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

//--------------------------------------------------------------------------------------------------
/// Handle grouping of small contributors in accumulated volume percentage based on threshold.
/// Group small contributors in "Others" if percentage value for well is below threshold at every
/// time step. If percentage value is above threshold for one time step or more, show data for well
/// at every time step.
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimeCollection::groupAccumulatedFlowVolumePercentages(
    std::map<QString, std::map<QDateTime, double>>& rWellValuesMap,
    double                                          thresholdPercent )
{
    auto getMaxValue = []( const std::map<QDateTime, double>& valuesMap ) -> double {
        double maxValue = 0.0;
        for ( const auto& [timeStep, value] : valuesMap )
        {
            maxValue = value > maxValue ? value : maxValue;
        }
        return maxValue;
    };

    std::vector<QString> contributingWells;
    std::vector<QString> groupedWells;
    for ( const auto& [well, values] : rWellValuesMap )
    {
        const double maxValue = getMaxValue( values );
        if ( maxValue > thresholdPercent )
        {
            contributingWells.push_back( well );
        }
        else
        {
            groupedWells.push_back( well );
        }
    }

    std::map<QString, std::map<QDateTime, double>> groupedWellValuesMap;
    for ( const auto& well : contributingWells )
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
