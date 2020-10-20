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

#include "RigFemPartResultCalculatorDSM.h"

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
RigFemPartResultCalculatorDSM::RigFemPartResultCalculatorDSM( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorDSM::~RigFemPartResultCalculatorDSM()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorDSM::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "DSM" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorDSM::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "DSM" );

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

    float tanFricAng        = tan( m_resultCollection->parameterFrictionAngleRad() );
    float cohPrTanFricAngle = (float)( m_resultCollection->parameterCohesion() / tanFricAng );
    int   frameCount        = se1Frames->frameCount();
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
            dstFrameData[vIdx] = dsm( se1Data[vIdx], se3Data[vIdx], tanFricAng, cohPrTanFricAngle );
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RigFemPartResultCalculatorDSM::dsm( float p1, float p3, float tanFricAng, float cohPrTanFricAngle )
{
    if ( p1 == HUGE_VAL || p3 == HUGE_VAL )
    {
        return std::nan( "" );
    }

    CVF_ASSERT( p1 > p3 );

    float pi_4 = 0.785398163397448309616f;
    float rho  = 2.0f * ( atan( sqrt( ( p1 + cohPrTanFricAngle ) / ( p3 + cohPrTanFricAngle ) ) ) - pi_4 );

    return tan( rho ) / tanFricAng;
}
