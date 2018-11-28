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

#pragma once
#include "cvfBase.h"
#include "cvfArray.h"
#include <list>
#include <map>
#include "cvfArrayWrapperConst.h"
#include "cvfMatrix3.h"

namespace cvf
{


class EdgeSplitStorage;
template <typename IndexType> class EdgeIntersectStorage;

class GeometryTools
{
public:
    static cvf::Vec3d computeFaceCenter(const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3);
    static cvf::Mat3f computePlaneHorizontalRotationMx(const cvf::Vec3f& inPlaneVec0, const cvf::Vec3f& inPlaneVec1);

    static cvf::Vec3d projectPointOnLine(const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3, double* normalizedIntersection);

    static double     linePointSquareDist(const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3);
    static int        intersectLineSegmentTriangle( const cvf::Vec3d p0, const cvf::Vec3d p1, 
                                                    const cvf::Vec3d t0, const cvf::Vec3d t1, const cvf::Vec3d t2,
                                                    cvf::Vec3d* intersectionPoint, 
                                                    bool * isLineDirDotNormalNegative);
    static cvf::Vec3d barycentricCoords(const cvf::Vec3d&  t0, const cvf::Vec3d&  t1, const cvf::Vec3d&  t2, const cvf::Vec3d&  p);
    static cvf::Vec4d barycentricCoords(const cvf::Vec3d&  v0, const cvf::Vec3d&  v1, const cvf::Vec3d&  v2, const cvf::Vec3d& v3, const cvf::Vec3d&  p);
    template<typename DataType>
    static DataType interpolateQuad(const cvf::Vec3d& v1, DataType s1,
                                    const cvf::Vec3d& v2, DataType s2,
                                    const cvf::Vec3d& v3, DataType s3,
                                    const cvf::Vec3d& v4, DataType s4,
                                    const cvf::Vec3d& point);

    static int        findClosestAxis(const cvf::Vec3d& vec );
    static double     getAngle(const cvf::Vec3d& positiveNormalAxis, const cvf::Vec3d& v1, const cvf::Vec3d& v2);
    static double     getAngle(const cvf::Vec3d& v1, const cvf::Vec3d& v2);

    static cvf::Vec3d polygonAreaNormal3D(const std::vector<cvf::Vec3d>& polygon);

    enum IntersectionStatus
    {
        NO_INTERSECTION,
        LINES_INTERSECT_OUTSIDE,
        LINES_TOUCH,
        LINES_CROSSES,
        LINES_OVERLAP
    };

    static void addMidEdgeNodes(std::list<std::pair<cvf::uint, bool> >* polygon, const cvf::Vec3dArray& nodes, EdgeSplitStorage& edgeSplitStorage, std::vector<cvf::Vec3d>* createdVertexes);

     template<typename VerticeArrayType,  typename IndexType>
    static bool insertVertexInPolygon(               std::vector<IndexType> * polygon, 
                                                     ArrayWrapperConst<VerticeArrayType, cvf::Vec3d> nodeCoords, 
                                                     IndexType vertexIndex, 
                                                     double tolerance);

    static IntersectionStatus inPlaneLineIntersect3D(const cvf::Vec3d& planeNormal, 
                                                     const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3, const cvf::Vec3d& p4, 
                                                     cvf::Vec3d* intersectionPoint, double* fractionAlongLine1, double* fractionAlongLine2, 
                                                     double tolerance = 1e-6);

    template<typename VerticeArrayType, typename PolygonArrayType, typename IndexType>
    static bool isPointTouchingIndexedPolygon(        const cvf::Vec3d& polygonNormal, 
                                                      ArrayWrapperConst<VerticeArrayType, cvf::Vec3d> vertices, 
                                                      ArrayWrapperConst<PolygonArrayType, IndexType> indices, 
                                                      const cvf::Vec3d& point, 
                                                      int* touchedEdgeIndex,  
                                                      double tolerance = 1e-6);


    template<typename VerticeArrayType,  typename IndexType>
    static bool calculateOverlapPolygonOfTwoQuads(    std::vector<IndexType> * polygon, 
                                                      std::vector<cvf::Vec3d>* createdVertexes, 
                                                      EdgeIntersectStorage<IndexType>* edgeIntersectionStorage, 
                                                      ArrayWrapperConst<VerticeArrayType, cvf::Vec3d> nodes, 
                                                      const IndexType cv1CubeFaceIndices[4], 
                                                      const IndexType cv2CubeFaceIndices[4],
                                                      double tolerance);

