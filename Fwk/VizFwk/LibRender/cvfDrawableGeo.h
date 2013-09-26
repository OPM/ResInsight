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


#pragma once

#include "cvfDrawable.h"
#include "cvfArray.h"
#include "cvfMatrix4.h"
#include "cvfBoundingBox.h"
#include "cvfCollection.h"
#include "cvfHitDetail.h"

namespace cvf {

class VertexBundle;
class VertexAttribute;
class PrimitiveSet;



//==================================================================================================
//
// A polygon based drawable 
//
//==================================================================================================
class DrawableGeo : public Drawable
{
public:
    enum RenderMode
    {
        VERTEX_ARRAY,           ///< Rendering using client vertex arrays (copy arrays from client memory to graphics memory on each draw)
        BUFFER_OBJECT           ///< Rendering using buffer objects (arrays stored in graphics memory)
    };

public:
    DrawableGeo();
    virtual ~DrawableGeo();

    ref<DrawableGeo>    shallowCopy() const;

    virtual void        render(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState& matrixState);
    virtual void        renderFixedFunction(OpenGLContext* oglContext, const MatrixState& matrixState);
    virtual void        renderImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState);
    
    virtual void        createUploadBufferObjectsGPU(OpenGLContext* oglContext);
    virtual void        releaseBufferObjectsGPU();

    virtual size_t      vertexCount() const;
    virtual size_t      triangleCount() const;
    virtual size_t      faceCount() const;

    void                setRenderMode(RenderMode renderMode);
    RenderMode          renderMode() const;

    void                setVertexArray(Vec3fArray* vertexArray);
    void                setNormalArray(Vec3fArray* normalArray);
    void                setColorArray(Color3ubArray* colorArray);
    const Vec3fArray*   vertexArray() const;
    const Vec3fArray*   normalArray() const;

    void                setTextureCoordArray(Vec2fArray* textureCoordArray);
    const Vec2fArray*   textureCoordArray() const;

    void                setVertexAttribute(VertexAttribute* vertexAttrib);

    size_t              primitiveSetCount() const;
    void                addPrimitiveSet(PrimitiveSet* primitiveSet);
    const PrimitiveSet* primitiveSet(size_t index) const;
    PrimitiveSet*       primitiveSet(size_t index);
    int                 convertFromUIntToUShort();
    
    void                getFaceIndices(size_t indexOfFace, UIntArray* indices) const;

    void                setFromFaceList(const UIntArray& faceList);
    ref<UIntArray>      toFaceList() const;

    void                setFromTriangleVertexArray(Vec3fArray* vertexArray);
    void                setFromQuadVertexArray(Vec3fArray* vertexArray);

    void                weldVertices(double weldDistance);
    void                mergeInto(const Collection<DrawableGeo>& drawableGeos);
    void                mergeInto(const DrawableGeo& drawableGeo, const Mat4d* transformation);
    void                transform(const Mat4d& transformation);

    void                computeNormals();

    virtual BoundingBox boundingBox() const;
    void                recomputeBoundingBox();

    bool                rayIntersect(const Ray& ray, Vec3d* intersectionPoint, uint* faceHit) const;
    bool                rayIntersect(const Ray& ray, Vec3dArray* intersectionPoints, UIntArray* facesHit) const;
    virtual bool        rayIntersectCreateDetail(const Ray& ray, Vec3d* intersectionPoint, ref<HitDetail>* hitDetail) const;

private:
    ref<VertexBundle>           m_vertexBundle;     // Contains all the per-vertex data, both fixed attributes such as vertices and normals and generic attributes. Bundle always exists.
    Collection<PrimitiveSet>    m_primitiveSets;    // Collection of primitive sets with the connectivity indices
    BoundingBox                 m_boundingBox;      // Computed bounding box
    RenderMode                  m_renderMode;       // Render using client memory or buffer objects?
};



//==================================================================================================
//
// 
//
//==================================================================================================
class HitDetailDrawableGeo : public HitDetail
{
public:
    HitDetailDrawableGeo(uint faceIndex);

    uint    faceIndex() const;

private:
    uint    m_faceIndex;    // Index of the face that was hit
};


}
