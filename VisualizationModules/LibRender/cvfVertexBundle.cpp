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
#include "cvfVertexBundle.h"
#include "cvfBufferObjectManaged.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfShaderProgram.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::VertexBundle
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VertexBundle::VertexBundle()
:   m_vertexCount(0),
    m_hasNormals(false),
    m_hasTexCoords(false),
    m_hasColors(false),
    m_hasGenericAttribs(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VertexBundle::~VertexBundle()
{
}


//--------------------------------------------------------------------------------------------------
/// Make a shallow copy of this vertex bundle
///
/// Note that no buffer objects will be copied to the newly created object
//--------------------------------------------------------------------------------------------------
ref<VertexBundle> VertexBundle::shallowCopy() const
{
    ref<VertexBundle> newVB = new VertexBundle;

    newVB->m_vertexCount       = m_vertexCount;
    newVB->m_hasNormals        = m_hasNormals;
    newVB->m_hasTexCoords      = m_hasTexCoords;
    newVB->m_hasColors         = m_hasColors;
    newVB->m_hasGenericAttribs = m_hasGenericAttribs;

    // For the fixed vertex data, we pass on a pointer to the raw arrays, thus producing new vertex attribute wrappers
    // For this we must cast away the const on the arrays, but this should be OK - We're doing the same elsewhere in constshallowCopy() implementations
    newVB->setVertexArray(const_cast<Vec3fArray*>(vertexArray()));
    newVB->setNormalArray(const_cast<Vec3fArray*>(normalArray()));
    newVB->setTextureCoordArray(const_cast<Vec2fArray*>(textureCoordArray()));
    newVB->setColorArray(const_cast<Color3ubArray*>(colorArray()));
    
    // For the generic vertex attributes we just hand over the vertex attribute pointers
    size_t numGenericAttributes = m_genericAttributes.size();
    for (size_t i = 0; i < numGenericAttributes; i++)
    {
        VertexAttribute* va = const_cast<VertexAttribute*>(m_genericAttributes.at(i));
        CVF_TIGHT_ASSERT(va);
        newVB->setGenericAttribute(va);
    }

    return newVB;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t VertexBundle::vertexCount() const
{
    return m_vertexCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3fArray* VertexBundle::vertexArray() const
{
    if (m_attribVertices.notNull())
    {
        return m_attribVertices->arrayPtr();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::setVertexArray(Vec3fArray* vertexArray)
{
    m_vertexCount = 0;
    m_attribVertices = NULL;
    m_boVertices = NULL;

    if (vertexArray)
    {
        m_attribVertices = new Vec3fVertexAttribute("", vertexArray);
        m_vertexCount = vertexArray->size();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3fArray* VertexBundle::normalArray() const
{
    if (m_attribNormals.notNull())
    {
        return m_attribNormals->arrayPtr();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::setNormalArray(Vec3fArray* normalArray)
{
    m_hasNormals = false;
    m_attribNormals = NULL;
    m_boNormals = NULL;

    if (normalArray)
    {
        m_hasNormals = true;
        m_attribNormals = new Vec3fVertexAttribute("", normalArray);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec2fArray* VertexBundle::textureCoordArray() const
{
    if (m_attribTextureCoords.notNull())
    {
        return m_attribTextureCoords->arrayPtr();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::setTextureCoordArray(Vec2fArray* textureCoordArray)
{
    m_hasTexCoords = false;
    m_attribTextureCoords = NULL;
    m_boTextureCoords = NULL;

    if (textureCoordArray)
    {
        m_hasTexCoords = true;
        m_attribTextureCoords = new Vec2fVertexAttribute("", textureCoordArray);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Color3ubArray* VertexBundle::colorArray() const
{
    if (m_attribColors.notNull())
    {
        return m_attribColors->arrayPtr();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::setColorArray(Color3ubArray* colorArray)
{
    m_hasColors = false;
    m_attribColors = NULL;
    m_boColors = NULL;

    if (colorArray)
    {
        m_hasColors = true;
        m_attribColors = new Color3ubVertexAttribute("", colorArray);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t VertexBundle::genericAttributeCount() const
{
    return m_genericAttributes.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VertexAttribute* VertexBundle::genericAttribute(size_t index)
{
    CVF_TIGHT_ASSERT(index < genericAttributeCount());
    return m_genericAttributes[index].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const VertexAttribute* VertexBundle::genericAttribute(size_t index) const
{
    CVF_TIGHT_ASSERT(index < genericAttributeCount());
    return m_genericAttributes[index].p();
}


//--------------------------------------------------------------------------------------------------
/// Set or add a generic vertex attribute 
/// 
/// If a vertex attribute with the same name as the incoming attribute is already present, the 
/// existing attribute will be replaced.
//--------------------------------------------------------------------------------------------------
void VertexBundle::setGenericAttribute(VertexAttribute* vertexAttribute)
{
    CVF_ASSERT(vertexAttribute);

    // Check if attribute is already in the set
    size_t numAttribs = m_genericAttributes.size();
    size_t i;
    for(i = 0; i < numAttribs; ++i)
    {
        if (System::strcmp(m_genericAttributes[i]->name(), vertexAttribute->name()) == 0)
        {
            // Found attribute with the same name, replace it
            if (m_genericAttributes[i] != vertexAttribute)
            {
                m_genericAttributes[i] = vertexAttribute;
                m_genericBufferObjects[i] = NULL;
            }

            return;
        }
    }

    m_hasGenericAttribs = true;
    m_genericAttributes.push_back(vertexAttribute);
    m_genericBufferObjects.push_back(NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::removeGenericAttribute(const VertexAttribute* vertexAttribute)
{
    size_t idx = m_genericAttributes.indexOf(vertexAttribute);
    if (idx != UNDEFINED_SIZE_T)
    {
        m_genericAttributes.eraseAt(idx);
        m_genericBufferObjects.eraseAt(idx);

        if (m_genericAttributes.size() == 0)
        {
            m_hasGenericAttribs = false;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Clear all data in the vertex bundle
//--------------------------------------------------------------------------------------------------
void VertexBundle::clear()
{
    m_vertexCount = 0;
    m_hasNormals = false;
    m_hasTexCoords = false;
    m_hasColors = false;
    m_hasGenericAttribs = false;
    m_attribVertices = NULL;
    m_attribNormals = NULL;
    m_attribTextureCoords = NULL;
    m_attribColors = NULL;
    m_boVertices = NULL;
    m_boNormals = NULL;
    m_boTextureCoords = NULL;
    m_boColors = NULL;

    m_genericAttributes.clear();
    m_genericBufferObjects.clear();

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::createUploadBufferObjectsGPU(OpenGLContext* oglContext)
{
    CVF_TIGHT_ASSERT(oglContext);
    CVF_TIGHT_ASSERT(BufferObjectManaged::supportedOpenGL(oglContext));

    ref<OpenGLResourceManager> rcMgr = oglContext->resourceManager();

    if (m_attribVertices.notNull())
    {
        if (m_boVertices.isNull() || !m_boVertices->isUploaded())
        {
            m_boVertices = rcMgr->getOrCreateManagedBufferObject(oglContext, GL_ARRAY_BUFFER, m_attribVertices->arrayDataByteCount(), m_attribVertices->arrayDataPtrVoid());
        }
    }

    if (m_attribNormals.notNull())
    {
        if (m_boNormals.isNull() || !m_boNormals->isUploaded())
        {
            m_boNormals = rcMgr->getOrCreateManagedBufferObject(oglContext, GL_ARRAY_BUFFER, m_attribNormals->arrayDataByteCount(), m_attribNormals->arrayDataPtrVoid());
        }
    }

    if (m_attribTextureCoords.notNull())
    {
        if (m_boTextureCoords.isNull() || !m_boTextureCoords->isUploaded())
        {
            m_boTextureCoords = rcMgr->getOrCreateManagedBufferObject(oglContext, GL_ARRAY_BUFFER, m_attribTextureCoords->arrayDataByteCount(), m_attribTextureCoords->arrayDataPtrVoid());
        }
    }

    if (m_attribColors.notNull())
    {
        if (m_boColors.isNull() || !m_boColors->isUploaded())
        {
            m_boColors = rcMgr->getOrCreateManagedBufferObject(oglContext, GL_ARRAY_BUFFER, m_attribColors->arrayDataByteCount(), m_attribColors->arrayDataPtrVoid());
        }
    }

    size_t numGenericAttributes = m_genericAttributes.size();
    for (size_t i = 0; i < numGenericAttributes; i++)
    {
        const VertexAttribute* va = m_genericAttributes.at(i);
        CVF_TIGHT_ASSERT(va);

        BufferObjectManaged* bo = m_genericBufferObjects.at(i);
        if (!bo || !bo->isUploaded())
        {
            m_genericBufferObjects[i] = rcMgr->getOrCreateManagedBufferObject(oglContext, GL_ARRAY_BUFFER, va->arrayDataByteCount(), va->arrayDataPtrVoid());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::releaseBufferObjectsGPU()
{
    m_boVertices = NULL;
    m_boNormals = NULL;
    m_boTextureCoords = NULL;
    m_boColors = NULL;

    size_t numAttributes = m_genericBufferObjects.size();
    for (size_t i = 0; i < numAttributes; i++)
    {
        m_genericBufferObjects[i] = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Set up vertex attrib pointers and enable the arrays for all vertex attributes in the bundle
/// 
/// This function will set up all vertex data in the bundle using OpenGL's generic vertex attributes
/// so that it is ready to use with our shaders.
/// If buffer objects have been created and uploaded for the bundle, they will be used. Otherwise 
/// the vertex attrib pointers will be set up from client memory.
/// 
/// \warning Remember to call finishUseBundle() when finsihed drawing with this bundle.
/// \warning Requires at least OpenGL2 capability. Will assert if this condition is not met.
//--------------------------------------------------------------------------------------------------
void VertexBundle::useBundle(OpenGLContext* oglContext, VertexBundleUsage* bundleUsage, ShaderProgram* shaderProgram) const
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(bundleUsage);
    CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsOpenGL2());

    bundleUsage->setFixedFunction(false);

    const bool hasVertices  = (m_vertexCount > 0);
    const bool hasNormals   = m_hasNormals;
    const bool hasTexCoords = m_hasTexCoords;
    const bool hasColors    = m_hasColors;
    const bool hasGenAttribs= m_hasGenericAttribs;

    // Vertices
    if (hasVertices)
    {
        if (m_boVertices.notNull() && m_boVertices->isUploaded())
        {
            m_boVertices->bindBuffer(oglContext);
            glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
            //m_attribVertices->setupAttribPointerBufferObject(oglContext, ShaderProgram::VERTEX, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, m_attribVertices->arrayDataPtrVoid());
            //m_attribVertices->setupAttribPointerClientMemory(oglContext, ShaderProgram::VERTEX);
        }

        glEnableVertexAttribArray(ShaderProgram::VERTEX);
    }

    // Normals
    if (hasNormals)
    {
        if (m_boNormals.notNull() && m_boNormals->isUploaded())
        {
            m_boNormals->bindBuffer(oglContext);
            glVertexAttribPointer(ShaderProgram::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
            //m_attribNormals->setupAttribPointerBufferObject(oglContext, ShaderProgram::NORMAL, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribPointer(ShaderProgram::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, m_attribNormals->arrayDataPtrVoid());
            //m_attribNormals->setupAttribPointerClientMemory(oglContext, ShaderProgram::NORMAL);
        }

        glEnableVertexAttribArray(ShaderProgram::NORMAL);
    }

    // TextureCoords
    if (hasTexCoords)
    {
        if (m_boTextureCoords.notNull() && m_boTextureCoords->isUploaded())
        {
            m_boTextureCoords->bindBuffer(oglContext);
            m_attribTextureCoords->setupAttribPointerBufferObject(oglContext, ShaderProgram::TEX_COORD_2F_0, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            m_attribTextureCoords->setupAttribPointerClientMemory(oglContext, ShaderProgram::TEX_COORD_2F_0);
        }

        glEnableVertexAttribArray(ShaderProgram::TEX_COORD_2F_0);
    }

    // Colors
    if (hasColors)
    {
        if (m_boColors.notNull() && m_boColors->isUploaded())
        {
            m_boColors->bindBuffer(oglContext);
            m_attribColors->setupAttribPointerBufferObject(oglContext, ShaderProgram::COLOR, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            m_attribColors->setupAttribPointerClientMemory(oglContext, ShaderProgram::COLOR);
        }

        glEnableVertexAttribArray(ShaderProgram::COLOR);
    }

    // Setup the generic vertex attributes
    if (hasGenAttribs)
    {
        size_t numGenericAttributes = m_genericAttributes.size();
        for (size_t i = 0; i < numGenericAttributes; i++)
        {
            const VertexAttribute* va = m_genericAttributes.at(i);
            CVF_TIGHT_ASSERT(va);
            int attribLocationIndex = shaderProgram->attributeLocation(oglContext, va->name());
            if (attribLocationIndex >= 0)
            {
                const BufferObjectManaged* bo = m_genericBufferObjects.at(i);
                if (bo && bo->isUploaded())
                {
                    bo->bindBuffer(oglContext);
                    va->setupAttribPointerBufferObject(oglContext, static_cast<uint>(attribLocationIndex), 0, 0);
                }
                else
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    va->setupAttribPointerClientMemory(oglContext, static_cast<uint>(attribLocationIndex));
                }

                glEnableVertexAttribArray(static_cast<GLuint>(attribLocationIndex));
                bundleUsage->registerUsedGenAttrib(attribLocationIndex);
            }
        }
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// Setup vertex data using using conventional vertex arrays.
/// 
/// Will set up vertex data using the conventional (fixed function) vertex arrays such as
/// glVertexArray(), glNormalArray(), etc. Note that only the fixed fixed attributes will be set
/// up (vertices, normals, texture coordinates and colors). Generic attributes will not be set up.
//--------------------------------------------------------------------------------------------------
void VertexBundle::useBundleFixedFunction(OpenGLContext* oglContext, VertexBundleUsage* bundleUsage)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(bundleUsage);
    CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsFixedFunction());

    bundleUsage->setFixedFunction(true);

#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else

    if (m_attribVertices.notNull())
    {
        if (m_boVertices.notNull() && m_boVertices->isUploaded())
        {
            m_boVertices->bindBuffer(oglContext);
            glVertexPointer(3, GL_FLOAT, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            const GLvoid* ptrOrOffset = m_attribVertices->arrayDataPtrVoid();
            CVF_TIGHT_ASSERT(ptrOrOffset);
            glVertexPointer(3, GL_FLOAT, 0, ptrOrOffset);
        }

        glEnableClientState(GL_VERTEX_ARRAY);
    }
    
    if (m_attribNormals.notNull())
    {
        if (m_boNormals.notNull() && m_boNormals->isUploaded())
        {
            m_boNormals->bindBuffer(oglContext);
            glNormalPointer(GL_FLOAT, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            const GLvoid* ptrOrOffset = m_attribNormals->arrayDataPtrVoid();
            CVF_TIGHT_ASSERT(ptrOrOffset);
            glNormalPointer(GL_FLOAT, 0, ptrOrOffset);
        }

        glEnableClientState(GL_NORMAL_ARRAY);
    }

    if (m_attribTextureCoords.notNull())
    {
        if (m_boTextureCoords.notNull() && m_boTextureCoords->isUploaded())
        {
            m_boTextureCoords->bindBuffer(oglContext);
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            const GLvoid* ptrOrOffset = m_attribTextureCoords->arrayDataPtrVoid();
            CVF_TIGHT_ASSERT(ptrOrOffset);
            glTexCoordPointer(2, GL_FLOAT, 0, ptrOrOffset);
        }

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (m_attribColors.notNull())
    {
        if (m_boColors.notNull() && m_boColors->isUploaded())
        {
            m_boColors->bindBuffer(oglContext);
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            const GLvoid* ptrOrOffset = m_attribColors->arrayDataPtrVoid();
            CVF_TIGHT_ASSERT(ptrOrOffset);
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, ptrOrOffset);
        }
        
        glEnableClientState(GL_COLOR_ARRAY);
    }

    CVF_CHECK_OGL(oglContext);

#endif // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundle::finishUseBundle(OpenGLContext* oglContext, VertexBundleUsage* bundleUsage) const
{
    CVF_TIGHT_ASSERT(bundleUsage);

    if (bundleUsage->fixedFunction())
    {
#ifdef CVF_OPENGL_ES
        CVF_FAIL_MSG("Not supported on OpenGL ES");
#else       
        CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsFixedFunction());
        
        if (m_vertexCount > 0)  glDisableClientState(GL_VERTEX_ARRAY);
        if (m_hasNormals)       glDisableClientState(GL_NORMAL_ARRAY);
        if (m_hasTexCoords)     glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (m_hasColors)        glDisableClientState(GL_COLOR_ARRAY);
#endif
    }
    else
    {
        CVF_CALLSITE_OPENGL(oglContext);
        CVF_TIGHT_ASSERT(oglContext->capabilities()->supportsOpenGL2());

        if (m_vertexCount > 0)  glDisableVertexAttribArray(static_cast<GLuint>(ShaderProgram::VERTEX));
        if (m_hasNormals)       glDisableVertexAttribArray(static_cast<GLuint>(ShaderProgram::NORMAL));
        if (m_hasTexCoords)     glDisableVertexAttribArray(static_cast<GLuint>(ShaderProgram::TEX_COORD_2F_0));
        if (m_hasColors)        glDisableVertexAttribArray(static_cast<GLuint>(ShaderProgram::COLOR));

        if (m_hasGenericAttribs)
        {
            for (size_t i = 0; i < bundleUsage->usedGenAttribCount(); i++)
            {
                glDisableVertexAttribArray(static_cast<GLuint>(bundleUsage->usedGenAttrib(i)));
            }
        }
    }
}


//==================================================================================================
///
/// \class cvf::VertexBundleUsage
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VertexBundleUsage::VertexBundleUsage()
:   m_fixedFunction(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundleUsage::setFixedFunction(bool fixedFunction)
{
    m_fixedFunction = fixedFunction;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool VertexBundleUsage::fixedFunction() const
{
    return m_fixedFunction;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t VertexBundleUsage::usedGenAttribCount() const
{
    return m_usedGenAttribIndices.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int VertexBundleUsage::usedGenAttrib(size_t i) const
{
    return m_usedGenAttribIndices[i];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VertexBundleUsage::registerUsedGenAttrib(int attribLocationIndex)
{
    m_usedGenAttribIndices.push_back(attribLocationIndex);
}


} // namespace cvf

