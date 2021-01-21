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

#include "RigFemPartResultCalculatorInitialPorosity.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorInitialPorosity::RigFemPartResultCalculatorInitialPorosity( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorInitialPorosity::~RigFemPartResultCalculatorInitialPorosity()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorInitialPorosity::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "PORO-PERM" && resVarAddr.componentName == "PHI0" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorInitialPorosity::calculate( int                        partIndex,
                                                                                const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription( "Calculating Initial Porosity" );

    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* voidRatioFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType, "VOIDR", "" ) );

    RigFemScalarResultFrames* porosityFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "PHI0" ) );
    frameCountProgress.incrementProgress();

    const RigFemPart* femPart = m_resultCollection->parts()->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    frameCountProgress.setNextProgressIncrement( 1u );

    int frameCount = voidRatioFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& voidRatioData     = voidRatioFrames->frameData( 0 );
        std::vector<float>&       porosityFrameData = porosityFrames->frameData( fIdx );

        size_t valCount = voidRatioData.size();
        porosityFrameData.resize( valCount );

        int elementCount = femPart->elementCount();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            int elmNodeCount = RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) );

            if ( elmType == HEX8P )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < voidRatioData.size() )
                    {
                        int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );

                        // Calculate initial porosity
                        double voidr           = voidRatioData[elmNodResIdx];
                        double initialPorosity = voidr / ( 1.0 + voidr );

                        porosityFrameData[elmNodResIdx] = initialPorosity;
                    }
                }
            }
            else
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < voidRatioData.size() )
                    {
                        porosityFrameData[elmNodResIdx] = inf;
                    }
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedResultFrames = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedResultFrames;
}
