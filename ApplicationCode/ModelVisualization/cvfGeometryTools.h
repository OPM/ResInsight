#pragma once
#include "cvfBase.h"
#include "cvfArray.h"
#include <list>
#include <map>
#include <hash_map>

namespace cvf
{


class EdgeSplitStorage;
class EdgeIntersectStorage;

class GeometryTools
{
public:
    static cvf::Vec3d computeFaceCenter(const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3);

    static double     linePointSquareDist(const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3);
    static int        intersectLineSegmentTriangle( const cvf::Vec3d p0, const cvf::Vec3d p1, 
                                                    const cvf::Vec3d t0, const cvf::Vec3d t1, const cvf::Vec3d t2,
                                                    cvf::Vec3d* intersectionPoint );
    static cvf::Vec3d barycentricCoords(const cvf::Vec3d&  t0, const cvf::Vec3d&  t1, const cvf::Vec3d&  t2, const cvf::Vec3d&  p);

    static int        findClosestAxis(const cvf::Vec3d& vec );
    static double     getAngle(const cvf::Vec3d& positiveNormalAxis, const cvf::Vec3d& v1, const cvf::Vec3d& v2);
    static double     getAngle(const cvf::Vec3d& v1, const cvf::Vec3d& v2);

    enum IntersectionStatus
    {
        NO_INTERSECTION,
        LINES_INTERSECT_OUTSIDE,
        LINES_TOUCH,
        LINES_CROSSES,
        LINES_OVERLAP
    };

    static bool insertVertexInPolygon(std::list<std::pair<cvf::uint, bool> >* polygon, const cvf::Vec3dArray& nodeCoords, cvf::uint vertexIndex, double tolerance);
    static void addMidEdgeNodes(std::list<std::pair<cvf::uint, bool> >* polygon, const cvf::Vec3dArray& nodes, EdgeSplitStorage& edgeSplitStorage, std::vector<cvf::Vec3d>* createdVertexes);

    static IntersectionStatus inPlaneLineIntersect3D(const cvf::Vec3d& planeNormal, 
                                                     const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3, const cvf::Vec3d& p4, 
                                                     cvf::Vec3d* intersectionPoint, double* fractionAlongLine1, double* fractionAlongLine2, 
                                                     double tolerance = 1e-6);

    //template<typename VerticeArrayType, typename PolygonArrayType, typename IndexType>
    static bool isPointTouchingIndexedPolygon(const cvf::Vec3d& polygonNormal, const cvf::Vec3d* vertices, const size_t* indices, size_t numIndices, 
                                              const cvf::Vec3d& point, int* touchedEdgeIndex,  double tolerance = 1e-6);



    static bool calculateOverlapPolygonOfTwoQuads(    std::vector<size_t> * polygon, std::vector<cvf::Vec3d>* createdVertexes, 
                                                      EdgeIntersectStorage& edgeIntersectionStorage, 
                                                      const cvf::Vec3dArray& nodes, 
                                                      const size_t cv1CubeFaceIndices[4], 
                                                      const size_t cv2CubeFaceIndices[4],
                                                      double tolerance);

    static void calculatePartiallyFreeCubeFacePolygon(const cvf::Vec3dArray&       nodeCoords, 
                                                      const std::vector<size_t>*   completeFacePolygon, 
                                                      const cvf::Vec3d&            faceNormal, 
                                                      const std::vector< std::vector<size_t>* >& faceOverlapPolygons, 
                                                      const std::vector<bool>      faceOverlapPolygonWindingSameAsCubeFaceFlags,
                                                      std::vector<size_t>*         partialFacePolygon, 
                                                      bool*                        m_partiallyFreeCubeFaceHasHoles);
};


class EdgeIntersectStorage
{
public:
    void setVertexCount(size_t size);
    bool findIntersection(  size_t e1P1, size_t e1P2, size_t e2P1, size_t e2P2, 
                            size_t* vxIndexIntersectionPoint, GeometryTools::IntersectionStatus* intersectionStatus, 
                            double* fractionAlongEdge1, double* fractionAlongEdge2);
    void addIntersection(   size_t e1P1, size_t e1P2, size_t e2P1, size_t e2P2, 
                            size_t vxIndexIntersectionPoint, GeometryTools::IntersectionStatus intersectionStatus, 
                            double fractionAlongEdge1, double fractionAlongEdge2);

private:
    struct IntersectData
    {
        size_t intersectionPointIndex;
        GeometryTools::IntersectionStatus intersectionStatus;
        double fractionAlongEdge1;
        double fractionAlongEdge2;
    };

    void canonizeAddress(size_t& e1P1, size_t& e1P2, size_t& e2P1, size_t& e2P2, bool& flipE1, bool& flipE2, bool& flipE1E2);

    // A map containing the intersection data. The addressing is :
    // ( when leastVxIdxEdge1 < leastVxIdxEdge2 )
    // leastVxIdxEdge1, largestVxIdxEdge1, leastVxIdxEdge2, largestVxIdxEdge2, { vxIdxIntersection, fractionAlongEdg1, fractionAlonEdge2 }

    std::vector< std::map<size_t, std::map<size_t, std::map<size_t, IntersectData > > > > m_edgeIntsectMap;
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

    virtual bool calculateTriangles(std::vector<cvf::uint>* triangles);

protected:
    bool    isTriangleValid( std::list<size_t>::const_iterator u, std::list<size_t>::const_iterator v, std::list<size_t>::const_iterator w) const;
    bool    isPointInsideTriangle(const cvf::Vec3d& A, const cvf::Vec3d& B, const cvf::Vec3d& C, const cvf::Vec3d& P) const;
    double  calculatePolygonArea() const;

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
    void setCenterNode(size_t centerNodeIndex );

    virtual bool calculateTriangles(std::vector<cvf::uint>* triangles);
private:
    bool isTriangleValid( size_t u, size_t v, size_t w);
    size_t m_centerNodeIndex;
};

}