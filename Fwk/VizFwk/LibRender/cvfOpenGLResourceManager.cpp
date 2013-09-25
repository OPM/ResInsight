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
#include "cvfTrace.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfBufferObjectManaged.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfOglRc.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::OpenGLResourceManager
/// \ingroup Render
///
/// OpenGL resource manager used to manage and partially track the usage of OpenGL resources
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Private constructor
//--------------------------------------------------------------------------------------------------
OpenGLResourceManager::OpenGLResourceManager()
:   m_cachedVboMemoryUsage(0)
{
}


//--------------------------------------------------------------------------------------------------
/// Private destructor.
/// 
/// Asserts that there are no active resources left.
//--------------------------------------------------------------------------------------------------
OpenGLResourceManager::~OpenGLResourceManager()
{
    // If you hit these asserts you're probably failing to clean up the OpenGL resources being used
    // The resources being managed must be deleted before the OpenGL contexts get deleted. 
    // Currently, you'll have to call deleteAllOpenGLResources() before deleting the OpenGL contexts
    CVF_ASSERT(bufferObjectMemoryUsage() == 0);
    CVF_ASSERT(bufferObjectCount() == 0);

    CVF_ASSERT(m_oglResources.size() == 0);

    CVF_ASSERT(m_textShaderProgram.isNull());
    CVF_ASSERT(m_nudgeShaderProgram.isNull());
    CVF_ASSERT(m_unlitColorShaderProgram.isNull());
    CVF_ASSERT(m_unlitTextureShaderProgram.isNull());
    CVF_ASSERT(m_vectorDrawerShaderProgram.isNull());
}


//--------------------------------------------------------------------------------------------------
/// Get existing buffer object or create a new buffer object that is initialized with the passed data.
/// 
/// The passed \a data pointer is used as the lookup key when determining if an existing buffer object 
/// can be returned or if a new buffer object must be created.
/// 
/// \return Returns NULL if memory for buffer object could not be allocated.
//--------------------------------------------------------------------------------------------------
ref<BufferObjectManaged> OpenGLResourceManager::getOrCreateManagedBufferObject(OpenGLContext* oglContext, cvfGLenum target, size_t sizeInBytes, const void* data)
{
    // Will not work with NULL pointers since the key in the map is the pointer
    CVF_ASSERT(data);
    CVF_ASSERT(sizeInBytes > 0);

    // Using the passed raw data pointer as the key, do a lookup to see if we already have a buffer object 
    DataPtrBufferObjectMapIterator it = m_dataPtrBufferObjectMap.find(data);
    if (it != m_dataPtrBufferObjectMap.end())
    {
        // We're almost ready to return the object, but we need to check the ref count first
        // If count is 1, this is an orphaned buffer that should be deleted 
        if (it->second->refCount() > 1)
        {
            return it->second;
        }
    }

    // Must create a new buffer object
    ref<BufferObjectManaged> bo = BufferObjectManaged::create(oglContext, target, sizeInBytes, data);
    if (bo.isNull())
    {
        return NULL;
    }

    // Insert the newly created buffer object into our map from raw data pointer to buffer object
    m_dataPtrBufferObjectMap[data] = bo;
    m_cachedVboMemoryUsage += bo->byteCount();

    return bo;
}


