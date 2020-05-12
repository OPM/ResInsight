/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

//==================================================================================================
///
//==================================================================================================
#include "RigStatisticsCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigNNCData.h"

#include "cvfArray.h"

class RigEclipseAllanFaultsStatCalc : public RigStatisticsCalculator
{
public:
    RigEclipseAllanFaultsStatCalc( RigNNCData* cellResultsData, const RigEclipseResultAddress& scalarResultIndex );

    void   minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max ) override;
    void   posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg ) override;
    void   valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount ) override;
    void   addDataToHistogramCalculator( size_t timeStepIndex, RigHistogramCalculator& histogramCalculator ) override;
    void   uniqueValues( size_t timeStepIndex, std::set<int>& values ) override;
    size_t timeStepCount() override;
    void   mobileVolumeWeightedMean( size_t timeStepIndex, double& result ) override;

private:
    RigNNCData*             m_caseData;
    RigEclipseResultAddress m_resultAddress;

    template <typename StatisticsAccumulator>
    void traverseCells( StatisticsAccumulator& accumulator, size_t timeStepIndex )
    {
        const std::vector<double>* values = m_caseData->staticConnectionScalarResult( m_resultAddress );

        if ( values && !values->empty() )
        {
            for ( const auto& v : *values )
            {
                accumulator.addValue( v );
            }
        }
    }
};
