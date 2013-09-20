//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfVertexWelder.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::VertexWelder
/// \ingroup Geometry
///
/// Supports welding of vertices based on vertex distance
///
/// \internal Adapted from the book: "Real time collision detection' by Christer Ericson
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VertexWelder::VertexWelder()
{
    m_weldEpsilon = 0;
    m_cellSize = 0;
    m_numBuckets = 0;
}


//--------------------------------------------------------------------------------------------------
/// Initialize, must be done before usage of object
/// 
/// The cell size must be at least 2*weldingDistance, but should normally be much larger. If the 
/// specified cell size is too small, it will be set to approximately 2*weldingDistance
//--------------------------------------------------------------------------------------------------
void VertexWelder::initialize(double weldingDistance, double cellSize, uint numBuckets)
{
    CVF_ASSERT(weldingDistance >= 0);
    CVF_ASSERT(cellSize > 2*weldingDistance);
    CVF_ASSERT(numBuckets > 0);

    m_weldEpsilon = weldingDistance;
    m_cellSize = cellSize;
    if (m_cellSize < 2.1*weldingDistance)
    {
        m_cellSize = 2.1*weldingDistance;
    }

    m_numBuckets = numBuckets;
    m_first.resize(numBuckets);
    m_first.setAll(UNDEFINED_UINT);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexWelder::reserveVertices(uint vertexCount)
{
    m_vertex.reserve(vertexCount);
    m_next.reserve(vertexCount);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint VertexWelder::weldVertex(const Vec3f& vertex, bool* wasWelded)
{
    // Must be initialized
    CVF_ASSERT(m_cellSize > 0 && m_numBuckets > 0);

    // Make sure welding distance (epsilon) is not too small for the coordinates used!
    // Not sure if we need to handle this case - unsure of what happens if epsilon gets too small.
    // Guess the only trouble is that we won't look into neighbor cells, which in turn
    // means that we won't be able to weld vertices even if they are closer than epsilon.
    //CVF_ASSERT(vertex.x() - m_weldEpsilon != vertex.x() && vertex.x() + m_weldEpsilon != vertex.x());
    //CVF_ASSERT(vertex.y() - m_weldEpsilon != vertex.y() && vertex.y() + m_weldEpsilon != vertex.y());
    //CVF_ASSERT(vertex.z() - m_weldEpsilon != vertex.z() && vertex.z() + m_weldEpsilon != vertex.z());

    // Compute cell coordinates of bounding box of vertex epsilon neighborhood
    int left    = int((vertex.x() - m_weldEpsilon) / m_cellSize);
    int right   = int((vertex.x() + m_weldEpsilon) / m_cellSize);
    int front   = int((vertex.y() - m_weldEpsilon) / m_cellSize);
    int back    = int((vertex.y() + m_weldEpsilon) / m_cellSize);
    int bottom  = int((vertex.z() - m_weldEpsilon) / m_cellSize);
    int top     = int((vertex.z() + m_weldEpsilon) / m_cellSize);

    // To lessen effects of worst-case behavior, track previously tested buckets
    // 4 in 2D, 8 in 3D
    uint prevBucket[8]; 
    int numPrevBuckets = 0;

    // Loop over all overlapped cells and test against their buckets
    int i;
    for (i = left; i <= right; i++) 
    {
        int j;
        for (j = front; j <= back; j++) 
        {
            int k;
            for (k = bottom; k <= top; k++) 
            {
                uint bucket = getGridCellBucket(i, j, k);

                // If this bucket already tested, don't test it again
                bool bucketAlreadyTested = false;
                for (int b = 0; b < numPrevBuckets; b++)
                {
                    if (bucket == prevBucket[b]) 
                    {
                        bucketAlreadyTested = true;
                        break;
                    }
                }

                if (!bucketAlreadyTested)
                {
                    // Add this bucket to visited list, then test against its contents
                    CVF_ASSERT(numPrevBuckets < 8);
                    prevBucket[numPrevBuckets++] = bucket;

                    // Call function to step through linked list of bucket, testing
                    // if vertex is within the epsilon of one of the vertices in the bucket
                    uint indexOfLocatedVertex = locateVertexInBucket(vertex, bucket);
                    if (indexOfLocatedVertex != UNDEFINED_UINT) 
                    {
                        if (wasWelded) *wasWelded = true;

                        return indexOfLocatedVertex;
                    }
                }
            }
        }
    }

    // Couldn't locate vertex, so add it to grid
    int x = int(vertex.x() / m_cellSize);
    int y = int(vertex.y() / m_cellSize);
    int z = int(vertex.z() / m_cellSize);
    uint indexOfAddedVertex = addVertexToBucket(vertex, getGridCellBucket(x, y, z));

    if (wasWelded) *wasWelded = false;

    return indexOfAddedVertex;
}


//--------------------------------------------------------------------------------------------------
/// Maps unbounded grid cell coordinates (x, y, z) into an index into a fixed-size array of hash buckets
//--------------------------------------------------------------------------------------------------
uint VertexWelder::getGridCellBucket(int x, int y, int z) const
{
    // Large multiplicative constants; here arbitrarily chosen primes
    const uint magic1 = 0x8da6b343; 
    const uint magic2 = 0xd8163841; 
    const uint magic3 = 0xcb1ab31f; 

    uint index = magic1*x + magic2*y + z*magic3;

    // Bring index into [0, m_numBuckets) range
    return index % m_numBuckets;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint VertexWelder::locateVertexInBucket(const Vec3f& v, uint bucket) const
{
    const double weldEpsilonSqared = m_weldEpsilon*m_weldEpsilon;

    // Scan through linked list of vertices at this bucket
    uint index = m_first[bucket];
    while (index != UNDEFINED_UINT)
    {
        // Weld this vertex to existing vertex if within given distance tolerance
        float sqDistPointPoint = (m_vertex[index] - v).lengthSquared();
        if (sqDistPointPoint < weldEpsilonSqared) 
        {
            return index;
        }

        index = m_next[index];
    }

    // No vertex found to weld to. 
    return UNDEFINED_UINT;
} 


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint VertexWelder::addVertexToBucket(const Vec3f& v, uint bucket)
{
    CVF_TIGHT_ASSERT(bucket < m_numBuckets);
    CVF_TIGHT_ASSERT(m_numBuckets == m_first.size());

    // Fill next available vertex buffer entry and link it into vertex list
    m_vertex.push_back(v);
    m_next.push_back(m_first[bucket]);
    CVF_TIGHT_ASSERT(m_vertex.size() == m_next.size());

    uint indexOfAddedVertex = static_cast<uint>(m_vertex.size() - 1);
    m_first[bucket] = indexOfAddedVertex;

    return indexOfAddedVertex;
} 


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint VertexWelder::vertexCount() const
{
    return static_cast<uint>(m_vertex.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3f& VertexWelder::vertex(uint index) const
{
    return m_vertex[index];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> VertexWelder::createVertexArray() const
{
    ref<Vec3fArray> va = new Vec3fArray(m_vertex);
    return va;

}


} // namespace cvf

