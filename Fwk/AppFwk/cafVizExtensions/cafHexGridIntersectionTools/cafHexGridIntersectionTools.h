
#pragma once

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

namespace cvf {
    class Plane;
};


namespace caf {



//==================================================================================================
//
//
//==================================================================================================
class HexGridIntersectionTools
{
public:

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    struct ClipVx
    {
        ClipVx();

        cvf::Vec3d  vx;

        double      normDistFromEdgeVx1;
        size_t      clippedEdgeVx1Id;
        size_t      clippedEdgeVx2Id;

        bool        isVxIdsNative;  //< Pointing to real vertices, or indices to ClipVx's in the supplied triangle vertices array
        int         derivedVxLevel; //< Helper data to make it possible to track what set of ClipVx's the indices is reffering to in case of consecutive clips
    };

    static bool planeLineIntersect(const cvf::Plane& plane, 
                                   const cvf::Vec3d& a, 
                                   const cvf::Vec3d& b, 
                                   cvf::Vec3d* intersection, 
                                   double* normalizedDistFromA);

    static bool planeTriangleIntersection(const cvf::Plane& plane,
                                          const cvf::Vec3d& p1, 
                                          size_t p1Id,
                                          const cvf::Vec3d& p2, 
                                          size_t p2Id,
                                          const cvf::Vec3d& p3, 
                                          size_t p3Id,
                                          ClipVx* newVx1, 
                                          ClipVx* newVx2,
                                          bool* isMostVxesOnPositiveSide);

    static void clipTrianglesBetweenTwoParallelPlanes(const std::vector<ClipVx>& triangleVxes,
                                                      const std::vector<int>& cellFaceForEachTriangleEdge,
                                                      const cvf::Plane& p1Plane, 
                                                      const cvf::Plane& p2Plane,
                                                      std::vector<ClipVx>* clippedTriangleVxes,
                                                      std::vector<int>* cellFaceForEachClippedTriangleEdge);

    static cvf::Vec3d planeLineIntersectionForMC(const cvf::Plane& plane, 
                                                 const cvf::Vec3d& p1, 
                                                 const cvf::Vec3d& p2, 
                                                 double* normalizedDistFromP1);

    static int planeHexIntersectionMC(const cvf::Plane& plane,
                                      const cvf::Vec3d cell[8],
                                      const size_t hexCornersIds[8],
                                      std::vector<ClipVx>* triangleVxes,
                                      std::vector<int>* cellFaceForEachTriangleEdge);

};

}; // namespace caf