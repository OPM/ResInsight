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

#include "RigFemPartResultCalculatorFOS.h"

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
RigFemPartResultCalculatorFOS::RigFemPartResultCalculatorFOS( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorFOS::~RigFemPartResultCalculatorFOS()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorFOS::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "FOS" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorFOS::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "FOS" );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* dsmFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType, "SE", "DSM" ) );

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = dsmFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& dsmData = dsmFrames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = dsmData.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            float dsm          = dsmData[vIdx];
            dstFrameData[vIdx] = 1.0f / dsm;
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}
