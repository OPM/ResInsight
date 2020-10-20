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

#include "RigFemPartResultCalculatorTimeLapse.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultCalculatorGamma.h"
#include "RigFemPartResultCalculatorNormalized.h"
#include "RigFemPartResultCalculatorStressAnisotropy.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorTimeLapse::RigFemPartResultCalculatorTimeLapse( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorTimeLapse::~RigFemPartResultCalculatorTimeLapse()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorTimeLapse::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return resVarAddr.isTimeLapse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorTimeLapse::calculate( int                        partIndex,
                                                                          const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.isTimeLapse() );

    RigFemPartResultCalculatorStressAnisotropy anisotropyCalculator( *m_resultCollection );
    if ( resVarAddr.fieldName == "Gamma" )
    {
        return calculateGammaTimeLapse( partIndex, resVarAddr );
    }
    else if ( anisotropyCalculator.isMatching( resVarAddr ) )
    {
        return anisotropyCalculator.calculateTimeLapse( partIndex, resVarAddr );
    }
    else
    {
        return calculateTimeLapse( partIndex, resVarAddr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultCalculatorTimeLapse::calculateTimeLapse( int partIndex, const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemResultAddress resVarNative( resVarAddr.resultPosType,
                                      resVarAddr.fieldName,
                                      resVarAddr.componentName,
                                      RigFemResultAddress::noTimeLapseValue(),
                                      resVarAddr.refKLayerIndex );
    resVarNative.normalizedByHydrostaticPressure = resVarAddr.normalizedByHydrostaticPressure;

    RigFemScalarResultFrames* srcDataFrames = nullptr;
    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    // Normalizable result: needs to be normalized before the diff is calculated
    if ( resVarAddr.normalizeByHydrostaticPressure() && RigFemPartResultsCollection::isNormalizableResult( resVarAddr ) )
    {
        RigFemPartResultCalculatorNormalized normalizedCalculator( *m_resultCollection );
        srcDataFrames = normalizedCalculator.calculate( partIndex, resVarNative );
    }
    else
    {
        srcDataFrames = m_resultCollection->findOrLoadScalarResult( partIndex, resVarNative );
    }

    frameCountProgress.incrementProgress();

    int frameCount   = srcDataFrames->frameCount();
    int baseFrameIdx = resVarAddr.timeLapseBaseFrameIdx;
    if ( baseFrameIdx >= frameCount ) return dstDataFrames;
    const std::vector<float>& baseFrameData = srcDataFrames->frameData( baseFrameIdx );
    if ( baseFrameData.empty() ) return dstDataFrames;

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        if ( srcFrameData.empty() ) continue; // Create empty results

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = srcFrameData.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = srcFrameData[vIdx] - baseFrameData[vIdx];
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultCalculatorTimeLapse::calculateGammaTimeLapse( int partIndex, const RigFemResultAddress& resVarAddr )
{
    // Gamma time lapse needs to be calculated as ST_dt / POR_dt and not Gamma - Gamma_baseFrame see github
    // issue #937

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemResultAddress totStressCompAddr( resVarAddr.resultPosType, "ST", "", resVarAddr.timeLapseBaseFrameIdx );
    {
        std::string scomp;
        std::string gcomp = resVarAddr.componentName;
        if ( gcomp == "Gamma1" )
            scomp = "S1";
        else if ( gcomp == "Gamma2" )
            scomp = "S2";
        else if ( gcomp == "Gamma3" )
            scomp = "S3";
        else if ( gcomp == "Gamma11" )
            scomp = "S11";
        else if ( gcomp == "Gamma22" )
            scomp = "S22";
        else if ( gcomp == "Gamma33" )
            scomp = "S33";
        totStressCompAddr.componentName = scomp;
    }

    RigFemScalarResultFrames* srcDataFrames = m_resultCollection->findOrLoadScalarResult( partIndex, totStressCompAddr );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* srcPORDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( RIG_NODAL,
                                                                         "POR-Bar",
                                                                         "",
                                                                         resVarAddr.timeLapseBaseFrameIdx ) );
    RigFemScalarResultFrames* dstDataFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    frameCountProgress.incrementProgress();

    RigFemPartResultCalculatorGamma::calculateGammaFromFrames( partIndex,
                                                               m_resultCollection->parts(),
                                                               srcDataFrames,
                                                               srcPORDataFrames,
                                                               dstDataFrames,
                                                               &frameCountProgress );
    if ( resVarAddr.normalizeByHydrostaticPressure() && RigFemPartResultsCollection::isNormalizableResult( resVarAddr ) )
    {
        RigFemPartResultCalculatorNormalized normalizedCalculator( *m_resultCollection );
        dstDataFrames = normalizedCalculator.calculate( partIndex, resVarAddr );
    }

    return dstDataFrames;
}
