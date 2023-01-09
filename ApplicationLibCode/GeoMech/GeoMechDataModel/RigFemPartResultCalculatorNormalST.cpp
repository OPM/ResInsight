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

#include "RigFemPartResultCalculatorNormalST.h"

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
RigFemPartResultCalculatorNormalST::RigFemPartResultCalculatorNormalST( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorNormalST::~RigFemPartResultCalculatorNormalST()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorNormalST::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.fieldName == "ST" ) && ( resVarAddr.componentName == "S11" || resVarAddr.componentName == "S22" ||
                                                   resVarAddr.componentName == "S33" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorNormalST::calculate( int                        partIndex,
                                                                         const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo stepCountProgress( m_resultCollection->timeStepCount() * 3, "" );
    stepCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    RigFemScalarResultFrames* srcSDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType,
                                                                         "S-Bar",
                                                                         resVarAddr.componentName ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    RigFemScalarResultFrames* srcPORDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, "POR-Bar", "" ) );

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );
    const RigFemPart*         femPart       = m_resultCollection->parts()->part( partIndex );

    stepCountProgress.incrementProgress();

    constexpr float inf = std::numeric_limits<float>::infinity();

    int timeSteps = srcPORDataFrames->timeStepCount();
    for ( int stepIdx = 0; stepIdx < timeSteps; stepIdx++ )
    {
        for ( int fIdx = 0; fIdx < srcPORDataFrames->frameCount( stepIdx ); fIdx++ )
        {
            const std::vector<float>& srcSFrameData   = srcSDataFrames->frameData( stepIdx, fIdx );
            const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData( stepIdx, fIdx );

            int elementCount = femPart->elementCount();

            std::vector<float>& dstFrameData = dstDataFrames->frameData( stepIdx, fIdx );

            size_t valCount = srcSFrameData.size();
            dstFrameData.resize( valCount );

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
                        if ( elmNodResIdx < srcSFrameData.size() )
                        {
                            int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );

                            float por = srcPORFrameData[nodeIdx];
                            if ( por == inf ) por = 0.0f;

                            // ST = SE_abacus + porePressure
                            // porePressure is POR-Bar.
                            double SE_abacus           = -srcSFrameData[elmNodResIdx];
                            dstFrameData[elmNodResIdx] = SE_abacus + por;
                        }
                    }
                }
                else
                {
                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        if ( elmNodResIdx < srcSFrameData.size() )
                        {
                            dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx];
                        }
                    }
                }
            }
        }
        stepCountProgress.incrementProgress();
    }

    return dstDataFrames;
}
