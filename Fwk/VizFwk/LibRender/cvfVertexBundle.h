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

#include "cvfCollection.h"
#include "cvfVertexAttribute.h"

namespace cvf {

class ShaderProgram;
class VertexBundleUsage;



//==================================================================================================
//
// 
//
//==================================================================================================
class VertexBundle : public Object
{
public:
    VertexBundle();
    ~VertexBundle();

    ref<VertexBundle>       shallowCopy() const;

    size_t                  vertexCount() const;
    const Vec3fArray*       vertexArray() const;
    void                    setVertexArray(Vec3fArray* vertexArray);

    const Vec3fArray*       normalArray() const;
    void                    setNormalArray(Vec3fArray* normalArray);

    const Vec2fArray*       textureCoordArray() const;
    void                    setTextureCoordArray(Vec2fArray* textureCoordArray);

    const Color3ubArray*    colorArray() const;
    void                    setColorArray(Color3ubArray* colorArray);

    size_t                  genericAttributeCount() const;
    VertexAttribute*        genericAttribute(size_t index);
    const VertexAttribute*  genericAttribute(size_t index) const;
    void                    setGenericAttribute(VertexAttribute* vertexAttribute);
    void                    removeGenericAttribute(const VertexAttribute* vertexAttribute);

    void                    clear();

    void                    createUploadBufferObjectsGPU(OpenGLContext* oglContext);
    void                    releaseBufferObjectsGPU();

    void                    useBundle(OpenGLContext* oglContext, VertexBundleUsage* bundleUsage, ShaderProgram* shaderProgram) const;
    void                    useBundleFixedFunction(OpenGLContext* oglContext, VertexBundleUsage* bundleUsage);
    void                    finishUseBundle(OpenGLContext* oglContext, VertexBundleUsage* bundleUsage) const;

private:
    size_t                          m_vertexCount;              // Cached vertex count for performance to avoid accessing the into the array itself 
    bool                            m_hasNormals;               // Cached flags to indicate the kind of attributes that are present
    bool                            m_hasTexCoords;             // :
    bool                            m_hasColors;                // :
    bool                            m_hasGenericAttribs;        // :
    ref<Vec3fVertexAttribute>       m_attribVertices;           // Wrapper attribute object for vertex coordinates
    ref<Vec3fVertexAttribute>       m_attribNormals;            // Wrapper attribute object for vertex normals
    ref<Vec2fVertexAttribute>       m_attribTextureCoords;      // Wrapper attribute object for texture coordinates
    ref<Color3ubVertexAttribute>    m_attribColors;             // Wrapper attribute object for vertex colors
    Collection<VertexAttribute>     m_genericAttributes;        // Collection of generic attributes

    ref<BufferObjectManaged>        m_boVertices;               // Buffer objects for the well known fixed attributes
    ref<BufferObjectManaged>        m_boNormals;                // :
    ref<BufferObjectManaged>        m_boTextureCoords;          // :
    ref<BufferObjectManaged>        m_boColors;                 // :
    Collection<BufferObjectManaged> m_genericBufferObjects;     // Buffer objects for the generic attributes
};


//==================================================================================================
//
// 
//
//==================================================================================================
class VertexBundleUsage
{
public:
    VertexBundleUsage();

    void    setFixedFunction(bool fixedFunction);
    bool    fixedFunction() const;

    size_t  usedGenAttribCount() const;
    int     usedGenAttrib(size_t i) const;
    void    registerUsedGenAttrib(int attribLocationIndex);

private:
    std::vector<int>    m_usedGenAttribIndices; // Indices of the generic vertex attributes being used
    bool                m_fixedFunction;        // Used with fixed function or shader based?

    CVF_DISABLE_COPY_AND_ASSIGN(VertexBundleUsage);
};



}

