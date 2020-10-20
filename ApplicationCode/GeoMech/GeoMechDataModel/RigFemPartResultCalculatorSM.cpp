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

#include "RigFemPartResultCalculatorSM.h"

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
RigFemPartResultCalculatorSM::RigFemPartResultCalculatorSM( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorSM::~RigFemPartResultCalculatorSM()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorSM::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.fieldName == "ST" || resVarAddr.fieldName == "SE" ) && resVarAddr.componentName == "SM" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorSM::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( isMatching( resVarAddr ) );

    QString progressText = "Calculating " +
                           QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName );

    caf::ProgressInfo frameCountProgress( static_cast<size_t>( m_resultCollection->frameCount() ) * 4, progressText );

    auto loadFrameLambda = [&]( const QString& component ) {
        auto task = frameCountProgress.task( component );
        return m_resultCollection->findOrLoadScalarResult( partIndex,
                                                           resVarAddr.copyWithComponent( component.toStdString() ) );
    };

    RigFemScalarResultFrames* st11 = loadFrameLambda( "S11" );
    RigFemScalarResultFrames* st22 = loadFrameLambda( "S22" );
    RigFemScalarResultFrames* st33 = loadFrameLambda( "S33" );

    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    int frameCount = st11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        auto task = frameCountProgress.task( QString( "Frame %1" ).arg( fIdx ) );

        const std::vector<float>& st11Data = st11->frameData( fIdx );
        const std::vector<float>& st22Data = st22->frameData( fIdx );
        const std::vector<float>& st33Data = st33->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = st11Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = ( st11Data[vIdx] + st22Data[vIdx] + st33Data[vIdx] ) / 3.0f;
        }
    }

    return dstDataFrames;
}
