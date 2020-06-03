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

#include "RigFemPartResultCalculatorNodalGradients.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigHexGradientTools.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorNodalGradients::RigFemPartResultCalculatorNodalGradients( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorNodalGradients::~RigFemPartResultCalculatorNodalGradients()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorNodalGradients::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.fieldName == "POR-Bar" ) && ( resVarAddr.resultPosType == RIG_NODAL ) &&
             ( resVarAddr.componentName == "X" || resVarAddr.componentName == "Y" || resVarAddr.componentName == "Z" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorNodalGradients::calculate( int                        partIndex,
                                                                               const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "POR-Bar" );
    CVF_ASSERT( resVarAddr.componentName == "X" || resVarAddr.componentName == "Y" || resVarAddr.componentName == "Z" );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 5, "" );
    frameCountProgress.setProgressDescription(
        "Calculating gradient: " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* dataFramesX =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "X" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* dataFramesY =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "Y" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* dataFramesZ =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "Z" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemResultAddress       porResultAddr( RIG_NODAL, "POR-Bar", "" );
    RigFemScalarResultFrames* srcDataFrames = m_resultCollection->findOrLoadScalarResult( partIndex, porResultAddr );

    frameCountProgress.incrementProgress();

    const RigFemPart* femPart = m_resultCollection->parts()->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;
    size_t                         nodeCount  = femPart->nodes().nodeIds.size();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData  = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameDataX = dataFramesX->frameData( fIdx );
        std::vector<float>&       dstFrameDataY = dataFramesY->frameData( fIdx );
        std::vector<float>&       dstFrameDataZ = dataFramesZ->frameData( fIdx );

        if ( srcFrameData.empty() ) continue; // Create empty results if we have no POR result.

        size_t valCount = femPart->elementNodeResultCount();
        dstFrameDataX.resize( valCount, inf );
        dstFrameDataY.resize( valCount, inf );
        dstFrameDataZ.resize( valCount, inf );

        int elementCount = femPart->elementCount();

#pragma omp parallel for schedule( dynamic )
        for ( long nodeIdx = 0; nodeIdx < static_cast<long>( nodeCount ); nodeIdx++ )
        {
            const std::vector<int> elements = femPart->elementsUsingNode( nodeIdx );

            // Compute the average of the elements contributing to this node
            cvf::Vec3d result         = cvf::Vec3d::ZERO;
            int        nValidElements = 0;
            for ( int elmIdx : elements )
            {
                RigElementType elmType = femPart->elementType( elmIdx );
                if ( elmType == HEX8P )
                {
                    // Find the corner coordinates and values for the node
                    std::array<cvf::Vec3d, 8> hexCorners;
                    std::array<double, 8>     cornerValues;

                    int elmNodeCount = RigFemTypes::elementNodeCount( elmType );
                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx   = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        size_t resultValueIdx = femPart->resultValueIdxFromResultPosType( RIG_NODAL, elmIdx, elmNodIdx );

                        cornerValues[elmNodIdx] = srcFrameData[resultValueIdx];
                        hexCorners[elmNodIdx]   = cvf::Vec3d( nodeCoords[resultValueIdx] );
                    }

                    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx   = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        size_t resultValueIdx = femPart->resultValueIdxFromResultPosType( RIG_NODAL, elmIdx, elmNodIdx );
                        // Only use the gradient for particular corner corresponding to the node
                        if ( static_cast<size_t>( nodeIdx ) == resultValueIdx )
                        {
                            result.add( gradients[elmNodIdx] );
                        }
                    }

                    nValidElements++;
                }
            }

            if ( nValidElements > 0 )
            {
                // Round very small values to zero to avoid ugly coloring when gradients
                // are dominated by floating point math artifacts.
                double epsilon = 1.0e-6;
                result /= static_cast<double>( nValidElements );
                dstFrameDataX[nodeIdx] = std::abs( result.x() ) > epsilon ? result.x() : 0.0;
                dstFrameDataY[nodeIdx] = std::abs( result.y() ) > epsilon ? result.y() : 0.0;
                dstFrameDataZ[nodeIdx] = std::abs( result.z() ) > epsilon ? result.z() : 0.0;
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedGradient = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    CVF_ASSERT( requestedGradient );
    return requestedGradient;
}
