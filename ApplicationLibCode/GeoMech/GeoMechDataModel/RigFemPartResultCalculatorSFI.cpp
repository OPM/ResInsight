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
RigFemScalarResultFrames* RigFemPartResultCalculatorSFI::calculate( int partIndex, const RigFemResultAddress& resAddr )
{
    CVF_ASSERT( resAddr.fieldName == "SE" && resAddr.componentName == "SFI" );

    QString progressText = "Calculating " + QString::fromStdString( resAddr.fieldName + ": " + resAddr.componentName );

    caf::ProgressInfo frameCountProgress( static_cast<size_t>( m_resultCollection->frameCount() ) * 3, progressText );

    auto loadFrameLambda = [&]( const QString& component ) {
        auto task = frameCountProgress.task( "Loading " + component, m_resultCollection->frameCount() );
        return m_resultCollection->findOrLoadScalarResult( partIndex, resAddr.copyWithComponent( component.toStdString() ) );
    };

    RigFemScalarResultFrames* se1Frames = loadFrameLambda( "S1" );
    RigFemScalarResultFrames* se3Frames = loadFrameLambda( "S3" );

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resAddr );

    float cohPrFricAngle =
        (float)( m_resultCollection->parameterCohesion() / tan( m_resultCollection->parameterFrictionAngleRad() ) );
    float sinFricAng = sin( m_resultCollection->parameterFrictionAngleRad() );

    int frameCount = se1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        auto task = frameCountProgress.task( QString( "Frame %1" ).arg( fIdx ) );

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
    }

    return dstDataFrames;
}
