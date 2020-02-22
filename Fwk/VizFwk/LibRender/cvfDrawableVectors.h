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
#include "cvfBoundingBox.h"

namespace cvf {

class DrawableGeo;
class ShaderProgram;
class BufferObjectManaged;
class PrimitiveSetIndexedUShort;

//==================================================================================================
//
// DrawableVectors implements drawing of vector arrows
//
//==================================================================================================
class DrawableVectors : public Drawable
{
public:
    DrawableVectors();
    DrawableVectors(String vectorMatrixUniformName, String colorUniformName);
    virtual ~DrawableVectors();

    void                setGlyph(UShortArray* triangles, Vec3fArray* vertices);
    void                setSingleColor(Color3f color);

    void                setUniformNames(String vectorMatrixUniformName, String colorUniformName);
    void                setVectors(Vec3fArray* vertexArray, Vec3fArray* vectorArray);
    void                setColors(Color3fArray* vectorColorArray);
    size_t              vectorCount() const;

    virtual void        render(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState& matrixState);
    virtual void        renderFixedFunction(OpenGLContext* oglContext, const MatrixState& matrixState);
    virtual void        renderImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState);

    virtual void        createUploadBufferObjectsGPU(OpenGLContext* oglContext);
    virtual void        releaseBufferObjectsGPU();

    virtual size_t      vertexCount() const;
    virtual size_t      triangleCount() const;
    virtual size_t      faceCount() const;

    virtual BoundingBox boundingBox() const;

    virtual bool        rayIntersectCreateDetail(const Ray& ray, Vec3d* intersectionPoint, ref<HitDetail>* hitDetail) const;

private:
    void                vectorMatrix(size_t vectorIndex, float vectorMatArray[16]);

private:
    String                      m_vectorMatrixUniformName;
    String                      m_colorUniformName;
    Color3f                     m_singleColor;          // Color used by all vectors if no color array is specified
    ref<Vec3fArray>             m_vertexArray;          // Positions of the vectors
    ref<Vec3fArray>             m_vectorArray;          // Direction & magnitude of the vectors.
                                                        // A vector is drawn from m_vertexArray[i] to (m_vertexArray[i] + m_vectorArray[i])
    ref<Color3fArray>           m_colorArray;

    ref<DrawableGeo>                m_vectorGlyph;          // A drawable representing one arrow with the current config
    ref<PrimitiveSetIndexedUShort>  m_vectorGlyphPrimSet;   // A drawable representing one arrow with the current config

    BoundingBox                 m_boundingBox;

    bool                        m_renderWithVBO;
    ref<BufferObjectManaged>    m_glyphVerticesAndNormalsBO; // Buffer object for holding the glyph vertices and normals for uploading data to GPU
    ref<BufferObjectManaged>    m_indicesBO;
};

}  // namespace cvf
