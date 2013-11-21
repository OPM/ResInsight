#include "cvfGeometryTools.h"

#pragma warning (disable : 4503)
namespace cvf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d GeometryTools::computeFaceCenter(const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3)
{
    cvf::Vec3d centerCoord = v0;
    centerCoord += v1;
    centerCoord += v2;
    centerCoord += v3;
    centerCoord *= 0.25;

    return centerCoord;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

int GeometryTools::findClosestAxis(const cvf::Vec3d& vec )
{
    int closestAxis = 0;
    double maxComponent = fabs(vec.x());

    if (fabs(vec.y()) > maxComponent)
    {
        maxComponent = (float)fabs(vec.y());
        closestAxis = 1;
    }

    if (fabs(vec.z()) > maxComponent)
    {
        closestAxis = 2;
    }

    return closestAxis;
}

//--------------------------------------------------------------------------------------------------
/// Return angle between vectors if v1 x v2 is same way as normal
/// else return 2PI - angle
/// This means if the angle is slightly "negative", using the right hand rule, this method will return
/// nearly 2*PI
//--------------------------------------------------------------------------------------------------

double const MY_PI = 4 * atan(1.0);

double GeometryTools::getAngle(const cvf::Vec3d& positiveNormalAxis, const cvf::Vec3d& v1, const cvf::Vec3d& v2)
{
    bool isOk = false;
    cvf::Vec3d v1N = v1.getNormalized(&isOk);
    if (!isOk) return 0;
    cvf::Vec3d v2N = v2.getNormalized();
    if (!isOk) return 0;

    double cosAng = v1N * v2N;
    // Guard acos against out-of-domain input
    if (cosAng <= -1.0) 
    {
        cosAng = -1.0; 
    }
    else if (cosAng >= 1.0)
    {
        cosAng = 1.0; 
    }
    double angle = acos(cosAng);

    cvf::Vec3d crossProd = v1N ^ v2N;
    double sign = positiveNormalAxis * crossProd;
    if (sign < 0)
    {
        angle = 2*MY_PI - angle;
    }
    return angle;
}

//--------------------------------------------------------------------------------------------------
/// Return angle in radians between vectors [0, Pi]
/// If v1 or v2 is zero, the method will return 0.
//--------------------------------------------------------------------------------------------------

double GeometryTools::getAngle(const cvf::Vec3d& v1, const cvf::Vec3d& v2)
{
    bool isOk = false;
    cvf::Vec3d v1N = v1.getNormalized(&isOk);
    if (!isOk) return 0;
    cvf::Vec3d v2N = v2.getNormalized();
    if (!isOk) return 0;

    double cosAng = v1N * v2N;
    // Guard acos against out-of-domain input
    if (cosAng <= -1.0) 
    {
        cosAng = -1.0; 
    }
    else if (cosAng >= 1.0)
    {
        cosAng = 1.0; 
    }
    double angle = acos(cosAng);

    return angle;
}

/*
   Determine the intersection point of two line segments  
   From Paul Bourke, but modified to really handle coincident lines
   and lines with touching vertexes.
   Returns an intersection status telling what kind of intersection it is (if any)
   */

GeometryTools::IntersectionStatus inPlaneLineIntersect(
    double x1, double y1,
    double x2, double y2,
    double x3, double y3,
    double x4, double y4,
    double l1NormalizedTolerance, double l2NormalizedTolerance,
    double *x, double *y, double* fractionAlongLine1, double* fractionAlongLine2)
{
   double mua, mub;
   double denom, numera, numerb;

   denom  = (y4-y3) * (x2-x1) - (x4-x3) * (y2-y1);
   numera = (x4-x3) * (y1-y3) - (y4-y3) * (x1-x3);
   numerb = (x2-x1) * (y1-y3) - (y2-y1) * (x1-x3);

    double EPS = 1e-40;

   // Are the line coincident? 
   if (fabs(numera) < EPS && fabs(numerb) < EPS && fabs(denom) < EPS) 
   {
#if 0
       *x = 0;
       *y = 0;
       *fractionAlongLine1 = 0;
       *fractionAlongLine2 = 0;
       return GeometryTools::LINES_OVERLAP;
#else
       cvf::Vec3d p12(x2-x1, y2-y1, 0);
       cvf::Vec3d p13(x3-x1, y3-y1, 0);
       cvf::Vec3d p34(x4-x3, y4-y3, 0);

       double length12 = p12.length();
       double length34 = p34.length();

       // Check if the p1 p2 line is a point
      
       if (length12 < EPS )
       {
           cvf::Vec3d p34(x4-x3, y4-y3, 0);
           *x = x1;
           *y = y1;
           *fractionAlongLine1 = 1;
           *fractionAlongLine2 = p13.length()/p34.length();
           return GeometryTools::LINES_OVERLAP;
       }

       cvf::Vec3d p14(x4-x1, y4-y1, 0);
       cvf::Vec3d p32(x2-x3, y2-y3, 0);

       cvf::Vec3d e12 = p12.getNormalized(); 
       double normDist13 =  e12*p13 / length12;
       double normDist14 =  e12*p14 / length12;

       // Check if both points on the p3 p4 line is outside line p1 p2.
       if( (normDist13 < 0 - l1NormalizedTolerance && normDist14 < 0-l1NormalizedTolerance )|| (normDist13 > 1 +l1NormalizedTolerance  && normDist14 > 1+l1NormalizedTolerance ) ) 
      {
           *x = 0;
           *y = 0;
           *fractionAlongLine1 = 0;
           *fractionAlongLine2 = 0;
          return GeometryTools::NO_INTERSECTION;
       }

       double normDist32 =  e12*p32 / length34;
       double normDist31 = -e12*p13 / length34;

       // Set up fractions along lines to the edge2 vertex actually touching edge 1.
       /// if two, select the one furthest from the start
       bool pt3IsInside = false;
       bool pt4IsInside = false;
       if ((0.0 - l1NormalizedTolerance) <= normDist13 && normDist13 <= (1.0 +l1NormalizedTolerance) ) pt3IsInside = true;
       if ((0.0 - l1NormalizedTolerance) <= normDist14 && normDist14 <= (1.0 +l1NormalizedTolerance) ) pt4IsInside = true;

       if (pt3IsInside && !pt4IsInside)
       {
           *fractionAlongLine1 = normDist13;
           *fractionAlongLine2 = 0.0;
           *x = x3;
           *y = y3;
       }
       else if (pt4IsInside && !pt3IsInside)
       {
           *fractionAlongLine1 = normDist14;
           *fractionAlongLine2 = 1.0;
           *x = x4;
           *y = y4;
       }
       else if (pt3IsInside && pt4IsInside)
       {
           // Return edge 2 vertex furthest along edge 1
           if (normDist13 <= normDist14)
           {
               *fractionAlongLine1 =  normDist14 ;
               *fractionAlongLine2 =  1.0;
               *x = x4;
               *y = y4;
           }
           else
           {
               *fractionAlongLine1 = normDist13;
               *fractionAlongLine2 =  0.0;
               *x = x3;
               *y = y3;
           }
      }
       else // both outside on each side
       {
           // Return End of edge 1
            *fractionAlongLine1 = 1.0;
            *fractionAlongLine2 = normDist32;
            *x = x2;
            *y = y2;
       } 
  
      return GeometryTools::LINES_OVERLAP;
#endif
   }

   /* Are the line parallel */
   if (fabs(denom) < EPS) {
      *x = 0;
      *y = 0;
      *fractionAlongLine1 = 0;
      *fractionAlongLine2 = 0;

      return GeometryTools::NO_INTERSECTION;
   }

   /* Is the intersection along the the segments */
   mua = numera / denom;
   mub = numerb / denom;

   *x = x1 + mua * (x2 - x1);
   *y = y1 + mua * (y2 - y1);
   *fractionAlongLine1 = mua;
   *fractionAlongLine2 = mub;

   if (mua < 0 - l1NormalizedTolerance || 1 + l1NormalizedTolerance < mua  || mub < 0 - l2NormalizedTolerance ||  1 + l2NormalizedTolerance < mub) 
   {
      return GeometryTools::LINES_INTERSECT_OUTSIDE;
   }
   else if (fabs(mua) < l1NormalizedTolerance || fabs(1-mua) < l1NormalizedTolerance || 
            fabs(mub) < l2NormalizedTolerance || fabs(1-mub) < l2NormalizedTolerance )
   {
       if (fabs(mua)   < l1NormalizedTolerance) *fractionAlongLine1 = 0;
       if (fabs(1-mua) < l1NormalizedTolerance) *fractionAlongLine1 = 1;
       if (fabs(mub)   < l2NormalizedTolerance) *fractionAlongLine2 = 0;
       if (fabs(1-mub) < l2NormalizedTolerance) *fractionAlongLine2 = 1;
       return GeometryTools::LINES_TOUCH;
   }
   else
   {
       return GeometryTools::LINES_CROSSES;
   }
}
//----------------------------------------------------------------------------------------------------------
/// Supposed to find the intersection point if lines intersect
/// It returns the intersection status telling if the lines only touch or are overlapping
//----------------------------------------------------------------------------------------------------------

GeometryTools::IntersectionStatus 
GeometryTools::inPlaneLineIntersect3D(  const cvf::Vec3d& planeNormal, 
                                        const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3, const cvf::Vec3d& p4, 
                                        cvf::Vec3d* intersectionPoint, double* fractionAlongLine1, double* fractionAlongLine2,  double tolerance)
{
    CVF_ASSERT (intersectionPoint != NULL);

    int Z = findClosestAxis(planeNormal);
    int X = (Z + 1) % 3;
    int Y = (Z + 2) % 3;
    double x, y;

    // Todo: handle zero length edges
    double l1NormTol = tolerance / (p2-p1).length();
    double l2NormTol = tolerance / (p4-p3).length();

    IntersectionStatus intersectionStatus = inPlaneLineIntersect(p1[X], p1[Y], p2[X], p2[Y], p3[X], p3[Y], p4[X], p4[Y], l1NormTol, l2NormTol, &x, &y, fractionAlongLine1, fractionAlongLine2);

    // Check if we have a valid intersection point
    if (intersectionStatus == NO_INTERSECTION || intersectionStatus == LINES_OVERLAP)
    {
        intersectionPoint->setZero();
    }
    else
    {
        *intersectionPoint = p1 + (*fractionAlongLine1)*(p2-p1);
    }
   
    return intersectionStatus;
}


//--------------------------------------------------------------------------------------------------
/// \brief	Test if a point touches a polygon within the specified tolerance
///
/// \param	polygonNorm  Polygon normal
/// \param	pPolygonVerts  Array of polygon vertice coordinates
/// \param	piVertexIndices  Array of integer node indices for this polygon
/// \param	iNumVerts  Number of vertices in polygon
/// \param	point  The point to be checked 
/// \param  tolerance Tolerance in length
/// \param  touchedEdgeIndex returns -1 if point is inside, and edge index if point touches an edge.
/// \return	true if point lies inside or on the border of the polygon.
/// 
/// \assumpt	Assumes that the polygon is planar
/// \comment	First check if point is on an edge, Then check if it is inside by
///             counting the number of times a ray from point along positive X axis 
///             crosses an edge. Odd number says inside.
/// \author		SP (really by Eric Haines) and JJS
//--------------------------------------------------------------------------------------------------
bool GeometryTools::isPointTouchingIndexedPolygon(const cvf::Vec3d& polygonNormal, const cvf::Vec3d* vertices, const size_t* indices, size_t numIndices, const cvf::Vec3d& point, int* touchedEdgeIndex,  double tolerance )
{
    int Z = findClosestAxis(polygonNormal);
    int X = (Z + 1) % 3;
    int Y = (Z + 2) % 3;

    int	crossings;

    int xBelowVx0;
    int yBelowVx0;
    int yBelowVx1 = 0;

    const double* vtx0;
    const double* vtx1 = NULL;

    double dv0;

    cvf::uint	j;

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
                crossings += (vtx0[X] -	dv0*(vtx1[X] - vtx0[X])/(vtx1[Y] - vtx0[Y])) >= point[X];
            }	    
        }	
    }

    // test if crossings is odd. If we care about its winding number > 0, then just: inside_flag = crossings > 0; 
    if (crossings & 0x01) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double	GeometryTools::linePointSquareDist(const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3)
{
    cvf::Vec3d v31 = p3 - p1;
    cvf::Vec3d v21 = p2 - p1;

    double geomTolerance = 1e-24;
    if (v21.lengthSquared() < geomTolerance)
    {
        // P2 and P1 coincide, use distance from P3 to P1
        return v31.lengthSquared();
    }

    double u = (v31*v21)/(v21*v21);
    cvf::Vec3d pOnLine(0,0,0);
    if (0 < u && u < 1) pOnLine = p1 + u*v21;
    else if (u <= 0 ) pOnLine = p1;
    else pOnLine = p2;

    return (p3-pOnLine).lengthSquared();
}

