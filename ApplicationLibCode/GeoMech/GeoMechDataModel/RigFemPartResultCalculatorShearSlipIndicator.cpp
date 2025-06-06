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

#include "RigFemPartResultCalculatorShearSlipIndicator.h"

#include "RigFemAddressDefines.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "Well/RigGeoMechWellLogExtractor.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorShearSlipIndicator::RigFemPartResultCalculatorShearSlipIndicator( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorShearSlipIndicator::~RigFemPartResultCalculatorShearSlipIndicator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorShearSlipIndicator::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "ST" && resVarAddr.componentName == "DPN" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorShearSlipIndicator::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( isMatching( resVarAddr ) );

    caf::ProgressInfo stepCountProgress( m_resultCollection->timeStepCount() * 3, "" );
    stepCountProgress.setProgressDescription( "Calculating Shear Slip Indicator." );

    // Pore pressure
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* porePressureDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType, RigFemAddressDefines::porBar(), "" ) );
    stepCountProgress.incrementProgress();

    // Total vertical stress (ST.S33)
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* stressDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S33" ) );
    stepCountProgress.incrementProgress();

    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    RigFemScalarResultFrames* shearSlipIndicatorFrames =
        m_resultCollection->createScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "DPN" ) );

    const RigFemPart*     femPart     = m_resultCollection->parts()->part( partIndex );
    const RigFemPartGrid* femPartGrid = femPart->getOrCreateStructGrid();

    float inf = std::numeric_limits<float>::infinity();

    stepCountProgress.setNextProgressIncrement( 1u );

    const int timeSteps = stressDataFrames->timeStepCount();
    for ( int stepIdx = 0; stepIdx < timeSteps; stepIdx++ )
    {
        const int frameCount = stressDataFrames->frameCount( stepIdx );
        for ( int fIdx = 0; fIdx < frameCount; fIdx++ )
        {
            const std::vector<float>& porFrameData = porePressureDataFrames->frameData( stepIdx, fIdx );
            if ( porFrameData.empty() ) continue;

            const std::vector<float>& stressFrameData = stressDataFrames->frameData( stepIdx, fIdx );
            if ( stressFrameData.empty() ) continue;

            std::vector<float>& shearSlipIndicatorFrameData = shearSlipIndicatorFrames->frameData( stepIdx, fIdx );

            size_t valCount = stressFrameData.size();
            shearSlipIndicatorFrameData.resize( valCount );

            int elementCount = femPart->elementCount();

#pragma omp parallel for
            for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
            {
                RigElementType elmType = femPart->elementType( elmIdx );

                int elmNodeCount = RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) );

                if ( femPart->isHexahedron( elmIdx ) )
                {
                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        // Use hydrostatic pressure from cell centroid.
                        // Use centroid to avoid intra-element differences
                        cvf::Vec3d cellCentroid       = femPartGrid->cellCentroid( elmIdx );
                        double     cellCentroidTvdMSL = -cellCentroid.z();

                        double waterDensity = m_resultCollection->waterDensityShearSlipIndicator();
                        double cellCenterHydroStaticPressure =
                            RigGeoMechWellLogExtractor::hydroStaticPorePressureAtDepth( cellCentroidTvdMSL, waterDensity );

                        size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        if ( elmNodResIdx < stressFrameData.size() )
                        {
                            // Pore pressure (unit: Bar)
                            float porePressureBar     = porFrameData[elmNodResIdx];
                            float totalVerticalStress = stressFrameData[elmNodResIdx];

                            float shearSlipIndicator = inf;
                            if ( porePressureBar != inf && totalVerticalStress - cellCenterHydroStaticPressure != 0.0 )
                            {
                                shearSlipIndicator = ( porePressureBar - cellCenterHydroStaticPressure ) /
                                                     ( totalVerticalStress - cellCenterHydroStaticPressure );
                            }

                            shearSlipIndicatorFrameData[elmNodResIdx] = shearSlipIndicator;
                        }
                    }
                }
                else
                {
                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        if ( elmNodResIdx < stressFrameData.size() )
                        {
                            shearSlipIndicatorFrameData[elmNodResIdx] = inf;
                        }
                    }
                }
            }
        }
        stepCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedResultFrames = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedResultFrames;
}
