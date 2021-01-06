/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-    Equinor ASA
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

#include "RigCaseCellResultCalculator.h"

#include "RiaLogging.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigResultModifier.h"
#include "RigResultModifierFactory.h"

#include "RimEclipseCase.h"
#include "RimProject.h"

#include "cvfAssert.h"

#include <algorithm>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultCalculator::computeDifference( RigEclipseCaseData*            sourceCase,
                                                     RiaDefines::PorosityModelType  porosityModel,
                                                     const RigEclipseResultAddress& address )
{
    CVF_ASSERT( address.isValid() );
    CVF_ASSERT( address.hasDifferenceCase() || address.isTimeLapse() );

    // Assume at this stage that data for the case is available
    // It is up to the caller to make sure the case is read from file

    RigEclipseCaseData* baseCase = sourceCase;

    if ( address.hasDifferenceCase() )
    {
        {
            auto eclipseCases = RimProject::current()->eclipseCases();
            for ( RimEclipseCase* c : eclipseCases )
            {
                if ( c && c->caseId() == address.m_differenceCaseId && c->eclipseCaseData() )
                {
                    baseCase = c->eclipseCaseData();
                }
            }
        }
    }

    if ( !baseCase || !sourceCase )
    {
        RiaLogging::error( "Missing input case for difference calculator" );

        return false;
    }

    RigMainGrid* sourceMainGrid = sourceCase->mainGrid();
    RigMainGrid* baseMainGrid   = baseCase->mainGrid();

    if ( !RigGridManager::isMainGridDimensionsEqual( sourceMainGrid, baseMainGrid ) )
    {
        RiaLogging::error( "Case difference : Grid cases do not match" );

        return false;
    }

    RigCaseCellResultsData* baseCaseResults   = baseCase->results( porosityModel );
    RigCaseCellResultsData* sourceCaseResults = sourceCase->results( porosityModel );

    if ( !baseCaseResults || !sourceCaseResults )
    {
        RiaLogging::error( "Missing result data for difference calculator" );

        return false;
    }

    RigEclipseResultAddress nativeAddress( address );
    nativeAddress.m_differenceCaseId      = RigEclipseResultAddress::noCaseDiffValue();
    nativeAddress.m_timeLapseBaseFrameIdx = RigEclipseResultAddress::noTimeLapseValue();
    if ( !sourceCaseResults->ensureKnownResultLoaded( nativeAddress ) )
    {
        RiaLogging::error( "Failed to load destination diff result" );

        return false;
    }

    if ( !baseCaseResults->ensureKnownResultLoaded( nativeAddress ) )
    {
        RiaLogging::error( "Failed to load difference result" );

        return false;
    }

    // Initialize difference result with infinity for correct number of time steps and values per time step
    {
        const std::vector<std::vector<double>>& srcFrames = sourceCaseResults->cellScalarResults( nativeAddress );
        std::vector<std::vector<double>>*       diffResultFrames =
            sourceCaseResults->modifiableCellScalarResultTimesteps( address );
        diffResultFrames->resize( srcFrames.size() );
        for ( size_t fIdx = 0; fIdx < srcFrames.size(); ++fIdx )
        {
            const std::vector<double>& srcVals = srcFrames[fIdx];
            std::vector<double>&       dstVals = diffResultFrames->at( fIdx );

            // Clear the values, and resize with infinity as default value
            dstVals.clear();
            dstVals.resize( srcVals.size(), std::numeric_limits<double>::infinity() );
        }
    }

    size_t baseFrameCount   = baseCaseResults->cellScalarResults( nativeAddress ).size();
    size_t sourceFrameCount = sourceCaseResults->cellScalarResults( nativeAddress ).size();
    size_t maxFrameCount    = 0;
    if ( address.isTimeLapse() )
    {
        // We have one defined time step for base case, loop over all source time steps
        maxFrameCount = sourceFrameCount;
    }
    else
    {
        // We compare cases, diff is computed time index by time index. Use minimum frame count.
        maxFrameCount = std::min( baseFrameCount, sourceFrameCount );
    }

    size_t maxGridCount = std::min( baseMainGrid->gridCount(), sourceMainGrid->gridCount() );

    for ( size_t gridIdx = 0; gridIdx < maxGridCount; ++gridIdx )
    {
        auto                     grid           = sourceMainGrid->gridByIndex( gridIdx );
        const RigActiveCellInfo* activeCellInfo = sourceCaseResults->activeCellInfo();

        for ( size_t fIdx = 0; fIdx < maxFrameCount; ++fIdx )
        {
            cvf::ref<RigResultAccessor> sourceResultAccessor =
                RigResultAccessorFactory::createFromResultAddress( sourceCase, gridIdx, porosityModel, fIdx, nativeAddress );

            cvf::ref<RigResultModifier> resultModifier =
                RigResultModifierFactory::createResultModifier( sourceCase, gridIdx, porosityModel, fIdx, address );

            size_t baseFrameIdx = fIdx;
            if ( address.isTimeLapse() )
            {
                baseFrameIdx = address.m_timeLapseBaseFrameIdx;
            }

            cvf::ref<RigResultAccessor> baseResultAccessor =
                RigResultAccessorFactory::createFromResultAddress( baseCase, gridIdx, porosityModel, baseFrameIdx, nativeAddress );

            for ( size_t localGridCellIdx = 0; localGridCellIdx < grid->cellCount(); localGridCellIdx++ )
            {
                size_t reservoirCellIndex = grid->reservoirCellIndex( localGridCellIdx );
                if ( activeCellInfo->isActive( reservoirCellIndex ) )
                {
                    double sourceVal = sourceResultAccessor->cellScalar( localGridCellIdx );
                    double baseVal   = baseResultAccessor->cellScalar( localGridCellIdx );

                    double difference = sourceVal - baseVal;

                    resultModifier->setCellScalar( localGridCellIdx, difference );
                }
            }
        }
    }

    return true;
}
