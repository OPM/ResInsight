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

#include "cvfObject.h"
#include "cvfOpenGLTypes.h"
#include "cvfCollection.h"

#include <map>
#include <list>

namespace cvf {

class BufferObjectManaged;
class ShaderProgram;
class OpenGLContext;
class OglRc;
class OglRcShader;
class OglRcProgram;
class OglRcTexture;
class OglRcRenderbuffer;
class OglRcFramebuffer;


//==================================================================================================
//
// OpenGL resource manager 
//
//==================================================================================================
class OpenGLResourceManager : public Object
{
public:
    OpenGLResourceManager();
    virtual ~OpenGLResourceManager();

    ref<BufferObjectManaged>        getOrCreateManagedBufferObject(OpenGLContext* oglContext, cvfGLenum target, size_t sizeInBytes, const void* data); 
    int                             bufferObjectCount() const;
    size_t                          bufferObjectMemoryUsage() const;

    ref<OglRcShader>                createOglRcShader(OpenGLContext* oglContext, cvfGLenum shaderType);
    ref<OglRcProgram>               createOglRcProgram(OpenGLContext* oglContext);
    ref<OglRcTexture>               createOglRcTexture(OpenGLContext* oglContext);
    ref<OglRcRenderbuffer>          createOglRcRenderbuffer(OpenGLContext* oglContext);
    ref<OglRcFramebuffer>           createOglRcFramebuffer(OpenGLContext* oglContext);

    ShaderProgram*                  getLinkedTextShaderProgram(OpenGLContext* oglContext);
    ShaderProgram*                  getLinkedNudgeShaderProgram(OpenGLContext* oglContext);
    ShaderProgram*                  getLinkedUnlitColorShaderProgram(OpenGLContext* oglContext);
    ShaderProgram*                  getLinkedUnlitTextureShaderProgram(OpenGLContext* oglContext);
    ShaderProgram*                  getLinkedVectorDrawerShaderProgram(OpenGLContext* oglContext);

    void                            evictOrphanedOpenGLResources(OpenGLContext* oglContext);
    void                            deleteAllOpenGLResources(OpenGLContext* oglContext);
    bool                            hasAnyOpenGLResources() const;

    void                            dumpCurrentResourceUsage() const;

private:
    void                            deleteAllManagedBufferObjects(OpenGLContext* oglContext);
    void                            deleteOrphanedManagedBufferObjects(OpenGLContext* oglContext);
    void                            deleteAllOglRcObjects(OpenGLContext* oglContext);
    void                            deleteOrphanedOglRcObjects(OpenGLContext* oglContext);
    void                            deleteAllShaderPrograms(OpenGLContext* oglContext);

private:
    typedef std::map<const void*, ref<BufferObjectManaged> >             DataPtrBufferObjectMap;
    typedef std::map<const void*, ref<BufferObjectManaged> >::iterator   DataPtrBufferObjectMapIterator;
    typedef std::list<ref<OglRc> >                                       OglResourcesList;
    typedef std::list<ref<OglRc> >::iterator                             OglResourcesListIterator;

    DataPtrBufferObjectMap  m_dataPtrBufferObjectMap;   // Mapping from data pointer to BufferObject
    mutable size_t          m_cachedVboMemoryUsage;     // 

    OglResourcesList        m_oglResources;

    ref<ShaderProgram>      m_textShaderProgram;
    ref<ShaderProgram>      m_nudgeShaderProgram;
    ref<ShaderProgram>      m_unlitColorShaderProgram;
    ref<ShaderProgram>      m_unlitTextureShaderProgram;
    ref<ShaderProgram>      m_vectorDrawerShaderProgram;
};

}
