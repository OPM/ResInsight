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

#include "RigFemPartResultCalculatorMudWeightWindow.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechBoreHoleStressCalculator.h"
#include "RigGeoMechWellLogExtractor.h"

#include "RiaOffshoreSphericalCoords.h"

#include "cafProgressInfo.h"

#include "cvfBoundingBox.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorMudWeightWindow::RigFemPartResultCalculatorMudWeightWindow( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorMudWeightWindow::~RigFemPartResultCalculatorMudWeightWindow()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorMudWeightWindow::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "MUD-WEIGHT" &&
             ( resVarAddr.componentName == "MWW" || resVarAddr.componentName == "MWM" ||
               resVarAddr.componentName == "UMWL" || resVarAddr.componentName == "LMWL" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorMudWeightWindow::calculate( int                        partIndex,
                                                                                const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( isMatching( resVarAddr ) );

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * 4, "" );
    frameCountProgress.setProgressDescription( "Calculating Mud Weight Window" );

    double wellPathDeviation = 12.3;
    double wellPathAzimuth   = 32.4;
    double ucsBar            = 3.4;
    double poissonsRatio     = 0.45;
    double K0_FG             = 0.445;
    double airGap            = 123.0;

    UpperLimitParameter upperLimitParameter = UpperLimitParameter::FG;
    LowerLimitParameter lowerLimitParameter = LowerLimitParameter::MAX_OF_PORE_PRESSURE_AND_SFG;

    // Pore pressure
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* porePressureDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, "POR-Bar", "" ) );
    frameCountProgress.incrementProgress();

    // Stress (ST.S3)
    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );
    RigFemScalarResultFrames* stressDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S3" ) );
    frameCountProgress.incrementProgress();

    frameCountProgress.setNextProgressIncrement( m_resultCollection->frameCount() );

    RigFemScalarResultFrames* mudWeightWindowFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "MWW" ) );
    RigFemScalarResultFrames* mudWeightMiddleFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "MWM" ) );
    RigFemScalarResultFrames* upperMudWeightLimitFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "UMWL" ) );
    RigFemScalarResultFrames* lowerMudWeightLimitFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "LMWL" ) );
    frameCountProgress.incrementProgress();

    const RigFemPart*     femPart     = m_resultCollection->parts()->part( partIndex );
    const RigFemPartGrid* femPartGrid = femPart->getOrCreateStructGrid();

    float inf = std::numeric_limits<float>::infinity();

    frameCountProgress.setNextProgressIncrement( 1u );

    int frameCount = stressDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& porFrameData    = porePressureDataFrames->frameData( fIdx );
        const std::vector<float>& stressFrameData = stressDataFrames->frameData( fIdx );

        std::vector<float>& mudWeightWindowFrameData     = mudWeightWindowFrames->frameData( fIdx );
        std::vector<float>& mudWeightMiddleFrameData     = mudWeightMiddleFrames->frameData( fIdx );
        std::vector<float>& upperMudWeightLimitFrameData = upperMudWeightLimitFrames->frameData( fIdx );
        std::vector<float>& lowerMudWeightLimitFrameData = lowerMudWeightLimitFrames->frameData( fIdx );

        size_t valCount = stressFrameData.size();
        mudWeightWindowFrameData.resize( valCount );
        mudWeightMiddleFrameData.resize( valCount );
        upperMudWeightLimitFrameData.resize( valCount );
        lowerMudWeightLimitFrameData.resize( valCount );

        int elementCount = femPart->elementCount();

        // Load stress
        RigFemResultAddress     stressResAddr( RIG_ELEMENT_NODAL, "ST", "" );
        std::vector<caf::Ten3f> vertexStressesFloat = m_resultCollection->tensors( stressResAddr, partIndex, fIdx );

        std::vector<caf::Ten3d> vertexStresses;
        vertexStresses.reserve( vertexStressesFloat.size() );
        for ( const caf::Ten3f& floatTensor : vertexStressesFloat )
        {
            vertexStresses.push_back( caf::Ten3d( floatTensor ) );
        }

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            int elmNodeCount = RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) );

            if ( elmType == HEX8P )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    // Use hydrostatic pressure from cell centroid.
                    // Use centroid to avoid intra-element differences
                    cvf::Vec3d cellCentroid       = femPartGrid->cellCentroid( elmIdx );
                    double     cellCentroidTvdRKB = std::abs( cellCentroid.z() ) + airGap;
                    double     cellCenterHydroStaticPressure =
                        RiaWellLogUnitTools<double>::hydrostaticPorePressureBar( cellCentroidTvdRKB );

                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < stressFrameData.size() )
                    {
                        int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );

                        // Pore pressure (unit: Bar)
                        double porePressureBar = porFrameData[nodeIdx];

                        // FG is for sands, SFG for shale. Sands has valid PP, shale does not.
                        bool isSand = ( porePressureBar != inf );

                        caf::Ten3d segmentStress = caf::Ten3d( vertexStressesFloat[nodeIdx] );

                        cvf::Vec3d wellPathTangent = calculateWellPathTangent( wellPathAzimuth, wellPathDeviation );
                        caf::Ten3d wellPathStressFloat =
                            RigGeoMechWellLogExtractor::transformTensorToWellPathOrientation( wellPathTangent,
                                                                                              segmentStress );
                        caf::Ten3d wellPathStressDouble( wellPathStressFloat );

                        RigGeoMechBoreHoleStressCalculator sigmaCalculator( wellPathStressDouble,
                                                                            porePressureBar,
                                                                            poissonsRatio,
                                                                            ucsBar,
                                                                            32 );

                        // Calculate upper limit
                        float upperLimit = inf;
                        if ( upperLimitParameter == UpperLimitParameter::FG )
                        {
                            upperLimit = sigmaCalculator.solveFractureGradient();
                        }
                        else if ( upperLimitParameter == UpperLimitParameter::SH_MIN )
                        {
                            upperLimit = stressFrameData[elmNodResIdx];
                        }

                        // Calculate lower limit
                        float lowerLimit = inf;
                        if ( lowerLimitParameter == LowerLimitParameter::PORE_PRESSURE )
                        {
                            lowerLimit = porePressureBar;
                        }
                        else if ( lowerLimitParameter == LowerLimitParameter::MAX_OF_PORE_PRESSURE_AND_SFG )
                        {
                            if ( isSand )
                            {
                                lowerLimit = porePressureBar;
                            }
                            else
                            {
                                double SFG = sigmaCalculator.solveStassiDalia();
                                lowerLimit = std::max( porePressureBar, SFG );
                            }
                        }

                        // Normalize by hydrostatic pore pressure
                        upperMudWeightLimitFrameData[elmNodResIdx] = upperLimit / cellCenterHydroStaticPressure;
                        lowerMudWeightLimitFrameData[elmNodResIdx] = lowerLimit / cellCenterHydroStaticPressure;
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
                        mudWeightWindowFrameData[elmNodResIdx]     = inf;
                        mudWeightMiddleFrameData[elmNodResIdx]     = inf;
                        upperMudWeightLimitFrameData[elmNodResIdx] = inf;
                        lowerMudWeightLimitFrameData[elmNodResIdx] = inf;
                    }
                }
            }
        }

        size_t kRefLayer = 28; // resVarAddr.refKLayerIndex;

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            int elmNodeCount = RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) );

            size_t i, j, k;
            bool   validIndex = femPartGrid->ijkFromCellIndex( elmIdx, &i, &j, &k );
            size_t kMin       = std::min( k, kRefLayer );
            size_t kMax       = std::max( k, kRefLayer );

            if ( elmType == HEX8P && validIndex )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );

                    float maxLowerMudWeightLimit = 0.0;
                    float minUpperMudWeightLimit = 0.0;
                    if ( kMin == kMax )
                    {
                        maxLowerMudWeightLimit = lowerMudWeightLimitFrameData[elmNodResIdx];
                        minUpperMudWeightLimit = upperMudWeightLimitFrameData[elmNodResIdx];
                    }
                    else
                    {
                        for ( size_t currentK = kMin; currentK < kMax; currentK++ )
                        {
                            size_t kElmIdx = femPartGrid->cellIndexFromIJK( i, j, currentK );
                            if ( kElmIdx != cvf::UNDEFINED_SIZE_T && femPart->elementType( kElmIdx ) == HEX8P )
                            {
                                size_t kElmNodResIdx = femPart->elementNodeResultIdx( kElmIdx, elmNodIdx );

                                float currentLowerMudWeightLimit = lowerMudWeightLimitFrameData[kElmNodResIdx];
                                if ( currentLowerMudWeightLimit > maxLowerMudWeightLimit )
                                {
                                    maxLowerMudWeightLimit = currentLowerMudWeightLimit;
                                }

                                float currentUpperMudWeightLimit = upperMudWeightLimitFrameData[kElmNodResIdx];
                                if ( currentUpperMudWeightLimit > minUpperMudWeightLimit )
                                {
                                    minUpperMudWeightLimit = currentUpperMudWeightLimit;
                                }
                            }
                        }
                    }

                    float mudWeightWindow                  = minUpperMudWeightLimit - maxLowerMudWeightLimit;
                    mudWeightWindowFrameData[elmNodResIdx] = mudWeightWindow;

                    float mudWeightMiddle = inf;
                    if ( mudWeightWindow > 0.0 )
                    {
                        mudWeightMiddle = maxLowerMudWeightLimit + mudWeightWindow / 2.0;
                    }
                    mudWeightMiddleFrameData[elmNodResIdx] = mudWeightMiddle;
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedResultFrames = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedResultFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartResultCalculatorMudWeightWindow::calculateWellPathTangent( double azimuth, double inclination )
{
    double aziRad = cvf::Math::toRadians( azimuth );
    double incRad = cvf::Math::toRadians( inclination );
    return RiaOffshoreSphericalCoords::unitVectorFromAziInc( aziRad, incRad );
}
