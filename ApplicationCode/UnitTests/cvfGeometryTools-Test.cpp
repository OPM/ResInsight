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

#include "gtest/gtest.h"

#include "cvfLibCore.h"
#include "cvfLibViewing.h"
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"

#include "cvfArrayWrapperToEdit.h"
#include "cvfArrayWrapperConst.h"

#include "cvfGeometryTools.h"
#include "cvfBoundingBoxTree.h"

#include <array>

using namespace cvf;

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ControlVolume::calculateCubeFaceStatus(const cvf::Vec3dArray& nodeCoords, double areaTolerance)
{
    int cubeFace;
    cvf::uint cubeFaceIndices[4];
    for (cubeFace = 0; cubeFace < 6; ++cubeFace)
    {
        surfaceNodeIndices(static_cast<Defines::CubeFace>(cubeFace), cubeFaceIndices);

        std::vector<const brv::Connection*> conns;
        connections(static_cast<Defines::CubeFace>(cubeFace), &conns);

        if (!conns.size()) 
        {
            m_cubeFaceStatus[cubeFace] = FREE_FACE;
        }
        else
        {
            double area = 0.5 * (nodeCoords[cubeFaceIndices[1]]-nodeCoords[cubeFaceIndices[0]] ^ nodeCoords[cubeFaceIndices[3]]-nodeCoords[cubeFaceIndices[0]]).length();
            area +=  0.5 * (nodeCoords[cubeFaceIndices[3]]-nodeCoords[cubeFaceIndices[2]] ^ nodeCoords[cubeFaceIndices[1]]-nodeCoords[cubeFaceIndices[2]]).length();
            double totConnectionArea = 0;
            size_t i;
            for (i = 0; i < conns.size(); ++i)
            {
                totConnectionArea += conns[i]->brfArea();
            }

            if ( totConnectionArea < area - areaTolerance )
            {
                m_cubeFaceStatus[cubeFace] = PARTIALLY_COVERED;
            }
            else
            {
                m_cubeFaceStatus[cubeFace] = COMPLETELY_COVERED;
            }
        }

        // Create a polygon to store the complete polygon of the faces 
        // not completely covered by connections
        // This polygon will be filled with nodes later

        if (m_cubeFaceStatus[cubeFace] != COMPLETELY_COVERED )
        {
            m_freeFacePolygons[cubeFace] = new std::list<std::pair<cvf::uint, bool> >;
        }
    }
}
#endif


