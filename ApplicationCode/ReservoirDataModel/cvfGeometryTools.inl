/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include <cmath>


#pragma warning (disable : 4503)
namespace cvf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
DataType GeometryTools::interpolateQuad(const cvf::Vec3d& v1, DataType s1,
                                        const cvf::Vec3d& v2, DataType s2,
                                        const cvf::Vec3d& v3, DataType s3,
                                        const cvf::Vec3d& v4, DataType s4,
                                        const cvf::Vec3d& point)
{
    cvf::Vec4d bc = barycentricCoords(v1, v2, v3, v4, point);

    return s1 * bc[0] + s2 * bc[1] + s3 * bc[2] + s4 * bc[3];
}

//--------------------------------------------------------------------------------------------------
/// Inserts the vertex into the polygon if it fits along one of the edges within the tolerance.
/// The method returns true if it was inserted, or if it was already in the polygon, or if it was 
/// within the tolerance of an existing vertex in the polygon. 
/// In the latter situation it replaces the previous vertex in the polygon.
/// 
/// Todo: If a vertex is replaced, the VxToCv map in TimeStepGeometry should be updated
//--------------------------------------------------------------------------------------------------
template<typename VerticeArrayType, typename IndexType>
bool GeometryTools::insertVertexInPolygon(  std::vector<IndexType> * polygon,  
                                            ArrayWrapperConst<VerticeArrayType, cvf::Vec3d> nodeCoords, 
                                            IndexType vertexIndex, 
                                            double tolerance)
{
    CVF_ASSERT(polygon);


    // Check if vertex is directly included already

    for(typename std::vector<IndexType>::iterator it = polygon->begin(); it != polygon->end(); ++it)
    {
        if (*it == vertexIndex) return true;
    }

#if 1
    // Check if the new point is within tolerance of one of the polygon vertices 

    bool existsOrInserted = false;
    for(typename std::vector<IndexType>::iterator it = polygon->begin(); it != polygon->end(); ++it)
    {
        if ( (nodeCoords[*it] - nodeCoords[vertexIndex]).length() < tolerance)
        {
            if (vertexIndex < *it) *it = vertexIndex;
            existsOrInserted = true;
        }
    }

    if (existsOrInserted) return true;
#endif


    // Copy the start polygon to a list

    std::list<IndexType> listPolygon; 
    for (size_t pcIdx = 0; pcIdx < polygon->size(); ++pcIdx)
    {
        listPolygon.push_back((*polygon)[pcIdx]);
    }

    // Insert vertex in polygon if the distance to one of the edges is small enough

    typename std::list<IndexType >::iterator it2;
    typename std::list<IndexType >::iterator insertBefore;

    for (typename std::list<IndexType >::iterator  it = listPolygon.begin(); it != listPolygon.end(); ++it)
    {
        it2 = it;
        ++it2; insertBefore = it2; if (it2 == listPolygon.end()) it2 = listPolygon.begin();

        double sqDistToLine = GeometryTools::linePointSquareDist(nodeCoords[*it], nodeCoords[*it2], nodeCoords[vertexIndex]);
        if (fabs(sqDistToLine) < tolerance*tolerance )
        {
            it = listPolygon.insert(insertBefore, vertexIndex);
            existsOrInserted = true;
        }
    }

    // Write polygon back into the vector

    polygon->clear();
    for (typename std::list<IndexType >::iterator  it = listPolygon.begin(); it != listPolygon.end(); ++it)
    {
        polygon->push_back(*it);
    }

    return existsOrInserted;
}


