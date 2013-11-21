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

#include "gtest/gtest.h"

#include "cvfLibCore.h"
#include "cvfLibViewing.h"
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cafFixedArray.h"

#include "cvfArrayWrapperToEdit.h"
#include "cvfArrayWrapperConst.h"

#include "cvfGeometryTools.h"
#include "cvfBoundingBoxTree.h"

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
NodeType  quadNormal (const ArrayWrapperConst<NodeArrayType, NodeType>& nodeCoords,  
    const IndexType cubeFaceIndices[4] )
{
    return ( nodeCoords[cubeFaceIndices[2]] - nodeCoords[cubeFaceIndices[0]]) ^  
        ( nodeCoords[cubeFaceIndices[3]] - nodeCoords[cubeFaceIndices[1]]); 
}

#if 1

#endif
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

class QuadFaceIntersectorImplHandle
{
public:
    virtual ~QuadFaceIntersectorImplHandle() {}
    virtual bool intersect() = 0;
};

template < typename NodeArrayType, typename NodeType, typename IndicesArrayType, typename IndicesType>
class QuadFaceIntersectorImpl : public QuadFaceIntersectorImplHandle
{
public:
    QuadFaceIntersectorImpl( ArrayWrapperToEdit<NodeArrayType, NodeType> nodeArray,  ArrayWrapperToEdit<IndicesArrayType, IndicesType> indices)
        : m_nodeArray(nodeArray),
    m_indices(indices){}

 
    virtual bool intersect()
    {
        size_t nodeCount = m_nodeArray.size();
        NodeType a = m_nodeArray[0];
        IndicesType idx = m_indices[0];
        return true;
    }


private:

     ArrayWrapperToEdit<NodeArrayType, NodeType> m_nodeArray;
     ArrayWrapperToEdit<IndicesArrayType, IndicesType> m_indices;
};


class QuadFaceIntersector
{
public:
    template <typename NodeArrayType, typename NodeType, typename IndicesArrayType, typename IndicesType>
    void setup( ArrayWrapperToEdit<NodeArrayType, NodeType> nodeArray,  ArrayWrapperToEdit<IndicesArrayType, IndicesType> indices) 
    {

        m_implementation = new QuadFaceIntersectorImpl< NodeArrayType,  NodeType,  IndicesArrayType,  IndicesType>(  nodeArray,  indices);
    }

    bool intersect()  { return m_implementation->intersect(); }
private:
    QuadFaceIntersectorImplHandle * m_implementation;
};


template <typename ArrayType, typename ElmType>
void arrayWrapperConstTestFunction(const ArrayWrapperConst< ArrayType, ElmType> cinRefArray)
{
    ElmType e;
    size_t size;

    size = cinRefArray.size();
    e = cinRefArray[size-1];
    // cinRefArray[size-1] = e;
    {
        const ElmType& cre = cinRefArray[size-1];
        //ElmType& re = cinRefArray[size-1];
        //re = e;
    }
}

template <typename ArrayType, typename ElmType>
void arrayWrapperConstRefTestFunction(const ArrayWrapperConst< ArrayType, ElmType>& cinRefArray)
{
    ElmType e;
    size_t size;

    size = cinRefArray.size();
    e = cinRefArray[size-1];
    // cinRefArray[size-1] = e;
    {
        const ElmType& cre = cinRefArray[size-1];
        //ElmType& re = cinRefArray[size-1];
        //re = e;
    }
}


template <typename ArrayType, typename ElmType>
void arrayWrapperTestFunction(ArrayWrapperToEdit< ArrayType, ElmType> cinRefArray)
{
    ElmType e, e2;
    size_t size;

    size = cinRefArray.size();
    e = cinRefArray[size-1];
    e2 = cinRefArray[0];
    cinRefArray[0] = e;
    {
        const ElmType& cre = cinRefArray[size-1];
        ElmType& re = cinRefArray[size-1];
        re = e2;
    }
}

template <typename ArrayType, typename ElmType>
void arrayWrapperRefTestFunction(ArrayWrapperToEdit< ArrayType, ElmType>& cinRefArray)
{
    ElmType e, e2;
    size_t size;

    size = cinRefArray.size();
    e = cinRefArray[size-1];
    e2 = cinRefArray[0];
    cinRefArray[0] = e;
    {
        const ElmType& cre = cinRefArray[size-1];
        ElmType& re = cinRefArray[size-1];
        re = e2;
    }
}

