/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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


#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"

#include "RivPipeGeometryGenerator.h"
#include <vector>
#include "cafEffectGenerator.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivPipeGeometryGenerator::RivPipeGeometryGenerator()
{
    m_radius = 1.0;
    m_crossSectionNodeCount = 8;
    m_minimumBendAngle = 80.0;
    m_bendScalingFactor = 0.00001;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivPipeGeometryGenerator::~RivPipeGeometryGenerator()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::setPipeCenterCoords(const cvf::Vec3dArray* coords)
{
    m_originalPipeCenterCoords = coords;

    clearComputedData();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::setMinimumBendAngle(double degrees)
{
    m_minimumBendAngle = degrees;

    clearComputedData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::setBendScalingFactor(double scaleFactor)
{
    m_bendScalingFactor = scaleFactor;

    clearComputedData();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::setRadius(double radius)
{
    CVF_ASSERT(0 <= radius && radius < 1e100);
    m_radius = radius;

    clearComputedData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::setCrossSectionVertexCount(size_t nodeCount)
{
    CVF_ASSERT( 2 < nodeCount && nodeCount < 1000000);
    m_crossSectionNodeCount = nodeCount;

    clearComputedData();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPipeGeometryGenerator::createPipeSurface()
{
    if (m_radius == 0.0)
    {
        return NULL;
    }

    updateFilteredPipeCenterCoords();

    cvf::ref<cvf::Vec3dArray> activeCoords = new cvf::Vec3dArray(m_filteredPipeCenterCoords);

    return RivPipeGeometryGenerator::generateExtrudedCylinder(m_radius, m_crossSectionNodeCount, activeCoords.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPipeGeometryGenerator::createCenterLine()
{
    return generateLine(m_originalPipeCenterCoords.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::pipeSurfaceTextureCoords(cvf::Vec2fArray* textureCoords, const std::vector<double>& segmentResults, const cvf::ScalarMapper* mapper) const
{
    CVF_ASSERT(textureCoords);
    CVF_ASSERT(mapper);
    CVF_ASSERT(segmentResults.size() == m_originalPipeCenterCoords->size() - 1);

    size_t nodeCountPerSegment = m_crossSectionNodeCount * 4;

    size_t vertexCount = m_filteredPipeSegmentToResult.size() * nodeCountPerSegment;
    if (textureCoords->size() != vertexCount) textureCoords->resize(vertexCount);

    size_t i;
    for (i = 0; i < m_filteredPipeSegmentToResult.size(); i++)
    {
        size_t resultIndex = m_filteredPipeSegmentToResult[i];

        cvf::Vec2f texCoord = mapper->mapToTextureCoord(segmentResults[resultIndex]);

        size_t j;
        for (j = 0; j < nodeCountPerSegment; j++)
        {
            textureCoords->set(i * nodeCountPerSegment + j, texCoord);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::centerlineTextureCoords(cvf::Vec2fArray* textureCoords, const std::vector<double>& segmentResults, const cvf::ScalarMapper* mapper) const
{
    CVF_ASSERT(textureCoords);
    CVF_ASSERT(mapper);
    CVF_ASSERT(segmentResults.size() == m_originalPipeCenterCoords->size() - 1);

    size_t vertexCount = segmentResults.size() * 2;
    if (textureCoords->size() != vertexCount) textureCoords->resize(vertexCount);

    for (size_t vxIdx = 0; vxIdx < vertexCount; ++vxIdx)
    {
        cvf::Vec2f texCoord = mapper->mapToTextureCoord(segmentResults[vxIdx/2]);
        textureCoords->set(vxIdx, texCoord);
    }
}

//--------------------------------------------------------------------------------------------------
/// The circle points are generated in positive direction with circle normal pointing along yDir ^ zDir
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::computeCircle(double radius, size_t tesselationCount, const cvf::Vec3d& center, const cvf::Vec3d& yDirection, const cvf::Vec3d& zDirection, std::vector<cvf::Vec3d>* nodes)
{
    cvf::Vec3d normal;
    cvf::Vec3d expandedCoord;

    double delta = (2 * cvf::PI_D) / tesselationCount;
    double angle = 0.0;

    size_t i;
    for (i = 0; i < tesselationCount; i++)
    {
        // These are the local coordinates on the circle
        double fLocalNormCoordY = cvf::Math::cos(angle);
        double fLocalNormCoordZ = cvf::Math::sin(angle);

        // Compute normal (vector going from center to the point on the circle)
        // Do it this way and we can use this normal directly as long as both input orientation vectors are normalized (which they should be)

        normal = yDirection*fLocalNormCoordY + zDirection*fLocalNormCoordZ;
        normal.normalize();

        // get the global expanded coord by scaling by radius

        expandedCoord = center + radius*normal;
        nodes->push_back(expandedCoord);

        angle += delta;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPipeGeometryGenerator::generateLine(const cvf::Vec3dArray* coords)
{
    CVF_ASSERT(coords != NULL);

    if (coords->size() < 2 ) return NULL;

    size_t duplicateVertexCount = 2 * (coords->size() - 1);

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray;
    indices->resize(duplicateVertexCount);

    size_t i;
    for (i = 0; i < duplicateVertexCount; i++)
    {
        indices->set(i, static_cast<cvf::uint>(i));
    }

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    // Convert double data to float data before sending them to ceeViz. 
    // Sigh ....
    cvf::ref<cvf::Vec3fArray> floatCoords = new cvf::Vec3fArray;
    floatCoords->resize(duplicateVertexCount);
    
    (*floatCoords)[0] = cvf::Vec3f((*coords)[0]);

    for (i = 1; i < coords->size() - 1; ++i)
    {
        (*floatCoords)[2 * i - 1] = cvf::Vec3f((*coords)[i]);
        (*floatCoords)[2 * i + 0] = cvf::Vec3f((*coords)[i]);
    }

    (*floatCoords)[duplicateVertexCount - 1] = cvf::Vec3f((*coords)[coords->size() - 1]);

    geo->setVertexArray(floatCoords.p());

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(indices.p());

    geo->addPrimitiveSet(prim.p());

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPipeGeometryGenerator::generateExtrudedCylinder(double radius, size_t crossSectionNodeCount, const cvf::Vec3dArray* cylinderCenterCoords)
{
    CVF_ASSERT(cylinderCenterCoords != NULL);

    if (cylinderCenterCoords->size() < 2) return NULL;

    std::vector<cvf::Vec3f> crossSectionVertices;
    std::vector<cvf::Vec3f> cylinderSegmentNormals;

    cvf::Vec3d dir = (*cylinderCenterCoords)[1] - (*cylinderCenterCoords)[0];
    dir.normalize();

    cvf::Vec3d orient1 = dir.perpendicularVector();
    orient1.normalize();

    cvf::Vec3d orient2 = dir ^ orient1;
    orient2.normalize();

    std::vector<cvf::Vec3d> extrudedNodes;
    computeCircle(radius, crossSectionNodeCount, (*cylinderCenterCoords)[0], orient1, orient2, &extrudedNodes);

    // Insert the first set of vertices

    size_t i;
    for (i = 0; i < extrudedNodes.size(); i++)
    {
        crossSectionVertices.push_back(cvf::Vec3f(extrudedNodes[i]));
    }


    // Calculate first valid pipe direction, to be able to handle centerNodes in the same place

    cvf::Vec3d lastValidPipeDirection(0,0,-1);
    for (i = 0; i < cylinderCenterCoords->size() - 1; i++)
    {
        cvf::Vec3d candidateDir = (*cylinderCenterCoords)[i] - (*cylinderCenterCoords)[i+1];
        if (candidateDir.normalize()) 
        {
            lastValidPipeDirection = candidateDir;
            break;
        }
    }

    if (i >= cylinderCenterCoords->size()-1) return NULL; // The pipe coordinates is all the same point

    // Loop along the cylinder center coords and calculate the cross section vertexes in each center vertex

    for (size_t ccIdx = 0; ccIdx < cylinderCenterCoords->size() - 1; ccIdx++)
    {
        size_t nextCcIdx = ccIdx + 1;

        // Calculate this and next pipe direction, and intersection plane 

        cvf::Vec3d firstCoord = (*cylinderCenterCoords)[ccIdx];
        cvf::Vec3d secondCoord = (*cylinderCenterCoords)[nextCcIdx];

        cvf::Vec3d candidateDir = secondCoord - firstCoord;
        if (!candidateDir.normalize())
        {
            candidateDir = lastValidPipeDirection;    
        }
        else
        {
            lastValidPipeDirection = candidateDir;
        }

        cvf::Vec3d nextDir = lastValidPipeDirection;
        if (nextCcIdx + 1 < cylinderCenterCoords->size())
        {
            cvf::Vec3d thirdCoord = (*cylinderCenterCoords)[nextCcIdx + 1];
            nextDir = thirdCoord - secondCoord;
        }

        // If the next vector is too small, just assume the pipe is heading straight on
        if (!nextDir.normalize()) nextDir = candidateDir;

        cvf::Vec3d intersectionPlaneNormal = candidateDir + nextDir;

        // if (intersectionPlaneNormal.lengthSquared() < 1e-10) intersectionPlaneNormal = nextDir; // candidateDir == -nextDir => 180 deg turn

        computeExtrudedCoordsAndNormals(secondCoord, intersectionPlaneNormal, candidateDir, crossSectionNodeCount, &extrudedNodes, &crossSectionVertices, &cylinderSegmentNormals);
    }

    size_t crossSectionCount = crossSectionVertices.size() / crossSectionNodeCount;

    if (crossSectionCount < 2) return NULL;

    CVF_ASSERT(crossSectionVertices.size() - crossSectionNodeCount == cylinderSegmentNormals.size());

    size_t segmentCount = crossSectionCount - 1;
    size_t vertexCount = segmentCount * crossSectionNodeCount * 4;

    cvf::ref<cvf::Vec3fArray> quadVertexArray = new cvf::Vec3fArray();
    quadVertexArray->reserve(vertexCount);

    cvf::ref<cvf::Vec3fArray> quadNormalArray = new cvf::Vec3fArray();
    quadNormalArray->reserve(vertexCount);

    size_t segmentIdx = 0;
    for (segmentIdx = 0; segmentIdx < segmentCount; segmentIdx++)
    {
        size_t i;
        for (i = 0; i < crossSectionNodeCount - 1; i++)
        {
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 0) * crossSectionNodeCount) + i + 0]);
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 0) * crossSectionNodeCount) + i + 1]);
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 1) * crossSectionNodeCount) + i + 1]);
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 1) * crossSectionNodeCount) + i + 0]);

            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + i + 0]);
            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + i + 1]);
            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + i + 1]);
            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + i + 0]);
        }

        // Last quad closing the cylinder
        quadVertexArray->add(crossSectionVertices[( (segmentIdx + 0) * crossSectionNodeCount) + crossSectionNodeCount - 1]);
        quadVertexArray->add(crossSectionVertices[( (segmentIdx + 0) * crossSectionNodeCount) + 0]);
        quadVertexArray->add(crossSectionVertices[( (segmentIdx + 1) * crossSectionNodeCount) + 0]);
        quadVertexArray->add(crossSectionVertices[( (segmentIdx + 1) * crossSectionNodeCount) + crossSectionNodeCount - 1]);

        quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + crossSectionNodeCount - 1]);
        quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + 0]);
        quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + 0]);
        quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + crossSectionNodeCount - 1]);
    }

    CVF_ASSERT(vertexCount == quadVertexArray->size());
    CVF_ASSERT(vertexCount == quadNormalArray->size());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromQuadVertexArray(quadVertexArray.p());
    geo->setNormalArray(quadNormalArray.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::computeExtrudedCoordsAndNormals( cvf::Vec3d intersectionCoord,
                                                                cvf::Vec3d intersectionPlaneNormal,
                                                                cvf::Vec3d segmentDirection,
                                                                size_t crossSectionNodeCount,
                                                                std::vector<cvf::Vec3d>* extrudedNodes,
                                                                std::vector<cvf::Vec3f>* crossSectionVertices,
                                                                std::vector<cvf::Vec3f>* cylinderSegmentNormals)
{
    cvf::Plane intersectionPlane;
    intersectionPlane.setFromPointAndNormal(intersectionCoord, intersectionPlaneNormal);

    cvf::Ray ray;
    ray.setDirection(segmentDirection);

    // Calculate next set of extruded vertices

    std::vector<cvf::Vec3d> nextExtrudedNodes;

    for (size_t csnIdx = 0; csnIdx < crossSectionNodeCount; csnIdx++)
    {
        ray.setOrigin((*extrudedNodes)[csnIdx]);

        cvf::Vec3d intersection;
        if (ray.planeIntersect(intersectionPlane, &intersection))
        {
            nextExtrudedNodes.push_back(intersection);
        }
        else
        {
            cvf::Ray oppositeRay(ray);
            oppositeRay.setDirection(-segmentDirection);
            if (oppositeRay.planeIntersect(intersectionPlane, &intersection))
            {
                nextExtrudedNodes.push_back(intersection);
            }
            else
            {
                // Parallel plane and ray
                CVF_ASSERT(false);
            }
        }
    }


    // Calculate the normals for the cylinder segment

    cvf::Plane cylinderPlane;
    cylinderPlane.setFromPointAndNormal(intersectionCoord, segmentDirection);

    for (size_t csnIdx = 0; csnIdx < crossSectionNodeCount; csnIdx++)
    {
        crossSectionVertices->push_back(cvf::Vec3f(nextExtrudedNodes[csnIdx]));

        cvf::Vec3d pipeNormal;
        cylinderPlane.projectVector(nextExtrudedNodes[csnIdx] - intersectionCoord, &pipeNormal);
        cylinderSegmentNormals->push_back(cvf::Vec3f(pipeNormal));
    }

    *extrudedNodes = nextExtrudedNodes;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::updateFilteredPipeCenterCoords()
{
    if (m_originalPipeCenterCoords->size() < 2) return;
    if (m_filteredPipeCenterCoords.size() > 0) return;


    double squareDistanceTolerance = 1e-4*1e-4;
    double angleTolerance = 1e-5;

    size_t firstSegmentWithLength = 0;
    size_t i;
    for (i = 0; i < m_originalPipeCenterCoords->size() - 1; i++)
    {
        cvf::Vec3d candidateDir = (*m_originalPipeCenterCoords)[i] - (*m_originalPipeCenterCoords)[i+1];
        double dirLengthSq = candidateDir.lengthSquared();
        if (dirLengthSq > squareDistanceTolerance && candidateDir.normalize()) 
        {
            firstSegmentWithLength = i;
            break;
        }
    }

    // Only zero-length segments
    if (i == m_originalPipeCenterCoords->size() - 1) return;

    m_filteredPipeCenterCoords.push_back(m_originalPipeCenterCoords->get(firstSegmentWithLength));
    m_filteredPipeSegmentToResult.push_back(firstSegmentWithLength);

    cvf::Vec3d lastValidDirectionAB;
    size_t lastValidSegment = 0;

    for (i = firstSegmentWithLength + 1; i < m_originalPipeCenterCoords->size() - 1; i++)
    {
        cvf::Vec3d coordA = m_originalPipeCenterCoords->get(i - 1);
        cvf::Vec3d coordB = m_originalPipeCenterCoords->get(i + 0);
        cvf::Vec3d coordC = m_originalPipeCenterCoords->get(i + 1);

        cvf::Vec3d directionAB = coordB - coordA;

        // Skip segment lengths below tolerance
        double dirLengthSq = directionAB.lengthSquared();
        if (directionAB.lengthSquared() > squareDistanceTolerance)
        {
            lastValidDirectionAB = directionAB.getNormalized();
            lastValidSegment = i;
        }

        cvf::Vec3d directionBC = coordC - coordB;
        if (directionBC.lengthSquared() < squareDistanceTolerance)
        {
            continue;
        }

        double cosMinBendAngle = cvf::Math::cos(cvf::Math::toRadians(m_minimumBendAngle));
        double dotProduct = lastValidDirectionAB * (-directionBC).getNormalized();
        if (dotProduct > cosMinBendAngle)
        {
            bool success = false;
            cvf::Vec3d pipeIntermediateDirection = (lastValidDirectionAB + directionBC.getNormalized()).getNormalized(&success);
            if (!success)
            {
                pipeIntermediateDirection = lastValidDirectionAB.perpendicularVector();
            }

            double bendRadius = m_bendScalingFactor * m_radius + 1.0e-30;
            cvf::Vec3d firstIntermediate = coordB - pipeIntermediateDirection * bendRadius;

            m_filteredPipeCenterCoords.push_back(firstIntermediate);
            m_filteredPipeSegmentToResult.push_back(lastValidSegment);

            m_filteredPipeCenterCoords.push_back(coordB);
            m_filteredPipeSegmentToResult.push_back(i);

            cvf::Vec3d secondIntermediate = coordB + pipeIntermediateDirection * bendRadius;
            m_filteredPipeCenterCoords.push_back(secondIntermediate);
            m_filteredPipeSegmentToResult.push_back(i);
        }
        else
        {
            m_filteredPipeCenterCoords.push_back(coordB);
            m_filteredPipeSegmentToResult.push_back(i);
        }
    }

    // Add last cross section if not duplicate coordinate
    cvf::Vec3d coordA = m_originalPipeCenterCoords->get(m_originalPipeCenterCoords->size() - 2);
    cvf::Vec3d coordB = m_originalPipeCenterCoords->get(m_originalPipeCenterCoords->size() - 1);

    cvf::Vec3d directionAB = coordB - coordA;
    if (directionAB.lengthSquared() > squareDistanceTolerance)
    {
        m_filteredPipeCenterCoords.push_back(m_originalPipeCenterCoords->get(m_originalPipeCenterCoords->size() - 1));
    }
    else
    {
        // Remove last segment as the length is below tolerance
        m_filteredPipeSegmentToResult.pop_back();
    }

    CVF_ASSERT(m_filteredPipeCenterCoords.size() - 1 == m_filteredPipeSegmentToResult.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::clearComputedData()
{
    m_filteredPipeCenterCoords.clear();
    m_filteredPipeSegmentToResult.clear();
}