//--------------------------------------------------------------------------------------------------
/// \brief    Test if a point touches a polygon within the specified tolerance
///
/// \param    polygonNorm  Polygon normal
/// \param    pPolygonVerts  Array of polygon vertice coordinates
/// \param    piVertexIndices  Array of integer node indices for this polygon
/// \param    iNumVerts  Number of vertices in polygon
/// \param    point  The point to be checked 
/// \param  tolerance Tolerance in length
/// \param  touchedEdgeIndex returns -1 if point is inside, and edge index if point touches an edge.
/// \return    true if point lies inside or on the border of the polygon.
/// 
/// \assumpt    Assumes that the polygon is planar
/// \comment    First check if point is on an edge, Then check if it is inside by
///             counting the number of times a ray from point along positive X axis 
///             crosses an edge. Odd number says inside.
/// \author        SP (really by Eric Haines) and JJS
//--------------------------------------------------------------------------------------------------
template<typename VerticeArrayType, typename PolygonArrayType, typename IndexType>
bool GeometryTools::isPointTouchingIndexedPolygon(  const cvf::Vec3d& polygonNormal, 
                                                    cvf::ArrayWrapperConst<VerticeArrayType, cvf::Vec3d> vertices, 
                                                    cvf::ArrayWrapperConst<PolygonArrayType, IndexType> indices, 
                                                    const cvf::Vec3d& point, 
                                                    int* touchedEdgeIndex,  
                                                    double tolerance)
{
    size_t numIndices = indices.size();

    int Z = findClosestAxis(polygonNormal);
    int X = (Z + 1) % 3;
    int Y = (Z + 2) % 3;

    int    crossings;

    int xBelowVx0;
    int yBelowVx0;
    int yBelowVx1 = 0;

    const double* vtx0;
    const double* vtx1 = nullptr;

    double dv0;

    cvf::uint    j;

    // Check if point is on an edge or vertex
    size_t firstIdx;
    size_t secondIdx;

    CVF_TIGHT_ASSERT(touchedEdgeIndex);

    *touchedEdgeIndex = -1;
    for (firstIdx = 0, secondIdx = 1; firstIdx < numIndices; ++firstIdx, ++secondIdx)
    {
        if (secondIdx >= numIndices) secondIdx = 0;
        const cvf::Vec3d& vx0 = vertices[indices[firstIdx]];
        const cvf::Vec3d& vx1 = vertices[indices[secondIdx]];

        double sqDist = GeometryTools::linePointSquareDist(vx0, vx1, point);
        if (sqDist < tolerance*tolerance)
        {
            *touchedEdgeIndex = static_cast<int>(firstIdx);
            return true;
        }
    }

    vtx0 = vertices[indices[numIndices-1]].ptr();

    // get test bit for above/below Y axis. Y of Point is under Y of vtx0 
    yBelowVx0 = ( dv0 = vtx0[Y] - point[Y] ) >= 0.0;

    crossings = 0;
    for (j = 0; j < numIndices; j++) 
    {
        // cleverness:  bobble between filling endpoints of edges, so that the previous edge's shared endpoint is maintained. 
        if (j & 0x1) 
        {        
            vtx0 = vertices[indices[j]].ptr();
            yBelowVx0 = (dv0 = vtx0[Y] - point[Y]) >= 0.0;
        } 
        else 
        {        
            vtx1 = vertices[indices[j]].ptr();
            yBelowVx1 = (vtx1[Y] >= point[Y]);        
        }

        // check if Y of point is between Y of Vx0 and Vx1
        if (yBelowVx0 != yBelowVx1) 
        {        
            // check if X of point is not between X of Vx0 and Vx1
            if ( (xBelowVx0 = (vtx0[X] >= point[X])) == (vtx1[X] >= point[X]) ) 
            {
                if (xBelowVx0) crossings++;
            } 
            else 
            {
                // compute intersection of polygon segment with X ray, note if > point's X. 
                crossings += (vtx0[X] -    dv0*(vtx1[X] - vtx0[X])/(vtx1[Y] - vtx0[Y])) >= point[X];
            }        
        }    
    }

    // test if crossings is odd. If we care about its winding number > 0, then just: inside_flag = crossings > 0; 
    if (crossings & 0x01) return true;

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if we get an actual polygon
/// The returned polygon will keep the winding from the first face.
/// The second face must have opposite winding of the first
//--------------------------------------------------------------------------------------------------
template<typename VerticeArrayType, typename IndexType>
bool GeometryTools::calculateOverlapPolygonOfTwoQuads(std::vector<IndexType> * polygon, 
                                 std::vector<cvf::Vec3d>* createdVertexes, 
                                 EdgeIntersectStorage<IndexType>* edgeIntersectionStorage, 
                                 ArrayWrapperConst<VerticeArrayType, cvf::Vec3d> nodes,
                                 const IndexType cv1CubeFaceIndices[4], 
                                 const IndexType cv2CubeFaceIndices[4],
                                 double tolerance)
{
    CVF_ASSERT(polygon);
    CVF_ASSERT(createdVertexes);

    // Topology analysis

    IndexType newVertexIndex = static_cast<IndexType>(nodes.size() + createdVertexes->size());

    bool cv1VxTouchCv2[4] = { false, false, false, false };
    bool cv2VxTouchCv1[4] = { false, false, false, false };
    int  cv1VxTouchCv2Edge[4] = { -1, -1, -1, -1 };
    int  cv2VxTouchCv1Edge[4] = { -1, -1, -1, -1 };

    int cv1Idx, cv2Idx;
    int numMatchedNodes = 0;

    // First check for complete topological match.

    for (cv1Idx = 0 ; cv1Idx < 4 ; ++cv1Idx)
    {
        for (cv2Idx = 0; cv2Idx < 4; ++cv2Idx)
        {
            if (cv1CubeFaceIndices[cv1Idx] == cv2CubeFaceIndices[cv2Idx])
            {
                cv1VxTouchCv2[cv1Idx] = true;
                cv2VxTouchCv1[cv2Idx] = true;
                ++numMatchedNodes;
                continue;
            }
        }
    }

    if (numMatchedNodes >= 4) // Todo: Handle collapsed cells
    {
        int k;
        for (k = 0; k < 4; ++k)
        {
            polygon->push_back(cv1CubeFaceIndices[k]);
        }

        return true;
    }
   
    cvf::Vec3d diag1 = nodes[cv1CubeFaceIndices[2]] - nodes[cv1CubeFaceIndices[0]];
    cvf::Vec3d diag2 = nodes[cv1CubeFaceIndices[3]] - nodes[cv1CubeFaceIndices[1]];
    cvf::Vec3d normal = diag1^diag2;
    int numCv1VxesOnCv2 = numMatchedNodes;
    int numCv2VxesOnCv1 = numMatchedNodes;

    for (cv1Idx = 0 ; cv1Idx < 4 ; ++cv1Idx)
    {
       if (!cv1VxTouchCv2[cv1Idx])
       {
            cv1VxTouchCv2[cv1Idx] = GeometryTools::isPointTouchingIndexedPolygon(normal, 
                                                                                 nodes, 
                                                                                 wrapArrayConst(cv2CubeFaceIndices, 4),  
                                                                                 nodes[cv1CubeFaceIndices[cv1Idx]], 
                                                                                 &(cv1VxTouchCv2Edge[cv1Idx]), 
                                                                                 tolerance);        
            if (cv1VxTouchCv2[cv1Idx]) ++numCv1VxesOnCv2;
       }

       if (!cv2VxTouchCv1[cv1Idx])
       {
           cv2VxTouchCv1[cv1Idx] = GeometryTools::isPointTouchingIndexedPolygon(normal, 
                                                                                nodes, 
                                                                                wrapArrayConst(cv1CubeFaceIndices, 4),  
                                                                                nodes[cv2CubeFaceIndices[cv1Idx]], 
                                                                                &(cv2VxTouchCv1Edge[cv1Idx]), 
                                                                                tolerance);        
           if (cv2VxTouchCv1[cv1Idx]) ++numCv2VxesOnCv1;
       }
    } 

    // Handle case where one of the faces are completely covered by the other

    if (numCv1VxesOnCv2 >= 4)
    {
        int k;
        for (k = 0; k < 4; ++k)
        {
            polygon->push_back(cv1CubeFaceIndices[k]);
        }

        return true;
    }

    if (numCv2VxesOnCv1 >= 4)
    {
  
        int k;
        for (k = 3; k >= 0; --k) // Return opposite winding, to match winding of face 1
        {
            polygon->push_back(cv2CubeFaceIndices[k]);
        }
        return true;
    }

    // Handle partial coverage
    // Algorithm outline as follows:

    // Loop over edges in the face of Cv1. Intersect each one with all the edges of the Cv2 face.
    // Add first point of the cv1 edge to polygon if it really touches Cv2 ( touch of edge is considered as not touching)
    // Add each intersection point along the Cv1 edge if present
    // and finally: if the cv1 edge is going out of cv2, the add the cv2 vertexes from that intersection as long as they touch cv1.

    int nextCv1Idx = 1;
    for (cv1Idx = 0 ; cv1Idx < 4 ; ++cv1Idx, ++nextCv1Idx)
    {
        if (nextCv1Idx > 3) nextCv1Idx = 0;

        if (cv1VxTouchCv2[cv1Idx] && cv1VxTouchCv2Edge[cv1Idx] == -1) // Start of cv1 edge is touching inside the cv2 polygon (not on an cv2 edge)
        {
            if (polygon->empty() || polygon->back() != cv1CubeFaceIndices[cv1Idx])
            {
                polygon->push_back(cv1CubeFaceIndices[cv1Idx]);
            }

            if (cv1VxTouchCv2[nextCv1Idx] && cv1VxTouchCv2Edge[nextCv1Idx] == -1)
            {
                // Both ends of this cv1 edge is touching inside(not on an edge) cv2 polygon, no intersections possible (assuming convex cube-face)
                // Continue with next Cv1-edge.
                continue;
            }
        }

        // Find intersection(s) on this edge

        std::vector<IndexType>  intersectionVxIndices;
        std::vector<int>        intersectedCv2EdgeIdxs;
        std::vector<double>     intersectionFractionsAlongEdge;

        int nextCv2Idx = 1;
        for (cv2Idx = 0; cv2Idx < 4; ++cv2Idx, ++nextCv2Idx)
        {
            if (nextCv2Idx > 3) nextCv2Idx = 0;

            // Find a possible real intersection point. 

            cvf::Vec3d intersection(0,0,0);
            double fractionAlongEdge1;
            GeometryTools::IntersectionStatus intersectStatus = GeometryTools::NO_INTERSECTION;
            IndexType intersectionVxIndex = cvf::UNDEFINED_UINT;

            // First handle some "trivial" ones to ease the burden for the real intersection calculation
            // It could be tested whether it really is necessary to do 
            if (cv1VxTouchCv2Edge[cv1Idx] == cv2Idx && cv1VxTouchCv2Edge[nextCv1Idx] == cv2Idx )
            {
                intersectStatus = GeometryTools::LINES_OVERLAP;
                fractionAlongEdge1 = 1;
                intersectionVxIndex = cv1CubeFaceIndices[nextCv1Idx];
            }
            else if (cv1VxTouchCv2Edge[cv1Idx] == cv2Idx )
            {
                // When this happens, the cv1 vertex will already have been added to the polygon 
                // by the statements in the top of the cv1 edge loop. Should it be treated specially ?
                intersectStatus = GeometryTools::LINES_TOUCH;
                fractionAlongEdge1 = 0;
                intersectionVxIndex = cv1CubeFaceIndices[cv1Idx];
            }
            else if (cv1VxTouchCv2Edge[nextCv1Idx] == cv2Idx )
            {
                intersectStatus = GeometryTools::LINES_TOUCH;
                fractionAlongEdge1 = 1;
                intersectionVxIndex = cv1CubeFaceIndices[nextCv1Idx];
            } 
            else
            {
                double fractionAlongEdge2;
                bool found = false;
                if (edgeIntersectionStorage)
                    found = edgeIntersectionStorage->findIntersection( 
                                    cv1CubeFaceIndices[cv1Idx],     
                                    cv1CubeFaceIndices[nextCv1Idx], 
                                    cv2CubeFaceIndices[cv2Idx], 
                                    cv2CubeFaceIndices[nextCv2Idx], 
                                    &intersectionVxIndex, &intersectStatus, 
                                    &fractionAlongEdge1, &fractionAlongEdge2);

                if (!found)
                {

                     intersectStatus = GeometryTools::inPlaneLineIntersect3D(normal, 
                         nodes[cv1CubeFaceIndices[cv1Idx]], 
                         nodes[cv1CubeFaceIndices[nextCv1Idx]], 
                         nodes[cv2CubeFaceIndices[cv2Idx]], 
                         nodes[cv2CubeFaceIndices[nextCv2Idx]], 
                         &intersection, &fractionAlongEdge1, &fractionAlongEdge2,
                         tolerance);

                     switch (intersectStatus)
                     {
                     case GeometryTools::LINES_CROSSES:
                         {
                             intersectionVxIndex = newVertexIndex;
                             createdVertexes->push_back(intersection);
                             ++newVertexIndex;
                         }
                         break;
                     case GeometryTools::LINES_TOUCH:
                         {
                             if      (fractionAlongEdge1 <= 0.0) intersectionVxIndex = cv1CubeFaceIndices[cv1Idx];
                             else if (fractionAlongEdge1 >= 1.0) intersectionVxIndex = cv1CubeFaceIndices[nextCv1Idx];
                             else if (fractionAlongEdge2 <= 0.0) intersectionVxIndex = cv2CubeFaceIndices[cv2Idx];
                             else if (fractionAlongEdge2 >= 1.0) intersectionVxIndex = cv2CubeFaceIndices[nextCv2Idx];
                             else CVF_ASSERT(false); // Tolerance trouble 
                         }
                         break;
                     case GeometryTools::LINES_OVERLAP:
                         {
                             if      (fractionAlongEdge1 <= 0.0) intersectionVxIndex = cv1CubeFaceIndices[cv1Idx];
                             else if (fractionAlongEdge1 >= 1.0) intersectionVxIndex = cv1CubeFaceIndices[nextCv1Idx];
                             else if (fractionAlongEdge2 <= 0.0) intersectionVxIndex = cv2CubeFaceIndices[cv2Idx];
                             else if (fractionAlongEdge2 >= 1.0) intersectionVxIndex = cv2CubeFaceIndices[nextCv2Idx];
                             else CVF_ASSERT(false); // Tolerance trouble
                         }
                         break;
                     default:
                         break;
                     }

                     if(edgeIntersectionStorage)
                         edgeIntersectionStorage->addIntersection( cv1CubeFaceIndices[cv1Idx],     
                         cv1CubeFaceIndices[nextCv1Idx], 
                         cv2CubeFaceIndices[cv2Idx], 
                         cv2CubeFaceIndices[nextCv2Idx], 
                         intersectionVxIndex, intersectStatus, 
                         fractionAlongEdge1, fractionAlongEdge2);

                }
            }

            // Store data for each intersection along the Cv1-edge
            
            if (   (intersectStatus == GeometryTools::LINES_CROSSES) 
                || (intersectStatus == GeometryTools::LINES_TOUCH) 
                || (intersectStatus == GeometryTools::LINES_OVERLAP) ) 
            {
                CVF_ASSERT(intersectionVxIndex != cvf::UNDEFINED_UINT);

                intersectionFractionsAlongEdge.push_back(fractionAlongEdge1);
                intersectedCv2EdgeIdxs.push_back(cv2Idx);
                intersectionVxIndices.push_back(intersectionVxIndex);
            }
        }

        // Insert the intersections into the polygon in the correct order along the Cv1 edge.
        // Find the last intersection in order to possibly continue the polygon along Cv2 into Cv1

        size_t i;
        size_t lastIntersection = std::numeric_limits<size_t>::max();
        double largestFraction = -1;
        for (i = 0; i < intersectionFractionsAlongEdge.size(); ++i)
        {
            if (intersectionFractionsAlongEdge[i] > largestFraction)
            {
                lastIntersection = i;
                largestFraction = intersectionFractionsAlongEdge[i];
            }
        }

        // Insert indices to the new intersection vertices into the polygon of 
        // this connection according to fraction along edge

        std::map<double,IndexType> sortingMap;
        for (i = 0; i < intersectionFractionsAlongEdge.size(); ++i)
        {
            sortingMap[intersectionFractionsAlongEdge[i]] = intersectionVxIndices[i];
        }

        typename std::map<double, IndexType>::iterator it;
        for (it = sortingMap.begin(); it != sortingMap.end(); ++it)
        {
            if (polygon->empty() || polygon->back() != it->second) 
            {
                polygon->push_back(it->second);
            }
        }

        // Then if the Cv1 edge is going out of Cv2, add to the polygon, all the Cv2 face vertex-indices
        // from the intersection that touches Cv1. 

        // if cv1 edge in any way touches cv2 and ends up outside, it went out.

        /*
        if cv1 edge is going out of cv2 then
           if intersected cv2 edge has endpoint touching cv1 then 
              add endpoint to polygon. continue to add next endpoint until it does not touch Cv1
        */
        if ( !cv1VxTouchCv2[nextCv1Idx] 
             && ( cv1VxTouchCv2[cv1Idx] || ( intersectedCv2EdgeIdxs.size() ) ) ) // Two touches along edge also qualifies  
        {
            if(lastIntersection < intersectedCv2EdgeIdxs.size())
            {
                cv2Idx = intersectedCv2EdgeIdxs[lastIntersection];
                int count = 0;
                // Continue the polygon along the Cv2 edges as long as they touch cv1.
                // Depending on the faces having opposite winding, which is guaranteed as long as
                // no intersecting CVs share a connection  
                while (cv2VxTouchCv1[cv2Idx] && count < 4 && (cv2VxTouchCv1Edge[cv2Idx] == -1)) // Touch of edge is regarded as being outside, so we must stop
                {
                    if (polygon->empty() || polygon->back() != cv2CubeFaceIndices[cv2Idx]) 
                    {
                        polygon->push_back(cv2CubeFaceIndices[cv2Idx]);
                    }
                    --cv2Idx;
                    if (cv2Idx < 0 ) cv2Idx = 3;
                    ++count;
                }
            }
            else
            {
                CVF_ASSERT(lastIntersection < intersectedCv2EdgeIdxs.size());
            }
        }
    }

    if (polygon->size() > 2)
    {
        if (polygon->back() == polygon->front()) polygon->pop_back();
    }

    // Sanity checks
    if (polygon->size() < 3) 
    {
       // cvf::Trace::show(cvf::String("Degenerated connection polygon detected. (Less than 3 vertexes) Cv's probably not in contact: %1 , %2").arg(m_ownerCvId).arg(m_neighborCvId));
        polygon->clear();

        return false;
    }
  
  
    return true;    
}


//--------------------------------------------------------------------------------------------------
/// This method assumes that all intersection and mid edge vertexes are created an are already 
/// merged into all the polygons. We can also assume that all the connection polygons are completely
/// inside (or sharing edges with) the cube face polygon initially
//--------------------------------------------------------------------------------------------------
#define DEBUG_PRINT 0

//template <typename NodeArrayType, typename NodeType, typename IndicesArrayType, typename IndicesType>
//void setup( ArrayWrapper<NodeArrayType, NodeType> nodeArray,  ArrayWrapper<IndicesArrayType, IndicesType> indices) 
template<typename VerticeArrayType,  typename PolygonArrayType, typename IndexType>
void GeometryTools::calculatePartiallyFreeCubeFacePolygon(ArrayWrapperConst<VerticeArrayType, cvf::Vec3d>       nodeCoords, 
    ArrayWrapperConst<PolygonArrayType, IndexType>   completeFacePolygon, 
    const cvf::Vec3d&            faceNormal, 
    const std::vector< std::vector<IndexType>* >& faceOverlapPolygons, 
    const std::vector<bool>&      faceOverlapPolygonWindingSameAsCubeFaceFlags,
    std::vector<IndexType>*         partialFacePolygon, 
    bool*                        m_partiallyFreeCubeFaceHasHoles)
{
    // Vertex Index to position in polygon
    typedef std::map< IndexType, typename std::vector<IndexType>::const_iterator > VxIdxToPolygonPositionMap;

    CVF_ASSERT(m_partiallyFreeCubeFaceHasHoles);
    CVF_ASSERT(partialFacePolygon != NULL);

    // Copy the start polygon
    std::list<IndexType> resultPolygon; 
    for (size_t pcIdx = 0; pcIdx < completeFacePolygon.size(); ++pcIdx)
    {
        resultPolygon.push_back(completeFacePolygon[pcIdx]);
    }

    // First build search maps to fast find whether and where an index is positioned in a polygon
    // Map from Vertex-index to position in polygon

    std::vector< VxIdxToPolygonPositionMap > polygonSearchMaps;
    std::vector<bool> isConnectionPolygonMerged;

    polygonSearchMaps.resize(faceOverlapPolygons.size());
    isConnectionPolygonMerged.resize(faceOverlapPolygons.size(), false);

    // Build search maps
    {
        size_t count;
        for (size_t i = 0; i < faceOverlapPolygons.size(); ++i)
        {
            count = 0;
            for (typename std::vector<IndexType >::const_iterator pcIt = faceOverlapPolygons[i]->begin(); 
                pcIt !=  faceOverlapPolygons[i]->end(); 
                ++pcIt)
            {
                polygonSearchMaps[i][*pcIt] = pcIt;
                ++count;
            }

            if (count < 3) isConnectionPolygonMerged[i] = true; // Ignore false polygons
        }
    }

#if DEBUG_PRINT
    {
        cvf::Trace::show("Circumference polygon: ");
        std::list<IndexType>::const_iterator polIt;
        for ( polIt = resultPolygon.begin(); polIt != resultPolygon.end(); ++polIt)
        {
            cvf::Trace::show(cvf::String("%1 \t%2 %3 %4").arg((int)(*polIt))
                .arg(nodeCoords[*polIt].x())
                .arg(nodeCoords[*polIt].y())
                .arg(nodeCoords[*polIt].z()));
        }
    }
#endif

#if DEBUG_PRINT
    {
        cvf::Trace::show("Connection polygons: ");
        for (size_t cIdx = 0; cIdx < faceOverlapPolygons.size(); cIdx++)
        {
            std::vector<IndexType >::const_iterator polIt;
            cvf::Trace::show("Connection " + cvf::String((long long)cIdx));
            for (polIt = faceOverlapPolygons[cIdx]->begin(); polIt !=  faceOverlapPolygons[cIdx]->end(); ++polIt)
            {
                cvf::Trace::show(cvf::String("%1 \t%2 %3 %4").arg((int)(*polIt))
                    .arg(nodeCoords[*polIt].x())
                    .arg(nodeCoords[*polIt].y())
                    .arg(nodeCoords[*polIt].z()));
            }
        }
    }
#endif

    // Merge connection polygons with the main polygon as long as one of them has something in common.

    // For each vx in the m_freeFacePolygons[cubeFace] polygon .
    //    loop over all connections
    //       if it has the node in common and that the edge angle will decrease if inserting
    //          merge the connection polygon into the main polygon, 
    //          and remove the connection polygon from the merge able connection polygons.


    for (typename std::list<IndexType>::iterator pIt =  resultPolygon.begin(); pIt != resultPolygon.end(); ++pIt)
    {
        // Set iterator to previous node in polygon
        typename std::list<IndexType>::iterator prevPIt = pIt;
        if (prevPIt == resultPolygon.begin()) prevPIt = resultPolygon.end();
        --prevPIt;

        cvf::Vec3d pToPrev = nodeCoords[*prevPIt] - nodeCoords[*pIt];

        // Set iterator to next node in polygon. Used to insert before and as pointer to the next point
        typename std::list<IndexType>::iterator nextPIt = pIt;
        ++nextPIt;
        typename std::list<IndexType>::iterator insertBeforePIt = nextPIt;
        if (nextPIt == resultPolygon.end()) nextPIt = resultPolygon.begin();

        // Calculate existing edge to edge angle

        cvf::Vec3d pToNext = nodeCoords[*nextPIt] - nodeCoords[*pIt];
        double mainPolygonEdgeAngle = GeometryTools::getAngle(faceNormal, pToNext , pToPrev);

        // Find connections containing the pIt vertex index. Merge them into the main polygon

        for (size_t opIdx = 0; opIdx < faceOverlapPolygons.size(); ++opIdx)
        {
            if (isConnectionPolygonMerged[opIdx]) continue; // Already merged

            // Find position of pIt vertex index in the current connection polygon
            typename VxIdxToPolygonPositionMap::iterator vxIndexPositionInPolygonIt = polygonSearchMaps[opIdx].find(*pIt);

            if (vxIndexPositionInPolygonIt != polygonSearchMaps[opIdx].end())
            {
                // Merge the connection polygon into the main polygon 
                // if the angle prevPIt pIt nextPIt is larger than angle prevPIt pIt (startCPIt++)

                typename std::vector<IndexType>::const_iterator startCPIt;
                startCPIt = vxIndexPositionInPolygonIt->second;

                // First vx to insert is the one after the match

                bool hasSameWinding = faceOverlapPolygonWindingSameAsCubeFaceFlags[opIdx];
                if (hasSameWinding)
                {
                    // Same winding as main polygon. We need to go the opposite way
                    if (startCPIt == faceOverlapPolygons[opIdx]->begin()) startCPIt = faceOverlapPolygons[opIdx]->end();
                    --startCPIt;
                }
                else
                {
                    // Opposite winding. Go forward when merging
                    ++startCPIt; if (startCPIt == faceOverlapPolygons[opIdx]->end()) startCPIt = faceOverlapPolygons[opIdx]->begin();
                }

                // Calculate possible new edge-to-edge angle and test against existing angle
                cvf::Vec3d pToStart = nodeCoords[*startCPIt] - nodeCoords[*pIt];
                double candidatePolygonEdgeAngle = GeometryTools::getAngle(faceNormal, pToStart , pToPrev);

                if (candidatePolygonEdgeAngle < mainPolygonEdgeAngle )
                {
                    // Merge ok
                    typename std::vector<IndexType >::const_iterator pcIt = startCPIt;
                    if (hasSameWinding)
                    {
                        do 
                        {
                            resultPolygon.insert(insertBeforePIt, (*pcIt));

                            if (pcIt == faceOverlapPolygons[opIdx]->begin()) pcIt = faceOverlapPolygons[opIdx]->end();
                            --pcIt;

                        } while (pcIt != startCPIt);
                    }
                    else
                    {
                        do
                        {
                            resultPolygon.insert(insertBeforePIt, (*pcIt));

                            ++pcIt; 
                            if (pcIt == faceOverlapPolygons[opIdx]->end()) pcIt = faceOverlapPolygons[opIdx]->begin();

                        } while (pcIt != startCPIt);
                    }

                    isConnectionPolygonMerged[opIdx] = true;

                    // Recalculate the next position to point into the new nodes
                    // Set iterator in the main polygon to insert before and to the next point
                    nextPIt = pIt;
                    ++nextPIt;
                    insertBeforePIt = nextPIt;
                    if (nextPIt == resultPolygon.end()) nextPIt = resultPolygon.begin();

                    // Recalculate the existing edge to edge angle
                    pToNext = nodeCoords[*nextPIt] - nodeCoords[*pIt];
                    mainPolygonEdgeAngle = GeometryTools::getAngle(faceNormal, pToNext , pToPrev);
                }
            }
        }
    }

    // Now remove all double edges

    bool goneAround = false;
    for ( typename std::list<IndexType>::iterator pIt = resultPolygon.begin(); pIt != resultPolygon.end() && !goneAround; ++pIt)
    {
        // Set iterator to next node in polygon.
        typename std::list<IndexType>::iterator  nextPIt = pIt;
        ++nextPIt;
        if (nextPIt == resultPolygon.end()) 
        { 
            nextPIt = resultPolygon.begin();
            goneAround = true; // Gone around polygon. Stop even if pIt is jumping over end()
        }

        // Set iterator to previous node in polygon

        typename std::list<IndexType>::iterator prevPIt = pIt;

        if (prevPIt == resultPolygon.begin()) prevPIt = resultPolygon.end();
        --prevPIt;

        // If previous and next node are the same, erase 
        while(*nextPIt == *prevPIt) 
        {
            resultPolygon.erase(pIt);
            resultPolygon.erase(prevPIt);

            if ( resultPolygon.begin() ==  resultPolygon.end()) break;  // Polygon has been completely removed. Nothing left. Break out of while 

            pIt = nextPIt;
            ++nextPIt;
            if (nextPIt == resultPolygon.end())
            { 
                nextPIt = resultPolygon.begin();
                goneAround = true; // Gone around polygon pIt is jumping over end()
            }

            prevPIt = pIt;
            if (prevPIt == resultPolygon.begin()) prevPIt = resultPolygon.end();
            --prevPIt;
        }

        if ( resultPolygon.begin() ==  resultPolygon.end()) break; // Polygon has been completely removed. Nothing left. Break out of for loop

    }

    // Check for holes 

    bool hasHoles = false;
    for (size_t i = 0; i < isConnectionPolygonMerged.size(); ++i)
    {
        hasHoles = !isConnectionPolygonMerged[i];
        if(hasHoles)
        {
            *m_partiallyFreeCubeFaceHasHoles = true;
            break;
        }
    }

#if DEBUG_PRINT
    {
        cvf::Trace::show("Polygon: ");
        for (std::list<IndexType>::iterator pIt = resultPolygon.begin(); pIt != resultPolygon.end(); ++pIt)
        {
            cvf::Trace::show(cvf::String("%1 \t%2 %3 %4").arg((int)(*pIt))
                .arg(nodeCoords[*pIt].x())
                .arg(nodeCoords[*pIt].y())
                .arg(nodeCoords[*pIt].z()));
        }
    }
#endif

    // Copy the result polygon to the output variable 

    partialFacePolygon->clear();
    for (typename std::list<IndexType>::iterator pIt = resultPolygon.begin(); pIt != resultPolygon.end(); ++pIt)
    {
        partialFacePolygon->push_back(*pIt);
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename IndexType>
void EdgeIntersectStorage<IndexType>::setVertexCount(size_t size)
{
    m_edgeIntsectMap.resize(size);
}

template <typename IndexType>
void EdgeIntersectStorage<IndexType>::canonizeAddress(IndexType& e1P1, IndexType& e1P2, IndexType& e2P1, IndexType& e2P2,
                                           bool& flipE1, bool& flipE2, bool& flipE1E2)
{
    flipE1 = e1P1 > e1P2;
    flipE2 = e2P1 > e2P2;

    flipE1E2 = (flipE1 ? e1P2: e1P1) > (flipE2 ? e2P2: e2P1);

    static IndexType temp;
    if (flipE1)
    {
        temp = e1P1;
        e1P1 = e1P2;
        e1P2 = temp;
    }

    if (flipE2)
    {
        temp = e2P1;
        e2P1 = e2P2;
        e2P2 = temp;
    }

    if (flipE1E2)
    {
        temp = e1P1;
        e1P1 = e2P1;
        e2P1 = temp;

        temp = e1P2;
        e1P2 = e2P2;
        e2P2 = temp;
    }
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename IndexType>
void EdgeIntersectStorage<IndexType>::addIntersection(IndexType e1P1, IndexType e1P2, IndexType e2P1, IndexType e2P2, 
                                           IndexType vxIndexIntersectionPoint, GeometryTools::IntersectionStatus intersectionStatus, 
                                           double fractionAlongEdge1, double fractionAlongEdge2)
{
    static bool flipE1  ; 
    static bool flipE2  ;
    static bool flipE1E2;

    canonizeAddress(e1P1, e1P2, e2P1, e2P2, flipE1, flipE2, flipE1E2);

    static IntersectData iData;

    iData.fractionAlongEdge1 = flipE1 ? 1 - fractionAlongEdge1 : fractionAlongEdge1;
    iData.fractionAlongEdge2 = flipE2 ? 1 - fractionAlongEdge2 : fractionAlongEdge2;
    iData.intersectionStatus = intersectionStatus;

    if (flipE1E2)
    {
        double temp = iData.fractionAlongEdge1;
        iData.fractionAlongEdge1 = iData.fractionAlongEdge2;
        iData.fractionAlongEdge2 = temp;
    }

    iData.intersectionPointIndex = vxIndexIntersectionPoint;
    CVF_ASSERT(e1P1 < m_edgeIntsectMap.size());
    m_edgeIntsectMap[e1P1][e1P2][e2P1][e2P2] = iData;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename IndexType>
bool EdgeIntersectStorage<IndexType>::findIntersection(IndexType e1P1, IndexType e1P2, IndexType e2P1, IndexType e2P2, 
                                            IndexType* vxIndexIntersectionPoint, GeometryTools::IntersectionStatus* intersectionStatus, 
                                            double* fractionAlongEdge1, double* fractionAlongEdge2)
{
    static bool flipE1  ; 
    static bool flipE2  ;
    static bool flipE1E2;

    canonizeAddress(e1P1, e1P2, e2P1, e2P2, flipE1, flipE2, flipE1E2);

    if (!m_edgeIntsectMap[e1P1].size()) return false;

    typename std::map<IndexType, std::map<IndexType, std::map<IndexType, IntersectData > > >::iterator it;
    it = m_edgeIntsectMap[e1P1].find(e1P2);
    if (it == m_edgeIntsectMap[e1P1].end()) return false;

    typename std::map<IndexType, std::map<IndexType, IntersectData > >::iterator it2;
    it2 = it->second.find(e2P1);
    if (it2 == it->second.end()) return false;

    typename std::map<IndexType, IntersectData >::iterator it3;
    it3 = it2->second.find(e2P2);
    if (it3 == it2->second.end()) return false;

    *vxIndexIntersectionPoint = it3->second.intersectionPointIndex; 
    *intersectionStatus = it3->second.intersectionStatus;

    if (flipE1E2)
    {
        *fractionAlongEdge1 = it3->second.fractionAlongEdge2;
        *fractionAlongEdge2 = it3->second.fractionAlongEdge1;
    }
    else
    {
        *fractionAlongEdge1 = it3->second.fractionAlongEdge1;
        *fractionAlongEdge2 = it3->second.fractionAlongEdge2;
    }

    if (flipE1) *fractionAlongEdge1 = 1 - *fractionAlongEdge1;
    if (flipE2) *fractionAlongEdge2 = 1 - *fractionAlongEdge2;

    return true;
}



}
