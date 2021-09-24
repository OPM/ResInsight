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

#include "RigFemPartResultCalculatorNormalized.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "RiaWellLogUnitTools.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorNormalized::RigFemPartResultCalculatorNormalized( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorNormalized::~RigFemPartResultCalculatorNormalized()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorNormalized::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.normalizeByHydrostaticPressure() &&
             RigFemPartResultsCollection::isNormalizableResult( resVarAddr ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorNormalized::calculate( int                        partIndex,
                                                                           const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.normalizeByHydrostaticPressure() && isNormalizableResult( resVarAddr ) );

    RigFemResultAddress unscaledResult = resVarAddr;
    if ( unscaledResult.resultPosType == RIG_NODAL && unscaledResult.fieldName == "POR-Bar" )
        unscaledResult.resultPosType = RIG_ELEMENT_NODAL;
    unscaledResult.normalizedByHydrostaticPressure = false;

    CAF_ASSERT( unscaledResult.resultPosType == RIG_ELEMENT_NODAL );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 4, "Calculating Normalized Result" );

    RigFemScalarResultFrames* porDataFrames = nullptr;
    RigFemScalarResultFrames* srcDataFrames = nullptr;
    RigFemScalarResultFrames* dstDataFrames = nullptr;

    {
        auto task = frameCountProgress.task( "Loading POR Result", m_resultCollection->frameCount() );
        porDataFrames =
            m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, "POR-Bar", "" ) );
        if ( !porDataFrames ) return nullptr;
    }

    {
        auto task     = frameCountProgress.task( "Loading Unscaled Result", m_resultCollection->frameCount() );
        srcDataFrames = m_resultCollection->findOrLoadScalarResult( partIndex, unscaledResult );
        if ( !srcDataFrames ) return nullptr;
    }
    {
        auto task = frameCountProgress.task( "Creating Space for Normalized Result", m_resultCollection->frameCount() );
        dstDataFrames = m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr ) );
        if ( !dstDataFrames ) return nullptr;
    }

    frameCountProgress.setProgressDescription( "Normalizing Result" );
    frameCountProgress.setNextProgressIncrement( 1u );

    const RigFemPart*     femPart     = m_resultCollection->parts()->part( partIndex );
    const RigFemPartGrid* femPartGrid = femPart->getOrCreateStructGrid();

    const float                    inf          = std::numeric_limits<float>::infinity();
    int                            elmNodeCount = femPart->elementCount();
    const std::vector<cvf::Vec3f>& nodeCoords   = femPart->nodes().coordinates;

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& porFrameData = porDataFrames->frameData( fIdx );
        if ( porFrameData.empty() ) continue;
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData = dstDataFrames->frameData( fIdx );

        size_t resultCount = srcFrameData.size();
        dstFrameData.resize( resultCount );

        if ( unscaledResult.resultPosType == RIG_ELEMENT_NODAL )
        {
#pragma omp parallel for schedule( dynamic )
            for ( int elmIdx = 0; elmIdx < femPart->elementCount(); ++elmIdx )
            {
                RigElementType elmType = femPart->elementType( elmIdx );
                if ( !( elmType == HEX8 || elmType == HEX8P ) ) continue;

                bool porRegion = false;
                for ( int elmLocalNodeIdx = 0; elmLocalNodeIdx < 8; ++elmLocalNodeIdx )
                {
                    size_t    elmNodeResIdx     = femPart->elementNodeResultIdx( elmIdx, elmLocalNodeIdx );
                    const int nodeIdx           = femPart->nodeIdxFromElementNodeResultIdx( elmNodeResIdx );
                    dstFrameData[elmNodeResIdx] = srcFrameData[elmNodeResIdx];
                    if ( porFrameData[elmNodeResIdx] != std::numeric_limits<float>::infinity() )
                    {
                        porRegion = true;
                    }
                }
                if ( porRegion )
                {
                    // This is in the POR-region. Use hydrostatic pressure from the individual nodes
                    for ( int elmLocalNodeIdx = 0; elmLocalNodeIdx < 8; ++elmLocalNodeIdx )
                    {
                        size_t    elmNodeResIdx = femPart->elementNodeResultIdx( elmIdx, elmLocalNodeIdx );
                        const int nodeIdx       = femPart->nodeIdxFromElementNodeResultIdx( elmNodeResIdx );
                        double tvdRKB = std::abs( nodeCoords[nodeIdx].z() ) + m_resultCollection->normalizationAirGap();
                        double hydrostaticPressure = RiaWellLogUnitTools<double>::hydrostaticPorePressureBar( tvdRKB );
                        dstFrameData[elmNodeResIdx] /= hydrostaticPressure;
                    }
                }
                else
                {
                    // Over/under/sideburden. Use hydrostatic pressure from cell centroid.
                    cvf::Vec3d cellCentroid   = femPartGrid->cellCentroid( elmIdx );
                    double cellCentroidTvdRKB = std::abs( cellCentroid.z() ) + m_resultCollection->normalizationAirGap();
                    double cellCenterHydroStaticPressure =
                        RiaWellLogUnitTools<double>::hydrostaticPorePressureBar( cellCentroidTvdRKB );

                    for ( int elmLocalNodeIdx = 0; elmLocalNodeIdx < 8; ++elmLocalNodeIdx )
                    {
                        size_t elmNodeResIdx = femPart->elementNodeResultIdx( elmIdx, elmLocalNodeIdx );
                        dstFrameData[elmNodeResIdx] /= cellCenterHydroStaticPressure;
                    }
                }
            }
        }
    }
    return dstDataFrames;
}
