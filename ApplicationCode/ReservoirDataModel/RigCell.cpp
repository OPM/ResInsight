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

#include "RIStdInclude.h"

#include "RigCell.h"
#include "RigMainGrid.h"
#include "cvfPlane.h"

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
    m_activeIndexInFractureModel(cvf::UNDEFINED_SIZE_T),
    m_cellIndex(cvf::UNDEFINED_SIZE_T)
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

