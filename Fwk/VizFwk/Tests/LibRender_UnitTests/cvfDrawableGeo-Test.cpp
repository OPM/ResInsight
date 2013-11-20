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
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryUtils.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUShort.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, BasicConstruction)
{
    ref<DrawableGeo> geo  = new DrawableGeo;
    ASSERT_EQ(1, geo->refCount());

    // Check default values
    ASSERT_EQ(DrawableGeo::VERTEX_ARRAY, geo->renderMode());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, ShallowCopy)
{
    ref<Vec3fArray> verts = new Vec3fArray;
    ref<Vec3fArray> norms = new Vec3fArray;

    ref<DrawableGeo> geo  = new DrawableGeo;
    geo->setVertexArray(verts.p());
    geo->setNormalArray(norms.p());

    ref<DrawableGeo> cpy = geo->shallowCopy();
    EXPECT_EQ(verts.p(), cpy->vertexArray());
    EXPECT_EQ(norms.p(), cpy->normalArray());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, RenderModeTest)
{
    DrawableGeo geo;

    geo.setRenderMode(DrawableGeo::BUFFER_OBJECT);

    ASSERT_EQ(DrawableGeo::BUFFER_OBJECT, geo.renderMode());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, SetFromTriangleVertexArray)
{
    ref<DrawableGeo> geo  = new DrawableGeo;

    ref<Vec3fArray> verts = new Vec3fArray;
    verts->reserve(6);
    verts->add(Vec3f(1, 1, 1));
    verts->add(Vec3f(2, 1, 1));
    verts->add(Vec3f(3, 1, 1));
    verts->add(Vec3f(4, 1, 1));
    verts->add(Vec3f(5, 1, 1));
    verts->add(Vec3f(6, 1, 1));

    geo->setFromTriangleVertexArray(verts.p());

    ASSERT_EQ(1, geo->primitiveSetCount());
    const PrimitiveSet* primSet = geo->primitiveSet(0);
    EXPECT_EQ(PT_TRIANGLES, primSet->primitiveType());
    ASSERT_EQ(2, primSet->triangleCount());

    ASSERT_EQ(2, geo->faceCount());
    ASSERT_EQ(2, geo->triangleCount());

    {
        UIntArray indices;
        geo->getFaceIndices(0, &indices);
        EXPECT_EQ(0, indices[0]);
        EXPECT_EQ(1, indices[1]);
        EXPECT_EQ(2, indices[2]);
    }

    {
        UIntArray indices;
        geo->getFaceIndices(1, &indices);
        EXPECT_EQ(3, indices[0]);
        EXPECT_EQ(4, indices[1]);
        EXPECT_EQ(5, indices[2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, SetFromQuadVertexArray)
{
    ref<DrawableGeo> geo  = new DrawableGeo;

    ref<Vec3fArray> verts = new Vec3fArray;
    verts->reserve(8);
    verts->add(Vec3f(1, 1, 1));
    verts->add(Vec3f(2, 1, 1));
    verts->add(Vec3f(3, 1, 1));
    verts->add(Vec3f(4, 1, 1));
    verts->add(Vec3f(5, 1, 1));
    verts->add(Vec3f(6, 1, 1));
    verts->add(Vec3f(7, 1, 1));
    verts->add(Vec3f(8, 1, 1));

    geo->setFromQuadVertexArray(verts.p());

    ASSERT_EQ(1, geo->primitiveSetCount());
    const PrimitiveSet* primSet = geo->primitiveSet(0);
    EXPECT_EQ(PT_TRIANGLES, primSet->primitiveType());
    ASSERT_EQ(4, primSet->triangleCount());

    ASSERT_EQ(4, geo->faceCount());
    ASSERT_EQ(4, geo->triangleCount());

    {
        UIntArray indices;
        geo->getFaceIndices(0, &indices);
        EXPECT_EQ(0, indices[0]);
        EXPECT_EQ(1, indices[1]);
        EXPECT_EQ(2, indices[2]);
    }

    {
        UIntArray indices;
        geo->getFaceIndices(1, &indices);
        EXPECT_EQ(0, indices[0]);
        EXPECT_EQ(2, indices[1]);
        EXPECT_EQ(3, indices[2]);
    }

	{
		UIntArray indices;
		geo->getFaceIndices(2, &indices);
		EXPECT_EQ(4, indices[0]);
		EXPECT_EQ(5, indices[1]);
		EXPECT_EQ(6, indices[2]);
	}

	{
		UIntArray indices;
		geo->getFaceIndices(3, &indices);
		EXPECT_EQ(4, indices[0]);
		EXPECT_EQ(6, indices[1]);
		EXPECT_EQ(7, indices[2]);
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, GetFacelistFromTriangleVertexArray)
{
	ref<DrawableGeo> geo  = new DrawableGeo;
	ASSERT_EQ(1, geo->refCount());
	
	{
		// Test for valid facelist with no values
		ref<UIntArray> faceList = geo->toFaceList();
		UIntArray* faceListArray = faceList.p();
		ASSERT_TRUE(faceListArray != NULL);
		EXPECT_EQ(0u, faceListArray->size());

		const Vec3fArray* vertices = geo->vertexArray();
		ASSERT_TRUE(vertices == NULL);
	}

	ref<Vec3fArray> triangles = new Vec3fArray;
	triangles->reserve(6);
	triangles->add(Vec3f(1, 1, 1));
	triangles->add(Vec3f(2, 1, 1));
	triangles->add(Vec3f(3, 1, 1));
	triangles->add(Vec3f(4, 1, 1));
	triangles->add(Vec3f(5, 1, 1));
	triangles->add(Vec3f(6, 1, 1));

	geo->setFromTriangleVertexArray(triangles.p());

	ref<UIntArray> faceList = geo->toFaceList();
	UIntArray* faceListArray = faceList.p();

	ASSERT_TRUE(faceListArray != NULL);
	EXPECT_EQ(8, faceListArray->size());
	
	EXPECT_EQ(3, faceListArray->get(0));
	EXPECT_EQ(0, faceListArray->get(1));
	EXPECT_EQ(1, faceListArray->get(2));
	EXPECT_EQ(2, faceListArray->get(3));
	EXPECT_EQ(3, faceListArray->get(4));
	EXPECT_EQ(3, faceListArray->get(5));
	EXPECT_EQ(4, faceListArray->get(6));
	EXPECT_EQ(5, faceListArray->get(7));

	const Vec3fArray* vertices = geo->vertexArray();
	ASSERT_TRUE(vertices != NULL);
	EXPECT_EQ(6, vertices->size());

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, SetFromFaceListTesselation)
{
    cvf::uint fl[] =
    {
        3, 0, 1, 2,                 // 1 tri
        4, 3, 0, 1, 2,              // 2 tris
        5, 1, 2, 3, 4, 5,           // 3 tris
        6, 2, 3, 1, 5, 23, 12       // 4 tris
    };

    ref<UIntArray> faceList = new UIntArray(fl, sizeof(fl)/sizeof(cvf::uint));
    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromFaceList(*faceList);

    ASSERT_EQ(1, geo->primitiveSetCount());

    PrimitiveSetIndexedUShort* primSet = dynamic_cast<PrimitiveSetIndexedUShort*>(geo->primitiveSet(0));
    ASSERT_TRUE(primSet != NULL);

    EXPECT_EQ(PT_TRIANGLES, primSet->primitiveType());
    EXPECT_EQ(10, primSet->triangleCount());
    EXPECT_EQ(30, primSet->indexCount());

    // Tri
    EXPECT_EQ(0, primSet->index(0));   
    EXPECT_EQ(1, primSet->index(1));   
    EXPECT_EQ(2, primSet->index(2));

    // Quad
    EXPECT_EQ(3, primSet->index(3));   
    EXPECT_EQ(0, primSet->index(4));   
    EXPECT_EQ(1, primSet->index(5));   

    EXPECT_EQ(3, primSet->index(6));   
    EXPECT_EQ(1, primSet->index(7));   
    EXPECT_EQ(2, primSet->index(8));   

    // 5-Poly
    EXPECT_EQ(1, primSet->index(9));   
    EXPECT_EQ(2, primSet->index(10));   
    EXPECT_EQ(3, primSet->index(11));   

    EXPECT_EQ(1, primSet->index(12));   
    EXPECT_EQ(3, primSet->index(13));   
    EXPECT_EQ(4, primSet->index(14));   

    EXPECT_EQ(1, primSet->index(15));   
    EXPECT_EQ(4, primSet->index(16));   
    EXPECT_EQ(5, primSet->index(17));   

    // .. and last one of the 6-poly
    EXPECT_EQ(2, primSet->index(27));
    EXPECT_EQ(23, primSet->index(28));
    EXPECT_EQ(12, primSet->index(29));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, SetFromFaceListOneLargePoly)
{
    cvf::uint fl[] =
    {
        20, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19         // 18 tris        
    };

    ref<UIntArray> faceList = new UIntArray(fl, sizeof(fl)/sizeof(cvf::uint));
    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromFaceList(*faceList);

    ASSERT_EQ(1, geo->primitiveSetCount());

    PrimitiveSetIndexedUShort* primSet = dynamic_cast<PrimitiveSetIndexedUShort*>(geo->primitiveSet(0));
    ASSERT_TRUE(primSet != NULL);
    EXPECT_EQ(PT_TRIANGLES, primSet->primitiveType());
    EXPECT_EQ(18, primSet->triangleCount());
    EXPECT_EQ(54, primSet->indexCount());
    
    EXPECT_EQ(0, primSet->index(0));
    EXPECT_EQ(1, primSet->index(1));
    EXPECT_EQ(2, primSet->index(2));

    EXPECT_EQ(0, primSet->index(3));
    EXPECT_EQ(2, primSet->index(4));
    EXPECT_EQ(3, primSet->index(5));

    EXPECT_EQ(0, primSet->index(6));
    EXPECT_EQ(3, primSet->index(7));
    EXPECT_EQ(4, primSet->index(8));

    EXPECT_EQ(0, primSet->index(51));
    EXPECT_EQ(18, primSet->index(52));
    EXPECT_EQ(19, primSet->index(53));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, MergeIntoEmpty)
{
    size_t geo1VerticesSize = 0;

    size_t geo2VerticesSize = 0;

    ref<DrawableGeo> geo1 = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createBox(Vec3f(0,0,0), 2.0, 2.0, 2.0, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        geo1VerticesSize = vertices->size();
        EXPECT_EQ(24, geo1VerticesSize);

        ref<UIntArray> faceList = builder.faceList();

        geo1->setVertexArray(vertices.p());
        geo1->setFromFaceList(*faceList);
    }

    EXPECT_EQ(geo1VerticesSize, geo1->vertexCount());

    ref<DrawableGeo> geo2 = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(10, 2, 2, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        geo2VerticesSize = vertices->size();
        EXPECT_EQ(4, geo2VerticesSize);

        ref<UIntArray> faceList = builder.faceList();

        geo2->setVertexArray(vertices.p());
        geo2->setFromFaceList(*faceList);
    }

    EXPECT_EQ(geo2VerticesSize, geo2->vertexCount());

    Collection<DrawableGeo> myCollection;
    myCollection.push_back(geo1.p());
    myCollection.push_back(geo2.p());

    ref<DrawableGeo> mergedGeo = new DrawableGeo;
    mergedGeo->mergeInto(myCollection);
    EXPECT_EQ(geo1VerticesSize + geo2VerticesSize, mergedGeo->vertexCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, MergeIntoExisting)
{
    size_t geo1VerticesSize = 0;

    size_t geo2VerticesSize = 0;

    ref<DrawableGeo> geo1 = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createBox(Vec3f(0,0,0), 2.0, 2.0, 2.0, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        geo1VerticesSize = vertices->size();
        EXPECT_EQ(24, geo1VerticesSize);

        ref<UIntArray> faceList = builder.faceList();

        geo1->setVertexArray(vertices.p());
        geo1->setFromFaceList(*faceList);
    }

    EXPECT_EQ(geo1VerticesSize, geo1->vertexCount());

    ref<DrawableGeo> geo2 = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(10, 2, 2, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        geo2VerticesSize = vertices->size();
        EXPECT_EQ(4, geo2VerticesSize);

        ref<UIntArray> faceList = builder.faceList();

        geo2->setVertexArray(vertices.p());
        geo2->setFromFaceList(*faceList);
    }

    EXPECT_EQ(geo2VerticesSize, geo2->vertexCount());

    Collection<DrawableGeo> myCollection;
    myCollection.push_back(geo2.p());

    geo1->mergeInto(myCollection);
    EXPECT_EQ(geo1VerticesSize + geo2VerticesSize, geo1->vertexCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, GetFacelistFromTriangleStrip)
{
    // Tri strip
    // -------------------------------------------------------------------------
    ref<DrawableGeo> geo1 = new DrawableGeo;

    ref<PrimitiveSetIndexedUInt> triStrip = new PrimitiveSetIndexedUInt(PT_TRIANGLE_STRIP);

    ref<UIntArray> triStripIndices = new UIntArray;
    triStripIndices->reserve(6);

    triStripIndices->add(0);
    triStripIndices->add(1);
    triStripIndices->add(2);
    triStripIndices->add(3);
    triStripIndices->add(4);
    triStripIndices->add(5);

    triStrip->setIndices(triStripIndices.p());

    ref<Vec3fArray> triStripVertices = new Vec3fArray;
    triStripVertices->reserve(6);

    triStripVertices->add(Vec3f(0,1,0));
    triStripVertices->add(Vec3f(0,0,0));
    triStripVertices->add(Vec3f(1,1,0));
    triStripVertices->add(Vec3f(2.5,0,0));
    triStripVertices->add(Vec3f(3,1,0));
    triStripVertices->add(Vec3f(3.5,-0.5,0));

    geo1->addPrimitiveSet(triStrip.p());
    geo1->setVertexArray(triStripVertices.p());

    ref<UIntArray> faceList = geo1->toFaceList();
    EXPECT_EQ(4 * 4, faceList->size());

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, GetFacelistFromTriangleFan)
{
    // Tri fan
    // -------------------------------------------------------------------------
    ref<DrawableGeo> geo2 = new DrawableGeo;

    ref<PrimitiveSetIndexedUInt> triFan = new PrimitiveSetIndexedUInt(PT_TRIANGLE_FAN);

    ref<UIntArray> triFanIndices = new UIntArray;
    triFanIndices->reserve(5);

    triFanIndices->add(0);
    triFanIndices->add(1);
    triFanIndices->add(2);
    triFanIndices->add(3);
    triFanIndices->add(4);

    triFan->setIndices(triFanIndices.p());

    ref<Vec3fArray> triFanVertices = new Vec3fArray;
    triFanVertices->reserve(5);

    // Note that the example in the RedBook on page 36 (1 ed) leads to triangles with inward normal!
    triFanVertices->add(Vec3f(4,0,0));
    triFanVertices->add(Vec3f(5,-1.5,0));
    triFanVertices->add(Vec3f(5.5,0,0));
    triFanVertices->add(Vec3f(5,1.5,0));
    triFanVertices->add(Vec3f(4,2,0));

    geo2->addPrimitiveSet(triFan.p());
    geo2->setVertexArray(triFanVertices.p());

    ref<UIntArray> faceList = geo2->toFaceList();
    EXPECT_EQ(3 * 4, faceList->size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, ConvertFromUIntToUShort)
{
	ref<DrawableGeo> geo = new DrawableGeo;

	ref<PrimitiveSetIndexedUInt> triStrip = new PrimitiveSetIndexedUInt(PT_TRIANGLE_STRIP);

	ref<UIntArray> triStripIndices = new UIntArray;
	triStripIndices->reserve(6);

	triStripIndices->add(0);
	triStripIndices->add(1);
	triStripIndices->add(2);
	triStripIndices->add(3);
	triStripIndices->add(4);
	triStripIndices->add(5);

	triStrip->setIndices(triStripIndices.p());

	ref<Vec3fArray> triStripVertices = new Vec3fArray;
	triStripVertices->reserve(6);

	triStripVertices->add(Vec3f(0,1,0));
	triStripVertices->add(Vec3f(0,0,0));
	triStripVertices->add(Vec3f(1,1,0));
	triStripVertices->add(Vec3f(2.5,0,0));
	triStripVertices->add(Vec3f(3,1,0));
	triStripVertices->add(Vec3f(3.5,-0.5,0));

	geo->addPrimitiveSet(triStrip.p());
	geo->setVertexArray(triStripVertices.p());

    size_t numPrimitives = geo->primitiveSetCount();
    size_t i;
    for (i = 0; i < numPrimitives; i++)
    {
        const PrimitiveSet* primitive = geo->primitiveSet(i);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUInt*>(primitive) != NULL);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUShort*>(primitive) == NULL);
    }

    // First conversion
    geo->convertFromUIntToUShort();
    for (i = 0; i < numPrimitives; i++)
    {
        const PrimitiveSet* primitive = geo->primitiveSet(i);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUShort*>(primitive) != NULL);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUInt*>(primitive) == NULL);
    }

    // Second conversion, run same test as for first conversion
    geo->convertFromUIntToUShort();
    for (i = 0; i < numPrimitives; i++)
    {
        const PrimitiveSet* primitive = geo->primitiveSet(i);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUShort*>(primitive) != NULL);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUInt*>(primitive) == NULL);
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, ConvertFromUIntToUShortTooLargeModel)
{
    ref<DrawableGeo> geo2 = new DrawableGeo;
    GeometryBuilderFaceList builder;
    GeometryUtils::createSphere(10, 300, 300, &builder);

    ref<Vec3fArray> vertices = builder.vertices();
    ref<UIntArray> faceList = builder.faceList();

    geo2->setVertexArray(vertices.p());
    geo2->setFromFaceList(*faceList);

    size_t numPrimitives = geo2->primitiveSetCount();
    size_t i;
    for (i = 0; i < numPrimitives; i++)
    {
        const PrimitiveSet* primitive = geo2->primitiveSet(i);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUInt*>(primitive) != NULL);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUShort*>(primitive) == NULL);
    }

    // As this model has too many vertices, the conversion will not
    // change the internal primitive set type
    // Run the same test as above to verify that the types has not changed
    geo2->convertFromUIntToUShort();
    for (i = 0; i < numPrimitives; i++)
    {
        const PrimitiveSet* primitive = geo2->primitiveSet(i);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUInt*>(primitive) != NULL);
        EXPECT_TRUE(dynamic_cast<const PrimitiveSetIndexedUShort*>(primitive) == NULL);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, MergeParts)
{
    Vec3fArray* verts = new Vec3fArray;
    verts->reserve(3);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(1, 0, 0));
    verts->add(Vec3f(1, 1, 0));

    Vec3fArray* norms = new Vec3fArray;
    norms->resize(3);
    norms->set(0, Vec3f::Z_AXIS);
    norms->set(1, Vec3f::Z_AXIS);
    norms->set(2, Vec3f::Z_AXIS);

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromTriangleVertexArray(verts);
    geo->setNormalArray(norms);

    ref<DrawableGeo> geo2 = new DrawableGeo;
    geo2->setFromTriangleVertexArray(verts);
    geo2->setNormalArray(norms);

    Mat4d matrix;
    matrix.setTranslation(Vec3d(10, 20, 30));

    ref<DrawableGeo> geoMerged = new DrawableGeo;
    geoMerged->mergeInto(*geo, NULL);
    EXPECT_EQ(3, geoMerged->vertexArray()->size());

    const Vec3fArray* vertices = geoMerged->vertexArray();
    Vec3f v0 = vertices->get(0);
    EXPECT_EQ(0, v0.x());
    EXPECT_EQ(0, v0.y());
    EXPECT_EQ(0, v0.z());

    Vec3f v1 = vertices->get(1);
    EXPECT_EQ(1, v1.x());
    EXPECT_EQ(0, v1.y());
    EXPECT_EQ(0, v1.z());

    Vec3f v2 = vertices->get(2);
    EXPECT_EQ(1, v2.x());
    EXPECT_EQ(1, v2.y());
    EXPECT_EQ(0, v2.z());
    
    geoMerged->mergeInto(*geo2, &matrix);
    vertices = geoMerged->vertexArray();
    EXPECT_EQ(6, vertices->size());

    Vec3f v3 = vertices->get(3);
    EXPECT_EQ(10, v3.x());
    EXPECT_EQ(20, v3.y());
    EXPECT_EQ(30, v3.z());

    Vec3f v4 = vertices->get(4);
    EXPECT_EQ(11, v4.x());
    EXPECT_EQ(20, v4.y());
    EXPECT_EQ(30, v4.z());

    Vec3f v5 = vertices->get(5);
    EXPECT_EQ(11, v5.x());
    EXPECT_EQ(21, v5.y());
    EXPECT_EQ(30, v5.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, Transform)
{
    ref<Vec3fArray> verts = new Vec3fArray;
    verts->reserve(3);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(1, 0, 0));
    verts->add(Vec3f(1, 1, 0));

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromTriangleVertexArray(verts.p());
	BoundingBox bb0 = geo->boundingBox();
	EXPECT_TRUE(bb0.min() == Vec3d(0, 0, 0));
	EXPECT_TRUE(bb0.max() == Vec3d(1, 1, 0));

    Mat4d matrix;
    matrix.setTranslation(Vec3d(10, 20, 30));

    geo->transform(matrix);

    const Vec3fArray* vertices = geo->vertexArray();

    Vec3f v1 = vertices->get(1);
    EXPECT_EQ(11, v1.x());
    EXPECT_EQ(20, v1.y());
    EXPECT_EQ(30, v1.z());

	BoundingBox bb1 = geo->boundingBox();
	EXPECT_TRUE(bb1.min() == Vec3d(10, 20, 30));
	EXPECT_TRUE(bb1.max() == Vec3d(11, 21, 30));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, ComputeNormals)
{
    ref<Vec3fArray> verts = new Vec3fArray;
    verts->reserve(3);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(1, 0, 0));
    verts->add(Vec3f(1, 1, 0));

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromTriangleVertexArray(verts.p());
    geo->computeNormals();

    {
        const Vec3fArray* norms = geo->normalArray();
        ASSERT_TRUE(norms != NULL);
        EXPECT_EQ(3, norms->size());
    }

    verts->reserve(6);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(0, 1, 0));
    verts->add(Vec3f(0, 1, 1));
    geo->setFromTriangleVertexArray(verts.p());
    geo->computeNormals();
        
    {
        const Vec3fArray* norms = geo->normalArray();
        ASSERT_TRUE(norms != NULL);
        EXPECT_EQ(6, norms->size());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DrawableGeoTest, GetOpenGLPrimitiveConnByIndex)
{
    UIntArray faceList;
    faceList.reserve(100);

    faceList.add(3);
    faceList.add(10);
    faceList.add(11);
    faceList.add(12);

    faceList.add(3);
    faceList.add(13);
    faceList.add(14);
    faceList.add(15);

    faceList.add(4);
    faceList.add(20);
    faceList.add(21);
    faceList.add(22);
    faceList.add(23);

    faceList.add(3);
    faceList.add(30);
    faceList.add(31);
    faceList.add(32);

    DrawableGeo geo;
    geo.setFromFaceList(faceList);

    ASSERT_EQ(5, geo.faceCount());


    UIntArray conn;

    geo.getFaceIndices(0, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(10u, conn[0]);
    EXPECT_EQ(11u, conn[1]);
    EXPECT_EQ(12u, conn[2]);

    geo.getFaceIndices(1, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(13u, conn[0]);
    EXPECT_EQ(14u, conn[1]);
    EXPECT_EQ(15u, conn[2]);

    geo.getFaceIndices(2, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(20u, conn[0]);
    EXPECT_EQ(21u, conn[1]);
    EXPECT_EQ(22u, conn[2]);

	geo.getFaceIndices(3, &conn);
	ASSERT_EQ(3u, conn.size());
	EXPECT_EQ(20u, conn[0]);
	EXPECT_EQ(22u, conn[1]);
	EXPECT_EQ(23u, conn[2]);

    geo.getFaceIndices(4, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(30u, conn[0]);
    EXPECT_EQ(31u, conn[1]);
    EXPECT_EQ(32u, conn[2]);
}
