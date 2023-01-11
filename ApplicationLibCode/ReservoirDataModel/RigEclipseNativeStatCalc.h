/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#pragma once

#include "RiaOpenMPTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigStatisticsCalculator.h"

class RigHistogramCalculator;

//==================================================================================================
///
//==================================================================================================

class RigEclipseNativeStatCalc : public RigStatisticsCalculator
{
public:
    RigEclipseNativeStatCalc( RigCaseCellResultsData* cellResultsData, const RigEclipseResultAddress& eclipseResultAddress );

    bool   hasPreciseP10p90() const override;
    void   p10p90CellScalarValues( double& min, double& max ) override;
    void   p10p90CellScalarValues( size_t timeStepIndex, double& min, double& max ) override;
    void   minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max ) override;
    void   posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg ) override;
    void   valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount ) override;
    void   addDataToHistogramCalculator( size_t timeStepIndex, RigHistogramCalculator& histogramCalculator ) override;
    void   uniqueValues( size_t timeStepIndex, std::set<int>& values ) override;
    size_t timeStepCount() override;
    void   mobileVolumeWeightedMean( size_t timeStepIndex, double& mean ) override;

private:
    RigCaseCellResultsData* m_resultsData;
    RigEclipseResultAddress m_eclipseResultAddress;

    // Add all result values to an accumulator for a given time step
    template <typename StatisticsAccumulator>
    void traverseCells( StatisticsAccumulator& accumulator, size_t timeStepIndex )
    {
        if ( timeStepIndex >= m_resultsData->cellScalarResults( m_eclipseResultAddress ).size() )
        {
            return;
        }

        const std::vector<double>& values = m_resultsData->cellScalarResults( m_eclipseResultAddress, timeStepIndex );
        for ( const auto& val : values )
        {
            accumulator.addValue( val );
        }
    }

    // Create one accumulator for each available thread, and add values to accumulator for a given time step
    template <typename StatisticsAccumulator>
    void threadTraverseCells( std::vector<StatisticsAccumulator>& accumulators, size_t timeStepIndex )
    {
        if ( timeStepIndex >= m_resultsData->cellScalarResults( m_eclipseResultAddress ).size() )
        {
            return;
        }

        const std::vector<double>& values = m_resultsData->cellScalarResults( m_eclipseResultAddress, timeStepIndex );

        int numberOfThreads = RiaOpenMPTools::availableThreadCount();
        accumulators.resize( numberOfThreads );

#pragma omp parallel
        {
            int myThread = RiaOpenMPTools::currentThreadIndex();

#pragma omp for
            for ( long long i = 0; i < (long long)values.size(); i++ )
            {
                accumulators[myThread].addValue( values[i] );
            }
        }
    }
};
