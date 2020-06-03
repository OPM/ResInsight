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

#include "RigFemPartResultCalculatorGamma.h"

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
RigFemPartResultCalculatorGamma::RigFemPartResultCalculatorGamma( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorGamma::~RigFemPartResultCalculatorGamma()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorGamma::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "Gamma" &&
             ( resVarAddr.componentName == "Gamma1" || resVarAddr.componentName == "Gamma2" ||
               resVarAddr.componentName == "Gamma3" || resVarAddr.componentName == "Gamma11" ||
               resVarAddr.componentName == "Gamma22" || resVarAddr.componentName == "Gamma33" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorGamma::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemResultAddress totStressCompAddr( resVarAddr.resultPosType, "ST", "" );
    {
        std::string scomp;
        std::string gcomp = resVarAddr.componentName;
        if ( gcomp == "Gamma1" )
            scomp = "S1";
        else if ( gcomp == "Gamma2" )
            scomp = "S2";
        else if ( gcomp == "Gamma3" )
            scomp = "S3";
        else if ( gcomp == "Gamma11" )
            scomp = "S11";
        else if ( gcomp == "Gamma22" )
            scomp = "S22";
        else if ( gcomp == "Gamma33" )
            scomp = "S33";
        totStressCompAddr.componentName = scomp;
    }

    RigFemScalarResultFrames* srcDataFrames = m_resultCollection->findOrLoadScalarResult( partIndex, totStressCompAddr );

    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* srcPORDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, "POR-Bar", "" ) );
    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    frameCountProgress.incrementProgress();

    calculateGammaFromFrames( partIndex,
                              m_resultCollection->parts(),
                              srcDataFrames,
                              srcPORDataFrames,
                              dstDataFrames,
                              &frameCountProgress );

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultCalculatorGamma::calculateGammaFromFrames( int                         partIndex,
                                                                const RigFemPartCollection* femParts,
                                                                const RigFemScalarResultFrames* totalStressComponentDataFrames,
                                                                const RigFemScalarResultFrames* srcPORDataFrames,
                                                                RigFemScalarResultFrames*       dstDataFrames,
                                                                caf::ProgressInfo*              frameCountProgress )
{
    const RigFemPart* femPart    = femParts->part( partIndex );
    int               frameCount = totalStressComponentDataFrames->frameCount();
    float             inf        = std::numeric_limits<float>::infinity();

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcSTFrameData  = totalStressComponentDataFrames->frameData( fIdx );
        const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );

        size_t valCount = srcSTFrameData.size();
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
                    if ( elmNodResIdx < srcSTFrameData.size() )
                    {
                        int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );

                        float por = srcPORFrameData[nodeIdx];

                        if ( por == inf || fabs( por ) < 0.01e6 * 1.0e-5 )
                            dstFrameData[elmNodResIdx] = inf;
                        else
                            dstFrameData[elmNodResIdx] = srcSTFrameData[elmNodResIdx] / por;
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

        frameCountProgress->incrementProgress();
    }
}