//--------------------------------------------------------------------------------------------------
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
// http://www.softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm
//
/// Intersect a line segment with a 3D triangle
///    Input:  A line segment p0, p1. A triangle t0, t1, t2.
///    Output: *intersectionPoint = intersection point (when it exists)
///    Return: -1 = triangle is degenerate (a segment or point)
///             0 = disjoint (no intersect)
///             1 = intersect in unique point I1
///             2 = are in the same plane
//--------------------------------------------------------------------------------------------------

#define SMALL_NUM  0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v)   ((u).x() * (v).x() + (u).y() * (v).y() + (u).z() * (v).z())

int GeometryTools::intersectLineSegmentTriangle( const cvf::Vec3d p0, const cvf::Vec3d p1, 
                                                 const cvf::Vec3d t0, const cvf::Vec3d t1, const cvf::Vec3d t2,
                                                 cvf::Vec3d* intersectionPoint )
{
    CVF_ASSERT(intersectionPoint != NULL);
    cvf::Vec3d u, v, n;             // triangle vectors
    cvf::Vec3d dir, w0, w;          // ray vectors
    double     r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = t1 - t0;
    v = t2 - t0;
    n = u ^ v;                     // cross product
    if (n == cvf::Vec3d::ZERO)     // triangle is degenerate
        return -1;                 // do not deal with this case

    dir = p1 - p0;                 // ray direction vector
    w0  = p0 - t0;
    a   = -dot(n, w0);
    b   =  dot(n, dir);
    if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
        if (a == 0)                // ray lies in triangle plane
            return 2;
        else return 0;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect

    if (r > 1.0)                   // Line segment does not reach triangle
        return 0;

    *intersectionPoint = p0 + r * dir;             // intersect point of ray and plane

    // is I inside T?
    double    uu, uv, vv, wu, wv, D;
    uu = dot(u, u);
    uv = dot(u, v);
    vv = dot(v, v);
    w = *intersectionPoint - t0;
    wu = dot(w, u);
    wv = dot(w, v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    double s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return 0;

    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
}

/*
//    t0 = (x0, y0, z0)
//    t1 = (x1, y1, z1)
//    t2 = (x2, y2, z2)
//
//    p = (xp, yp, zp)

cvf::Vec3d barycentricCoordsExperiment(const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2, const cvf::Vec3d& p)
{
    det = x0(y1*z2 - y2*z1) + x1(y2*z0 - z2*y0) + x2(y0*z1 - y1*z0);

    b0 = ((x1 * y2 - x2*y1)*zp + xp*(y1*z2-y2*z1) + yp*(x2*z1-x1*z2)) / det;
    b1 = ((x2 * y0 - x0*y2)*zp + xp*(y2*z0-y0*z2) + yp*(x0*z2-x2*z0)) / det;
    b2 = ((x0 * y1 - x1*y0)*zp + xp*(y0*z1-y1*z0) + yp*(x1*z0-x0*z1)) / det;
}

*/

inline double TriArea2D(double x1, double y1, double x2, double y2, double x3, double y3)
{
    return (x1-x2)*(y2-y3) - (x2-x3)*(y1-y2);
}

//--------------------------------------------------------------------------------------------------
// Compute barycentric coordinates (area coordinates) (u, v, w) for 
// point p with respect to triangle (t0, t1, t2)
// These can be used as weights for interpolating scalar values across the triangle
// Based on section 3.4 in "Real Time collision detection" by Christer Ericson
//--------------------------------------------------------------------------------------------------
cvf::Vec3d GeometryTools::barycentricCoords(const cvf::Vec3d&  t0, const cvf::Vec3d&  t1, const cvf::Vec3d&  t2, const cvf::Vec3d&  p)
{
    // Unnormalized triangle normal
    cvf::Vec3d m = (t1 - t0 ^ t2 - t0);

    // Absolute components for determining projection plane
    int X = 0, Y = 1, Z = 2;
    Z = findClosestAxis(m);
    switch (Z)
    {
    case 0: X = 1; Y = 2; break; // x is largest, project to the yz plane
    case 1: X = 0; Y = 2; break; // y is largest, project to the xz plane
    case 2: X = 0; Y = 1; break; // z is largest, project to the xy plane
    }

    // Compute areas in plane of largest projection
    // Nominators and one-over-denominator for u and v ratios
    double nu, nv, ood;
    nu = TriArea2D(p[X], p[Y], t1[X], t1[Y], t2[X], t2[Y]); // Area of PBC in yz plane
    nv = TriArea2D(p[X], p[Y], t2[X], t2[Y], t0[X], t0[Y]); // Area of PCA in yz plane
    ood = 1.0f / m[Z];                             // 1/(2*area of ABC in yz plane)

    if (Z == 1) ood = -ood; // For some reason not explained

    // Normalize

    m[0] = nu * ood;
    m[1] = nv * ood;
    m[2] = 1.0f - m[0] - m[1];

    return m;
}

//--------------------------------------------------------------------------------------------------
/// Inserts the vertex into the polygon if it fits along one of the edges within the tolerance.
/// The method returns true if it was inserted, or if it was already in the polygon, or if it was 
/// within the tolerance of an existing vertex in the polygon. 
/// In the latter situation it replaces the previous vertex in the polygon.
/// 
/// Todo: If a vertex is replaced, the VxToCv map in TimeStepGeometry should be updated
//--------------------------------------------------------------------------------------------------
bool GeometryTools::insertVertexInPolygon(std::list<std::pair<cvf::uint, bool> >* polygon, const cvf::Vec3dArray& nodeCoords, cvf::uint vertexIndex, double tolerance)
{
    std::list<std::pair<cvf::uint, bool> >::iterator it;
    for(it = polygon->begin(); it != polygon->end(); ++it)
    {
        if (it->first == vertexIndex) return true;
    }


#if 1
    bool existsOrInserted = false;
    for(it = polygon->begin(); it != polygon->end(); ++it)
    {
        if ( (nodeCoords[it->first] - nodeCoords[vertexIndex]).length() < tolerance)
        {
            if (vertexIndex < it->first) it->first = vertexIndex;
            existsOrInserted = true;
        }
    }

    if (existsOrInserted) return true;
#endif

    // Insert vertex in polygon if the distance to one of the edges is small enough

    std::list<std::pair<cvf::uint, bool> >::iterator it2;
    std::list<std::pair<cvf::uint, bool> >::iterator insertBefore;

    for (it = polygon->begin(); it != polygon->end(); ++it)
    {
        it2 = it;
        ++it2; insertBefore = it2; if (it2 == polygon->end()) it2 = polygon->begin();

        double sqDistToLine = GeometryTools::linePointSquareDist(nodeCoords[it->first], nodeCoords[it2->first], nodeCoords[vertexIndex]);
        if (fabs(sqDistToLine) < tolerance*tolerance )
        {
            it = polygon->insert(insertBefore, std::make_pair(vertexIndex, false));
            existsOrInserted = true;
        }
    }

    return existsOrInserted;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryTools::addMidEdgeNodes(std::list<std::pair<cvf::uint, bool> >* polygon, const cvf::Vec3dArray& nodes, EdgeSplitStorage& edgeSplitStorage, std::vector<cvf::Vec3d>* createdVertexes)
{
    cvf::uint newVertexIndex = nodes.size() + createdVertexes->size();
    std::list<std::pair<cvf::uint, bool> >::iterator it;
    std::list<std::pair<cvf::uint, bool> >::iterator it2;

    cvf::Vec3d midEdgeCoord(0,0,0);
    size_t midPointIndex = cvf::UNDEFINED_UINT;

    for (it = polygon->begin(); it != polygon->end(); ++it)
    {
        it2 = it;
        ++it2; if (it2 == polygon->end()) it2 = polygon->begin();

        // Find or Create and add a mid-edge node

        if (!edgeSplitStorage.findSplitPoint(it->first, it2->first, &midPointIndex))
        {

            midEdgeCoord.setZero();
            midEdgeCoord += (it->first  < nodes.size()) ? nodes[it->first]  : (*createdVertexes)[it->first - nodes.size()];
            midEdgeCoord += (it2->first < nodes.size()) ? nodes[it2->first] : (*createdVertexes)[it2->first - nodes.size()];
            midEdgeCoord *= 0.5;

            midPointIndex = newVertexIndex;
            createdVertexes->push_back(midEdgeCoord);
            ++newVertexIndex;

            edgeSplitStorage.addSplitPoint(it->first, it2->first, midPointIndex);
        }

        if (it2 != polygon->begin())
            polygon->insert(it2, std::make_pair(midPointIndex, true));
        else 
            polygon->insert(polygon->end(), std::make_pair(midPointIndex, true));

        ++it;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns true if we get an actual polygon
//--------------------------------------------------------------------------------------------------

bool GeometryTools::calculateOverlapPolygonOfTwoQuads(std::vector<size_t> * polygon, std::vector<cvf::Vec3d>* createdVertexes, 
                                 EdgeIntersectStorage& edgeIntersectionStorage, 
                                 const cvf::Vec3dArray& nodes, 
                                 const size_t cv1CubeFaceIndices[4], 
                                 const size_t cv2CubeFaceIndices[4],
                                 double tolerance)
{

    // Topology analysis

    if (createdVertexes == NULL) return false;

    size_t newVertexIndex = nodes.size() + createdVertexes->size();

    bool cv1VxTouchCv2[4] = { false, false, false, false };
    bool cv2VxTouchCv1[4] = { false, false, false, false };
    int  cv1VxTouchCv2Edge[4] = { -1, -1, -1, -1 };
    int  cv2VxTouchCv1Edge[4] = { -1, -1, -1, -1 };

    int cv1Idx, cv2Idx;
    int numMatchedNodes = 0;

    // First check for complete topological match.

    for (cv1Idx = 0 ; cv1Idx < 4 ; ++cv1Idx)
    {
        bool found = false;
        for (cv2Idx = 0; cv2Idx < 4; ++cv2Idx)
        {
            if (cv1CubeFaceIndices[cv1Idx] == cv2CubeFaceIndices[cv2Idx])
            {
                cv1VxTouchCv2[cv1Idx] = true;
                cv2VxTouchCv1[cv2Idx] = true;
                found = true;
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
            cv1VxTouchCv2[cv1Idx] = GeometryTools::isPointTouchingIndexedPolygon(normal, nodes.ptr(), &cv2CubeFaceIndices[0], 4,  nodes[cv1CubeFaceIndices[cv1Idx]], &(cv1VxTouchCv2Edge[cv1Idx]), tolerance);        
            if (cv1VxTouchCv2[cv1Idx]) ++numCv1VxesOnCv2;
       }

       if (!cv2VxTouchCv1[cv1Idx])
       {
           cv2VxTouchCv1[cv1Idx] = GeometryTools::isPointTouchingIndexedPolygon(normal, nodes.ptr(), &cv1CubeFaceIndices[0], 4,  nodes[cv2CubeFaceIndices[cv1Idx]], &(cv2VxTouchCv1Edge[cv1Idx]), tolerance);        
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
        for (k = 0; k < 4; ++k)
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

        std::vector<size_t>  intersectionVxIndices;
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
            size_t intersectionVxIndex = cvf::UNDEFINED_UINT;

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
               
                 bool found = edgeIntersectionStorage.findIntersection( cv1CubeFaceIndices[cv1Idx],     
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

                    edgeIntersectionStorage.addIntersection( cv1CubeFaceIndices[cv1Idx],     
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

        std::map<double,size_t> sortingMap;
        for (i = 0; i < intersectionFractionsAlongEdge.size(); ++i)
        {
            sortingMap[intersectionFractionsAlongEdge[i]] = intersectionVxIndices[i];
        }

        std::map<double, size_t>::iterator it;
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
// Vertex Index to position in polygon
typedef std::map< size_t, std::vector<size_t >::const_iterator > VxIdxToPolygonPositionMap;
#define DEBUG_PRINT 0

//template <typename NodeArrayType, typename NodeType, typename IndicesArrayType, typename IndicesType>
//void setup( ArrayWrapper<NodeArrayType, NodeType> nodeArray,  ArrayWrapper<IndicesArrayType, IndicesType> indices) 

void GeometryTools::calculatePartiallyFreeCubeFacePolygon(const cvf::Vec3dArray&       nodeCoords, 
                                                          const std::vector<size_t>*   completeFacePolygon, 
                                                          const cvf::Vec3d&            faceNormal, 
                                                          const std::vector< std::vector<size_t>* >& faceOverlapPolygons, 
                                                          const std::vector<bool>      faceOverlapPolygonWindingSameAsCubeFaceFlags,
                                                          std::vector<size_t>*         partialFacePolygon, 
                                                          bool*                        m_partiallyFreeCubeFaceHasHoles)
{
    CVF_ASSERT(m_partiallyFreeCubeFaceHasHoles);
    CVF_ASSERT(completeFacePolygon != NULL);
    CVF_ASSERT(partialFacePolygon != NULL);

    // Copy the start polygon
    std::list<size_t> resultPolygon; 
    for (size_t pcIdx = 0; pcIdx < completeFacePolygon->size(); ++pcIdx)
    {
        resultPolygon.push_back((*completeFacePolygon)[pcIdx]);
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
            for (std::vector<size_t >::const_iterator pcIt = faceOverlapPolygons[i]->begin(); 
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
        std::list<size_t>::const_iterator polIt;
        for ( polIt = resultPolygon.begin(); polIt != resultPolygon.end(); ++polIt)
        {
            cvf::Trace::show(cvf::String("%1 \t%2 %3 %4").arg(*polIt)
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
            std::vector<size_t >::const_iterator polIt;
            cvf::Trace::show("Connection " + cvf::String((long long)cIdx));
            for (polIt = faceOverlapPolygons[cIdx]->begin(); polIt !=  faceOverlapPolygons[cIdx]->end(); ++polIt)
            {
                cvf::Trace::show(cvf::String("%1 \t%2 %3 %4").arg(*polIt)
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


    for (std::list<size_t>::iterator pIt =  resultPolygon.begin(); pIt != resultPolygon.end(); ++pIt)
    {
        // Set iterator to previous node in polygon
        std::list<size_t>::iterator prevPIt = pIt;
        if (prevPIt == resultPolygon.begin()) prevPIt = resultPolygon.end();
        --prevPIt;

        cvf::Vec3d pToPrev = nodeCoords[*prevPIt] - nodeCoords[*pIt];

        // Set iterator to next node in polygon. Used to insert before and as pointer to the next point
        std::list<size_t>::iterator nextPIt = pIt;
        ++nextPIt;
        std::list<size_t>::iterator insertBeforePIt = nextPIt;
        if (nextPIt == resultPolygon.end()) nextPIt = resultPolygon.begin();

        // Calculate existing edge to edge angle

        cvf::Vec3d pToNext = nodeCoords[*nextPIt] - nodeCoords[*pIt];
        double mainPolygonEdgeAngle = GeometryTools::getAngle(faceNormal, pToNext , pToPrev);

        // Find connections containing the pIt vertex index. Merge them into the main polygon

        for (size_t opIdx = 0; opIdx < faceOverlapPolygons.size(); ++opIdx)
        {
            if (isConnectionPolygonMerged[opIdx]) continue; // Already merged

            // Find position of pIt vertex index in the current connection polygon
            VxIdxToPolygonPositionMap::iterator vxIndexPositionInPolygonIt = polygonSearchMaps[opIdx].find(*pIt);

            if (vxIndexPositionInPolygonIt != polygonSearchMaps[opIdx].end())
            {
                // Merge the connection polygon into the main polygon 
                // if the angle prevPIt pIt nextPIt is larger than angle prevPIt pIt (startCPIt++)

                std::vector<size_t>::const_iterator startCPIt;
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
                    std::vector<size_t >::const_iterator pcIt = startCPIt;
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
    for ( std::list<size_t>::iterator pIt = resultPolygon.begin(); pIt != resultPolygon.end() && !goneAround; ++pIt)
    {
        // Set iterator to next node in polygon.
        std::list<size_t>::iterator  nextPIt = pIt;
        ++nextPIt;
        if (nextPIt == resultPolygon.end()) 
        { 
            nextPIt = resultPolygon.begin();
            goneAround = true; // Gone around polygon. Stop even if pIt is jumping over end()
        }

        // Set iterator to previous node in polygon

        std::list<size_t>::iterator prevPIt = pIt;

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
        for (std::list<size_t>::iterator pIt = resultPolygon.begin(); pIt != resultPolygon.end(); ++pIt)
        {
            cvf::Trace::show(cvf::String("%1 \t%2 %3 %4").arg(*pIt)
                .arg(nodeCoords[*pIt].x())
                .arg(nodeCoords[*pIt].y())
                .arg(nodeCoords[*pIt].z()));
        }
    }
#endif

    // Copy the result polygon to the output variable 

    partialFacePolygon->clear();
    for (std::list<size_t>::iterator pIt = resultPolygon.begin(); pIt != resultPolygon.end(); ++pIt)
    {
        partialFacePolygon->push_back(*pIt);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EdgeIntersectStorage::setVertexCount(size_t size)
{
    m_edgeIntsectMap.resize(size);
}

void EdgeIntersectStorage::canonizeAddress(size_t& e1P1, size_t& e1P2, size_t& e2P1, size_t& e2P2, bool& flipE1, bool& flipE2, bool& flipE1E2)
{
    flipE1 = e1P1 > e1P2;
    flipE2 = e2P1 > e2P2;

    flipE1E2 = (flipE1 ? e1P2: e1P1) > (flipE2 ? e2P2: e2P1);

    static size_t temp;
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
void EdgeIntersectStorage::addIntersection(size_t e1P1, size_t e1P2, size_t e2P1, size_t e2P2, 
                                           size_t vxIndexIntersectionPoint, GeometryTools::IntersectionStatus intersectionStatus, 
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
bool EdgeIntersectStorage::findIntersection(size_t e1P1, size_t e1P2, size_t e2P1, size_t e2P2, 
                                            size_t* vxIndexIntersectionPoint, GeometryTools::IntersectionStatus* intersectionStatus, 
                                            double* fractionAlongEdge1, double* fractionAlongEdge2)
{
    static bool flipE1  ; 
    static bool flipE2  ;
    static bool flipE1E2;

    canonizeAddress(e1P1, e1P2, e2P1, e2P2, flipE1, flipE2, flipE1E2);

    if (!m_edgeIntsectMap[e1P1].size()) return false;

    std::map<size_t, std::map<size_t, std::map<size_t, IntersectData > > >::iterator it;
    it = m_edgeIntsectMap[e1P1].find(e1P2);
    if (it == m_edgeIntsectMap[e1P1].end()) return false;

    std::map<size_t, std::map<size_t, IntersectData > >::iterator it2;
    it2 = it->second.find(e2P1);
    if (it2 == it->second.end()) return false;

    std::map<size_t, IntersectData >::iterator it3;
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




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EdgeSplitStorage::setVertexCount(size_t size)
{
     m_edgeSplitMap.resize(size);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool EdgeSplitStorage::findSplitPoint(size_t edgeP1Index, size_t edgeP2Index, size_t* splitPointIndex)
{
    canonizeAddress(edgeP1Index, edgeP2Index);
    CVF_ASSERT(edgeP1Index < m_edgeSplitMap.size());    

    std::map< size_t, size_t >::iterator it;

    it = m_edgeSplitMap[edgeP1Index].find(edgeP2Index);
    if (it == m_edgeSplitMap[edgeP1Index].end()) return false;

    *splitPointIndex = it->second;
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EdgeSplitStorage::addSplitPoint(size_t edgeP1Index, size_t edgeP2Index, size_t splitPointIndex)
{
    canonizeAddress(edgeP1Index, edgeP2Index);
    CVF_ASSERT(edgeP1Index < m_edgeSplitMap.size());    
    m_edgeSplitMap[edgeP1Index][edgeP2Index] = splitPointIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EdgeSplitStorage::canonizeAddress(size_t& edgeP1Index, size_t& edgeP2Index)
{
    if (edgeP1Index > edgeP2Index)
    {
        size_t tmp;
        tmp = edgeP1Index;
        edgeP1Index = edgeP2Index;
        edgeP2Index = tmp;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EarClipTesselator::EarClipTesselator(): 
    m_X(-1), 
    m_Y(-1), 
    m_areaTolerance(1e-12), 
    m_nodeCoords(NULL)
{

}

//--------------------------------------------------------------------------------------------------
/// \brief  	Do the main processing/actual triangulation
/// \param  	triangleIndices Array that will receive the indices of the triangles resulting from the triangulation
/// \return	    true when a tesselation was successully created 
//--------------------------------------------------------------------------------------------------

bool EarClipTesselator::calculateTriangles( std::vector<cvf::uint>* triangleIndices ) 
{
    CVF_ASSERT(m_nodeCoords != NULL);
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    size_t numVertices = m_polygonIndices.size();

    if (numVertices < 3) return false;

    // We want m_polygonIndices to be a counter-clockwise polygon to make the validation test work

	if (calculatePolygonArea() < 0 )
	{
		m_polygonIndices.reverse();
	}

    std::list<size_t>::iterator u, v, w;

    // If we loop two times around polygon without clipping a single triangle we are toast.
	size_t count = 2*numVertices;   // error detection 

	v = m_polygonIndices.end();   //nv - 1;
    --v;

	while (numVertices > 2)
	{
		// if we loop, it is probably a non-simple polygon 
		if (count <= 0 )
		{
			// Triangulate: ERROR - probable bad polygon!
			return false;
		}
        --count; 

		// Three consecutive vertices in current polygon, <u,v,w> 
		// previous 
	    u = v; 
		if (u == m_polygonIndices.end()) u =  m_polygonIndices.begin(); // if (nv <= u) u = 0;     

		// new v
		v = u; ++v; //u + 1; 
		if (v == m_polygonIndices.end()) v =  m_polygonIndices.begin(); //if (nv <= v) v = 0;

		// next
		w = v; ++w; //v + 1; 
		if (w == m_polygonIndices.end()) w =  m_polygonIndices.begin(); //if (nv <= w) w = 0;     


		if ( isTriangleValid(u, v, w) )
		{
			// Indices of the vertices 
			triangleIndices->push_back(*u);
			triangleIndices->push_back(*v);
			triangleIndices->push_back(*w);

			// Remove v from remaining polygon 
            m_polygonIndices.erase(v);
            v = w;
			numVertices--;

			// Resets error detection counter 
			count = 2*numVertices;
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
/// Is this a valid triangle ? ( No points inside, and points not on a line. )  
//--------------------------------------------------------------------------------------------------

bool EarClipTesselator::isTriangleValid( std::list<size_t>::const_iterator u, std::list<size_t>::const_iterator v, std::list<size_t>::const_iterator w) const
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    cvf::Vec3d A = (*m_nodeCoords)[*u];
    cvf::Vec3d B = (*m_nodeCoords)[*v];
    cvf::Vec3d C = (*m_nodeCoords)[*w];

    if (  m_areaTolerance > (((B[m_X]-A[m_X])*(C[m_Y]-A[m_Y])) - ((B[m_Y]-A[m_Y])*(C[m_X]-A[m_X]))) ) return false;

	std::list<size_t>::const_iterator c;
    std::list<size_t>::const_iterator outside;
	for (c = m_polygonIndices.begin(); c != m_polygonIndices.end(); ++c)
	{
        // The polygon points that actually make up the triangle candidate does not count
        // (but the same points on different positions in the polygon does! 
        // Except those one off the triangle, that references the start or end of the triangle)

		if ( (c == u) || (c == v) || (c == w)) continue;

        // Originally the below tests was not included which resulted in missing triangles sometimes

        outside = w; ++outside; if (outside == m_polygonIndices.end()) outside = m_polygonIndices.begin();
        if (c == outside && *c == *u) 
        {
            continue;
        }

        outside = u; if (outside == m_polygonIndices.begin()) outside = m_polygonIndices.end(); --outside; 
        if (c == outside && *c == *w) 
        {
            continue;
        }

        cvf::Vec3d P = (*m_nodeCoords)[*c];

		if (isPointInsideTriangle(A, B, C, P)) return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
/// Decides if a point P is inside of the triangle defined by A, B, C.
/// By calculating the "double area" (cross product) of Corner to corner x Corner to point vectors
//--------------------------------------------------------------------------------------------------

bool EarClipTesselator::isPointInsideTriangle(const cvf::Vec3d& A, const cvf::Vec3d& B, const cvf::Vec3d& C, const cvf::Vec3d& P) const
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);
    
	double ax = C[m_X] - B[m_X];  double ay = C[m_Y] - B[m_Y];
	double bx = A[m_X] - C[m_X];  double by = A[m_Y] - C[m_Y];
	double cx = B[m_X] - A[m_X];  double cy = B[m_Y] - A[m_Y];

	double apx= P[m_X] - A[m_X];  double apy= P[m_Y] - A[m_Y];
	double bpx= P[m_X] - B[m_X];  double bpy= P[m_Y] - B[m_Y];
	double cpx= P[m_X] - C[m_X];  double cpy= P[m_Y] - C[m_Y];

	double aCROSSbp = ax*bpy - ay*bpx;
	double cCROSSap = cx*apy - cy*apx;
	double bCROSScp = bx*cpy - by*cpx;
    double tol = 0;
	return ((aCROSSbp >= tol) && (bCROSScp >= tol) && (cCROSSap >= tol));
};

//--------------------------------------------------------------------------------------------------
/// Computes area of the currently stored 2D polygon/contour
//--------------------------------------------------------------------------------------------------

double EarClipTesselator::calculatePolygonArea() const
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);

	double A = 0;

	std::list<size_t>::const_iterator p = m_polygonIndices.end();
    --p;

	std::list<size_t>::const_iterator q = m_polygonIndices.begin();
	while (q != m_polygonIndices.end())
	{
		A += (*m_nodeCoords)[*p][m_X] * (*m_nodeCoords)[*q][m_Y] - (*m_nodeCoords)[*q][m_X]*(*m_nodeCoords)[*p][m_Y];

		p = q;
		q++;
	}

	return A*0.5;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setNormal(const cvf::Vec3d& polygonNormal)
{
    int Z = GeometryTools::findClosestAxis(polygonNormal);
     m_X = (Z + 1) % 3;
     m_Y = (Z + 2) % 3;
     m_polygonNormal = polygonNormal;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setPolygonIndices(const std::list<size_t>& polygon)
{
    m_polygonIndices = polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setPolygonIndices(const std::vector<size_t>& polygon)
{
    size_t i;
    for (i = 0; i < polygon.size();  ++i)
    {
        m_polygonIndices.push_back(polygon[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setMinTriangleArea(double areaTolerance)
{
    m_areaTolerance = 2*areaTolerance; // Convert to trapesoidal area
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setGlobalNodeArray(const cvf::Vec3dArray& nodeCoords)
{
    m_nodeCoords = &nodeCoords;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FanEarClipTesselator::FanEarClipTesselator() : 
    m_centerNodeIndex(std::numeric_limits<size_t>::max())
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FanEarClipTesselator::setCenterNode(size_t centerNodeIndex)
{
    m_centerNodeIndex = centerNodeIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool FanEarClipTesselator::calculateTriangles(std::vector<cvf::uint>* triangles)
{
    CVF_ASSERT(m_centerNodeIndex != std::numeric_limits<size_t>::max());
    CVF_ASSERT(m_nodeCoords != NULL);
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    size_t nv = m_polygonIndices.size();

    if (nv < 3) return false;

    // We want m_polygonIndices to be a counter-clockwise polygon to make the validation test work

    if (calculatePolygonArea() < 0 )
    {
        m_polygonIndices.reverse();
    }

    std::list<size_t>::const_iterator it1;
    std::list<size_t>::const_iterator it2;

    std::list< std::list<size_t> > restPolygons;
    bool wasPreviousTriangleValid = true;

    for (it1 = m_polygonIndices.begin(); it1 != m_polygonIndices.end(); it1++)
    {
        it2 = it1;
        it2++;

        if (it2 == m_polygonIndices.end()) it2 = m_polygonIndices.begin();

        if (isTriangleValid(*it1, *it2, m_centerNodeIndex))
        {
            triangles->push_back(*it1);
            triangles->push_back(*it2);
            triangles->push_back(m_centerNodeIndex);
            wasPreviousTriangleValid = true;
        }
        else
        {
            if (wasPreviousTriangleValid)
            {
                // Create new rest polygon.
                restPolygons.push_back(std::list<size_t>());
                restPolygons.back().push_back(m_centerNodeIndex);
                restPolygons.back().push_back(*it1);
                restPolygons.back().push_back(*it2);
            }
            else
            {
                restPolygons.back().push_back(*it2);
            }
        }
    }

    EarClipTesselator triMaker;
    triMaker.setNormal(m_polygonNormal);
    triMaker.setMinTriangleArea(m_areaTolerance);
    triMaker.setGlobalNodeArray(*m_nodeCoords);
    std::list< std::list<size_t> >::iterator rpIt;

    for (rpIt = restPolygons.begin(); rpIt != restPolygons.end(); ++rpIt)
    {
        triMaker.setPolygonIndices(*rpIt);
        triMaker.calculateTriangles(triangles);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// This needs to be rewritten because we need to test for crossing edges, not only point inside.
/// In addition the test for polygon 
//--------------------------------------------------------------------------------------------------
bool FanEarClipTesselator::isTriangleValid(size_t u, size_t v, size_t w)
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    cvf::Vec3d A = (*m_nodeCoords)[u];
    cvf::Vec3d B = (*m_nodeCoords)[v];
    cvf::Vec3d C = (*m_nodeCoords)[w];

    if (  m_areaTolerance > (((B[m_X]-A[m_X])*(C[m_Y]-A[m_Y])) - ((B[m_Y]-A[m_Y])*(C[m_X]-A[m_X]))) ) return false;

    std::list<size_t>::const_iterator c;
    for (c = m_polygonIndices.begin(); c != m_polygonIndices.end(); ++c)
    {
        // The polygon points that actually make up the triangle candidate does not count
        // (but the same points on different positions in the polygon does! )
        // Todo so this test below is to accepting !! Bug !!
        if ( (*c == u) || (*c == v) || (*c == w)) continue;

        cvf::Vec3d P = (*m_nodeCoords)[*c];

        if (isPointInsideTriangle(A, B, C, P)) return false;
    }

    return true;
}


}