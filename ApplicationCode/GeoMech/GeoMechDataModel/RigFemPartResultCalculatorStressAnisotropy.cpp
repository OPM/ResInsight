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
    return (
        ( ( resVarAddr.fieldName == "ST" ) && ( resVarAddr.componentName == "STA12" || resVarAddr.componentName == "STA13" ||
                                                resVarAddr.componentName == "STA23" ) ) ||
        ( ( resVarAddr.fieldName == "SE" ) && ( resVarAddr.componentName == "SEA12" || resVarAddr.componentName == "SEA13" ||
                                                resVarAddr.componentName == "SEA23" ) ) );
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
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* s1Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType,
                                                                         resVarAddr.fieldName,
                                                                         "S1" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* s2Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType,
                                                                         resVarAddr.fieldName,
                                                                         "S2" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* s3Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType,
                                                                         resVarAddr.fieldName,
                                                                         "S3" ) );

    RigFemScalarResultFrames* s12Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     resVarAddr.fieldName + "A12" ) );
    RigFemScalarResultFrames* s13Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     resVarAddr.fieldName + "A13" ) );
    RigFemScalarResultFrames* s23Frames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     resVarAddr.fieldName + "A23" ) );

    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( 1u );

    int frameCount = s1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
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

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedStress = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedStress;
}
