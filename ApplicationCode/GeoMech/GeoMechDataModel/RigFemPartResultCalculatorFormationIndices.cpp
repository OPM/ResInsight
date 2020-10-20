/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigFemPartResultCalculatorFormationIndices.h"

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
RigFemPartResultCalculatorFormationIndices::RigFemPartResultCalculatorFormationIndices( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorFormationIndices::~RigFemPartResultCalculatorFormationIndices()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorFormationIndices::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.resultPosType == RIG_FORMATION_NAMES );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorFormationIndices::calculate( int                        partIndex,
                                                                                 const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );

    RigFemScalarResultFrames* resFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );
    resFrames->enableAsSingleFrameResult();

    const RigFemPart*   femPart      = m_resultCollection->parts()->part( partIndex );
    std::vector<float>& dstFrameData = resFrames->frameData( 0 );

    size_t valCount = femPart->elementNodeResultCount();
    float  inf      = std::numeric_limits<float>::infinity();
    dstFrameData.resize( valCount, inf );

    const RigFormationNames* activeFormNames = m_resultCollection->activeFormationNames();

    frameCountProgress.incrementProgress();

    if ( activeFormNames )
    {
        // Has to be done before the parallel loop because the first call allocates.
        const RigFemPartGrid* structGrid = femPart->getOrCreateStructGrid();

        int elementCount = femPart->elementCount();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType      = femPart->elementType( elmIdx );
            int            elmNodeCount = RigFemTypes::elementNodeCount( elmType );

            size_t i, j, k;
            bool   validIndex = structGrid->ijkFromCellIndex( elmIdx, &i, &j, &k );
            if ( validIndex )
            {
                int formNameIdx = activeFormNames->formationIndexFromKLayerIdx( k );

                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );

                    if ( formNameIdx != -1 )
                    {
                        dstFrameData[elmNodResIdx] = formNameIdx;
                    }
                    else
                    {
                        dstFrameData[elmNodResIdx] = HUGE_VAL;
                    }
                }
            }
        }
    }

    return resFrames;
}
