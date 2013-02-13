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


#include "RigCell.h"
#include "RigMainGrid.h"
#include "cvfPlane.h"
#include "cvfRay.h"

#include <math.h>

static size_t undefinedCornersArray[8] = {cvf::UNDEFINED_SIZE_T,
                                          cvf::UNDEFINED_SIZE_T,
                                          cvf::UNDEFINED_SIZE_T,
                                          cvf::UNDEFINED_SIZE_T,
                                          cvf::UNDEFINED_SIZE_T,
                                          cvf::UNDEFINED_SIZE_T,
                                          cvf::UNDEFINED_SIZE_T,
                                          cvf::UNDEFINED_SIZE_T };
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCell::RigCell() : 
    m_parentCellIndex(cvf::UNDEFINED_SIZE_T),
    m_mainGridCellIndex(cvf::UNDEFINED_SIZE_T),
    m_subGrid(NULL),
    m_hostGrid(NULL),
    m_isInvalid(false),
    m_isWellCell(false),
    m_activeIndexInMatrixModel(cvf::UNDEFINED_SIZE_T),
    m_cellIndex(cvf::UNDEFINED_SIZE_T),
    m_isInCoarseCell(false)
{
    memcpy(m_cornerIndices.m_array, undefinedCornersArray, 8*sizeof(size_t));

    m_cellFaceFaults[0] = false;
    m_cellFaceFaults[1] = false;
    m_cellFaceFaults[2] = false;
    m_cellFaceFaults[3] = false;
    m_cellFaceFaults[4] = false;
    m_cellFaceFaults[5] = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCell::~RigCell()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigCell::center() const
{
    cvf::Vec3d avg(cvf::Vec3d::ZERO);

    size_t i;
    for (i = 0; i < 8; i++)
    {
        avg += m_hostGrid->mainGrid()->nodes()[m_cornerIndices[i]];
    }

    avg /= 8.0;

    return avg;
}

bool isNear(const cvf::Vec3d& p1, const cvf::Vec3d& p2, double tolerance)
{
    if (   cvf::Math::abs(p1[0] - p2[0]) < tolerance 
        && cvf::Math::abs(p1[1] - p2[1]) < tolerance 
        && cvf::Math::abs(p1[2] - p2[2]) < tolerance )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCell::isLongPyramidCell(double maxHeightFactor, double nodeNearTolerance ) const
{
    cvf::ubyte faceVertexIndices[4];
    double squaredMaxHeightFactor = maxHeightFactor*maxHeightFactor;

    const std::vector<cvf::Vec3d>& nodes = m_hostGrid->mainGrid()->nodes();

    bool isPyramidCell = false;

    int face;
    for ( face = 0; face < 6 ; ++face)
    {
        cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);
        int zeroLengthEdgeCount = 0;

        const cvf::Vec3d& c0 =  nodes[m_cornerIndices[faceVertexIndices[0]]];
        const cvf::Vec3d& c1 =  nodes[m_cornerIndices[faceVertexIndices[1]]];
        const cvf::Vec3d& c2 =  nodes[m_cornerIndices[faceVertexIndices[2]]];
        const cvf::Vec3d& c3 =  nodes[m_cornerIndices[faceVertexIndices[3]]];

        if (isNear(c0, c1, nodeNearTolerance)) { ++zeroLengthEdgeCount;  }
        if (isNear(c1, c2, nodeNearTolerance)) { ++zeroLengthEdgeCount;  }
        if (isNear(c2, c3, nodeNearTolerance)) { ++zeroLengthEdgeCount;  }

        if (zeroLengthEdgeCount == 3)
        {
            return true;
            // Collapse of a complete face is detected. This is possibly the top of a pyramid

            // "face" has the index to the collapsed face. We need the size of the opposite face
            // to compare it with the pyramid "roof" length.

            cvf::StructGridInterface::FaceType oppositeFace = cvf::StructGridInterface::POS_I;
            switch (face)
            {
            case cvf::StructGridInterface::POS_I:
                oppositeFace = cvf::StructGridInterface::NEG_I;
                break;
            case cvf::StructGridInterface::POS_J:
                oppositeFace = cvf::StructGridInterface::NEG_J;
                break;
            case cvf::StructGridInterface::POS_K:
                oppositeFace = cvf::StructGridInterface::NEG_K;
                break;
            case cvf::StructGridInterface::NEG_I:
                oppositeFace = cvf::StructGridInterface::POS_I;
                break;
            case cvf::StructGridInterface::NEG_J:
                oppositeFace = cvf::StructGridInterface::POS_J;
                break;
            case cvf::StructGridInterface::NEG_K:
                oppositeFace = cvf::StructGridInterface::POS_K;
                break;
            default:
                CVF_ASSERT(false);
                break;
            }

            cvf::StructGridInterface::cellFaceVertexIndices(oppositeFace, faceVertexIndices);

            
            const cvf::Vec3d& c0opp =  nodes[m_cornerIndices[faceVertexIndices[0]]];
            const cvf::Vec3d& c1opp =  nodes[m_cornerIndices[faceVertexIndices[1]]];
            const cvf::Vec3d& c2opp =  nodes[m_cornerIndices[faceVertexIndices[2]]];
            const cvf::Vec3d& c3opp =  nodes[m_cornerIndices[faceVertexIndices[3]]];

            // Check if any of the opposite face vertexes are also degenerated to the pyramid top
            
            int okVertexCount = 0;
            cvf::Vec3d okVxs[4];
            if (!isNear(c0opp, c0, nodeNearTolerance)) { okVxs[okVertexCount] = c0opp; ++okVertexCount;  }
            if (!isNear(c1opp, c0, nodeNearTolerance)) { okVxs[okVertexCount] = c1opp; ++okVertexCount;  }
            if (!isNear(c2opp, c0, nodeNearTolerance)) { okVxs[okVertexCount] = c2opp; ++okVertexCount;  }
            if (!isNear(c3opp, c0, nodeNearTolerance)) { okVxs[okVertexCount] = c3opp; ++okVertexCount;  }

            if (okVertexCount < 2)
            {
                return true;
            }
            else
            {
                // Use the good vertices to calculate a face size that can be compared to the pyramid height:
                double typicalSquaredEdgeLength = 0;
                for (int i = 1; i < okVertexCount; ++i)
                {
                    typicalSquaredEdgeLength += (okVxs[i-1] - okVxs[i]).lengthSquared(); 
                }
                typicalSquaredEdgeLength /= okVertexCount;
                double pyramidHeightSquared = (okVxs[0] - c0).lengthSquared();

                if (pyramidHeightSquared > squaredMaxHeightFactor*typicalSquaredEdgeLength)
                {
                    return true;
                }
            }
        }

        // Check the ratio of the length of opposite edges.
        // both ratios have to be above threshold to detect a pyramid-ish cell
        // Only test this if we have all nonzero edge lenghts.
        else if  (zeroLengthEdgeCount == 0) // If the four first faces are ok, the two last must be as well
        {
            double e0SquareLenght = (c1 - c0).lengthSquared();
            double e2SquareLenght = (c3 - c2).lengthSquared();
            if (    e0SquareLenght / e2SquareLenght > squaredMaxHeightFactor 
                ||  e2SquareLenght / e0SquareLenght > squaredMaxHeightFactor )
            {
                double e1SquareLenght = (c2 - c1).lengthSquared();
                double e3SquareLenght = (c0 - c3).lengthSquared();

                if (   e1SquareLenght / e3SquareLenght > squaredMaxHeightFactor 
                    || e3SquareLenght / e1SquareLenght > squaredMaxHeightFactor )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigCell::faceCenter(cvf::StructGridInterface::FaceType face) const
{
    cvf::Vec3d avg(cvf::Vec3d::ZERO);
    
    cvf::ubyte faceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices(face, faceVertexIndices);

    size_t i;
    for (i = 0; i < 4; i++)
    {
        avg += m_hostGrid->mainGrid()->nodes()[m_cornerIndices[faceVertexIndices[i]]];
    }

    avg /= 4.0;

    return avg;
}

//--------------------------------------------------------------------------------------------------
/// Find the intersection between the cell and the ray. The point closest to the ray origin is returned
/// if no intersection is found, the intersection point is untouched.
//--------------------------------------------------------------------------------------------------
bool RigCell::firstIntersectionPoint(const cvf::Ray& ray, cvf::Vec3d* intersectionPoint) const
{
    CVF_ASSERT(intersectionPoint != NULL);

    cvf::ubyte faceVertexIndices[4];
    int face;
    const std::vector<cvf::Vec3d>& nodes = m_hostGrid->mainGrid()->nodes();

    cvf::Vec3d firstIntersection(cvf::Vec3d::ZERO);
    double minLsq = HUGE_VAL;

    for (face = 0; face < 6 ; ++face)
    {
        cvf::StructGridInterface::cellFaceVertexIndices(static_cast<cvf::StructGridInterface::FaceType>(face), faceVertexIndices);
        cvf::Vec3d intersection;
        cvf::Vec3d faceCenter = this->faceCenter(static_cast<cvf::StructGridInterface::FaceType>(face));

        ray.triangleIntersect(nodes[m_cornerIndices[faceVertexIndices[0]]], 
            nodes[m_cornerIndices[faceVertexIndices[1]]], faceCenter, &intersection);

        for (size_t i = 0; i < 4; ++i)
        {
            size_t next = i < 3 ? i+1 : 0;
            if ( ray.triangleIntersect( nodes[m_cornerIndices[faceVertexIndices[i]]], 
                                        nodes[m_cornerIndices[faceVertexIndices[next]]], 
                                        faceCenter, 
                                        &intersection))
            {
                double lsq = (intersection - ray.origin() ).lengthSquared();
                if (lsq < minLsq)
                {
                    firstIntersection = intersection;
                    minLsq = lsq;
                }
            }
        }
    }

    if (minLsq != HUGE_VAL)
    {
        *intersectionPoint = firstIntersection;
        return true;
    }

    return false;
}