    template<typename VerticeArrayType,  typename PolygonArrayType, typename IndexType>
    static void calculatePartiallyFreeCubeFacePolygon(ArrayWrapperConst<VerticeArrayType, cvf::Vec3d>  nodeCoords, 
                                                      ArrayWrapperConst<PolygonArrayType, IndexType>   completeFacePolygon, 
                                                      const cvf::Vec3d&            faceNormal, 
                                                      const std::vector< std::vector<IndexType>* >& faceOverlapPolygons, 
                                                      const std::vector<bool>&      faceOverlapPolygonWindingSameAsCubeFaceFlags,
                                                      std::vector<IndexType>*         partialFacePolygon, 
                                                      bool*                        m_partiallyFreeCubeFaceHasHoles);
};

template <typename IndexType>
class EdgeIntersectStorage
{
public:
    void setVertexCount(size_t size);
    bool findIntersection(  IndexType e1P1, IndexType e1P2, IndexType e2P1, IndexType e2P2, 
                            IndexType* vxIndexIntersectionPoint, GeometryTools::IntersectionStatus* intersectionStatus, 
                            double* fractionAlongEdge1, double* fractionAlongEdge2);
    void addIntersection(   IndexType e1P1, IndexType e1P2, IndexType e2P1, IndexType e2P2, 
                            IndexType vxIndexIntersectionPoint, GeometryTools::IntersectionStatus intersectionStatus, 
                            double fractionAlongEdge1, double fractionAlongEdge2);

private:
    struct IntersectData
    {
        IndexType intersectionPointIndex;
        GeometryTools::IntersectionStatus intersectionStatus;
        double fractionAlongEdge1;
        double fractionAlongEdge2;
    };

    void canonizeAddress(IndexType& e1P1, IndexType& e1P2, IndexType& e2P1, IndexType& e2P2, bool& flipE1, bool& flipE2, bool& flipE1E2);

    // A map containing the intersection data. The addressing is :
    // ( when leastVxIdxEdge1 < leastVxIdxEdge2 )
    // leastVxIdxEdge1, largestVxIdxEdge1, leastVxIdxEdge2, largestVxIdxEdge2, { vxIdxIntersection, fractionAlongEdg1, fractionAlonEdge2 }

    std::vector< std::map<IndexType, std::map<IndexType, std::map<IndexType, IntersectData > > > > m_edgeIntsectMap;
};

class EdgeSplitStorage
{
public:
    void setVertexCount(size_t size);
    bool findSplitPoint(size_t edgeP1Index, size_t edgeP2Index, size_t* splitPointIndex);
    void addSplitPoint(size_t edgeP1Index, size_t edgeP2Index, size_t splitPointIndex);

private:
    void canonizeAddress(size_t& e1P1, size_t& e1P2);

    // Least VxIdx, LargestVxIdx, VertexIdx of splitpoint
    std::vector< std::map< size_t, size_t > > m_edgeSplitMap;
};


class EarClipTesselator
{
public:
    EarClipTesselator();
    void setNormal(const cvf::Vec3d& polygonNormal );
    void setMinTriangleArea(double areaTolerance);
    void setGlobalNodeArray(const cvf::Vec3dArray& nodeCoords);

    void setPolygonIndices(const std::list<size_t>& polygon);
    void setPolygonIndices(const std::vector<size_t>& polygon);

    virtual bool calculateTriangles(std::vector<size_t>* triangles);

protected:
    bool    isTriangleValid( std::list<size_t>::const_iterator u, std::list<size_t>::const_iterator v, std::list<size_t>::const_iterator w) const;
    bool    isPointInsideTriangle(const cvf::Vec3d& A, const cvf::Vec3d& B, const cvf::Vec3d& C, const cvf::Vec3d& P) const;
    double  calculateProjectedPolygonArea() const;

protected:
    std::list<size_t>       m_polygonIndices;
    const cvf::Vec3dArray*  m_nodeCoords;
    int                     m_X, m_Y; // Index shift in vector to do simple 2D projection
    cvf::Vec3d              m_polygonNormal;
    double                  m_areaTolerance;

};


class FanEarClipTesselator : public EarClipTesselator
{
public:
    FanEarClipTesselator();

    bool calculateTriangles(std::vector<size_t>* triangles) override;
private:
    bool isTriangleValid( size_t u, size_t v, size_t w);
    size_t m_centerNodeIndex;
};

}

#include "cvfGeometryTools.inl"