template <typename NodeArrayType, typename NodeType, typename IndexType>
NodeType  quadNormal (ArrayWrapperConst<NodeArrayType, NodeType> nodeCoords,  
    const IndexType cubeFaceIndices[4] )
{
    return ( nodeCoords[cubeFaceIndices[2]] - nodeCoords[cubeFaceIndices[0]]) ^  
        ( nodeCoords[cubeFaceIndices[3]] - nodeCoords[cubeFaceIndices[1]]); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> createVertices()
{
    std::vector<cvf::Vec3d> vxs;
    vxs.resize(14, cvf::Vec3d::ZERO);

    vxs[ 0]= cvf::Vec3d( 0   , 0   , 0   );
    vxs[ 1]= cvf::Vec3d( 1   , 0   , 0   );
    vxs[ 2]= cvf::Vec3d( 1   , 1   , 0   );
    vxs[ 3]= cvf::Vec3d( 0   , 1   , 0   );
    vxs[ 4]= cvf::Vec3d(-0.4 ,-0.2 , 0.0 );
    vxs[ 5]= cvf::Vec3d( 0.4 , 0.6 , 0.0 );
    vxs[ 6]= cvf::Vec3d( 0.8 , 0.2 , 0.0 );
    vxs[ 7]= cvf::Vec3d( 0.0 ,-0.6 , 0.0 );
    vxs[ 8]= cvf::Vec3d( 1.0 , 1.2 , 0.0 );
    vxs[ 9]= cvf::Vec3d( 1.4 , 0.8 , 0.0 );
    vxs[10]= cvf::Vec3d( 0.4 ,-0.2 , 0.0 );
    vxs[11]= cvf::Vec3d( 1.2 , 0.6 , 0.0 );
    vxs[12]= cvf::Vec3d( 1.6 , 0.2 , 0.0 );
    vxs[13]= cvf::Vec3d( 0.8 ,-0.6 , 0.0 );
  
    return vxs;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::array<cvf::uint, 4>> getCubeFaces()
{
    std::vector<std::array<cvf::uint, 4>> cubeFaces;

    cubeFaces.resize(4);
    cubeFaces[0] = { 0, 1, 2, 3 };
    cubeFaces[1] = { 4, 5, 6, 7 };
    cubeFaces[2] = { 5, 8, 9, 6 };
    cubeFaces[3] = { 10, 11, 12, 13 };

    return cubeFaces;
}

std::ostream& operator<< (std::ostream& stream, std::vector<cvf::uint> v)
{
    for (size_t i = 0; i < v.size(); ++i)
    {
        stream << v[i] << " ";
    }
    return stream;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CellFaceIntersectionTst, Intersection1)
{
    std::vector<cvf::Vec3d> nodes = createVertices();

    std::vector<cvf::Vec3d> additionalVertices;

    std::vector< std::vector<cvf::uint> > overlapPolygons;
    auto faces = getCubeFaces();

    EdgeIntersectStorage<cvf::uint> edgeIntersectionStorage;
    edgeIntersectionStorage.setVertexCount(nodes.size());
    {
        std::vector<cvf::uint> polygon;
        bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads(
            &polygon,
            &additionalVertices,
            &edgeIntersectionStorage,
            wrapArrayConst(&nodes),
            faces[0].data(),
            faces[1].data(),
            1e-6);

        EXPECT_EQ( (size_t)5, polygon.size());
        EXPECT_EQ( (size_t)2, additionalVertices.size());
        EXPECT_TRUE(isOk);
        overlapPolygons.push_back(polygon);
        std::cout << polygon << std::endl;

    }
  
    {
        std::vector<cvf::uint> polygon;
        bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads(
            &polygon, 
            &additionalVertices, 
            &edgeIntersectionStorage, 
            wrapArrayConst(&nodes), 
            faces[0].data(), 
            faces[2].data(), 
            1e-6);

        EXPECT_EQ( (size_t)5, polygon.size());
        EXPECT_EQ( (size_t)4, additionalVertices.size());
        EXPECT_TRUE(isOk);
        overlapPolygons.push_back(polygon);
        std::cout << polygon << std::endl;

    }

    {
        std::vector<cvf::uint> polygon;
        bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads(
            &polygon, 
            &additionalVertices, 
            &edgeIntersectionStorage, 
            wrapArrayConst(&nodes), 
            faces[0].data(), 
            faces[3].data(), 
            1e-6);

        EXPECT_EQ( (size_t)3, polygon.size());
        EXPECT_EQ( (size_t)6, additionalVertices.size());
        EXPECT_TRUE(isOk);
        overlapPolygons.push_back(polygon);
        std::cout << polygon << std::endl;
    }

   nodes.insert(nodes.end(), additionalVertices.begin(), additionalVertices.end());
   std::vector<cvf::uint> basePolygon;
   basePolygon.insert(basePolygon.begin(), faces[0].data(), &(faces[0].data()[4]));

   for (cvf::uint vxIdx = 0; vxIdx < nodes.size(); ++vxIdx)
   {
      GeometryTools::insertVertexInPolygon(
           &basePolygon,
           wrapArrayConst(&nodes),
           vxIdx, 
           1e-6
           );
   }
   
   EXPECT_EQ( (size_t)8, basePolygon.size());
   std::cout << "Bp: " << basePolygon << std::endl;

   for (size_t pIdx = 0; pIdx < overlapPolygons.size(); ++pIdx)
   {
       for (cvf::uint vxIdx = 0; vxIdx < nodes.size(); ++vxIdx)
       {
           GeometryTools::insertVertexInPolygon(
               &overlapPolygons[pIdx],
               wrapArrayConst(&nodes),
               vxIdx, 
               1e-6
               );
       }

       if (pIdx == 0)
       {
           EXPECT_EQ((size_t)5, overlapPolygons[pIdx].size());
       }
       if (pIdx == 1)
       {
           EXPECT_EQ((size_t)5, overlapPolygons[pIdx].size());
       }
       if (pIdx == 2)
       {
           EXPECT_EQ((size_t)4, overlapPolygons[pIdx].size());
       }

       std::cout << "Op" << pIdx << ":" << overlapPolygons[pIdx] << std::endl;
   }


   Vec3d normal = quadNormal(wrapArrayConst(&nodes), faces[0].data());
   std::vector<bool>  faceOverlapPolygonWindingSameAsCubeFaceFlags;
   faceOverlapPolygonWindingSameAsCubeFaceFlags.resize(overlapPolygons.size(), true);

   {
       std::vector<cvf::uint>         freeFacePolygon;
       bool hasHoles = false;

       std::vector< std::vector<cvf::uint>* > overlapPolygonPtrs;
       for (size_t pIdx = 0; pIdx < overlapPolygons.size(); ++pIdx)
       {
           overlapPolygonPtrs.push_back(&(overlapPolygons[pIdx]));
       }

       GeometryTools::calculatePartiallyFreeCubeFacePolygon(
           wrapArrayConst(&nodes),
           wrapArrayConst(&basePolygon),
           normal, 
           overlapPolygonPtrs, 
           faceOverlapPolygonWindingSameAsCubeFaceFlags, 
           &freeFacePolygon,
           &hasHoles
           );

       EXPECT_EQ( (size_t)4, freeFacePolygon.size());
       EXPECT_FALSE(hasHoles);
       std::cout <<  "FF1: " << freeFacePolygon << std::endl;
   }

   {
       std::vector<cvf::uint>         freeFacePolygon;
       bool hasHoles = false;

       std::vector< std::vector<cvf::uint>* > overlapPolygonPtrs;
       for (size_t pIdx = 0; pIdx < 1; ++pIdx)
       {
           overlapPolygonPtrs.push_back(&(overlapPolygons[pIdx]));
       }

       GeometryTools::calculatePartiallyFreeCubeFacePolygon(
           wrapArrayConst(&nodes),
           wrapArrayConst(&basePolygon),
           normal, 
           overlapPolygonPtrs, 
           faceOverlapPolygonWindingSameAsCubeFaceFlags, 
           &freeFacePolygon,
           &hasHoles
           );

       EXPECT_EQ( (size_t)9, freeFacePolygon.size());
       EXPECT_FALSE(hasHoles);

       std::cout << "FF2: " << freeFacePolygon << std::endl;

   }


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

TEST(CellFaceIntersectionTst, Intersection)
{
  
    std::vector<cvf::Vec3d> additionalVertices;
    cvf::Vec3dArray nodes;
    std::vector<size_t> polygon;

 
    cvf::Array<size_t> ids;
    size_t cv1CubeFaceIndices[4] = {0, 1, 2, 3};
    size_t cv2CubeFaceIndices[4] = {4, 5, 6, 7};

    nodes.resize(8);
    nodes.setAll(cvf::Vec3d(0, 0, 0));
    EdgeIntersectStorage<size_t> edgeIntersectionStorage;
    edgeIntersectionStorage.setVertexCount(nodes.size());

    // Face 1
    nodes[0] = cvf::Vec3d(0, 0, 0);
    nodes[1] = cvf::Vec3d(1, 0, 0);
    nodes[2] = cvf::Vec3d(1, 1, 0);
    nodes[3] = cvf::Vec3d(0, 1, 0);
    // Face 2
    nodes[4] = cvf::Vec3d(0, 0, 0);
    nodes[5] = cvf::Vec3d(1, 0, 0);
    nodes[6] = cvf::Vec3d(1, 1, 0);
    nodes[7] = cvf::Vec3d(0, 1, 0);

 
    bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, &edgeIntersectionStorage, 
                                        wrapArrayConst(&nodes), cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( (size_t)4, polygon.size());
    EXPECT_EQ( (size_t)0, additionalVertices.size());
    EXPECT_TRUE(isOk);

    // Face 1
    nodes[0] = cvf::Vec3d(0, 0, 0);
    nodes[1] = cvf::Vec3d(1, 0, 0);
    nodes[2] = cvf::Vec3d(1, 1, 0);
    nodes[3] = cvf::Vec3d(0, 1, 0);
    // Face 2
    nodes[4] = cvf::Vec3d(0.5, -0.25, 0);
    nodes[5] = cvf::Vec3d(1.25, 0.5, 0);
    nodes[6] = cvf::Vec3d(0.5, 1.25, 0);
    nodes[7] = cvf::Vec3d(-0.25, 0.5, 0);
    polygon.clear();

    isOk =  GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, &edgeIntersectionStorage,
                                            wrapArrayConst(&nodes), cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( (size_t)8, polygon.size());
    EXPECT_EQ( (size_t)8, additionalVertices.size());
    EXPECT_TRUE(isOk);

   

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CellFaceIntersectionTst, FreeFacePolygon)
{

    std::vector<cvf::Vec3d> additionalVertices;
    cvf::Vec3dArray nodes;
    std::vector<size_t> polygon;


    cvf::Array<size_t> ids;
    size_t cv1CubeFaceIndices[4] = {0, 1, 2, 3};
    size_t cv2CubeFaceIndices[4] = {4, 5, 6, 7};

    nodes.resize(8);
    nodes.setAll(cvf::Vec3d(0, 0, 0));
    EdgeIntersectStorage<size_t> edgeIntersectionStorage;
    edgeIntersectionStorage.setVertexCount(nodes.size());

    // Face 1
    nodes[0] = cvf::Vec3d(0, 0, 0);
    nodes[1] = cvf::Vec3d(1, 0, 0);
    nodes[2] = cvf::Vec3d(1, 1, 0);
    nodes[3] = cvf::Vec3d(0, 1, 0);
    // Face 2
    nodes[4] = cvf::Vec3d(0, 0, 0);
    nodes[5] = cvf::Vec3d(1, 0, 0);
    nodes[6] = cvf::Vec3d(1, 1, 0);
    nodes[7] = cvf::Vec3d(0, 1, 0);


    bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, &edgeIntersectionStorage, 
        wrapArrayConst(&nodes), cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( (size_t)4, polygon.size());
    EXPECT_EQ( (size_t)0, additionalVertices.size());
    EXPECT_TRUE(isOk);

    std::vector< bool > faceOverlapPolygonWinding;
    std::vector< std::vector<size_t>* > faceOverlapPolygons;
    faceOverlapPolygons.push_back(&polygon);
    faceOverlapPolygonWinding.push_back(true);

    std::vector<size_t>         partialFacePolygon;
    bool hasHoles = false;
    GeometryTools::calculatePartiallyFreeCubeFacePolygon(
        wrapArrayConst(&nodes),
        wrapArrayConst(cv1CubeFaceIndices, 4),
        Vec3d(0,0,1),
        faceOverlapPolygons,
        faceOverlapPolygonWinding,
        &partialFacePolygon,
        &hasHoles);

    // Face 1
    nodes[0] = cvf::Vec3d(0, 0, 0);
    nodes[1] = cvf::Vec3d(1, 0, 0);
    nodes[2] = cvf::Vec3d(1, 1, 0);
    nodes[3] = cvf::Vec3d(0, 1, 0);
    // Face 2
    nodes[4] = cvf::Vec3d(0.5, -0.25, 0);
    nodes[5] = cvf::Vec3d(1.25, 0.5, 0);
    nodes[6] = cvf::Vec3d(0.5, 1.25, 0);
    nodes[7] = cvf::Vec3d(-0.25, 0.5, 0);
    polygon.clear();

    isOk =  GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, &edgeIntersectionStorage, 
            wrapArrayConst(&nodes), cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( (size_t)8, polygon.size());
    EXPECT_EQ( (size_t)8, additionalVertices.size());
    EXPECT_TRUE(isOk);



}
