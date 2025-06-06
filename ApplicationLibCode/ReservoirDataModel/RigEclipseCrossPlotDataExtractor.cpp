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

#include "RiaQDateTimeTools.h"

#include "RimEclipseResultDefinition.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"

#include <memory>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCrossPlotResult RigEclipseCrossPlotDataExtractor::extract( RigEclipseCaseData*               caseData,
                                                                     int                               resultTimeStep,
                                                                     const RimEclipseResultDefinition& xAddress,
                                                                     const RimEclipseResultDefinition& yAddress,
                                                                     RigGridCrossPlotCurveGrouping     groupingType,
                                                                     const RimEclipseResultDefinition& groupAddress,
                                                                     std::map<int, cvf::UByteArray>    timeStepCellVisibilityMap )
{
    RigEclipseCrossPlotResult result;

    if ( xAddress.eclipseResultAddress().isValid() && yAddress.eclipseResultAddress().isValid() )
    {
        RigCaseCellResultsData* xResultData = caseData->results( xAddress.porosityModel() );
        if ( !xResultData->ensureKnownResultLoaded( xAddress.eclipseResultAddress() ) )
        {
            return result;
        }

        RigCaseCellResultsData* yResultData = caseData->results( yAddress.porosityModel() );
        if ( !yResultData->ensureKnownResultLoaded( yAddress.eclipseResultAddress() ) )
        {
            return result;
        }

        const std::vector<std::vector<double>>& xValuesForAllSteps = xResultData->cellScalarResults( xAddress.eclipseResultAddress() );
        const std::vector<std::vector<double>>& yValuesForAllSteps = yResultData->cellScalarResults( yAddress.eclipseResultAddress() );

        RigCaseCellResultsData*                 groupResultData      = nullptr;
        const std::vector<std::vector<double>>* catValuesForAllSteps = nullptr;

        if ( groupingType == GROUP_BY_RESULT && groupAddress.eclipseResultAddress().isValid() )
        {
            groupResultData = caseData->results( groupAddress.porosityModel() );
            if ( groupResultData->ensureKnownResultLoaded( groupAddress.eclipseResultAddress() ) )
            {
                catValuesForAllSteps = &groupResultData->cellScalarResults( groupAddress.eclipseResultAddress() );
            }
        }

        std::set<int> timeStepsToInclude;
        if ( resultTimeStep == -1 )
        {
            size_t nStepsInData = std::max( xValuesForAllSteps.size(), yValuesForAllSteps.size() );
            bool   xValid       = xValuesForAllSteps.size() == 1u || xValuesForAllSteps.size() == nStepsInData;
            bool   yValid       = yValuesForAllSteps.size() == 1u || yValuesForAllSteps.size() == nStepsInData;

            if ( !( xValid && yValid ) ) return result;

            for ( size_t i = 0; i < nStepsInData; ++i )
            {
                timeStepsToInclude.insert( (int)i );
            }
        }
        else
        {
            timeStepsToInclude.insert( static_cast<size_t>( resultTimeStep ) );
        }

        for ( int timeStep : timeStepsToInclude )
        {
            const cvf::UByteArray* cellVisibility = nullptr;
            if ( timeStepCellVisibilityMap.count( timeStep ) )
            {
                cellVisibility = &timeStepCellVisibilityMap[timeStep];
            }

            int xIndex = timeStep >= (int)xValuesForAllSteps.size() ? 0 : timeStep;
            int yIndex = timeStep >= (int)yValuesForAllSteps.size() ? 0 : timeStep;

            const RigMainGrid* mainGrid = caseData->mainGrid();

            RigActiveCellsResultAccessor                  xAccessor( mainGrid, &xValuesForAllSteps[xIndex], xResultData->activeCellInfo() );
            RigActiveCellsResultAccessor                  yAccessor( mainGrid, &yValuesForAllSteps[yIndex], yResultData->activeCellInfo() );
            std::unique_ptr<RigActiveCellsResultAccessor> catAccessor;
            if ( catValuesForAllSteps )
            {
                int catIndex = timeStep >= (int)catValuesForAllSteps->size() ? 0 : timeStep;
                catAccessor  = std::make_unique<RigActiveCellsResultAccessor>( mainGrid,
                                                                              &( catValuesForAllSteps->at( catIndex ) ),
                                                                              groupResultData->activeCellInfo() );
            }

            for ( size_t globalCellIdx = 0; globalCellIdx < xResultData->activeCellInfo()->reservoirCellCount(); ++globalCellIdx )
            {
                if ( cellVisibility && !( *cellVisibility )[globalCellIdx] ) continue;

                double xValue = xAccessor.cellScalarGlobIdx( globalCellIdx );
                double yValue = yAccessor.cellScalarGlobIdx( globalCellIdx );

                if ( xValue == HUGE_VAL || yValue == HUGE_VAL ) continue;

                result.xValues.push_back( xValue );
                result.yValues.push_back( yValue );

                if ( groupingType == GROUP_BY_TIME )
                {
                    result.groupValuesDiscrete.push_back( timeStep );
                }
                else if ( groupingType == GROUP_BY_FORMATION )
                {
                    const RigFormationNames* activeFormationNames = xResultData->activeFormationNames();
                    if ( activeFormationNames )
                    {
                        int    category = 0;
                        size_t i( cvf::UNDEFINED_SIZE_T ), j( cvf::UNDEFINED_SIZE_T ), k( cvf::UNDEFINED_SIZE_T );
                        if ( mainGrid->ijkFromCellIndex( globalCellIdx, &i, &j, &k ) )
                        {
                            category = activeFormationNames->formationIndexFromKLayerIdx( k );
                        }
                        result.groupValuesDiscrete.push_back( category );
                    }
                }
                else if ( groupingType == GROUP_BY_RESULT )
                {
                    double catValue = HUGE_VAL;
                    if ( catAccessor )
                    {
                        catValue = catAccessor->cellScalarGlobIdx( globalCellIdx );
                    }
                    result.groupValuesContinuous.push_back( catValue );
                }
            }
        }
    }

    return result;
}
