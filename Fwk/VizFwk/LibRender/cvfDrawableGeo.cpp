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
#include "cvfOpenGL.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfPrimitiveSetIndexedUIntScoped.h"
#include "cvfPrimitiveSetIndexedUShortScoped.h"
#include "cvfVertexWelder.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfShaderProgram.h"
#include "cvfRay.h"
#include "cvfBufferObjectManaged.h"
#include "cvfVertexBundle.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::DrawableGeo
/// \ingroup Render
///
/// A polygon based drawable.
/// 
/// This class stores vertices (nodes, coordinates), normals and a collection of PrimitiveSet to
/// describe the geometry. Each PrimitiveSet describes an array of equal primitives (POINTS, LINES or TRIS).
/// 
/// The geometry can be rendered in immediate mode (glBegin..), with vertex arrays or using VBO.  
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DrawableGeo::DrawableGeo()
:   m_vertexBundle(new VertexBundle),
    m_renderMode(VERTEX_ARRAY)
{
}


//--------------------------------------------------------------------------------------------------
/// Deletes all OpenGL resources created by this drawable geo
//--------------------------------------------------------------------------------------------------
DrawableGeo::~DrawableGeo()
{
    releaseBufferObjectsGPU();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> DrawableGeo::shallowCopy() const
{
    ref<DrawableGeo> newGeo = new DrawableGeo;

    // Replace bundle in new drawable with a copy of our own
    newGeo->m_vertexBundle = m_vertexBundle->shallowCopy();

    size_t numPrimSets = m_primitiveSets.size();
    size_t i;
    for (i = 0; i < numPrimSets; i++)
    {
        newGeo->m_primitiveSets.push_back(const_cast<PrimitiveSet*>(m_primitiveSets.at(i)));
    }

    newGeo->m_boundingBox = m_boundingBox;
    newGeo->m_renderMode = m_renderMode;

    return newGeo;
}


//--------------------------------------------------------------------------------------------------
/// Main shader based rendering path for the geometry 
/// 
/// Will render from either client vertex arrays or buffer objects (VBOs). 
/// 
/// \note    In order to get rendering from buffer objects, the buffer objects must already be created
///          and uploaded through a previous call to createUploadBufferObjectsGPU().
/// \warning Requires at least OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void DrawableGeo::render(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState&)
{
    // This is shader based path so...
    CVF_TIGHT_ASSERT(ShaderProgram::supportedOpenGL(oglContext));

    size_t numPrimitiveSets = m_primitiveSets.size();
    if (numPrimitiveSets == 0 || m_vertexBundle->vertexCount() == 0)
    {
        return;
    }

    VertexBundleUsage bundleUsage;
    m_vertexBundle->useBundle(oglContext, &bundleUsage, shaderProgram);

    size_t i;
    for (i = 0; i < numPrimitiveSets; i++)
    {
        PrimitiveSet* primSet = m_primitiveSets[i].p();
        CVF_TIGHT_ASSERT(primSet);
        primSet->render(oglContext);
    }

    CVF_CHECK_OGL(oglContext);

    m_vertexBundle->finishUseBundle(oglContext, &bundleUsage);
}


//--------------------------------------------------------------------------------------------------
/// Render the geometry using fixed function style of specifying the arrays. 
///
/// The main difference between this render path and that of render() is that this path will
/// specify all client arrays or buffer objects using 'old style' glVertexArray(), glNormalArray() etc
/// 
/// \warning Requires at least OpenGL 1.5 since it uses buffer objects.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::renderFixedFunction(OpenGLContext* oglContext, const MatrixState&)
{
    CVF_ASSERT(BufferObjectManaged::supportedOpenGL(oglContext));

#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else

    size_t numPrimitiveSets = m_primitiveSets.size();
    if (numPrimitiveSets == 0 || m_vertexBundle->vertexCount() == 0)
    {
        return;
    }

    // Setup good old 'vertex arrays' for use with fixed function drawing
    VertexBundleUsage bundleUsage;
    m_vertexBundle->useBundleFixedFunction(oglContext, &bundleUsage);

    size_t i;
    for (i = 0; i < numPrimitiveSets; i++)
    {
        PrimitiveSet* primSet = m_primitiveSets[i].p();
        CVF_TIGHT_ASSERT(primSet);
        primSet->render(oglContext);
    }

    CVF_CHECK_OGL(oglContext);

    m_vertexBundle->finishUseBundle(oglContext, &bundleUsage);

#endif // CVF_OPENGL_ES
    }


//--------------------------------------------------------------------------------------------------
/// Create any needed buffer objects and upload data to the GPU
///
/// Buffer objects are created only if renderMode is set to VBO.
/// 
/// \warning Requires at least OpenGL 1.5
//--------------------------------------------------------------------------------------------------
void DrawableGeo::createUploadBufferObjectsGPU(OpenGLContext* oglContext)
{
    CVF_TIGHT_ASSERT(oglContext);
    CVF_TIGHT_ASSERT(BufferObjectManaged::supportedOpenGL(oglContext));

    if (m_renderMode != BUFFER_OBJECT)
    {
        return;
    }

    // Upload all data in the bundle
    m_vertexBundle->createUploadBufferObjectsGPU(oglContext);

    // Do the primitive sets separately
    size_t numPrimitiveSets = m_primitiveSets.size();
    for (size_t i = 0; i < numPrimitiveSets; i++)
    {
        PrimitiveSet* prim = m_primitiveSets[i].p();
        prim->createUploadBufferObjectsGPU(oglContext);
    }
}


//--------------------------------------------------------------------------------------------------
/// Releases all buffer objects (BOs) held by this drawable
///
/// \warning The OpenGL context in which the resources were created or a context that is being
///          shared must be current in the calling thread.
/// \warning In order to assure that the actual OpenGL resources get deleted, you must call 
///          OpenGLResourceManager::deleteOrphanedManagedBufferObjects() afterwards.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::releaseBufferObjectsGPU()
{
    m_vertexBundle->releaseBufferObjectsGPU();

    // Release all buffer objects in primitive sets
    size_t numPrimitiveSets = m_primitiveSets.size();
    for (size_t i = 0; i < numPrimitiveSets; i++)
    {
        PrimitiveSet* prim = m_primitiveSets[i].p();
        CVF_TIGHT_ASSERT(prim);
        prim->releaseBufferObjectsGPU();
    }
}


//--------------------------------------------------------------------------------------------------
/// Do immediate mode rendering
//--------------------------------------------------------------------------------------------------
void DrawableGeo::renderImmediateMode(OpenGLContext* oglContext, const MatrixState&)
{
#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
    CVF_ASSERT(oglContext);

    const Vec3fArray* vertexArr = m_vertexBundle->vertexArray();
    const Vec3fArray* normalArr = m_vertexBundle->normalArray();
    const Vec2fArray* texCoordArr = m_vertexBundle->textureCoordArray();
    const Color3ubArray* colorArr = m_vertexBundle->colorArray();

    size_t numPrimitiveSets = m_primitiveSets.size();
    size_t ip;
    for (ip = 0; ip < numPrimitiveSets; ip++)
    {
        const PrimitiveSet* primitiveSet = m_primitiveSets.at(ip);
        CVF_TIGHT_ASSERT(primitiveSet);

        glBegin(primitiveSet->primitiveTypeOpenGL());

        size_t numIndices = primitiveSet->indexCount();
        size_t i;
        for (i = 0; i < numIndices; i++)
        {
            uint index = primitiveSet->index(i);

            if (normalArr)
            {
                glNormal3fv((const float*)&normalArr->get(index));
            }

            if (colorArr)
            {
                glColor3ubv((const ubyte*)&colorArr->get(index));
            }

            if (texCoordArr)
            {
                glTexCoord2fv((const float*)&texCoordArr->get(index));
            }

            glVertex3fv((float*)&vertexArr->get(index));
        }

        glEnd();
    }

#endif // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// Get the number of vertices (nodes, points) in the drawable
//--------------------------------------------------------------------------------------------------
size_t DrawableGeo::vertexCount() const
{
    return m_vertexBundle->vertexCount();
}


//--------------------------------------------------------------------------------------------------
/// Get the number of triangles in the drawable. A quad is not 2 triangles.
//--------------------------------------------------------------------------------------------------
size_t DrawableGeo::triangleCount() const
{
    size_t count = 0;
    size_t numPrimitiveObjects = m_primitiveSets.size();

    size_t i;
    for (i = 0; i < numPrimitiveObjects; i++)
    {
        const PrimitiveSet* prim = m_primitiveSets[i].p();
        CVF_ASSERT(prim);

        count += prim->triangleCount();
    }

    return count;
}


//--------------------------------------------------------------------------------------------------
/// Get the total number of OpenGL primitives (sum of lines, points, quads, etc.) 
//--------------------------------------------------------------------------------------------------
size_t DrawableGeo::faceCount() const
{
    size_t count = 0;
    size_t numPrimitiveObjects = m_primitiveSets.size();

    size_t i;
    for (i = 0; i < numPrimitiveObjects; i++)
    {
        const PrimitiveSet* prim = m_primitiveSets[i].p();
        CVF_ASSERT(prim);

        count += prim->faceCount();
    }

    return count;
}


//--------------------------------------------------------------------------------------------------
/// Set the render mode (immediate, vertex array or VBO) to use when rendering the geometry
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setRenderMode(RenderMode renderMode)
{
    if (m_renderMode != renderMode)
    {
        m_renderMode = renderMode;
        releaseBufferObjectsGPU();
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the current render mode used to draw the geometry.
//--------------------------------------------------------------------------------------------------
DrawableGeo::RenderMode DrawableGeo::renderMode() const
{
    return m_renderMode;
}


//--------------------------------------------------------------------------------------------------
/// Set the vertices (node coordinates) of this geometry. 
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setVertexArray(Vec3fArray* vertexArray)
{
    m_vertexBundle->setVertexArray(vertexArray);
    recomputeBoundingBox();
}


//--------------------------------------------------------------------------------------------------
/// Set the per node normals of this geometry
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setNormalArray(Vec3fArray* normalArray)
{
    m_vertexBundle->setNormalArray(normalArray);
}


//--------------------------------------------------------------------------------------------------
/// Set the per node colors of this geometry
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setColorArray(Color3ubArray* colorArray)
{
    m_vertexBundle->setColorArray(colorArray);
}


//--------------------------------------------------------------------------------------------------
/// Returns the vertices (node coordinates) of this geometry
//--------------------------------------------------------------------------------------------------
const Vec3fArray* DrawableGeo::vertexArray() const
{
    return m_vertexBundle->vertexArray();
}


//--------------------------------------------------------------------------------------------------
/// Returns pointer to the normal array for this geometry
//--------------------------------------------------------------------------------------------------
const Vec3fArray* DrawableGeo::normalArray() const
{
    return m_vertexBundle->normalArray();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setTextureCoordArray(Vec2fArray* textureCoordArray)
{
    m_vertexBundle->setTextureCoordArray(textureCoordArray);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec2fArray* DrawableGeo::textureCoordArray() const
{
    return m_vertexBundle->textureCoordArray();
}


//--------------------------------------------------------------------------------------------------
/// Replace or add a vertex attribute to this DrawableGeo's vertex attribute set
/// 
/// If a vertex attribute with the same name as the incoming attribute is already present, the 
/// existing attribute will be replaced.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setVertexAttribute(VertexAttribute* vertexAttribute)
{
    m_vertexBundle->setGenericAttribute(vertexAttribute);
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of primitive sets in this geometry
//--------------------------------------------------------------------------------------------------
size_t DrawableGeo::primitiveSetCount() const
{
    return m_primitiveSets.size();
}


//--------------------------------------------------------------------------------------------------
/// Add the given primitive set to the drawable. primitives cannot be NULL.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::addPrimitiveSet(PrimitiveSet* primitiveSet)
{
    CVF_ASSERT(primitiveSet);

    m_primitiveSets.push_back(primitiveSet);
}


//--------------------------------------------------------------------------------------------------
/// Returns the primitive set at the given index
//--------------------------------------------------------------------------------------------------
const PrimitiveSet* DrawableGeo::primitiveSet(size_t index) const
{
    CVF_ASSERT(index < primitiveSetCount());
    return m_primitiveSets.at(index);
}


//--------------------------------------------------------------------------------------------------
/// Returns the primitive set at the given index
//--------------------------------------------------------------------------------------------------
PrimitiveSet* DrawableGeo::primitiveSet(size_t index) 
{
    CVF_ASSERT(index < primitiveSetCount());
    return m_primitiveSets.at(index);
}


//--------------------------------------------------------------------------------------------------
/// Get connectivity table for the specified face
/// 
/// \param indexOfFace  Index of the face being queried.
/// \param indices      Will receive the connectivity table for the specified face
//--------------------------------------------------------------------------------------------------
void DrawableGeo::getFaceIndices(size_t indexOfFace, UIntArray* indices) const
{
    CVF_ASSERT(indices);
    indices->setSizeZero();

    size_t idxFirstPolyInPrimSet = 0;

    size_t numPrimSets = m_primitiveSets.size();
    size_t ip;
    for (ip = 0; ip < numPrimSets; ip++)
    {
        const PrimitiveSet* primSet = m_primitiveSets.at(ip);

        size_t primitiveFaceCount = primSet->faceCount();
        if (indexOfFace >= idxFirstPolyInPrimSet && indexOfFace < idxFirstPolyInPrimSet + primitiveFaceCount)
        {
            size_t localPolygonIdx = indexOfFace - idxFirstPolyInPrimSet;
            primSet->getFaceIndices(localPolygonIdx, indices);

            return;
        }

        idxFirstPolyInPrimSet += primitiveFaceCount;
    }
}


//--------------------------------------------------------------------------------------------------
/// Sets the DrawableGeo object's geometry representation from a face list
/// 
/// \param faceList Face list
///
/// faceList contains number of items before each face connectivities. E.g. 3 0 1 2   3 2 3 1   3 2 1 3
///
/// \note This method will use more temporary memory than strictly needed in order to optimize 
///       performance.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setFromFaceList(const UIntArray& faceList)
{
    m_primitiveSets.clear();

    size_t numFaceListEntries = faceList.size();

    ref<UIntArray> triangleConnects = new UIntArray;
    triangleConnects->reserve(numFaceListEntries*3);        // Usually too much, but temporary and will be squeezed if kept.

    size_t i = 0;
    while (i < numFaceListEntries)
    {
        uint numConnects = faceList[i++];
        CVF_ASSERT(numConnects >= 3);

        if (numConnects == 3)
        {
            triangleConnects->add(faceList[i++]);
            triangleConnects->add(faceList[i++]);
            triangleConnects->add(faceList[i++]);
        }
        else 
        {
            size_t j;
            for (j = 0; j < numConnects - 2;  j++)
            {
                triangleConnects->add(faceList[i]);
                triangleConnects->add(faceList[i + 1 + j]);
                triangleConnects->add(faceList[i + 2 + j]);
            }

            i += numConnects;
        }
    }

    // Check if the largest index used in the triangle connects exceeds short representation
    if (triangleConnects->max() < std::numeric_limits<ushort>::max())
    {
        // Create an USHORT primitive set
        size_t arraySize = triangleConnects->size();

        ref<UShortArray> shortIndices = new UShortArray;
        shortIndices->resize(arraySize);

        size_t j;
        for (j = 0; j < arraySize; j++)
        {
            shortIndices->set(j, static_cast<ushort>(triangleConnects->get(j)));
        }

        ref<PrimitiveSetIndexedUShort> prim = new PrimitiveSetIndexedUShort(PT_TRIANGLES);
        prim->setIndices(shortIndices.p());

        m_primitiveSets.push_back(prim.p());
    }
    else
    {
        // Create a UINT primitive set
        ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt(PT_TRIANGLES);

        triangleConnects->squeeze();
        prim->setIndices(triangleConnects.p());
        m_primitiveSets.push_back(prim.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// Setup a geometry with triangles from an array of vertices (3 vertices per triangle).
/// 
/// This method sets the vertices to the passed vertexArray, and then creates one PrimitiveSet with 
/// one triangle for every three vertices in vertexArray.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setFromTriangleVertexArray(Vec3fArray* vertexArray)
{
    CVF_ASSERT(vertexArray);

    m_vertexBundle->setVertexArray(vertexArray);
    m_primitiveSets.clear();

    size_t numVertices = vertexArray->size();

    ref<UIntArray> indices = new UIntArray;
    indices->resize(numVertices);

    size_t i;
    for (i = 0; i < numVertices; i++)
    {
        indices->set(i, static_cast<uint>(i));
    }

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    prim->setIndices(indices.p());

    m_primitiveSets.push_back(prim.p());

    recomputeBoundingBox();
}


//--------------------------------------------------------------------------------------------------
/// Setup geometry with quads from an array of vertices (4 vertices per quad).
/// 
/// This method sets the vertices to the passed vertexArray, and then creates one PrimitiveSet with 
/// two triangles for every four vertices in vertexArray.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::setFromQuadVertexArray(Vec3fArray* vertexArray)
{
    CVF_ASSERT(vertexArray);

    m_vertexBundle->setVertexArray(vertexArray);
    m_primitiveSets.clear();

    size_t numVertices = vertexArray->size();
    size_t numQuads = numVertices/4;
    CVF_ASSERT(numVertices%4 == 0);

    // Two triangles per quad
    ref<UIntArray> indices = new UIntArray;
    indices->resize(numQuads*2*3);              

    size_t index = 0;
    uint i;
    for (i = 0; i < numQuads; i++)
    {
        indices->set(index++, i*4);
        indices->set(index++, i*4 + 1);
        indices->set(index++, i*4 + 2);

        indices->set(index++, i*4);
        indices->set(index++, i*4 + 2);
        indices->set(index++, i*4 + 3);
    }

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    prim->setIndices(indices.p());

    m_primitiveSets.push_back(prim.p());

    recomputeBoundingBox();
}


//--------------------------------------------------------------------------------------------------
/// Weld vertices based on vertex distance
///
/// \warning Calling this function will delete all vertex related data except the vertex positions
//--------------------------------------------------------------------------------------------------
void DrawableGeo::weldVertices(double weldDistance)
{
    size_t numVertices = m_vertexBundle->vertexCount();
    size_t numPrimSets = m_primitiveSets.size();
    if (numVertices == 0 || numPrimSets == 0)
    {
        return;
    }

    cref<Vec3fArray> sourceVertexArray = m_vertexBundle->vertexArray();
    CVF_ASSERT(sourceVertexArray.notNull());

    // Try and use bounding box to guess cell size
    // Probably needs more experimenting here
    double cellSize = m_boundingBox.radius()/100;

    VertexWelder welder;
    welder.initialize(weldDistance, cellSize, static_cast<uint>(numVertices));
    welder.reserveVertices(static_cast<uint>(numVertices));

    Collection<PrimitiveSet> newPrimSets;

    size_t ip;
    for (ip = 0; ip < numPrimSets; ip++)
    {
        const PrimitiveSet* srcPrimSet = m_primitiveSets.at(ip);
        PrimitiveType primType = srcPrimSet->primitiveType();
        size_t numIndices = srcPrimSet->indexCount();
        
        ref<UIntArray> newIndices = new UIntArray;
        newIndices->reserve(numIndices);

        size_t i;
        for (i = 0; i < numIndices; i++)
        {
            uint idx = srcPrimSet->index(i);
            Vec3f v = sourceVertexArray->get(idx);
            uint newIndexOfVertex = welder.weldVertex(v, NULL);
            newIndices->add(newIndexOfVertex);
        }

        ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(primType);
        primSet->setIndices(newIndices.p());

        newPrimSets.push_back(primSet.p());
    }

    releaseBufferObjectsGPU();

    m_vertexBundle->clear();
    m_vertexBundle->setVertexArray(welder.createVertexArray().p());
    m_primitiveSets = newPrimSets;

    recomputeBoundingBox();
}


//--------------------------------------------------------------------------------------------------
/// Merge a collection of drawable geometry objects into this drawable
/// 
/// \param drawableGeos Collection of drawable geometries to be merged
/// 
/// A new vertex array is created with the incoming vertex arrays appended to the existing contents. 
/// Primitives are copied and indices updated.
/// 
/// \warning All other vertex attribute data such as normals, texture coordinates etc will be set to NULL
//--------------------------------------------------------------------------------------------------
void DrawableGeo::mergeInto(const Collection<DrawableGeo>& drawableGeos)
{
    size_t totalVertexCount = m_vertexBundle->vertexCount();
    size_t i;
    for (i = 0; i < drawableGeos.size(); i++)
    {
        const DrawableGeo* geo = drawableGeos[i].p();
        totalVertexCount += geo->vertexCount();
    }

    // Nothing to do if no existing vertices and no new vertices
    if (totalVertexCount == 0)
    {
        return;
    }

    // Create a new vertex array and copy data from our array
    cref<Vec3fArray> oldVertexArray = m_vertexBundle->vertexArray();
    ref<Vec3fArray> newVertexArr = new Vec3fArray(totalVertexCount);
    size_t currentVertexIndex = 0;
    if (oldVertexArray.notNull() && oldVertexArray->size() > 0)
    {
        newVertexArr->copyData(*oldVertexArray, oldVertexArray->size(), 0, 0);
        currentVertexIndex = oldVertexArray->size();
    }

    // Then copy from the other drawable geos
    for (i = 0; i < drawableGeos.size(); i++)
    {
        const DrawableGeo* otherDrawable = drawableGeos[i].p();
        size_t j = 0;
        for (j = 0; j < otherDrawable->primitiveSetCount(); j++)
        {
            const PrimitiveSet* primSet = otherDrawable->primitiveSet(j);
            CVF_ASSERT(primSet);

            ref<UIntArray> indices = new UIntArray;
            indices->resize(primSet->indexCount());

            uint k;
            for (k = 0; k < primSet->indexCount(); k++)
            {
                uint val = primSet->index(k);

                val += static_cast<uint>(currentVertexIndex);
                indices->set(k, val);
            }

            ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt(primSet->primitiveType());
            prim->setIndices(indices.p());
            m_primitiveSets.push_back(prim.p());
        }

        const Vec3fArray* otherVertices = otherDrawable->vertexArray();
        CVF_ASSERT(otherVertices);

        // Append other drawable vertices vertex array and update vertex index
        newVertexArr->copyData(otherVertices->ptr(), otherVertices->size(), currentVertexIndex);
        currentVertexIndex += otherVertices->size();
    }

    // Clear all vertex attributes and set new vertex array
    m_vertexBundle->clear();
    m_vertexBundle->setVertexArray(newVertexArr.p());

    recomputeBoundingBox();
}


//--------------------------------------------------------------------------------------------------
/// Merge a drawable geometry object with this drawable possibly with transformation.
/// 
/// \param drawableGeo Drawable geometries to be merged
/// \param transformation Transformation matrix used to modify vertices
/// 
/// Vertices are converted if a transformation matrix is given.
/// Vertex arrays are appended to the merged vertex array. Primitives are copied and indices updated.
/// 
/// \warning All other vertex attribute data such as normals, texture coordinates etc will be set to NULL
//--------------------------------------------------------------------------------------------------
void DrawableGeo::mergeInto(const DrawableGeo& drawableGeo, const Mat4d* transformation)
{
    size_t totalVertexCount = m_vertexBundle->vertexCount();
    totalVertexCount += drawableGeo.vertexCount();

    // Nothing to do if no existing vertices and no new vertices
    if (totalVertexCount == 0)
    {
        return;
    }

    // Create a new vertex array and copy data from our array
    cref<Vec3fArray> oldVertexArray = m_vertexBundle->vertexArray();
    ref<Vec3fArray> newVertexArr = new Vec3fArray(totalVertexCount);
    size_t currentVertexIndex = 0;
    if (oldVertexArray.notNull() && oldVertexArray->size() > 0)
    {
        newVertexArr->copyData(*oldVertexArray, oldVertexArray->size(), 0, 0);
        currentVertexIndex = oldVertexArray->size();
    }

    // Do the primitive set
    size_t i = 0;
    for (i = 0; i < drawableGeo.primitiveSetCount(); i++)
    {
        const PrimitiveSet* primSet = drawableGeo.primitiveSet(i);
        CVF_ASSERT(primSet);

        ref<UIntArray> indices = new UIntArray;
        indices->resize(primSet->indexCount());

        uint k;
        for (k = 0; k < primSet->indexCount(); k++)
        {
            uint val = primSet->index(k);

            val += static_cast<uint>(currentVertexIndex);
            indices->set(k, val);
        }

        ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt(primSet->primitiveType());
        prim->setIndices(indices.p());
        m_primitiveSets.push_back(prim.p());
    }


    const Vec3fArray* srcVertices = drawableGeo.vertexArray();
    CVF_ASSERT(srcVertices);

    if (transformation)
    {
        size_t j;
        for (j = 0; j < srcVertices->size(); j++)
        {
            // Transform to double vector to be able to do a transform using a double matrix
            Vec3d tmpDoubleVec(srcVertices->get(j));
            tmpDoubleVec.transformPoint(*transformation);
            newVertexArr->set(currentVertexIndex, Vec3f(tmpDoubleVec));
            currentVertexIndex++;
        }
    }
    else
    {
        // Append other drawable vertices vertex array and update vertex index
        newVertexArr->copyData(*srcVertices, srcVertices->size(), currentVertexIndex, 0);
    }

    // Clear all vertex attributes and set new vertex array
    m_vertexBundle->clear();
    m_vertexBundle->setVertexArray(newVertexArr.p());

    recomputeBoundingBox();
}


//--------------------------------------------------------------------------------------------------
/// Convert indexed primitive set to unsigned short if possible
/// 
/// \return  The number of primitive sets that was converted.
//--------------------------------------------------------------------------------------------------
int DrawableGeo::convertFromUIntToUShort()
{
    int numConverted = 0;

    Collection<PrimitiveSet> myCollection;

    size_t numPrimitiveObjects = m_primitiveSets.size();
    size_t iPrim;
    for (iPrim = 0; iPrim < numPrimitiveObjects; iPrim++)
    {
        PrimitiveSet* primitive = m_primitiveSets[iPrim].p();

        PrimitiveSetIndexedUInt* primitiveSetUInt = dynamic_cast<PrimitiveSetIndexedUInt*>(primitive);
        PrimitiveSetIndexedUIntScoped* primitiveSetUIntScoped = dynamic_cast<PrimitiveSetIndexedUIntScoped*>(primitive);
        if (vertexCount() < std::numeric_limits<ushort>::max() && primitiveSetUInt)
        {
            const UIntArray* uiIndices = primitiveSetUInt->indices();
            
            ref<UShortArray> indices = new UShortArray;

            if (uiIndices)
            {
                size_t uiArraySize = uiIndices->size();
                
                indices->resize(uiArraySize);
                
                size_t j;
                for (j = 0; j < uiArraySize; j++)
                {
                    indices->set(j, static_cast<ushort>(uiIndices->get(j)));
                }                
            }
            
            ref<PrimitiveSetIndexedUShort> prim = new PrimitiveSetIndexedUShort(primitive->primitiveType());
            prim->setIndices(indices.p());

            myCollection.push_back(prim.p());
            numConverted++;
        }
        else if (vertexCount() < std::numeric_limits<ushort>::max() && primitiveSetUIntScoped)
        {
            const UIntArray* uiIndices = primitiveSetUIntScoped->indices();
            size_t uiArraySize = uiIndices->size();

            ref<UShortArray> indices = new UShortArray;
            indices->resize(uiArraySize);

            size_t j;
            for (j = 0; j < uiArraySize; j++)
            {
                indices->set(j, static_cast<ushort>(uiIndices->get(j)));
            }

            ref<PrimitiveSetIndexedUShortScoped> prim = new PrimitiveSetIndexedUShortScoped(primitive->primitiveType());
            prim->setIndices(indices.p(), primitiveSetUIntScoped->scopeFirstElement(), primitiveSetUIntScoped->scopeElementCount());

            myCollection.push_back(prim.p());
            numConverted++;
        }
        else
        {
            myCollection.push_back(primitive);
        }
    }

    m_primitiveSets.clear();
    m_primitiveSets = myCollection;

    return numConverted;
}

//--------------------------------------------------------------------------------------------------
/// Transforms all vertices in this drawable geometry using the specified matrix
/// 
/// \warning Calling this function will create a new internal vertex array.
/// \warning Normals may have to be recomputed after this function has been called.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::transform(const Mat4d& transformation)
{
    size_t numVertices = m_vertexBundle->vertexCount();
    if (numVertices == 0)
    {
        return;
    }

    cref<Vec3fArray> sourceVertexArray = m_vertexBundle->vertexArray();
    CVF_ASSERT(sourceVertexArray.notNull());

    ref<Vec3fArray> newVertexArr = new Vec3fArray(numVertices);

    size_t i;
    for (i = 0; i < numVertices; i++)
    {
        // Transform to double vector to be able to do a transform using a double matrix
        Vec3d tmpVertex(sourceVertexArray->get(i));
        tmpVertex.transformPoint(transformation);

        newVertexArr->set(i, Vec3f(tmpVertex));
    }

    m_vertexBundle->setVertexArray(newVertexArr.p());

    recomputeBoundingBox();
}


//--------------------------------------------------------------------------------------------------
/// Compute per node normals based on all primitive sets.
//--------------------------------------------------------------------------------------------------
void DrawableGeo::computeNormals()
{
    size_t numVertices = m_vertexBundle->vertexCount();
    if (numVertices == 0)
    {
        m_vertexBundle->setNormalArray(NULL);
        return;
    }

    cref<Vec3fArray> vertexArr = m_vertexBundle->vertexArray();
    CVF_ASSERT(vertexArr.notNull());

    ref<Vec3fArray> normalArr = new Vec3fArray(numVertices);
    normalArr->setAll(Vec3f::ZERO);

    size_t numPrimitiveObjects = m_primitiveSets.size();
    size_t iPrim;
    for (iPrim = 0; iPrim < numPrimitiveObjects; iPrim++)
    {
        const PrimitiveSet* prim = m_primitiveSets[iPrim].p();
        CVF_ASSERT(prim);

        PrimitiveType primType = prim->primitiveType();

        switch (primType)
        {
            case PT_TRIANGLES:
            {
                size_t indexCount = prim->indexCount();
                CVF_ASSERT(indexCount%3 == 0);

                size_t i;
                for (i = 0; i < indexCount; i += 3)
                {
                    uint a = prim->index(i + 0);
                    uint b = prim->index(i + 1);
                    uint c = prim->index(i + 2);

                    const Vec3f& v0 = vertexArr->get(a);
                    Vec3f v1 = vertexArr->get(b) - v0;
                    Vec3f v2 = vertexArr->get(c) - v0;

                    Vec3f n = v1 ^ v2;
                    n.normalize();

                    (*normalArr)[a] += n;
                    (*normalArr)[b] += n;
                    (*normalArr)[c] += n;
                }
            }
            break;

            // Normals not applicable for the following primitives
            case PT_POINTS:
            case PT_LINES:
            case PT_LINE_LOOP:
            case PT_LINE_STRIP:
            {
                break;
            }

            case PT_TRIANGLE_STRIP:
            {
                size_t indexCount = prim->indexCount();
                size_t i;
                for (i = 2; i < indexCount; i++)
                {
                    // In Tri strips, the winding flips every other triangle
                    // eg: v0,v1,v2 then v1,v3,v2, v2,v3,v4 etc.
                    // See OpenGL redbook for more info.
                    bool ccw = (i % 2 == 0);

                    uint a = prim->index(i - 2);
                    uint b = ccw ? prim->index(i - 1) : prim->index(i);
                    uint c = ccw ? prim->index(i)     : prim->index(i - 1);

                    const Vec3f& v0 = vertexArr->get(a);
                    Vec3f v1 = vertexArr->get(b) - v0;
                    Vec3f v2 = vertexArr->get(c) - v0;

                    Vec3f n = v1 ^ v2;
                    n.normalize();

                    (*normalArr)[a] += n;
                    (*normalArr)[b] += n;
                    (*normalArr)[c] += n;
                }

                break;
            }

            case PT_TRIANGLE_FAN:
            {
                size_t indexCount = prim->indexCount();
                size_t i;
                for (i = 2; i < indexCount; i++)
                {
                    uint a = prim->index(0);
                    uint b = prim->index(i - 1);
                    uint c = prim->index(i);

                    const Vec3f& v0 = vertexArr->get(a);
                    Vec3f v1 = vertexArr->get(b) - v0;
                    Vec3f v2 = vertexArr->get(c) - v0;

                    Vec3f n = v1 ^ v2;
                    n.normalize();

                    (*normalArr)[a] += n;
                    (*normalArr)[b] += n;
                    (*normalArr)[c] += n;
                }

                break;
            }

            default:
            {
                CVF_FAIL_MSG("Unhandled primitive for normal calculation");
                break;
            }
        }
    }

    size_t i;
    for (i = 0; i < normalArr->size(); i++)
    {
        Vec3f v = normalArr->get(i);
        v.normalize();
        normalArr->set(i, v);
    }

    m_vertexBundle->setNormalArray(normalArr.p());
}


//--------------------------------------------------------------------------------------------------
/// Updated the cached bounding box in this drawable geo.
/// 
/// Normally you will not need to call this function directly. Member functions that change the
/// vertex coordinates will automatically update the bounding box. The exception is if you manually
/// modify the vertices by directly manipulating the array returned by vertexArray().
//--------------------------------------------------------------------------------------------------
void DrawableGeo::recomputeBoundingBox()
{
    m_boundingBox.reset();
    if (m_vertexBundle->vertexCount() > 0)
    {
        cref<Vec3fArray> vertexArr = m_vertexBundle->vertexArray();
        CVF_ASSERT(vertexArr.notNull());
        m_boundingBox.add(*vertexArr);
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the content of this geometry as a face list.
//--------------------------------------------------------------------------------------------------
ref<UIntArray> DrawableGeo::toFaceList() const
{
    ref<UIntArray> faceList = new UIntArray;

    size_t i;
    for (i = 0; i < m_primitiveSets.size(); i++)
    {
        const PrimitiveSet* prim = m_primitiveSets[i].p();
        CVF_ASSERT(prim);

        PrimitiveType primType = prim->primitiveType();
        switch (primType)
        {
            case PT_TRIANGLES:
            {
                size_t indexCount = prim->indexCount();
                size_t newSize = faceList->size();
                newSize += (indexCount / 3) * 4;
                faceList->reserve(newSize);

                size_t i;
                for (i = 0; i < indexCount; i += 3)
                {
                    faceList->add(3);
                    faceList->add(prim->index(i + 0));
                    faceList->add(prim->index(i + 1));
                    faceList->add(prim->index(i + 2));
                }
            }
            break;

            case PT_POINTS:
            case PT_LINES:
            case PT_LINE_LOOP:
            case PT_LINE_STRIP:
            {
                break;
            }

            case PT_TRIANGLE_STRIP:
            {
                size_t indexCount = prim->indexCount();

                size_t newSize = faceList->size();
                newSize += (indexCount - 2) * 4;
                faceList->reserve(newSize);

                size_t i;
                for (i = 2; i < indexCount; i++)
                {
                    // In Tri strips, the winding flips every other triangle
                    // eg: v0,v1,v2 then v1,v3,v2, v2,v3,v4 etc.
                    // See OpenGL redbook for more info.
                    bool ccw = (i % 2 == 0);

                    uint a = prim->index(i - 2);
                    uint b = ccw ? prim->index(i - 1) : prim->index(i);
                    uint c = ccw ? prim->index(i)     : prim->index(i - 1);

                    faceList->add(3);
                    faceList->add(a);
                    faceList->add(b);
                    faceList->add(c);
                }

                break;
            }

            case PT_TRIANGLE_FAN:
            {
                size_t indexCount = prim->indexCount();

                size_t newSize = faceList->size();
                newSize += (indexCount - 2) * 4;
                faceList->reserve(newSize);

                size_t i;
                for (i = 2; i < indexCount; i++)
                {
                    uint a = prim->index(0);
                    uint b = prim->index(i - 1);
                    uint c = prim->index(i);

                    faceList->add(3);
                    faceList->add(a);
                    faceList->add(b);
                    faceList->add(c);
                }

                break;
            }

            default:
            {
                CVF_FAIL_MSG("Unhandled primitive for normal calculation");
                break;
            }
        }
    }

    return faceList;
}


//--------------------------------------------------------------------------------------------------
/// Get the bounding box
//--------------------------------------------------------------------------------------------------
BoundingBox DrawableGeo::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
/// Intersect the drawable geo with the ray and return the closest intersection point and the face hit
///
/// Returns true if anything was hit.
//--------------------------------------------------------------------------------------------------
bool DrawableGeo::rayIntersect(const Ray& ray, Vec3d* intersectionPoint, uint* faceHit) const
{
    bool anyHits = false;
    double minDistSquared = 1.0e300;
    Vec3d bestIntersectionPoint(0, 0, 0);
    size_t bestFaceHit = 0;

    cref<Vec3fArray> vertexArr = m_vertexBundle->vertexArray();

    size_t accumulatedFaceCount = 0;
    const size_t numPrimitiveSets = m_primitiveSets.size();
    for (size_t iPrimSet = 0; iPrimSet < numPrimitiveSets; iPrimSet++)
    {
        const PrimitiveSet* primSet = m_primitiveSets.at(iPrimSet);
        CVF_TIGHT_ASSERT(primSet);

        UIntArray conn;

        // Need to use signed integer type with OpenMP, so for now cast it
        CVF_ASSERT(primSet->faceCount() < static_cast<size_t>(std::numeric_limits<int>::max()));
        const int numPrimFaces = static_cast<int>(primSet->faceCount());

        #pragma omp parallel for private (conn)
        for (int i = 0; i < numPrimFaces; i++)
        {
            bool hitThisFace = false;
            Vec3d localIntersect;

            primSet->getFaceIndices(static_cast<size_t>(i), &conn);
            int numconn = static_cast<int>(conn.size());
            CVF_TIGHT_ASSERT(numconn <= 3);
            if (numconn == 3)
            {
                hitThisFace = ray.triangleIntersect(Vec3d(vertexArr->get(conn[0])), 
                                                    Vec3d(vertexArr->get(conn[1])), 
                                                    Vec3d(vertexArr->get(conn[2])), 
                                                    &localIntersect);
            }

            if (hitThisFace)
            {
                const double distSquared = (ray.origin() - localIntersect).lengthSquared();
                
                #pragma omp critical(critical_section_rayIntersect2)
                {
                    if (distSquared < minDistSquared)
                    {
                        bestIntersectionPoint = localIntersect;
                        bestFaceHit = i + accumulatedFaceCount;
                        minDistSquared = distSquared;
                    }

                    anyHits = true;
                }
            }
        } // End omp parallel for

        accumulatedFaceCount += numPrimFaces;
    }

    if (anyHits)
    {
        if (intersectionPoint)
        {
            *intersectionPoint = bestIntersectionPoint;
        }

        if (faceHit)
        {
            CVF_ASSERT(bestFaceHit < std::numeric_limits<uint>::max());
            *faceHit = static_cast<uint>(bestFaceHit);
        }

        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DrawableGeo::rayIntersect(const Ray& ray, Vec3dArray* intersectionPoints, UIntArray* facesHit) const
{
    if (intersectionPoints) intersectionPoints->setSizeZero();
    if (facesHit) facesHit->setSizeZero();

    std::vector<Vec3d> isectPts;
    std::vector<uint> faceIndices;

    cref<Vec3fArray> vertexArr = m_vertexBundle->vertexArray();

    size_t accumulatedFaceCount = 0;
    const size_t numPrimitiveSets = m_primitiveSets.size();
    for (size_t iPrimSet = 0; iPrimSet < numPrimitiveSets; iPrimSet++)
    {
        const PrimitiveSet* primSet = m_primitiveSets.at(iPrimSet);
        CVF_TIGHT_ASSERT(primSet);

        UIntArray conn;

        // Need to use signed integer type with OpenMP, so for now cast it
        CVF_ASSERT(primSet->faceCount() < static_cast<size_t>(std::numeric_limits<int>::max()));
        const int numPrimFaces = static_cast<int>(primSet->faceCount());

        #pragma omp parallel for private(conn)
        for (int i = 0; i < numPrimFaces; i++)
        {
            bool hitThisFace = false;
            Vec3d localIntersect;

            primSet->getFaceIndices(static_cast<size_t>(i), &conn);
            int numconn = static_cast<int>(conn.size());
            if (numconn == 3)
            {
                hitThisFace = ray.triangleIntersect(Vec3d(vertexArr->get(conn[0])), 
                                                    Vec3d(vertexArr->get(conn[1])), 
                                                    Vec3d(vertexArr->get(conn[2])), 
                                                    &localIntersect);
            }

            if (hitThisFace)
            {
                #pragma omp critical(critical_section_rayIntersect1)
                {
                    isectPts.push_back(localIntersect);

                    CVF_TIGHT_ASSERT(i + accumulatedFaceCount < std::numeric_limits<uint>::max());
                    faceIndices.push_back(static_cast<uint>(i + accumulatedFaceCount));
                }
            }
        }

        accumulatedFaceCount += numPrimFaces;
    }

    if (isectPts.size() > 0)
    {
        if (intersectionPoints) intersectionPoints->assign(isectPts);
        if (facesHit) facesHit->assign(faceIndices);

        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DrawableGeo::rayIntersectCreateDetail(const Ray& ray, Vec3d* intersectionPoint, ref<HitDetail>* hitDetail) const
{
    uint faceIdx = 0;
    if (rayIntersect(ray, intersectionPoint, &faceIdx))
    {
        if (hitDetail)
        {
            *hitDetail = new HitDetailDrawableGeo(faceIdx);
        }
        return true;
    }
    else
    {
        return false;
    }
}



//==================================================================================================
///
/// \class cvf::HitDetailDrawableGeo
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
HitDetailDrawableGeo::HitDetailDrawableGeo(uint faceIndex)
:   m_faceIndex(faceIndex)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint HitDetailDrawableGeo::faceIndex() const
{
    return m_faceIndex;
}


} // namespace cvf