std::ostream& operator<< (std::ostream& stream, cvf::Vec3d v)
{
    stream << v[0] << " " << v[1] << " " << v[2] ;
    return stream;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayWrapperTest, AllSpecializations)
{
    std::vector<cvf::Vec3d> vec3dStdVector;
    vec3dStdVector.push_back(Vec3d::ZERO);
    vec3dStdVector.push_back(Vec3d(1,1,1));

    const std::vector<cvf::Vec3d> &cvec3dStdVector = vec3dStdVector;

    cvf::Vec3dArray vec3dCvfArray(vec3dStdVector);
    const cvf::Vec3dArray& cvec3dCvfArray = vec3dCvfArray;

    cvf::Array<size_t> siztCvfArray(2);
    siztCvfArray[0] = 0;
    siztCvfArray[1] = 1;
    const cvf::Array<size_t>& csiztCvfArray = siztCvfArray;

    cvf::Array<uint> uintCvfArray(2);
    uintCvfArray[0] = 0;
    uintCvfArray[1] = 1;
    const cvf::Array<uint>& cuintCvfArray = uintCvfArray;
    
    size_t siztBarePtrArray[2] = {0, 1};

    size_t* siztBarePtr = new size_t[2];
    siztBarePtr[0] = 0;
    siztBarePtr[1] = 1;

    const size_t* csiztBarePtr = siztBarePtr;

    cvf::uint* uintBarePtr = new cvf::uint[2];
    uintBarePtr[0] = 0;
    uintBarePtr[1] = 1;
    const cvf::uint* cuintBarePtr = uintBarePtr;
    
    double*  doubleBarePtr = new double[2];
    doubleBarePtr[0] = 0;
    doubleBarePtr[1] = 1;
    const double* cdoubleBarePtr = doubleBarePtr;

    arrayWrapperConstTestFunction(wrapArrayConst(&vec3dStdVector));
    arrayWrapperConstTestFunction(wrapArrayConst(&cvec3dStdVector));
    arrayWrapperConstTestFunction(wrapArrayConst(&vec3dCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(&cvec3dCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(&uintCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(&cuintCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(siztBarePtrArray, 2));
    arrayWrapperConstTestFunction(wrapArrayConst(siztBarePtr, 2));
    arrayWrapperConstTestFunction(wrapArrayConst(csiztBarePtr, 2));
    arrayWrapperConstTestFunction(wrapArrayConst(doubleBarePtr,2));
    arrayWrapperConstTestFunction(wrapArrayConst(cdoubleBarePtr, 2));

    arrayWrapperConstRefTestFunction(wrapArrayConst(&vec3dStdVector));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&cvec3dStdVector));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&vec3dCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&cvec3dCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&uintCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&cuintCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(siztBarePtrArray, 2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(siztBarePtr, 2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(csiztBarePtr, 2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(doubleBarePtr,2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(cdoubleBarePtr, 2));

    arrayWrapperTestFunction(wrapArrayToEdit(&vec3dStdVector));
    //arrayWrapperTestFunction3(wrapArray(&cvec3dStdVector));
    EXPECT_EQ(Vec3d::ZERO, vec3dStdVector[1]);
    EXPECT_EQ(Vec3d(1,1,1), vec3dStdVector[0]);

    arrayWrapperTestFunction(wrapArrayToEdit(&vec3dCvfArray));
    EXPECT_EQ(Vec3d::ZERO, vec3dCvfArray[1]);
    EXPECT_EQ(Vec3d(1,1,1), vec3dStdVector[0]);
    //arrayWrapperTestFunction3(wrapArray(&cvec3dCvfArray));
    arrayWrapperTestFunction(wrapArrayToEdit(&uintCvfArray));
    //arrayWrapperTestFunction3(wrapArray(&cuintCvfArray));
    arrayWrapperTestFunction(wrapArrayToEdit(siztBarePtrArray, 2));
    //arrayWrapperTestFunction3(wrapArray(csiztBarePtr, 2));
    arrayWrapperTestFunction(wrapArrayToEdit(doubleBarePtr,2));
    //arrayWrapperTestFunction3(wrapArray(cdoubleBarePtr, 2));
    EXPECT_EQ(0.0, doubleBarePtr[1]);
    EXPECT_EQ(1.0, doubleBarePtr[0]);

    arrayWrapperRefTestFunction(wrapArrayToEdit(&vec3dStdVector));
    EXPECT_EQ(Vec3d::ZERO, vec3dStdVector[0]);
    EXPECT_EQ(Vec3d(1,1,1), vec3dStdVector[1]);
    //arrayWrapperRefTestFunction3(wrapArray(&cvec3dStdVector));
    arrayWrapperRefTestFunction(wrapArrayToEdit(&vec3dCvfArray));
    EXPECT_EQ(Vec3d::ZERO, vec3dCvfArray[0]);
    EXPECT_EQ(Vec3d(1,1,1), vec3dStdVector[1]);
    //arrayWrapperRefTestFunction3(wrapArray(&cvec3dCvfArray));
    arrayWrapperRefTestFunction(wrapArrayToEdit(&uintCvfArray));
    //arrayWrapperRefTestFunction3(wrapArray(&cuintCvfArray));
    arrayWrapperRefTestFunction(wrapArrayToEdit(siztBarePtrArray, 2));
    //arrayWrapperRefTestFunction3(wrapArray(csiztBarePtr, 2));
    arrayWrapperRefTestFunction(wrapArrayToEdit(doubleBarePtr,2));
    //arrayWrapperRefTestFunction3(wrapArray(cdoubleBarePtr, 2));

    EXPECT_EQ(0.0, doubleBarePtr[0]);
    EXPECT_EQ(1.0, doubleBarePtr[1]);
}

std::ostream& operator<<(std::ostream& stream, const std::vector<size_t>& array)
{
    for (size_t i = 0; i < array.size(); ++i)
    {
        stream << array[i] << " ";
    }
    stream << std::endl;
    return stream;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTree, Intersection)
{
    cvf::BoundingBoxTree bbtree;

    std::vector<cvf::BoundingBox> bbs;
    bbs.push_back(cvf::BoundingBox(Vec3d(0,0,0), Vec3d(1,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(1,0,0), Vec3d(2,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(2,0,0), Vec3d(3,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(3,0,0), Vec3d(4,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(4,0,0), Vec3d(5,1,1)));
    bbs.push_back(cvf::BoundingBox(Vec3d(0.5,0.5,0), Vec3d(5.5,1.5,1)));


    std::vector<size_t> ids;
    ids.push_back(10);
    ids.push_back(11);
    ids.push_back(12);
    ids.push_back(13);
    ids.push_back(14);
    ids.push_back(15);

    bbtree.buildTreeFromBoundingBoxes(bbs, &ids);
    {
        std::vector<size_t> intIds;
        bbtree.findIntersections(cvf::BoundingBox(Vec3d(0.25,0.25,0.25), Vec3d(4.5,0.4,0.4)), &intIds);
        size_t numBB = intIds.size();
        EXPECT_EQ(5, numBB);
        EXPECT_EQ(intIds[4], 13);
        //std::cout << intIds;
    }
    {
        std::vector<size_t> intIds;
        bbtree.findIntersections(cvf::BoundingBox(Vec3d(0.25,0.75,0.25), Vec3d(4.5,0.8,0.4)), &intIds);
        size_t numBB = intIds.size();
        EXPECT_EQ(6, numBB);
        EXPECT_EQ(intIds[5], 15);
        //std::cout << intIds;
    }
    {
        std::vector<size_t> intIds;
        bbtree.findIntersections(cvf::BoundingBox(Vec3d(2,0,0), Vec3d(3,1,1)), &intIds);
        size_t numBB = intIds.size();
        EXPECT_EQ(4, numBB);
        EXPECT_EQ(intIds[0], 11);
        //std::cout << intIds;
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
    EdgeIntersectStorage edgeIntersectionStorage;
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

 
    bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, edgeIntersectionStorage, nodes, cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( 4, polygon.size());
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

    isOk =  GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, edgeIntersectionStorage, nodes, cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( 8, polygon.size());
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
    EdgeIntersectStorage edgeIntersectionStorage;
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


    bool isOk = GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, edgeIntersectionStorage, nodes, cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( 4, polygon.size());
    EXPECT_EQ( (size_t)0, additionalVertices.size());
    EXPECT_TRUE(isOk);


    //GeometryTools::calculatePartiallyFreeCubeFacePolygon(nodes,  );

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

    isOk =  GeometryTools::calculateOverlapPolygonOfTwoQuads(&polygon, &additionalVertices, edgeIntersectionStorage, nodes, cv1CubeFaceIndices, cv2CubeFaceIndices, 1e-6);
    EXPECT_EQ( 8, polygon.size());
    EXPECT_EQ( (size_t)8, additionalVertices.size());
    EXPECT_TRUE(isOk);



}