//--------------------------------------------------------------------------------------------------
/// Deletes any orphaned OpenGL buffer objects being managed by this class
/// 
/// An orphaned buffer object is an object where the resource manager is holding the only reference
/// to the object. This function acts as a garbage collector for buffer objects.
/// 
/// \warning The OpenGL context in which the resources were created or a context that is being
///          shared must be current in the calling thread.
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::deleteOrphanedManagedBufferObjects(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    m_cachedVboMemoryUsage = 0;
    
    DataPtrBufferObjectMapIterator it = m_dataPtrBufferObjectMap.begin();
    while (it != m_dataPtrBufferObjectMap.end())
    {
        if (it->second->refCount() == 1)
        {
            // Explicitly delete the buffer object (the actual OpenGL object)
            it->second->deleteBuffer(oglContext);
            m_dataPtrBufferObjectMap.erase(it++);
        }
        else
        {
            m_cachedVboMemoryUsage += it->second->byteCount();
            ++it;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Deletes all OpenGL buffer objects being managed by this class
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::deleteAllManagedBufferObjects(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    DataPtrBufferObjectMapIterator it = m_dataPtrBufferObjectMap.begin();
    while (it != m_dataPtrBufferObjectMap.end())
    {
        // Explicitly delete the buffer object (the actual OpenGL object)
        it->second->deleteBuffer(oglContext);
        ++it;
    }

    m_cachedVboMemoryUsage = 0;
    m_dataPtrBufferObjectMap.clear();
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of managed buffer objects 
//--------------------------------------------------------------------------------------------------
int OpenGLResourceManager::bufferObjectCount() const
{
    return static_cast<int>(m_dataPtrBufferObjectMap.size());
}


//--------------------------------------------------------------------------------------------------
/// Returns the total amount of memory currently used by the managed buffer objects (in bytes).
//--------------------------------------------------------------------------------------------------
size_t OpenGLResourceManager::bufferObjectMemoryUsage() const
{   
    return m_cachedVboMemoryUsage;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcShader> OpenGLResourceManager::createOglRcShader(OpenGLContext* oglContext, cvfGLenum shaderType)
{
    ref<OglRcShader> rc = OglRcShader::create(oglContext, shaderType);
    if (rc.notNull())
    {
        m_oglResources.push_back(rc.p());
        return rc;
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcProgram> OpenGLResourceManager::createOglRcProgram(OpenGLContext* oglContext)
{
    ref<OglRcProgram>  rc = OglRcProgram::create(oglContext);
    if (rc.notNull())
    {
        m_oglResources.push_back(rc.p());
        return rc;
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcTexture> OpenGLResourceManager::createOglRcTexture(OpenGLContext* oglContext)
{
    ref<OglRcTexture>  rc = OglRcTexture::create(oglContext);
    if (rc.notNull())
    {
        m_oglResources.push_back(rc.p());
        return rc;
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcRenderbuffer> OpenGLResourceManager::createOglRcRenderbuffer(OpenGLContext* oglContext)
{
    ref<OglRcRenderbuffer>  rc = OglRcRenderbuffer::create(oglContext);
    if (rc.notNull())
    {
        m_oglResources.push_back(rc.p());
        return rc;
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcFramebuffer> OpenGLResourceManager::createOglRcFramebuffer(OpenGLContext* oglContext)
{
    ref<OglRcFramebuffer>  rc = OglRcFramebuffer::create(oglContext);
    if (rc.notNull())
    {
        m_oglResources.push_back(rc.p());
        return rc;
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::deleteAllOglRcObjects(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    OglResourcesListIterator it = m_oglResources.begin();
    while (it != m_oglResources.end())
    {
        OglRc* oglRc = (*it).p();
        oglRc->deleteResource(oglContext);
        ++it;
    }

    m_oglResources.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::deleteOrphanedOglRcObjects(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    //size_t originalCount = m_oglResources.size();
    //size_t deletedCount = 0;

    OglResourcesListIterator it = m_oglResources.begin();
    while (it != m_oglResources.end())
    {
        OglRc* oglRc = (*it).p();
        if (oglRc->refCount() == 1)
        {
            // Explicitly delete the actual OpenGL resource
            oglRc->deleteResource(oglContext);
            m_oglResources.erase(it++);
            //deletedCount++;
        }
        else
        {
            ++it;
        }
    }

    //cvf::Trace::show("Deleted %ld (of %ld) OglRc instances", deletedCount, originalCount);
}


//--------------------------------------------------------------------------------------------------
/// Get ready linked shader program for text drawing
//--------------------------------------------------------------------------------------------------
ShaderProgram* OpenGLResourceManager::getLinkedTextShaderProgram(OpenGLContext* oglContext)
{
    if (m_textShaderProgram.isNull())
    {
        ShaderProgramGenerator gen("TextShaderProgram", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_MinimalTexture);
        gen.addFragmentCode(ShaderSourceRepository::fs_Text);
        m_textShaderProgram = gen.generate();
        m_textShaderProgram->linkProgram(oglContext);

        m_textShaderProgram->disableUniformTrackingForUniform("u_texture2D");
        m_textShaderProgram->disableUniformTrackingForUniform("u_color");
    }

    return m_textShaderProgram.p();
}


//--------------------------------------------------------------------------------------------------
/// Get ready linked shader program for use with 'manual' occlusion query for points
/// 
/// Shader program drawing a point size 3 point with a very light gray color. Used with Add blending
/// to detect if the point was rendered or not. (hacky-occlusion query)
//--------------------------------------------------------------------------------------------------
ShaderProgram* OpenGLResourceManager::getLinkedNudgeShaderProgram(OpenGLContext* oglContext)
{
    if (m_nudgeShaderProgram.isNull())
    {

        static char nudgeFragSource[] = 
        "void main ()                                                       \n"
        "{                                                                  \n"
        "    gl_FragColor = vec4(0.02, 0.02, 0.02, 0.02);                   \n"
        "}                                                                  \n";


        static char nudgeVertSource[] = 
        " uniform mat4 cvfu_modelViewProjectionMatrix;                      \n"
        " attribute vec4 cvfa_vertex;                                       \n"
        "                                                                   \n"
        " void main ()                                                      \n"
        " {                                                                 \n"
        "    gl_PointSize = 3;                                              \n"
        "    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;      \n"
        " }                                                                 \n";

        ShaderProgramGenerator gen("Nudge Shader", ShaderSourceProvider::instance());
        gen.addVertexCode(nudgeVertSource);
        gen.addFragmentCode(nudgeFragSource);
        m_nudgeShaderProgram = gen.generate();
        m_nudgeShaderProgram->linkProgram(oglContext);
    }

    return m_nudgeShaderProgram.p();
}


//--------------------------------------------------------------------------------------------------
/// Get ready linked shader program for unlit color drawing
//--------------------------------------------------------------------------------------------------
ShaderProgram* OpenGLResourceManager::getLinkedUnlitColorShaderProgram(OpenGLContext* oglContext)
{
    if (m_unlitColorShaderProgram.isNull())
    {
        ShaderProgramGenerator gen("UnlitColorShaderProgram", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_Minimal);
        gen.addFragmentCode(ShaderSourceRepository::src_Color);
        gen.addFragmentCode(ShaderSourceRepository::fs_Unlit);
        m_unlitColorShaderProgram = gen.generate();
        m_unlitColorShaderProgram->linkProgram(oglContext);
    }

    return m_unlitColorShaderProgram.p();
}


//--------------------------------------------------------------------------------------------------
/// Get ready linked shader program for unlit textured drawing
//--------------------------------------------------------------------------------------------------
ShaderProgram* OpenGLResourceManager::getLinkedUnlitTextureShaderProgram(OpenGLContext* oglContext)
{
    if (m_unlitTextureShaderProgram.isNull())
    {
        ShaderProgramGenerator gen("UnlitTextureShaderProgram", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_MinimalTexture);
        gen.addFragmentCode(ShaderSourceRepository::src_Texture);
        gen.addFragmentCode(ShaderSourceRepository::fs_Unlit);
        m_unlitTextureShaderProgram = gen.generate();
        m_unlitTextureShaderProgram->linkProgram(oglContext);
    }

    return m_unlitTextureShaderProgram.p();
}


//--------------------------------------------------------------------------------------------------
/// Get ready linked shader program for vector drawing
//--------------------------------------------------------------------------------------------------
ShaderProgram* OpenGLResourceManager::getLinkedVectorDrawerShaderProgram(OpenGLContext* oglContext)
{
    if (m_vectorDrawerShaderProgram.isNull())
    {
        ShaderProgramGenerator gen("VectorDrawerShaderProgram", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_VectorDrawer);
        gen.addFragmentCode(ShaderSourceRepository::fs_VectorDrawer);
        m_vectorDrawerShaderProgram = gen.generate();
        m_vectorDrawerShaderProgram->linkProgram(oglContext);

        m_vectorDrawerShaderProgram->disableUniformTrackingForUniform("u_transformationMatrix");
        m_vectorDrawerShaderProgram->disableUniformTrackingForUniform("u_color");
    }

    return m_vectorDrawerShaderProgram.p();
}


//--------------------------------------------------------------------------------------------------
/// Deletes all OpenGL shader programs being managed by this class
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::deleteAllShaderPrograms(OpenGLContext* oglContext)
{
    if (m_textShaderProgram.notNull())
    {
        ref<ShaderProgram> progToDelete = m_textShaderProgram;
        m_textShaderProgram = NULL;

        if (progToDelete->isProgramUsed(oglContext))
        {
            ShaderProgram::useNoProgram(oglContext);
        }
        progToDelete->deleteProgram(oglContext);
    }

    if (m_nudgeShaderProgram.notNull())
    {
        ref<ShaderProgram> progToDelete = m_nudgeShaderProgram;
        m_nudgeShaderProgram = NULL;

        if (progToDelete->isProgramUsed(oglContext))
        {
            ShaderProgram::useNoProgram(oglContext);
        }
        progToDelete->deleteProgram(oglContext);
    }

    if (m_unlitColorShaderProgram.notNull())
    {
        ref<ShaderProgram> progToDelete = m_unlitColorShaderProgram;
        m_unlitColorShaderProgram = NULL;

        if (progToDelete->isProgramUsed(oglContext))
        {
            ShaderProgram::useNoProgram(oglContext);
        }
        progToDelete->deleteProgram(oglContext);
    }

    if (m_unlitTextureShaderProgram.notNull())
    {
        ref<ShaderProgram> progToDelete = m_unlitTextureShaderProgram;
        m_unlitTextureShaderProgram = NULL;

        if (progToDelete->isProgramUsed(oglContext))
        {
            ShaderProgram::useNoProgram(oglContext);
        }
        progToDelete->deleteProgram(oglContext);
    }

    if (m_vectorDrawerShaderProgram.notNull())
    {
        ref<ShaderProgram> progToDelete = m_vectorDrawerShaderProgram;
        m_vectorDrawerShaderProgram = NULL;

        if (progToDelete->isProgramUsed(oglContext))
        {
            ShaderProgram::useNoProgram(oglContext);
        }
        progToDelete->deleteProgram(oglContext);
    }
}


//--------------------------------------------------------------------------------------------------
/// Evicts (deletes) any orphaned OpenGL resources being managed by this class
/// 
/// An orphaned OpenGL resource is an object where the resource manager is holding the only reference
/// to the object. This function acts as a garbage collector for buffer objects.
/// 
/// \warning The OpenGL context passed in \a oglContext must be the same one (or shared with) used 
///          when creating the resources.
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::evictOrphanedOpenGLResources(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);
    CVF_ASSERT(oglContext->isCurrent());

    deleteOrphanedManagedBufferObjects(oglContext);
    deleteOrphanedOglRcObjects(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// Deletes all OpenGL resources known to this class
/// 
/// \warning The OpenGL context passed in \a oglContext must be the same one (or shared with) used 
///          when creating the resources.
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::deleteAllOpenGLResources(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);
    CVF_ASSERT(oglContext->isCurrent());

    deleteAllManagedBufferObjects(oglContext);
    deleteAllOglRcObjects(oglContext);
    deleteAllShaderPrograms(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OpenGLResourceManager::hasAnyOpenGLResources() const
{
    if (!m_dataPtrBufferObjectMap.empty())
    {
        return true;
    }

    if (!m_oglResources.empty())
    {
        return true;
    }

    if (m_textShaderProgram.notNull()           ||
        m_nudgeShaderProgram.notNull()          ||
        m_unlitColorShaderProgram.notNull()     ||
        m_unlitTextureShaderProgram.notNull()   ||
        m_vectorDrawerShaderProgram.notNull())
    {
        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Dump (using trace) resource info
//--------------------------------------------------------------------------------------------------
void OpenGLResourceManager::dumpCurrentResourceUsage() const
{
    Trace::show("OpenGL resource tracking:");
    Trace::show("Num. VBOs active: %6d Memory usage: %8d", bufferObjectCount(), bufferObjectMemoryUsage());
}



} // namespace cvf
