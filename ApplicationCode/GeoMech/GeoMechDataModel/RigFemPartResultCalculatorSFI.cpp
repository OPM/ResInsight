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

#include "RigFemPartResultCalculatorSFI.h"

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
RigFemPartResultCalculatorSFI::RigFemPartResultCalculatorSFI( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorSFI::~RigFemPartResultCalculatorSFI()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorSFI::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SFI" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorSFI::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SFI" );
    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* se1Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S1" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* se3Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S3" ) );

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    frameCountProgress.incrementProgress();

    float cohPrFricAngle =
        (float)( m_resultCollection->parameterCohesion() / tan( m_resultCollection->parameterFrictionAngleRad() ) );
    float sinFricAng = sin( m_resultCollection->parameterFrictionAngleRad() );

    int frameCount = se1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& se1Data = se1Frames->frameData( fIdx );
        const std::vector<float>& se3Data = se3Frames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = se1Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            float se1        = se1Data[vIdx];
            float se3        = se3Data[vIdx];
            float se1Se3Diff = se1 - se3;

            if ( fabs( se1Se3Diff ) < 1e-7 )
            {
                dstFrameData[vIdx] = std::numeric_limits<float>::infinity();
            }
            else
            {
                dstFrameData[vIdx] = ( ( cohPrFricAngle + 0.5 * ( se1Data[vIdx] + se3Data[vIdx] ) ) * sinFricAng ) /
                                     ( 0.5 * ( se1Data[vIdx] - se3Data[vIdx] ) );
            }
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}
