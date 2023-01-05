/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RigFemPartResultCalculatorKIndices.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigFormationNames.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorKIndices::RigFemPartResultCalculatorKIndices( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorKIndices::~RigFemPartResultCalculatorKIndices()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorKIndices::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "INDEX" && resVarAddr.componentName == "INDEX_K" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorKIndices::calculate( int                        partIndex,
                                                                         const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );

    RigFemScalarResultFrames* resFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );
    resFrames->enableAsSingleStepResult();

    const RigFemPart*   femPart      = m_resultCollection->parts()->part( partIndex );
    std::vector<float>& dstFrameData = resFrames->frameData( 0, 0 );

    const size_t valCount = femPart->elementNodeResultCount();
    dstFrameData.resize( valCount, std::numeric_limits<float>::infinity() );

    const RigFormationNames* activeFormNames = m_resultCollection->activeFormationNames();

    frameCountProgress.incrementProgress();

    if ( activeFormNames )
    {
        // Has to be done before the parallel loop because the first call allocates.
        const RigFemPartGrid* structGrid = femPart->getOrCreateStructGrid();

        const int elementCount = femPart->elementCount();

        // Using max() as std::numeric_limits<size_t>::infinity() returns 0
        constexpr size_t maxValue = std::numeric_limits<size_t>::max();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType      = femPart->elementType( elmIdx );
            int            elmNodeCount = RigFemTypes::elementNodeCount( elmType );

            size_t i, j, k = maxValue;
            bool   validIndex = structGrid->ijkFromCellIndex( elmIdx, &i, &j, &k );
            if ( validIndex )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx        = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    dstFrameData[elmNodResIdx] = k != maxValue ? static_cast<float>( k ) : HUGE_VALF;
                }
            }
        }
    }

    return resFrames;
}
