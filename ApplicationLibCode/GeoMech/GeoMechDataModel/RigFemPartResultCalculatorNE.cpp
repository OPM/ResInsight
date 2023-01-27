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

#include "RigFemPartResultCalculatorNE.h"

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
RigFemPartResultCalculatorNE::RigFemPartResultCalculatorNE( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorNE::~RigFemPartResultCalculatorNE()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorNE::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.fieldName == "NE" ) &&
             ( resVarAddr.componentName == "E11" || resVarAddr.componentName == "E22" || resVarAddr.componentName == "E33" ||
               resVarAddr.componentName == "E12" || resVarAddr.componentName == "E13" || resVarAddr.componentName == "E23" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorNE::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo stepCountProgress( m_resultCollection->timeStepCount() * 2, "" );
    stepCountProgress.setProgressDescription( "Calculating " +
                                              QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    RigFemScalarResultFrames* srcDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "E", resVarAddr.componentName ) );
    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    stepCountProgress.incrementProgress();

    const int timeSteps = srcDataFrames->timeStepCount();
    for ( int stepIdx = 0; stepIdx < timeSteps; stepIdx++ )
    {
        const int frameCount = srcDataFrames->frameCount( stepIdx );
        for ( int fIdx = 0; fIdx < frameCount; fIdx++ )
        {
            const std::vector<float>& srcFrameData = srcDataFrames->frameData( stepIdx, fIdx );
            std::vector<float>&       dstFrameData = dstDataFrames->frameData( stepIdx, fIdx );
            size_t                    valCount     = srcFrameData.size();
            dstFrameData.resize( valCount );

#pragma omp parallel for
            for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
            {
                dstFrameData[vIdx] = -srcFrameData[vIdx];
            }
        }
        stepCountProgress.incrementProgress();
    }

    return dstDataFrames;
}
