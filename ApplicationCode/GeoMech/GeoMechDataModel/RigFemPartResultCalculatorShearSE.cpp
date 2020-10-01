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

#include "RigFemPartResultCalculatorShearSE.h"

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
RigFemPartResultCalculatorShearSE::RigFemPartResultCalculatorShearSE( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorShearSE::~RigFemPartResultCalculatorShearSE()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorShearSE::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.fieldName == "SE" ) && ( resVarAddr.componentName == "S12" || resVarAddr.componentName == "S13" ||
                                                   resVarAddr.componentName == "S23" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorShearSE::calculate( int partIndex, const RigFemResultAddress& resAddr )
{
    QString progressText = "Calculating " + QString::fromStdString( resAddr.fieldName + ": " + resAddr.componentName );

    caf::ProgressInfo frameCountProgress( static_cast<size_t>( m_resultCollection->frameCount() ) * 2, progressText );

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resAddr );

    RigFemScalarResultFrames* srcDataFrames = nullptr;
    {
        auto task     = frameCountProgress.task( "S-Bar", m_resultCollection->frameCount() );
        srcDataFrames = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                                    RigFemResultAddress( resAddr.resultPosType,
                                                                                         "S-Bar",
                                                                                         resAddr.componentName ) );
    }

    const RigFemPart* femPart = m_resultCollection->parts()->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    int frameCount = srcDataFrames->frameCount();

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        auto task = frameCountProgress.task( QString( "Frame %1" ).arg( fIdx ) );

        const std::vector<float>& srcSFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData  = dstDataFrames->frameData( fIdx );
        size_t                    valCount      = srcSFrameData.size();
        dstFrameData.resize( valCount );

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
                    if ( elmNodResIdx < srcSFrameData.size() )
                    {
                        dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx];
                    }
                }
            }
            else
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < dstFrameData.size() )
                    {
                        dstFrameData[elmNodResIdx] = inf;
                    }
                }
            }
        }
    }

    return dstDataFrames;
}
