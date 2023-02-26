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

#include "RigFemPartResultCalculatorPrincipalStress.h"

#include "RiaOffshoreSphericalCoords.h"

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
RigFemPartResultCalculatorPrincipalStress::RigFemPartResultCalculatorPrincipalStress( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorPrincipalStress::~RigFemPartResultCalculatorPrincipalStress()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorPrincipalStress::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.fieldName == "SE" || resVarAddr.fieldName == "ST" ) &&
             ( resVarAddr.componentName == "S1" || resVarAddr.componentName == "S2" || resVarAddr.componentName == "S3" ||
               resVarAddr.componentName == "S1inc" || resVarAddr.componentName == "S1azi" || resVarAddr.componentName == "S2inc" ||
               resVarAddr.componentName == "S2azi" || resVarAddr.componentName == "S3inc" || resVarAddr.componentName == "S3azi" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorPrincipalStress::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.componentName == "S1" || resVarAddr.componentName == "S2" || resVarAddr.componentName == "S3" ||
                resVarAddr.componentName == "S1inc" || resVarAddr.componentName == "S1azi" || resVarAddr.componentName == "S2inc" ||
                resVarAddr.componentName == "S2azi" || resVarAddr.componentName == "S3inc" || resVarAddr.componentName == "S3azi" );

    caf::ProgressInfo stepCountProgress( m_resultCollection->timeStepCount() * 7, "" );
    stepCountProgress.setProgressDescription( "Calculating " +
                                              QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    RigFemScalarResultFrames* s11Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S11" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* s22Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S22" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* s33Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S33" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* s12Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S12" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* s13Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S13" ) );
    stepCountProgress.incrementProgress();
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* s23Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S23" ) );

    RigFemScalarResultFrames* s1Frames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S1" ) );
    RigFemScalarResultFrames* s2Frames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S2" ) );
    RigFemScalarResultFrames* s3Frames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S3" ) );

    RigFemScalarResultFrames* s1IncFrames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S1inc" ) );
    RigFemScalarResultFrames* s1AziFrames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S1azi" ) );
    RigFemScalarResultFrames* s2IncFrames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S2inc" ) );
    RigFemScalarResultFrames* s2AziFrames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S2azi" ) );
    RigFemScalarResultFrames* s3IncFrames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S3inc" ) );
    RigFemScalarResultFrames* s3AziFrames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S3azi" ) );

    stepCountProgress.incrementProgress();

    const int timeSteps = s11Frames->timeStepCount();
    for ( int stepIdx = 0; stepIdx < timeSteps; stepIdx++ )
    {
        const int frameCount = s11Frames->frameCount( stepIdx );
        for ( int fIdx = 0; fIdx < frameCount; fIdx++ )
        {
            const std::vector<float>& s11 = s11Frames->frameData( stepIdx, fIdx );
            const std::vector<float>& s22 = s22Frames->frameData( stepIdx, fIdx );
            const std::vector<float>& s33 = s33Frames->frameData( stepIdx, fIdx );
            const std::vector<float>& s12 = s12Frames->frameData( stepIdx, fIdx );
            const std::vector<float>& s13 = s13Frames->frameData( stepIdx, fIdx );
            const std::vector<float>& s23 = s23Frames->frameData( stepIdx, fIdx );

            std::vector<float>& s1 = s1Frames->frameData( stepIdx, fIdx );
            std::vector<float>& s2 = s2Frames->frameData( stepIdx, fIdx );
            std::vector<float>& s3 = s3Frames->frameData( stepIdx, fIdx );

            std::vector<float>& s1inc = s1IncFrames->frameData( stepIdx, fIdx );
            std::vector<float>& s1azi = s1AziFrames->frameData( stepIdx, fIdx );
            std::vector<float>& s2inc = s2IncFrames->frameData( stepIdx, fIdx );
            std::vector<float>& s2azi = s2AziFrames->frameData( stepIdx, fIdx );
            std::vector<float>& s3inc = s3IncFrames->frameData( stepIdx, fIdx );
            std::vector<float>& s3azi = s3AziFrames->frameData( stepIdx, fIdx );

            size_t valCount = s11.size();

            s1.resize( valCount );
            s2.resize( valCount );
            s3.resize( valCount );
            s1inc.resize( valCount );
            s1azi.resize( valCount );
            s2inc.resize( valCount );
            s2azi.resize( valCount );
            s3inc.resize( valCount );
            s3azi.resize( valCount );

#pragma omp parallel for schedule( dynamic )
            for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
            {
                caf::Ten3f T( s11[vIdx], s22[vIdx], s33[vIdx], s12[vIdx], s23[vIdx], s13[vIdx] );
                cvf::Vec3f principalDirs[3];
                cvf::Vec3f principals = T.calculatePrincipals( principalDirs );
                s1[vIdx]              = principals[0];
                s2[vIdx]              = principals[1];
                s3[vIdx]              = principals[2];

                if ( principals[0] != std::numeric_limits<float>::infinity() )
                {
                    RiaOffshoreSphericalCoords sphCoord1( principalDirs[0] );
                    s1inc[vIdx] = cvf::Math::toDegrees( sphCoord1.inc() );
                    s1azi[vIdx] = cvf::Math::toDegrees( sphCoord1.azi() );
                }
                else
                {
                    s1inc[vIdx] = std::numeric_limits<float>::infinity();
                    s1azi[vIdx] = std::numeric_limits<float>::infinity();
                }

                if ( principals[1] != std::numeric_limits<float>::infinity() )
                {
                    RiaOffshoreSphericalCoords sphCoord2( principalDirs[1] );
                    s2inc[vIdx] = cvf::Math::toDegrees( sphCoord2.inc() );
                    s2azi[vIdx] = cvf::Math::toDegrees( sphCoord2.azi() );
                }
                else
                {
                    s2inc[vIdx] = std::numeric_limits<float>::infinity();
                    s2azi[vIdx] = std::numeric_limits<float>::infinity();
                }

                if ( principals[2] != std::numeric_limits<float>::infinity() )
                {
                    RiaOffshoreSphericalCoords sphCoord3( principalDirs[2] );
                    s3inc[vIdx] = cvf::Math::toDegrees( sphCoord3.inc() );
                    s3azi[vIdx] = cvf::Math::toDegrees( sphCoord3.azi() );
                }
                else
                {
                    s3inc[vIdx] = std::numeric_limits<float>::infinity();
                    s3azi[vIdx] = std::numeric_limits<float>::infinity();
                }
            }
        }
        stepCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPrincipal = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );

    return requestedPrincipal;
}
