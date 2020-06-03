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

#include "RigFemPartResultCalculatorEnIpPorBar.h"

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
RigFemPartResultCalculatorEnIpPorBar::RigFemPartResultCalculatorEnIpPorBar( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorEnIpPorBar::~RigFemPartResultCalculatorEnIpPorBar()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorEnIpPorBar::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "POR-Bar" && resVarAddr.resultPosType != RIG_NODAL );
}

//--------------------------------------------------------------------------------------------------
/// Convert POR NODAL result to POR-Bar Element Nodal result
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorEnIpPorBar::calculate( int                        partIndex,
                                                                           const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemResultAddress       unconvertedResultAddr( RIG_NODAL, "POR", "" );
    RigFemScalarResultFrames* srcDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, unconvertedResultAddr );
    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    frameCountProgress.incrementProgress();

    const RigFemPart* femPart = m_resultCollection->parts()->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData = dstDataFrames->frameData( fIdx );

        if ( srcFrameData.empty() ) continue; // Create empty results if we have no POR result.

        size_t valCount = femPart->elementNodeResultCount();
        dstFrameData.resize( valCount, inf );

        int elementCount = femPart->elementCount();

#pragma omp parallel for schedule( dynamic )
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            if ( elmType == HEX8P )
            {
                int elmNodeCount = RigFemTypes::elementNodeCount( elmType );
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx        = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    int    nodeIdx             = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );
                    dstFrameData[elmNodResIdx] = 1.0e-5 * srcFrameData[nodeIdx];
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    m_resultCollection->deleteResult( unconvertedResultAddr );
    return dstDataFrames;
}
