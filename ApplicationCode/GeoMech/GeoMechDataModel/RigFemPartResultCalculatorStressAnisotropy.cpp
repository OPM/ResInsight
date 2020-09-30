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

#include "RigFemPartResultCalculatorStressAnisotropy.h"

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
RigFemPartResultCalculatorStressAnisotropy::RigFemPartResultCalculatorStressAnisotropy( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorStressAnisotropy::~RigFemPartResultCalculatorStressAnisotropy()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorStressAnisotropy::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( ( resVarAddr.fieldName == "ST" || resVarAddr.fieldName == "SE" ) &&
               ( resVarAddr.componentName == "SA12" || resVarAddr.componentName == "SA13" ||
                 resVarAddr.componentName == "SA23" ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorStressAnisotropy::calculate( int                        partIndex,
                                                                                 const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( isMatching( resVarAddr ) );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 4, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );

    RigFemScalarResultFrames* s1Frames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading S1.", m_resultCollection->frameCount() );
        s1Frames  = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                               RigFemResultAddress( resVarAddr.resultPosType,
                                                                                    resVarAddr.fieldName,
                                                                                    "S1" ) );
    }

    RigFemScalarResultFrames* s2Frames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading S2.", m_resultCollection->frameCount() );
        s2Frames  = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                               RigFemResultAddress( resVarAddr.resultPosType,
                                                                                    resVarAddr.fieldName,
                                                                                    "S2" ) );
    }

    RigFemScalarResultFrames* s3Frames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading S3.", m_resultCollection->frameCount() );
        s3Frames  = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                               RigFemResultAddress( resVarAddr.resultPosType,
                                                                                    resVarAddr.fieldName,
                                                                                    "S3" ) );
    }

    RigFemScalarResultFrames* s12Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "SA12" ) );
    RigFemScalarResultFrames* s13Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "SA13" ) );
    RigFemScalarResultFrames* s23Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "SA23" ) );

    int frameCount = s1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        auto task = frameCountProgress.task( QString( "Calculating %1/%2." ).arg( fIdx ).arg( frameCount - 1 ) );

        const std::vector<float>& s1 = s1Frames->frameData( fIdx );
        const std::vector<float>& s2 = s2Frames->frameData( fIdx );
        const std::vector<float>& s3 = s3Frames->frameData( fIdx );

        std::vector<float>& s12 = s12Frames->frameData( fIdx );
        std::vector<float>& s13 = s13Frames->frameData( fIdx );
        std::vector<float>& s23 = s23Frames->frameData( fIdx );

        size_t valCount = s1.size();

        s12.resize( valCount );
        s13.resize( valCount );
        s23.resize( valCount );

#pragma omp parallel for schedule( dynamic )
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            s12[vIdx] = 2.0 * ( s1[vIdx] - s2[vIdx] ) / ( s1[vIdx] + s2[vIdx] );
            s13[vIdx] = 2.0 * ( s1[vIdx] - s3[vIdx] ) / ( s1[vIdx] + s3[vIdx] );
            s23[vIdx] = 2.0 * ( s2[vIdx] - s3[vIdx] ) / ( s2[vIdx] + s3[vIdx] );
        }
    }

    RigFemScalarResultFrames* requestedStress = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedStress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultCalculatorStressAnisotropy::calculateTimeLapse( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( isMatching( resVarAddr ) );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 4, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );

    RigFemScalarResultFrames* s1Frames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading S1.", m_resultCollection->frameCount() );
        s1Frames  = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                               RigFemResultAddress( resVarAddr.resultPosType,
                                                                                    resVarAddr.fieldName,
                                                                                    "S1" ) );
    }

    RigFemScalarResultFrames* s2Frames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading S2.", m_resultCollection->frameCount() );
        s2Frames  = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                               RigFemResultAddress( resVarAddr.resultPosType,
                                                                                    resVarAddr.fieldName,
                                                                                    "S2" ) );
    }

    RigFemScalarResultFrames* s3Frames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading S3.", m_resultCollection->frameCount() );
        s3Frames  = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                               RigFemResultAddress( resVarAddr.resultPosType,
                                                                                    resVarAddr.fieldName,
                                                                                    "S3" ) );
    }

    RigFemScalarResultFrames* s12Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     "SA12",
                                                                     resVarAddr.timeLapseBaseFrameIdx ) );
    RigFemScalarResultFrames* s13Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     "SA13",
                                                                     resVarAddr.timeLapseBaseFrameIdx ) );
    RigFemScalarResultFrames* s23Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     "SA23",
                                                                     resVarAddr.timeLapseBaseFrameIdx ) );

    float inf          = std::numeric_limits<float>::infinity();
    int   frameCount   = s1Frames->frameCount();
    int   baseTimeStep = resVarAddr.timeLapseBaseFrameIdx;

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        auto task = frameCountProgress.task( QString( "Calculating %1/%2." ).arg( fIdx ).arg( frameCount - 1 ) );

        const std::vector<float>& s1t = s1Frames->frameData( fIdx );
        const std::vector<float>& s2t = s2Frames->frameData( fIdx );
        const std::vector<float>& s3t = s3Frames->frameData( fIdx );

        const std::vector<float>& s1b = s1Frames->frameData( baseTimeStep );
        const std::vector<float>& s2b = s2Frames->frameData( baseTimeStep );
        const std::vector<float>& s3b = s3Frames->frameData( baseTimeStep );

        std::vector<float>& s12 = s12Frames->frameData( fIdx );
        std::vector<float>& s13 = s13Frames->frameData( fIdx );
        std::vector<float>& s23 = s23Frames->frameData( fIdx );

        size_t valCount = s1t.size();

        s12.resize( valCount, 0.0 );
        s13.resize( valCount, 0.0 );
        s23.resize( valCount, 0.0 );

#pragma omp parallel for schedule( dynamic )
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            if ( fIdx != baseTimeStep )
            {
                double diffS1 = s1t[vIdx] - s1b[vIdx];
                double diffS2 = s2t[vIdx] - s2b[vIdx];
                double diffS3 = s3t[vIdx] - s3b[vIdx];
                if ( diffS1 + diffS2 != 0.0 )
                    s12[vIdx] = 2.0 * ( diffS1 - diffS2 ) / ( diffS1 + diffS2 );
                else
                    s12[vIdx] = inf;

                if ( diffS1 + diffS3 != 0.0 )
                    s13[vIdx] = 2.0 * ( diffS1 - diffS3 ) / ( diffS1 + diffS3 );
                else
                    s13[vIdx] = inf;

                if ( diffS2 + diffS3 != 0.0 )
                    s23[vIdx] = 2.0 * ( diffS2 - diffS3 ) / ( diffS2 + diffS3 );
                else
                    s23[vIdx] = inf;
            }
        }
    }

    RigFemScalarResultFrames* requestedStress = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedStress;
}
