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

#include "RigFemPartResultCalculatorSurfaceAlignedStress.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "cvfGeometryTools.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorSurfaceAlignedStress::RigFemPartResultCalculatorSurfaceAlignedStress(
    RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorSurfaceAlignedStress::~RigFemPartResultCalculatorSurfaceAlignedStress()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorSurfaceAlignedStress::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.resultPosType == RIG_ELEMENT_NODAL_FACE && resVarAddr.componentName != "Pazi" &&
             resVarAddr.componentName != "Pinc" && !resVarAddr.componentName.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultCalculatorSurfaceAlignedStress::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.componentName == "STH" || resVarAddr.componentName == "STQV" ||
                resVarAddr.componentName == "SN" || resVarAddr.componentName == "TPH" ||
                resVarAddr.componentName == "TPQV" || resVarAddr.componentName == "THQV" ||
                resVarAddr.componentName == "TP" || resVarAddr.componentName == "TPinc" ||
                resVarAddr.componentName == "FAULTMOB" || resVarAddr.componentName == "PCRIT" );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 7, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* s11Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S11" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* s22Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S22" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* s33Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S33" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* s12Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S12" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* s23Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S23" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* s13Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S13" ) );

    RigFemScalarResultFrames* SNFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "SN" ) );
    RigFemScalarResultFrames* STHFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "STH" ) );
    RigFemScalarResultFrames* STQVFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "STQV" ) );
    RigFemScalarResultFrames* TNHFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TPH" ) );
    RigFemScalarResultFrames* TNQVFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TPQV" ) );
    RigFemScalarResultFrames* THQVFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "THQV" ) );
    RigFemScalarResultFrames* TPFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TP" ) );
    RigFemScalarResultFrames* TPincFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TPinc" ) );
    RigFemScalarResultFrames* FAULTMOBFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType,
                                                                     resVarAddr.fieldName,
                                                                     "FAULTMOB" ) );
    RigFemScalarResultFrames* PCRITFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "PCRIT" ) );

    frameCountProgress.incrementProgress();

    const RigFemPart*              femPart         = m_resultCollection->parts()->part( partIndex );
    const std::vector<cvf::Vec3f>& nodeCoordinates = femPart->nodes().coordinates;

    float tanFricAng        = tan( m_resultCollection->parameterFrictionAngleRad() );
    float cohPrTanFricAngle = (float)( m_resultCollection->parameterCohesion() / tanFricAng );

    int frameCount = s11Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& s11 = s11Frames->frameData( fIdx );
        if ( s11.empty() ) continue;

        const std::vector<float>& s22 = s22Frames->frameData( fIdx );
        const std::vector<float>& s33 = s33Frames->frameData( fIdx );
        const std::vector<float>& s12 = s12Frames->frameData( fIdx );
        const std::vector<float>& s23 = s23Frames->frameData( fIdx );
        const std::vector<float>& s13 = s13Frames->frameData( fIdx );

        std::vector<float>& SNDat       = SNFrames->frameData( fIdx );
        std::vector<float>& STHDat      = STHFrames->frameData( fIdx );
        std::vector<float>& STQVDat     = STQVFrames->frameData( fIdx );
        std::vector<float>& TNHDat      = TNHFrames->frameData( fIdx );
        std::vector<float>& TNQVDat     = TNQVFrames->frameData( fIdx );
        std::vector<float>& THQVDat     = THQVFrames->frameData( fIdx );
        std::vector<float>& TPDat       = TPFrames->frameData( fIdx );
        std::vector<float>& TincDat     = TPincFrames->frameData( fIdx );
        std::vector<float>& FAULTMOBDat = FAULTMOBFrames->frameData( fIdx );
        std::vector<float>& PCRITDat    = PCRITFrames->frameData( fIdx );

        // HACK ! Todo : make it robust against other elements than Hex8
        size_t valCount = s11.size() * 3; // Number of Elm Node Face results 24 = 4 * num faces = 3* numElmNodes

        SNDat.resize( valCount );
        STHDat.resize( valCount );
        STQVDat.resize( valCount );
        TNHDat.resize( valCount );
        TNQVDat.resize( valCount );
        THQVDat.resize( valCount );
        TPDat.resize( valCount );
        TincDat.resize( valCount );
        FAULTMOBDat.resize( valCount );
        PCRITDat.resize( valCount );

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

                    size_t qElmNodeResIdx[4];
                    qElmNodeResIdx[0] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[0] );
                    qElmNodeResIdx[1] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[1] );
                    qElmNodeResIdx[2] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[2] );
                    qElmNodeResIdx[3] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[3] );

                    for ( int qIdx = 0; qIdx < 4; ++qIdx )
                    {
                        size_t elmNodResIdx = qElmNodeResIdx[qIdx];
                        float  t11          = s11[elmNodResIdx];
                        float  t22          = s22[elmNodResIdx];
                        float  t33          = s33[elmNodResIdx];
                        float  t12          = s12[elmNodResIdx];
                        float  t23          = s23[elmNodResIdx];
                        float  t13          = s13[elmNodResIdx];

                        caf::Ten3f tensor( t11, t22, t33, t12, t23, t13 );
                        caf::Ten3f xfTen            = tensor.rotated( rotMx );
                        int        elmNodFaceResIdx = elmNodFaceResIdxFaceStart + qIdx;

                        float szx = xfTen[caf::Ten3f::SZX];
                        float syz = xfTen[caf::Ten3f::SYZ];
                        float szz = xfTen[caf::Ten3f::SZZ];

                        STHDat[elmNodFaceResIdx]  = xfTen[caf::Ten3f::SXX];
                        STQVDat[elmNodFaceResIdx] = xfTen[caf::Ten3f::SYY];
                        SNDat[elmNodFaceResIdx]   = xfTen[caf::Ten3f::SZZ];

                        TNHDat[elmNodFaceResIdx]  = xfTen[caf::Ten3f::SZX];
                        TNQVDat[elmNodFaceResIdx] = xfTen[caf::Ten3f::SYZ];
                        THQVDat[elmNodFaceResIdx] = xfTen[caf::Ten3f::SXY];

                        float TP                = sqrt( szx * szx + syz * syz );
                        TPDat[elmNodFaceResIdx] = TP;

                        if ( TP > 1e-5 )
                        {
                            TincDat[elmNodFaceResIdx] = cvf::Math::toDegrees( acos( syz / TP ) );
                        }
                        else
                        {
                            TincDat[elmNodFaceResIdx] = std::numeric_limits<float>::infinity();
                        }

                        FAULTMOBDat[elmNodFaceResIdx] = TP / ( tanFricAng * ( szz + cohPrTanFricAngle ) );
                        PCRITDat[elmNodFaceResIdx]    = szz - TP / tanFricAng;
                    }
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedSurfStress = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedSurfStress;
}
