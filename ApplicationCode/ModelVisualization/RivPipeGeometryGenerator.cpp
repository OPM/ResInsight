/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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


#include "RivPipeGeometryGenerator.h"

#include "RivObjectSourceInfo.h"

#include "cafEffectGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfPlane.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRay.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivPipeGeometryGenerator::RivPipeGeometryGenerator()
{
    m_radius = 1.0;
    m_crossSectionNodeCount = 8;
    m_minimumBendAngle = 80.0;
    m_bendScalingFactor = 0.00001;
    m_firstVisibleSegmentIndex = 0;
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
        return nullptr;
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
    CVF_ASSERT(coords != nullptr);

    if (coords->size() < 2 ) return nullptr;

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
    CVF_ASSERT(cylinderCenterCoords != nullptr);

    if (cylinderCenterCoords->size() < 2) return nullptr;

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

    if (i >= cylinderCenterCoords->size()-1) return nullptr; // The pipe coordinates is all the same point

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

        if (intersectionPlaneNormal.lengthSquared() < 1e-10) // candidateDir == -nextDir => 180 deg turn
        {
            CVF_ASSERT(false); // This is never supposed to happen due to what's done in updateFilteredPipeCenterCoords(). So look there for the bug... 
            intersectionPlaneNormal = nextDir; 
        }

        computeExtrudedCoordsAndNormals(secondCoord, intersectionPlaneNormal, candidateDir, crossSectionNodeCount, &extrudedNodes, &crossSectionVertices, &cylinderSegmentNormals);
    }

    size_t crossSectionCount = crossSectionVertices.size() / crossSectionNodeCount;

    if (crossSectionCount < 2) return nullptr;

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
        for (size_t nodeIdx = 0; nodeIdx < crossSectionNodeCount - 1; nodeIdx++)
        {
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 0]);
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 1]);
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 1) * crossSectionNodeCount) + nodeIdx + 1]);
            quadVertexArray->add(crossSectionVertices[( (segmentIdx + 1) * crossSectionNodeCount) + nodeIdx + 0]);

            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 0]);
            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 1]);
            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 1]);
            quadNormalArray->add(cylinderSegmentNormals[( (segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 0]);
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
cvf::ref<cvf::DrawableGeo> RivPipeGeometryGenerator::generateVariableRadiusTube(size_t                 crossSectionNodeCount,
                                                                                const cvf::Vec3dArray* cylinderCenterCoords,
                                                                                const std::vector<double>& radii)
{
    const double epsilon = 1.0e-8;

    CVF_ASSERT(cylinderCenterCoords != nullptr);
   
    // Calculate first valid pipe direction, to be able to handle centerNodes in the same place
    cvf::Vec3d lastValidPipeDirection = cvf::Vec3d::UNDEFINED;
    for (size_t i = 0; i < cylinderCenterCoords->size() - 1; i++)
    {
        cvf::Vec3d candidateDir = (*cylinderCenterCoords)[i + 1] - (*cylinderCenterCoords)[i];
        if (candidateDir.normalize())
        {
            lastValidPipeDirection = candidateDir;
            break;
        }
    }

    if (lastValidPipeDirection.isUndefined()) return nullptr; // The pipe coordinates are all the same point

    cvf::Vec3d dir = (*cylinderCenterCoords)[1] - (*cylinderCenterCoords)[0];
    if (!dir.normalize())
    {
        dir = lastValidPipeDirection;
    }

    cvf::Vec3d orient1 = dir.perpendicularVector();
    orient1.normalize();

    cvf::Vec3d orient2 = dir ^ orient1;
    orient2.normalize();

    std::vector<cvf::Vec3d> lastExtrudedNodes;
    computeCircle(radii[0], crossSectionNodeCount, (*cylinderCenterCoords)[0], orient1, orient2, &lastExtrudedNodes);

    std::vector<cvf::Vec3f> crossSectionVertices;
    std::vector<cvf::Vec3f> cylinderSegmentNormals;

    // Insert the first set of vertices
    for (size_t i = 0; i < lastExtrudedNodes.size(); i++)
    {
        crossSectionVertices.push_back(cvf::Vec3f(lastExtrudedNodes[i]));
    }

    // Loop along the cylinder center coords and calculate the cross section vertexes in each center vertex
    
    for (size_t ccIdx = 0; ccIdx < cylinderCenterCoords->size()-1; ccIdx++)
    {
        size_t nextCcIdx = ccIdx + 1;

        // Calculate this and next pipe direction, and intersection plane
        cvf::Vec3d firstCoord  = (*cylinderCenterCoords)[ccIdx];
        cvf::Vec3d secondCoord = (*cylinderCenterCoords)[nextCcIdx];

        cvf::Vec3d nextDir = secondCoord - firstCoord;
        if (!nextDir.normalize())
        {
            nextDir = lastValidPipeDirection;
        }
        else
        {
            lastValidPipeDirection = nextDir;
        }

        orient1 = nextDir.perpendicularVector().getNormalized();
        orient2 = (nextDir ^ orient1).getNormalized();

        std::vector<cvf::Vec3d> extrudedNodes;
        computeCircle(radii[nextCcIdx], crossSectionNodeCount, (*cylinderCenterCoords)[nextCcIdx], orient1, orient2, &extrudedNodes);

        // Insert the next set of vertices and calculate normals for segment
        for (size_t i = 0; i < extrudedNodes.size(); i++)
        {
            cvf::Vec3f nextNodeAlongWellPath = cvf::Vec3f(extrudedNodes[i]);
            cvf::Vec3f currentNode           = cvf::Vec3f(lastExtrudedNodes[i]);

            cvf::Vec3f wellDirectionDir = nextNodeAlongWellPath - currentNode;
            if (!wellDirectionDir.normalize())
            {
                wellDirectionDir = cvf::Vec3f(lastValidPipeDirection);
            }

            cvf::Vec3f lastNodeAlongCircle;
            cvf::Vec3f nextNodeAlongCircle;
            if (i > 0)
            {
                lastNodeAlongCircle = cvf::Vec3f(lastExtrudedNodes[i - 1]);
            }
            else
            {
                lastNodeAlongCircle = cvf::Vec3f(lastExtrudedNodes.back());
            }
            if (i < extrudedNodes.size() - 1)
            {
                nextNodeAlongCircle = cvf::Vec3f(lastExtrudedNodes[i + 1]);
            }
            else
            {
                nextNodeAlongCircle = cvf::Vec3f(lastExtrudedNodes.front());
            }

            cvf::Vec3f circleDir = (nextNodeAlongCircle - lastNodeAlongCircle).getNormalized();
            cvf::Vec3f segmentNormal = (wellDirectionDir ^ circleDir).getNormalized();

            crossSectionVertices.push_back(cvf::Vec3f(nextNodeAlongWellPath));
            cylinderSegmentNormals.push_back(segmentNormal);

        }

        lastExtrudedNodes = extrudedNodes;
    }

    size_t crossSectionCount = crossSectionVertices.size() / crossSectionNodeCount;

    if (crossSectionCount < 2) return nullptr;

    CVF_ASSERT(crossSectionVertices.size() - crossSectionNodeCount == cylinderSegmentNormals.size());

    size_t segmentCount = crossSectionCount - 1;
    size_t vertexCount  = segmentCount * crossSectionNodeCount * 4;

    cvf::ref<cvf::Vec3fArray> quadVertexArray = new cvf::Vec3fArray();
    quadVertexArray->reserve(vertexCount);

    cvf::ref<cvf::Vec3fArray> quadNormalArray = new cvf::Vec3fArray();
    quadNormalArray->reserve(vertexCount);

    size_t segmentIdx = 0;
    for (segmentIdx = 0; segmentIdx < segmentCount; segmentIdx++)
    {
        for (size_t nodeIdx = 0; nodeIdx < crossSectionNodeCount - 1; nodeIdx++)
        {
            quadVertexArray->add(crossSectionVertices[((segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 0]);
            quadVertexArray->add(crossSectionVertices[((segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 1]);
            quadVertexArray->add(crossSectionVertices[((segmentIdx + 1) * crossSectionNodeCount) + nodeIdx + 1]);
            quadVertexArray->add(crossSectionVertices[((segmentIdx + 1) * crossSectionNodeCount) + nodeIdx + 0]);

            quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 0]);
            quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 1]);
            quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 1]);
            quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + nodeIdx + 0]);
        }

        // Last quad closing the cylinder
        quadVertexArray->add(crossSectionVertices[((segmentIdx + 0) * crossSectionNodeCount) + crossSectionNodeCount - 1]);
        quadVertexArray->add(crossSectionVertices[((segmentIdx + 0) * crossSectionNodeCount) + 0]);
        quadVertexArray->add(crossSectionVertices[((segmentIdx + 1) * crossSectionNodeCount) + 0]);
        quadVertexArray->add(crossSectionVertices[((segmentIdx + 1) * crossSectionNodeCount) + crossSectionNodeCount - 1]);

        quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + crossSectionNodeCount - 1]);
        quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + 0]);
        quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + 0]);
        quadNormalArray->add(cylinderSegmentNormals[((segmentIdx + 0) * crossSectionNodeCount) + crossSectionNodeCount - 1]);
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

    const size_t lastOriginalCoordIdx = m_originalPipeCenterCoords->size() - 1;

    size_t firstSegmentWithLength = findFirstSegmentWithLength(squareDistanceTolerance);

    // Return if we have only zero-length segments

    if (firstSegmentWithLength == cvf::UNDEFINED_SIZE_T) return;

    m_filteredPipeCenterCoords.push_back(m_originalPipeCenterCoords->get(firstSegmentWithLength));
    m_filteredPipeSegmentToResult.push_back(firstSegmentWithLength);

    cvf::Vec3d lastValidDirectionAB;
    size_t lastValidSegment = 0;

    // Go along the line, inserting bends, and skipping zero segments.
    // The zero segments are skipped by ignoring the _first_ coordinate(s) equal to the next ones

    for (size_t coordBIdx = firstSegmentWithLength + 1; coordBIdx < lastOriginalCoordIdx; coordBIdx++)
    {
        cvf::Vec3d coordA = m_originalPipeCenterCoords->get(coordBIdx - 1);
        cvf::Vec3d coordB = m_originalPipeCenterCoords->get(coordBIdx + 0);
        cvf::Vec3d coordC = m_originalPipeCenterCoords->get(coordBIdx + 1);

        cvf::Vec3d directionAB = coordB - coordA;

        if (directionAB.lengthSquared() > squareDistanceTolerance)
        {
            lastValidDirectionAB = directionAB.getNormalized();
            lastValidSegment = coordBIdx;
        }

        // Wait to store a segment until we find an endpoint that is the start point of a valid segment
         
        cvf::Vec3d directionBC = coordC - coordB;
        if (directionBC.lengthSquared() < squareDistanceTolerance)
        {
            continue;
        }

        // Check if the angle between AB and BC is sharper than m_minimumBendAngle (Straight == 180 deg)
        // Sharper angle detected, insert bending stuff

        double cosMinBendAngle = cvf::Math::cos(cvf::Math::toRadians(m_minimumBendAngle));
        double dotProduct = lastValidDirectionAB * (-directionBC).getNormalized();
        if (dotProduct > cosMinBendAngle) 
        {
            bool success = false;

            cvf::Vec3d pipeIntermediateDirection = (lastValidDirectionAB + directionBC.getNormalized());
            pipeIntermediateDirection.getNormalized(&success);

            if (pipeIntermediateDirection.lengthSquared() < squareDistanceTolerance)
            {
                pipeIntermediateDirection = lastValidDirectionAB.perpendicularVector();
            }
            else
            {
                pipeIntermediateDirection.normalize();
            }

            double bendRadius = m_bendScalingFactor * m_radius + 1.0e-30;
            cvf::Vec3d firstIntermediate = coordB - pipeIntermediateDirection * bendRadius;
            cvf::Vec3d secondIntermediate = coordB + pipeIntermediateDirection * bendRadius;

            m_filteredPipeCenterCoords.push_back(firstIntermediate);
            m_filteredPipeSegmentToResult.push_back(lastValidSegment);

            m_filteredPipeCenterCoords.push_back(coordB);
            m_filteredPipeSegmentToResult.push_back(coordBIdx);

            m_filteredPipeCenterCoords.push_back(secondIntermediate);
            m_filteredPipeSegmentToResult.push_back(coordBIdx);
        }
        else
        {
            m_filteredPipeCenterCoords.push_back(coordB);
            m_filteredPipeSegmentToResult.push_back(coordBIdx);
        }
    }

    // Add the last point, as the above loop will not end the last none-zero segment, but wait for the start of the next valid one.

    m_filteredPipeCenterCoords.push_back(m_originalPipeCenterCoords->get(lastOriginalCoordIdx));
   
    CVF_ASSERT(m_filteredPipeCenterCoords.size() - 1 == m_filteredPipeSegmentToResult.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RivPipeGeometryGenerator::findFirstSegmentWithLength(double squareDistanceTolerance)
{
    size_t segIdx;
    for ( segIdx = 0; segIdx < m_originalPipeCenterCoords->size() - 1; segIdx++ )
    {
        cvf::Vec3d candidateDir = (*m_originalPipeCenterCoords)[segIdx] - (*m_originalPipeCenterCoords)[segIdx+1];
        double dirLengthSq = candidateDir.lengthSquared();
        if ( dirLengthSq > squareDistanceTolerance && candidateDir.normalize() )
        {
            return segIdx;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::clearComputedData()
{
    m_filteredPipeCenterCoords.clear();
    m_filteredPipeSegmentToResult.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RivPipeGeometryGenerator::segmentIndexFromTriangleIndex(size_t triangleIndex) const
{
    size_t segIndex = triangleIndex / (m_crossSectionNodeCount * 2);

    CVF_ASSERT(segIndex < m_filteredPipeSegmentToResult.size());
    size_t resultIndex = m_filteredPipeSegmentToResult[segIndex];

    return resultIndex + m_firstVisibleSegmentIndex;
}

//--------------------------------------------------------------------------------------------------
/// Well pipes are clipped, set index to first segment in visible well path
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::setFirstVisibleSegmentIndex(size_t segmentIndex)
{
    m_firstVisibleSegmentIndex = segmentIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::cylinderWithCenterLineParts(cvf::Collection<cvf::Part>* destinationParts, const std::vector<cvf::Vec3d>& centerCoords, const cvf::Color3f& color, double radius)
{
    setRadius(radius);
    setCrossSectionVertexCount(12);

    cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray(centerCoords);
    setPipeCenterCoords(cvfCoords.p());

    cvf::ref<cvf::DrawableGeo> surfaceGeo = createPipeSurface();
    if (surfaceGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(surfaceGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }

    cvf::ref<cvf::DrawableGeo> centerLineGeo = createCenterLine();
    if (centerLineGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(centerLineGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivPipeGeometryGenerator::tubeWithCenterLinePartsAndVariableWidth(cvf::Collection<cvf::Part>*    destinationParts,
                                                                       const std::vector<cvf::Vec3d>& centerCoords,
                                                                       const std::vector<double>&     radii,
                                                                       const cvf::Color3f&            color)
{
    setCrossSectionVertexCount(12);

    cvf::ref<cvf::Vec3dArray> activeCoords = new cvf::Vec3dArray(centerCoords);
    setPipeCenterCoords(activeCoords.p());

    cvf::ref<cvf::DrawableGeo> surfaceGeo = generateVariableRadiusTube(m_crossSectionNodeCount, activeCoords.p(), radii);

    if (surfaceGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(surfaceGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }

    cvf::ref<cvf::DrawableGeo> centerLineGeo = createCenterLine();
    if (centerLineGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(centerLineGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }
}

