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

#include "RigFemPartResultCalculatorCompaction.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigHexIntersectionTools.h"

#include "cafProgressInfo.h"

#include "cvfBoundingBox.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// Internal definitions
//--------------------------------------------------------------------------------------------------
class RefElement
{
public:
    size_t              elementIdx;
    float               intersectionPointToCurrentNodeDistance;
    cvf::Vec3f          intersectionPoint;
    std::vector<size_t> elementFaceNodeIdxs;
};

static std::array<cvf::Vec3d, 8> coordsFromNodeIndices( const RigFemPart& part, const std::array<size_t, 8>& nodeIdxs );
static std::array<size_t, 8>     nodesForElement( const RigFemPart& part, size_t elementIdx );
static float                     horizontalDistance( const cvf::Vec3f& p1, const cvf::Vec3f& p2 );
static void findReferenceElementForNode( const RigFemPart& part, size_t nodeIdx, size_t kRefLayer, RefElement* refElement );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorCompaction::RigFemPartResultCalculatorCompaction( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorCompaction::~RigFemPartResultCalculatorCompaction()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorCompaction::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == RigFemPartResultsCollection::FIELD_NAME_COMPACTION );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorCompaction::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == RigFemPartResultsCollection::FIELD_NAME_COMPACTION );

    caf::ProgressInfo stepCountProgress( m_resultCollection->timeStepCount() + 1, "" );
    stepCountProgress.setProgressDescription( "Calculating " + QString::fromStdString( resVarAddr.fieldName ) );

    RigFemScalarResultFrames* u3Frames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "U", "U3" ) );
    stepCountProgress.incrementProgress();

    RigFemScalarResultFrames* compactionFrames = m_resultCollection->createScalarResult( partIndex, resVarAddr );

    const RigFemPart* part = m_resultCollection->parts()->part( partIndex );
    // Make sure AABB tree and struct grid is created
    part->ensureIntersectionSearchTreeIsBuilt();
    part->getOrCreateStructGrid();

    int timeSteps = u3Frames->timeStepCount();
    for ( int stepIdx = 0; stepIdx < timeSteps; stepIdx++ )
    {
        const int frameCount = u3Frames->frameCount( stepIdx );
        for ( int fIdx = 0; fIdx < frameCount; fIdx++ )
        {
            std::vector<float>& compactionFrame = compactionFrames->frameData( stepIdx, fIdx );
            size_t              nodeCount       = part->nodes().nodeIds.size();

            compactionFrame.resize( nodeCount );

#pragma omp parallel for
            for ( long n = 0; n < static_cast<long>( nodeCount ); n++ )
            {
                RefElement refElement;
                findReferenceElementForNode( *part, n, resVarAddr.refKLayerIndex, &refElement );

                if ( refElement.elementIdx != cvf::UNDEFINED_SIZE_T )
                {
                    float  shortestDist      = std::numeric_limits<float>::infinity();
                    size_t closestRefNodeIdx = cvf::UNDEFINED_SIZE_T;

                    for ( size_t nodeIdx : refElement.elementFaceNodeIdxs )
                    {
                        float dist = horizontalDistance( refElement.intersectionPoint, part->nodes().coordinates[nodeIdx] );
                        if ( dist < shortestDist )
                        {
                            shortestDist      = dist;
                            closestRefNodeIdx = nodeIdx;
                        }
                    }

                    cvf::Vec3f currentNodeCoord = part->nodes().coordinates[n];
                    if ( currentNodeCoord.z() >= refElement.intersectionPoint.z() )
                        compactionFrame[n] =
                            -( u3Frames->frameData( stepIdx, fIdx )[n] - u3Frames->frameData( stepIdx, fIdx )[closestRefNodeIdx] );
                    else
                        compactionFrame[n] =
                            -( u3Frames->frameData( stepIdx, fIdx )[closestRefNodeIdx] - u3Frames->frameData( stepIdx, fIdx )[n] );
                }
                else
                {
                    compactionFrame[n] = HUGE_VAL;
                }
            }
        }
        stepCountProgress.incrementProgress();
    }
    RigFemScalarResultFrames* requestedPrincipal = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedPrincipal;
}

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void findReferenceElementForNode( const RigFemPart& part, size_t nodeIdx, size_t kRefLayer, RefElement* refElement )
{
    static const double zMin = -1e6, zMax = 1e6;

    cvf::BoundingBox bb;
    cvf::Vec3f       currentNodeCoord = part.nodes().coordinates[nodeIdx];
    cvf::Vec3f       p1               = cvf::Vec3f( currentNodeCoord.x(), currentNodeCoord.y(), zMin );
    cvf::Vec3f       p2               = cvf::Vec3f( currentNodeCoord.x(), currentNodeCoord.y(), zMax );
    bb.add( p1 );
    bb.add( p2 );

    std::vector<size_t> refElementCandidates = part.findIntersectingElementIndices( bb );

    const RigFemPartGrid* grid = part.getOrCreateStructGrid();

    refElement->elementIdx                             = cvf::UNDEFINED_SIZE_T;
    refElement->intersectionPointToCurrentNodeDistance = std::numeric_limits<float>::infinity();
    size_t i, j, k;
    for ( const size_t elemIdx : refElementCandidates )
    {
        bool validIndex = grid->ijkFromCellIndex( elemIdx, &i, &j, &k );
        if ( validIndex && k == kRefLayer )
        {
            const std::array<size_t, 8> nodeIndices = nodesForElement( part, elemIdx );
            CVF_ASSERT( nodeIndices.size() == 8 );

            std::vector<HexIntersectionInfo> intersections;
            RigHexIntersectionTools::lineHexCellIntersection( cvf::Vec3d( p1 ),
                                                              cvf::Vec3d( p2 ),
                                                              coordsFromNodeIndices( part, nodeIndices ),
                                                              elemIdx,
                                                              &intersections );

            for ( const auto& intersection : intersections )
            {
                cvf::Vec3f intersectionPoint = cvf::Vec3f( intersection.m_intersectionPoint );

                float nodeToIntersectionDistance = currentNodeCoord.pointDistance( intersectionPoint );
                if ( nodeToIntersectionDistance < refElement->intersectionPointToCurrentNodeDistance )
                {
                    cvf::ubyte faceNodes[4];
                    RigFemPartGrid::cellFaceVertexIndices( intersection.m_face, faceNodes );
                    std::vector<size_t> topFaceCoords(
                        { nodeIndices[faceNodes[0]], nodeIndices[faceNodes[1]], nodeIndices[faceNodes[2]], nodeIndices[faceNodes[3]] } );

                    refElement->elementIdx                             = elemIdx;
                    refElement->intersectionPointToCurrentNodeDistance = nodeToIntersectionDistance;
                    refElement->intersectionPoint                      = intersectionPoint;
                    refElement->elementFaceNodeIdxs                    = topFaceCoords;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 8> coordsFromNodeIndices( const RigFemPart& part, const std::array<size_t, 8>& nodeIdxs )
{
    std::array<cvf::Vec3d, 8> out;
    for ( size_t i = 0; i < nodeIdxs.size(); i++ )
        out[i] = cvf::Vec3d( part.nodes().coordinates[nodeIdxs[i]] );
    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<size_t, 8> nodesForElement( const RigFemPart& part, size_t elementIdx )
{
    std::array<size_t, 8> nodeIdxs;
    const int*            nodeConn = part.connectivities( elementIdx );
    for ( int n = 0; n < 8; n++ )
        nodeIdxs[n] = nodeConn[n];
    return nodeIdxs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float horizontalDistance( const cvf::Vec3f& p1, const cvf::Vec3f& p2 )
{
    cvf::Vec3f p1_ = p1;
    cvf::Vec3f p2_ = p2;
    p1_.z() = p2_.z() = 0;
    return p1_.pointDistance( p2_ );
}
