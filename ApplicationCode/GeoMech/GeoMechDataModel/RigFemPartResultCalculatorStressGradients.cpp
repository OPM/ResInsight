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

#include "RigFemPartResultCalculatorStressGradients.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigHexGradientTools.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorStressGradients::RigFemPartResultCalculatorStressGradients( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorStressGradients::~RigFemPartResultCalculatorStressGradients()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorStressGradients::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    if ( resVarAddr.fieldName == "ST" || resVarAddr.fieldName == "SE" )
    {
        const std::vector<std::string> allowedComponentNames =
            RigFemPartResultsCollection::getStressGradientComponentNames();

        for ( auto& allowedComponentName : allowedComponentNames )
        {
            if ( resVarAddr.componentName == allowedComponentName )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorStressGradients::calculate( int                        partIndex,
                                                                                const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "ST" || resVarAddr.fieldName == "SE" );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating gradient: " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    QString origFieldName     = QString::fromStdString( resVarAddr.fieldName );
    QString origComponentName = QString::fromStdString( resVarAddr.componentName );
    // Split out the direction of the component name: SE-X => SE
    QString componentName = origComponentName.left( origComponentName.lastIndexOf( QChar( '-' ) ) );

    RigFemScalarResultFrames* inputResultFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType,
                                                                         resVarAddr.fieldName,
                                                                         componentName.toStdString() ) );

    RigFemScalarResultFrames* dataFramesX =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     componentName.toStdString() + "-X" ) );
    RigFemScalarResultFrames* dataFramesY =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     componentName.toStdString() + "-Y" ) );
    RigFemScalarResultFrames* dataFramesZ =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     componentName.toStdString() + "-Z" ) );
    frameCountProgress.incrementProgress();

    const RigFemPart*              femPart      = m_resultCollection->parts()->part( partIndex );
    int                            elementCount = femPart->elementCount();
    const std::vector<cvf::Vec3f>& nodeCoords   = femPart->nodes().coordinates;

    int frameCount = inputResultFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& inputData = inputResultFrames->frameData( fIdx );

        std::vector<float>& dstFrameDataX = dataFramesX->frameData( fIdx );
        std::vector<float>& dstFrameDataY = dataFramesY->frameData( fIdx );
        std::vector<float>& dstFrameDataZ = dataFramesZ->frameData( fIdx );
        size_t              valCount      = inputData.size();
        dstFrameDataX.resize( valCount );
        dstFrameDataY.resize( valCount );
        dstFrameDataZ.resize( valCount );

#pragma omp parallel for schedule( dynamic )
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            const int*     cornerIndices = femPart->connectivities( elmIdx );
            RigElementType elmType       = femPart->elementType( elmIdx );

            if ( !( elmType == HEX8P || elmType == HEX8 ) ) continue;

            // Find the corner coordinates for element
            std::array<cvf::Vec3d, 8> hexCorners;
            for ( int i = 0; i < 8; i++ )
            {
                hexCorners[i] = cvf::Vec3d( nodeCoords[cornerIndices[i]] );
            }

            // Find the corresponding corner values for the element
            std::array<double, 8> cornerValues;

            int elmNodeCount = RigFemTypes::elementNodeCount( elmType );
            for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
            {
                size_t elmNodResIdx     = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                cornerValues[elmNodIdx] = inputData[elmNodResIdx];
            }

            std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

            for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
            {
                size_t elmNodResIdx         = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                dstFrameDataX[elmNodResIdx] = gradients[elmNodIdx].x();
                dstFrameDataY[elmNodResIdx] = gradients[elmNodIdx].y();
                dstFrameDataZ[elmNodResIdx] = gradients[elmNodIdx].z();
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedStress = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    CVF_ASSERT( requestedStress );
    return requestedStress;
}
