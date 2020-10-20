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

#include "RigFemPartResultCalculatorSurfaceAngles.h"

#include "RiaOffshoreSphericalCoords.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "cafProgressInfo.h"

#include "cvfGeometryTools.h"
#include "cvfMath.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorSurfaceAngles::RigFemPartResultCalculatorSurfaceAngles( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorSurfaceAngles::~RigFemPartResultCalculatorSurfaceAngles()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorSurfaceAngles::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.resultPosType == RIG_ELEMENT_NODAL_FACE ) &&
             ( resVarAddr.componentName == "Pazi" || resVarAddr.componentName == "Pinc" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorSurfaceAngles::calculate( int                        partIndex,
                                                                              const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.componentName == "Pazi" || resVarAddr.componentName == "Pinc" );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 1, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );

    RigFemScalarResultFrames* PaziFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "Pazi" ) );
    RigFemScalarResultFrames* PincFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "Pinc" ) );

    const RigFemPart*              femPart         = m_resultCollection->parts()->part( partIndex );
    const std::vector<cvf::Vec3f>& nodeCoordinates = femPart->nodes().coordinates;
    int                            frameCount      = m_resultCollection->frameCount();

    // HACK ! Todo : make it robust against other elements than Hex8
    size_t valCount = femPart->elementCount() * 24; // Number of Elm Node Face results 24 = 4 * num faces = 3*
                                                    // numElmNodes

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        std::vector<float>& Pazi = PaziFrames->frameData( fIdx );
        std::vector<float>& Pinc = PincFrames->frameData( fIdx );

        Pazi.resize( valCount );
        Pinc.resize( valCount );

        int elementCount = femPart->elementCount();
#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType        = femPart->elementType( elmIdx );
            int            faceCount      = RigFemTypes::elementFaceCount( elmType );
            const int*     elmNodeIndices = femPart->connectivities( elmIdx );

            int elmNodFaceResIdxElmStart = elmIdx * 24; // HACK should get from part

            for ( int lfIdx = 0; lfIdx < faceCount; ++lfIdx )
            {
                int        faceNodeCount = 0;
                const int* localElmNodeIndicesForFace =
                    RigFemTypes::localElmNodeIndicesForFace( elmType, lfIdx, &faceNodeCount );
                if ( faceNodeCount == 4 )
                {
                    int        elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + lfIdx * 4; // HACK
                    cvf::Vec3f quadVxs[4];

                    quadVxs[0] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]] );
                    quadVxs[1] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]] );
                    quadVxs[2] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]] );
                    quadVxs[3] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]] );

                    cvf::Mat3f rotMx = cvf::GeometryTools::computePlaneHorizontalRotationMx( quadVxs[2] - quadVxs[0],
                                                                                             quadVxs[3] - quadVxs[1] );
                    RiaOffshoreSphericalCoords sphCoord(
                        cvf::Vec3f( rotMx.rowCol( 2, 0 ), rotMx.rowCol( 2, 1 ), rotMx.rowCol( 2, 2 ) ) ); // Use Ez
                                                                                                          // from the
                                                                                                          // matrix
                                                                                                          // as plane
                                                                                                          // normal

                    for ( int qIdx = 0; qIdx < 4; ++qIdx )
                    {
                        int elmNodFaceResIdx   = elmNodFaceResIdxFaceStart + qIdx;
                        Pazi[elmNodFaceResIdx] = cvf::Math::toDegrees( sphCoord.azi() );
                        Pinc[elmNodFaceResIdx] = cvf::Math::toDegrees( sphCoord.inc() );
                    }
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPlaneAngle = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedPlaneAngle;
}
