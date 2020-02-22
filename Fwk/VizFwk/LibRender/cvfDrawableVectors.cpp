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
#include "cvfDrawableVectors.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfBufferObjectManaged.h"
#include "cvfDrawableGeo.h"
#include "cvfQuat.h"
#include "cvfShaderProgram.h"
#include "cvfUniform.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfRay.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

namespace cvf {


//==================================================================================================
///
/// \class cvf::DrawableVectors
/// \ingroup Render
///
/// DrawableVectors implements drawing of vector arrows.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor to use when using fixed function rendering
//--------------------------------------------------------------------------------------------------
DrawableVectors::DrawableVectors()
:   m_singleColor(Color3f(Color3::WHITE)),
    m_renderWithVBO(true)
{
}


//--------------------------------------------------------------------------------------------------
///  Constructor to use when using shader based rendering
//--------------------------------------------------------------------------------------------------
DrawableVectors::DrawableVectors(String vectorMatrixUniformName, String colorUniformName)
:   m_vectorMatrixUniformName(vectorMatrixUniformName),
    m_colorUniformName(colorUniformName),
    m_singleColor(Color3f(Color3::WHITE)),
    m_renderWithVBO(true)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DrawableVectors::~DrawableVectors()
{
    releaseBufferObjectsGPU();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::setGlyph(UShortArray* triangles, Vec3fArray* vertices)
{
    m_vectorGlyph = new DrawableGeo;
    m_vectorGlyphPrimSet = new PrimitiveSetIndexedUShort(PT_TRIANGLES);

    m_vectorGlyphPrimSet->setIndices(triangles);
    m_vectorGlyph->setVertexArray(vertices);
    m_vectorGlyph->addPrimitiveSet(m_vectorGlyphPrimSet.p());
    m_vectorGlyph->computeNormals();

    if (m_renderWithVBO)
    {
        m_vectorGlyph->setRenderMode(DrawableGeo::BUFFER_OBJECT);
    }

    releaseBufferObjectsGPU();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::setSingleColor(Color3f color)
{
    m_singleColor = color;
}


void DrawableVectors::setUniformNames(String vectorMatrixUniformName, String colorUniformName)
{
    m_vectorMatrixUniformName = vectorMatrixUniformName;
    m_colorUniformName = colorUniformName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::setVectors(Vec3fArray* vertexArray, Vec3fArray* vectorArray)
{
    CVF_ASSERT(vertexArray && vectorArray);
    CVF_ASSERT(vertexArray->size() && vectorArray->size());

    m_vertexArray = vertexArray;
    m_vectorArray = vectorArray;

    m_boundingBox.add(*m_vertexArray);
}


//--------------------------------------------------------------------------------------------------
/// Set color of each corresponding vector
///
/// \param vectorColorArray Array of colors to be mapped one-by-one to the vector array
/// \param colorUniformName Name of uniform when used by a shader program. Ignored if not.
/// 
/// \sa setVectors
//--------------------------------------------------------------------------------------------------
void DrawableVectors::setColors(Color3fArray* vectorColorArray) 
{
    m_colorArray = vectorColorArray;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t DrawableVectors::vectorCount() const
{
    CVF_ASSERT(m_vertexArray->size() == m_vectorArray->size());

    return m_vertexArray->size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::render(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState&)
{
    CVF_CALLSITE_OPENGL(oglContext);

    CVF_ASSERT(shaderProgram);
    CVF_ASSERT(shaderProgram->isProgramUsed(oglContext));
    CVF_ASSERT(m_vertexArray->size() == m_vectorArray->size());
    CVF_ASSERT(m_colorArray.isNull() || (m_colorArray->size() == m_vectorArray->size()));
    CVF_ASSERT(m_vectorGlyph.notNull());
    CVF_ASSERT(m_vectorGlyph->primitiveSetCount() == 1);

    // Setup Vertex Arrays/vbos 
    const GLvoid* ptrOrOffset = 0;

    if (m_renderWithVBO && m_glyphVerticesAndNormalsBO.notNull() && m_glyphVerticesAndNormalsBO->isUploaded())
    {
        // Bind VBO for vertex and normal data
        m_glyphVerticesAndNormalsBO->bindBuffer(oglContext);

        glVertexAttribPointer(ShaderProgram::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)(sizeof(float)*3));
        glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
        glEnableVertexAttribArray(ShaderProgram::NORMAL);
        glEnableVertexAttribArray(ShaderProgram::VERTEX);

        m_indicesBO->bindBuffer(oglContext);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(ShaderProgram::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, m_vectorGlyph->normalArray()->ptr()->ptr());
        glEnableVertexAttribArray(ShaderProgram::NORMAL);
        glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, m_vectorGlyph->vertexArray()->ptr()->ptr());
        glEnableVertexAttribArray(ShaderProgram::VERTEX);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        ptrOrOffset = m_vectorGlyphPrimSet->indices()->ptr();
    }

    // Must use manual uniform setting, as setting the uniform through the Uniform* class requires a lookup for location
    // every time, and this is always the same
    GLint vectorMatrixUniformLocation = shaderProgram->uniformLocation(m_vectorMatrixUniformName.toAscii().ptr());
    CVF_ASSERT(vectorMatrixUniformLocation != -1);
    GLint colorUniformLocation = shaderProgram->uniformLocation(m_colorUniformName.toAscii().ptr());
    CVF_ASSERT(colorUniformLocation != -1);

#ifndef CVF_OPENGL_ES
    uint minIndex = m_vectorGlyphPrimSet->minIndex();
    uint maxIndex = m_vectorGlyphPrimSet->maxIndex();
#endif
    GLsizei indexCount = static_cast<GLsizei>(m_vectorGlyphPrimSet->indexCount());

    // Set the single color to use
    if (m_colorArray.isNull())
    {
        glUniform3fv(colorUniformLocation, 1, m_singleColor.ptr());
    }

    float vectorMat[16];
    size_t numVectors = m_vectorArray->size();
    size_t i;
    for (i = 0; i < numVectors; i++)
    {
        // Compute the transformation matrix
        vectorMatrix(i, vectorMat);

        // Set this as a uniform to the shader program
        glUniformMatrix4fv(vectorMatrixUniformLocation, 1, GL_FALSE, vectorMat); 

        if (m_colorArray.notNull())
        {
            glUniform3fv(colorUniformLocation, 1, m_colorArray->get(i).ptr());
        }

        // Draw the arrow
#ifdef CVF_OPENGL_ES
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, ptrOrOffset);
#else
        glDrawRangeElements(GL_TRIANGLES, minIndex, maxIndex, indexCount, GL_UNSIGNED_SHORT, ptrOrOffset);
#endif
    }

    // Cleanup
    glDisableVertexAttribArray(ShaderProgram::VERTEX);
    glDisableVertexAttribArray(ShaderProgram::NORMAL);

    CVF_CHECK_OGL(oglContext);

    // Things to consider for performance:

    // 1) Uniform arrays evt. Uniform buffer objects
    // 2) Texture
    // 3) Texture array

    // GL_ARB_draw_instanced();
    // glDrawElementsInstanced
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::renderFixedFunction(OpenGLContext* oglContext, const MatrixState& matrixState)
{
#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else    
    CVF_ASSERT(oglContext);
    CVF_ASSERT(m_vertexArray->size() == m_vectorArray->size());
    CVF_ASSERT(m_vectorGlyph.notNull());

    glEnable(GL_NORMALIZE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    if (m_colorArray.isNull())
    {
        glColor3fv(m_singleColor.ptr());
    }

    float vectorMat[16];
    size_t numVectors = m_vectorArray->size();
    size_t i;
    for (i = 0; i < numVectors; i++)
    {
        // Compute/retrieve "matrix"
        vectorMatrix(i, vectorMat);

        glPushMatrix();
        glMultMatrixf(vectorMat);

        if (m_colorArray.notNull())
        {
             glColor3fv(m_colorArray->get(i).ptr());
        }

        // Draw the geometry
        m_vectorGlyph->renderFixedFunction(oglContext, matrixState);

        glPopMatrix();
    }

    glDisable(GL_NORMALIZE);

    CVF_CHECK_OGL(oglContext);
#endif  // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::renderImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState)
{
#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else    
    CVF_ASSERT(oglContext);
    CVF_ASSERT(m_vertexArray->size() == m_vectorArray->size());
    CVF_ASSERT(m_vectorGlyph.notNull());

    glEnable(GL_NORMALIZE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    if (m_colorArray.isNull())
    {
        glColor3fv(m_singleColor.ptr());
    }

    float vectorMat[16];
    size_t numVectors = m_vectorArray->size();
    size_t i;
    for (i = 0; i < numVectors; i++)
    {
        // Compute/retrieve "matrix"
        vectorMatrix(i, vectorMat);

        glPushMatrix();
        glMultMatrixf(vectorMat);

        if (m_colorArray.notNull())
        {
            glColor3fv(m_colorArray->get(i).ptr());
        }

        // Draw the geometry
        m_vectorGlyph->renderImmediateMode(oglContext, matrixState);

        glPopMatrix();
    }

    glDisable(GL_NORMALIZE);

    CVF_CHECK_OGL(oglContext);
#endif  // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::createUploadBufferObjectsGPU(OpenGLContext* oglContext)
{
    if (!m_renderWithVBO || m_vertexArray->size() == 0 || m_vectorGlyph.isNull() || m_vectorGlyphPrimSet.isNull())
    {
        return;
    }

    if (m_glyphVerticesAndNormalsBO.isNull() || !m_glyphVerticesAndNormalsBO->isUploaded())
    {
        // Build a interleaved VBO for glyphs vertices and normals to get better performance 
        // during the main shader based draw path
        size_t numVertices = m_vectorGlyph->vertexArray()->size();

        Vec3fArray data;
        data.reserve(numVertices*2);
        size_t i;
        for (i = 0; i < numVertices; i++)
        {
            data.add(m_vectorGlyph->vertexArray()->get(i));
            data.add(m_vectorGlyph->normalArray()->get(i));
        }

        GLuint uiSizeInBytes = static_cast<GLuint>(data.size()*3*sizeof(float));
        m_glyphVerticesAndNormalsBO = oglContext->resourceManager()->getOrCreateManagedBufferObject(oglContext, GL_ARRAY_BUFFER, uiSizeInBytes, data.ptr()->ptr());
    }


    if (m_indicesBO.isNull() || !m_indicesBO->isUploaded())
    {
        const UShortArray* indices = m_vectorGlyphPrimSet->indices();
        size_t numIndices = indices->size();
        if (numIndices > 0) 
        {
            GLuint uiSizeInBytes = static_cast<GLuint>(numIndices*sizeof(GLushort));
            m_indicesBO = oglContext->resourceManager()->getOrCreateManagedBufferObject(oglContext, GL_ELEMENT_ARRAY_BUFFER, uiSizeInBytes, indices->ptr());
            CVF_CHECK_OGL(oglContext);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::releaseBufferObjectsGPU()
{
    m_glyphVerticesAndNormalsBO = NULL;

    if (m_vectorGlyph.notNull())
    {
        m_vectorGlyph->releaseBufferObjectsGPU();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t DrawableVectors::vertexCount() const
{
    if (m_vectorGlyph.isNull())
    {
        return 0;
    }

    return m_vectorGlyph->vertexCount()*vectorCount();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t DrawableVectors::triangleCount() const
{
    if (m_vectorGlyph.isNull())
    {
        return 0;
    }

    return m_vectorGlyph->triangleCount()*vectorCount();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t DrawableVectors::faceCount() const
{
    if (m_vectorGlyph.isNull())
    {
        return 0;
    }

    return m_vectorGlyph->faceCount()*vectorCount();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox DrawableVectors::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DrawableVectors::rayIntersectCreateDetail(const Ray& ray, Vec3d* intersectionPoint, ref<HitDetail>* hitDetail) const
{
    CVF_UNUSED(ray);
    CVF_UNUSED(intersectionPoint);
    CVF_UNUSED(hitDetail);

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableVectors::vectorMatrix(size_t vectorIndex, float vectorMatArray[16])
{
    const Vec3f& anchorPt = m_vertexArray->get(vectorIndex);
    Vec3f vec = m_vectorArray->get(vectorIndex);
    float length = vec.length();

    Quatf rotQuat;

    // This should really be a ratio eval based on length of vector
    // Copied from VTOVectorDrawer.cpp. TODO: refactor
    static const float TINY_VEC_COMP = 0.0000000001f;
    bool negZOnly = vec.z() < 0.0f && Math::abs(vec.x()) < TINY_VEC_COMP && Math::abs(vec.y()) < TINY_VEC_COMP;
    
    if (negZOnly) 
    {
        rotQuat.set(0.0f, 1.0f, 0.0f, 0.0f);
    }
    else
    {
        const Vec3f s = Vec3f::Z_AXIS;
        Vec3f d = vec/length;

        Vec3f v = s + d;
        Vec3f cross = v ^ d;
        float dot = v*d;

        rotQuat.set(cross.x(), cross.y(), cross.z(), dot);
    }

    float Nq = rotQuat.x()*rotQuat.x() + rotQuat.y()*rotQuat.y() + rotQuat.z()*rotQuat.z() + rotQuat.w()*rotQuat.w();
    float s = (Nq > 0.0f) ? (2.0f/Nq) : 0.0f;
    float xs = rotQuat.x()*s;	      
    float ys = rotQuat.y()*s;
    float zs = rotQuat.z()*s;
    float wx = rotQuat.w()*xs;
    float wy = rotQuat.w()*ys;
    float wz = rotQuat.w()*zs;
    float xx = rotQuat.x()*xs;
    float xy = rotQuat.x()*ys;
    float xz = rotQuat.x()*zs;
    float yy = rotQuat.y()*ys;
    float yz = rotQuat.y()*zs;
    float zz = rotQuat.z()*zs;

    vectorMatArray[0] = length*(1.0f - (yy + zz)); 
    vectorMatArray[1] = length*(xy + wz); 
    vectorMatArray[2] = length*(xz - wy);
    vectorMatArray[3] = 0.0f;

    vectorMatArray[4] = length*(xy - wz); 
    vectorMatArray[5] = length*(1.0f - (xx + zz)); 
    vectorMatArray[6] = length*(yz + wx);
    vectorMatArray[7] = 0.0f;

    vectorMatArray[8]  = length*(xz + wy); 
    vectorMatArray[9]  = length*(yz - wx); 
    vectorMatArray[10] = length*(1.0f - (xx + yy));
    vectorMatArray[11] = 0.0f;

    // Set the translation
    vectorMatArray[12] = anchorPt.x();
    vectorMatArray[13] = anchorPt.y();
    vectorMatArray[14] = anchorPt.z();
    vectorMatArray[15] = 1.0f;
}


} // namespace cvf
