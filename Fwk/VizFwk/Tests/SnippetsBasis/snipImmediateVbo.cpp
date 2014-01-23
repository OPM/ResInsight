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


#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "snipImmediateVbo.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


class ImmVbo
{
public:
    ImmVbo(OpenGLContext* oglContext);
    ~ImmVbo();

    OglId           vboId() const       { return m_vboId; }
    unsigned int    lastOffset() const  { return m_lastOffset; }

    void    init(unsigned int sizeInBytes);
    void*   mapWrite(unsigned int sizeInBytes);
    void    unmap();

    void    drawGeo(const DrawableGeo& geo);
    void    drawPrimitiveSetLegacy(const PrimitiveSet& primitiveSet, const Vec3fArray& vertexArray, const Vec3fArray& normalArray);
    void    drawPrimitiveSetVbo(const PrimitiveSet& primitiveSet, const Vec3fArray& vertexArray, const Vec3fArray& normalArray);

private:
    OpenGLContext*  m_ctx;
    unsigned int    m_vboSize;
    OglId           m_vboId;
    unsigned int    m_lastOffset;
    unsigned int    m_nextOffset;
};


ImmVbo::ImmVbo(OpenGLContext* oglContext)
{
    m_ctx = oglContext;
    m_vboSize = 0;
    m_vboId = 0;
    m_lastOffset = 0;
    m_nextOffset = 0;
}


ImmVbo::~ImmVbo()
{
    CVF_CALLSITE_OPENGL(m_ctx);
    glDeleteBuffers(1, &m_vboId);
}


void ImmVbo::init(unsigned int sizeInBytes)
{
    CVF_CALLSITE_OPENGL(m_ctx);
    glGenBuffers(1, &m_vboId);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);

    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, NULL, GL_STATIC_DRAW);
    m_vboSize = sizeInBytes;
}


void* ImmVbo::mapWrite(unsigned int sizeInBytes)
{
    CVF_CALLSITE_OPENGL(m_ctx);
    unsigned int paddedSize = (sizeInBytes/64)*64;
    if (paddedSize < sizeInBytes) paddedSize += 64;

    if (paddedSize > m_vboSize)
    {
        return NULL;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);

    if (m_nextOffset + paddedSize >= m_vboSize)
    {
        glBufferData(GL_ARRAY_BUFFER, m_vboSize, NULL, GL_STATIC_DRAW);
        m_nextOffset = 0;
        m_lastOffset = 0;
    }

    void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, m_nextOffset, paddedSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    //void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, m_nextOffset, paddedSize, GL_MAP_WRITE_BIT);

    m_lastOffset = m_nextOffset;
    m_nextOffset += paddedSize;

    //void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    return ptr;
}


void ImmVbo::unmap()
{
    CVF_CALLSITE_OPENGL(m_ctx);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);

    glUnmapBuffer(GL_ARRAY_BUFFER);
}


void ImmVbo::drawGeo(const DrawableGeo& geo)
{
    const Vec3fArray* vertexArray = geo.vertexArray();
    const Vec3fArray* normalArray = geo.normalArray();

    size_t numPrimitiveSets = geo.primitiveSetCount();
    size_t iPrimSet;
    for (iPrimSet = 0; iPrimSet < numPrimitiveSets; iPrimSet++)
    {
        const PrimitiveSet* primitiveSet = geo.primitiveSet(iPrimSet);
        drawPrimitiveSetVbo(*primitiveSet, *vertexArray, *normalArray);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ImmVbo::drawPrimitiveSetLegacy(const PrimitiveSet& primitiveSet, const Vec3fArray& vertexArray, const Vec3fArray& normalArray)
{
    const PrimitiveType primType = primitiveSet.primitiveType();
    CVF_ASSERT(primType == PT_TRIANGLES);

    size_t indexCount = primitiveSet.indexCount();

    glBegin(GL_TRIANGLES);

    size_t i;
    for (i = 0; i < indexCount; i++)
    {
        unsigned int idx = primitiveSet.index(i);
        glNormal3fv((float*)&normalArray[idx]);
        glVertex3fv((float*)&vertexArray[idx]);
    }

    glEnd();
}


void ImmVbo::drawPrimitiveSetVbo(const PrimitiveSet& primitiveSet, const Vec3fArray& vertexArray, const Vec3fArray& normalArray)
{
    const PrimitiveType primType = primitiveSet.primitiveType();
    CVF_ASSERT(primType == PT_TRIANGLES);

    size_t indexCount = primitiveSet.indexCount();

    size_t sizeInBytes = indexCount*2*3*4;

    Vec3f* ptr = (Vec3f*)mapWrite(static_cast<unsigned int>(sizeInBytes));
    CVF_ASSERT(ptr);

    size_t i;
    for (i = 0; i < indexCount; i++)
    {
        unsigned int idx = primitiveSet.index(i);
        ptr[2*i] = normalArray[idx];
        ptr[2*i + 1] = vertexArray[idx];
    }

    unmap();

    size_t offset = lastOffset();
    const unsigned int stride = 6*sizeof(float);
    glNormalPointer(GL_FLOAT, stride, (void*)offset);                     
    glVertexPointer(3, GL_FLOAT, stride, (void*)(offset + 3*sizeof(float)));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

	glDrawArrays(GL_TRIANGLES, 0, (unsigned int)indexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}






//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ImmediateVbo::ImmediateVbo()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ImmediateVbo::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateBoxes(&parts);

    ref<ModelBasicList> myModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    myModel->updateBoundingBoxesRecursive();

    Rendering* mainRendering = m_renderSequence->rendering(0);
    mainRendering->scene()->addModel(myModel.p());
    
    BoundingBox bb = myModel->boundingBox();
    m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
   
    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ImmediateVbo::onPaintEvent(PostEventAction* postEventAction)
{
    TestSnippet::onPaintEvent(postEventAction);

    testDrawImmediateVbo();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ImmediateVbo::testDrawImmediateVbo()
{
    ref<DrawableGeo> geo = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(2, 20, 20, &builder);
        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        geo->setVertexArray(vertices.p());
        geo->setFromFaceList(*faceList);
        geo->computeNormals();
    }


    RenderStatePolygonMode polyMode;
    RenderStateLighting_FF lighting;
    RenderStateMaterial_FF mat(RenderStateMaterial_FF::PURE_YELLOW);
    polyMode.applyOpenGL(m_openGLContext.p());
    lighting.applyOpenGL(m_openGLContext.p());
    mat.applyOpenGL(m_openGLContext.p());

    ImmVbo iv(m_openGLContext.p());
    iv.init(4096*1024);

    iv.drawGeo(*geo);

    //geo->render();
}


} // namespace snip

