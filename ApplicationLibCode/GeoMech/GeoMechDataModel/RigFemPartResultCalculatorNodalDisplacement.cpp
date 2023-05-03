/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RigFemPartResultCalculatorNodalDisplacement.h"

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
RigFemPartResultCalculatorNodalDisplacement::RigFemPartResultCalculatorNodalDisplacement( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorNodalDisplacement::~RigFemPartResultCalculatorNodalDisplacement()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorNodalDisplacement::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.resultPosType == RIG_NODAL ) && ( resVarAddr.fieldName == "U" ) && ( resVarAddr.componentName == "U_LENGTH" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorNodalDisplacement::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "U" );
    CVF_ASSERT( resVarAddr.componentName == "U_LENGTH" );

    // Why calculating by 5?
    caf::ProgressInfo stepCountProgress( m_resultCollection->timeStepCount() * 5, "" );
    stepCountProgress.setProgressDescription( "Calculating vector length: " +
                                              QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    const RigFemScalarResultFrames* srcFramesU1 =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "U1" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    const RigFemScalarResultFrames* srcFramesU2 =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "U2" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    const RigFemScalarResultFrames* srcFramesU3 =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "U3" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    CVF_ASSERT( srcFramesU1->timeStepCount() == srcFramesU2->timeStepCount() && srcFramesU2->timeStepCount() == srcFramesU3->timeStepCount() );

    RigFemResultAddress       dstVarAddrULength( RIG_NODAL, resVarAddr.fieldName, resVarAddr.componentName );
    RigFemScalarResultFrames* dstFramesULength = m_resultCollection->createScalarResult( partIndex, dstVarAddrULength );

    stepCountProgress.incrementProgress();

    const RigFemPart* femPart       = m_resultCollection->parts()->part( partIndex );
    constexpr float   inf           = std::numeric_limits<float>::infinity();
    const int         timeStepCount = m_resultCollection->timeStepCount();
    for ( int tIdx = 0; tIdx < timeStepCount; ++tIdx )
    {
        const int frameCount = m_resultCollection->frameCount( tIdx );
        for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
        {
            std::vector<float>&       dstFrameDataULength = dstFramesULength->frameData( tIdx, fIdx );
            const std::vector<float>& srcFrameDataU1      = srcFramesU1->frameData( tIdx, fIdx );
            const std::vector<float>& srcFrameDataU2      = srcFramesU2->frameData( tIdx, fIdx );
            const std::vector<float>& srcFrameDataU3      = srcFramesU3->frameData( tIdx, fIdx );

            // Create empty results if:
            // - One or more vector components are missing
            // - Vector components are not of equal size
            if ( srcFrameDataU1.empty() || srcFrameDataU2.empty() || srcFrameDataU3.empty() ||
                 ( srcFrameDataU1.size() != srcFrameDataU2.size() || srcFrameDataU2.size() != srcFrameDataU3.size() ) )
            {
                dstFrameDataULength.clear();
                continue;
            }

            dstFrameDataULength.resize( srcFrameDataU1.size(), 0.0 );
#pragma omp parallel for
            for ( int i = 0; i < static_cast<int>( dstFrameDataULength.size() ); ++i )
            {
                const float u1         = srcFrameDataU1[i];
                const float u2         = srcFrameDataU2[i];
                const float u3         = srcFrameDataU3[i];
                const float uLength    = sqrt( u1 * u1 + u2 * u2 + u3 * u3 );
                dstFrameDataULength[i] = uLength;
            }
        }
        stepCountProgress.incrementProgress();
    }
    return dstFramesULength;
}
