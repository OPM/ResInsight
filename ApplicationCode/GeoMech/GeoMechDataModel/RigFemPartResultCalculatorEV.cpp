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

#include "RigFemPartResultCalculatorEV.h"

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
RigFemPartResultCalculatorEV::RigFemPartResultCalculatorEV( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorEV::~RigFemPartResultCalculatorEV()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorEV::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "EV" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorEV::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "EV" );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 4, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* ea11 = nullptr;
    RigFemScalarResultFrames* ea22 = nullptr;
    RigFemScalarResultFrames* ea33 = nullptr;

    {
        ea11 = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                           RigFemResultAddress( resVarAddr.resultPosType, "NE", "E11" ) );
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
        ea22 = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                           RigFemResultAddress( resVarAddr.resultPosType, "NE", "E22" ) );
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
        ea33 = m_resultCollection->findOrLoadScalarResult( partIndex,
                                                           RigFemResultAddress( resVarAddr.resultPosType, "NE", "E33" ) );
    }

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = ea11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& ea11Data = ea11->frameData( fIdx );
        const std::vector<float>& ea22Data = ea22->frameData( fIdx );
        const std::vector<float>& ea33Data = ea33->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = ea11Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = ( ea11Data[vIdx] + ea22Data[vIdx] + ea33Data[vIdx] );
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}
