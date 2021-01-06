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

#include "RigFemPartResultCalculatorBarConverted.h"

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
RigFemPartResultCalculatorBarConverted::RigFemPartResultCalculatorBarConverted( RigFemPartResultsCollection& collection,
                                                                                const std::string&           fieldName,
                                                                                const std::string& fieldNameToConvert )
    : RigFemPartResultCalculator( collection )
    , m_fieldName( fieldName )
    , m_fieldNameToConvert( fieldNameToConvert )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorBarConverted::~RigFemPartResultCalculatorBarConverted()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorBarConverted::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    // TODO: split in multiple classes??
    if ( m_fieldName == "POR-Bar" )
    {
        return ( ( resVarAddr.fieldName == "POR-Bar" ) && ( resVarAddr.resultPosType == RIG_NODAL ) &&
                 !( resVarAddr.componentName == "X" || resVarAddr.componentName == "Y" || resVarAddr.componentName == "Z" ) );
    }
    else
    {
        return ( resVarAddr.fieldName == "S-Bar" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorBarConverted::calculate( int                        partIndex,
                                                                             const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemResultAddress unconvertedResultAddr( resVarAddr.resultPosType, m_fieldNameToConvert, resVarAddr.componentName );
    RigFemScalarResultFrames* srcDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, unconvertedResultAddr );
    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData = dstDataFrames->frameData( fIdx );
        size_t                    valCount     = srcFrameData.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = 1.0e-5 * srcFrameData[vIdx];
        }

        frameCountProgress.incrementProgress();
    }
    m_resultCollection->deleteResult( unconvertedResultAddr );
    return dstDataFrames;
}
