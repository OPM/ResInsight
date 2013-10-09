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

#include "cvfVector4.h"
#include "cvfArray.h"
#include "cvfCharArray.h"
#include "cvfOpenGLTypes.h"

namespace cvf {

class OpenGLContext;
class BufferObjectManaged;



//==================================================================================================
//
// Abstract class for generic vertex attribute arrays
//
//==================================================================================================
class VertexAttribute : public Object
{
public:
    // The data type of the components stored in the data array
    // Enum values are rigged to match the OpenGL types
    enum ComponentType
    {
        UBYTE   = 0x1401,   // GL_UNSIGNED_BYTE
        INT     = 0x1404,   // GL_INT
        FLOAT   = 0x1406    // GL_FLOAT
    };

public:
    virtual ~VertexAttribute() {}

    virtual const char*     name() const = 0;                   ///< Attribute name
    virtual uint            componentCount() const = 0;         ///< Number of components per attribute element. 
    virtual ComponentType   componentType() const = 0;          ///< The data type of the components stored in the data array 
    virtual size_t          arrayDataByteCount() const = 0;     ///< The size of the entire data array in bytes
    virtual const void*     arrayDataPtrVoid() const = 0;       ///< Void pointer to data in contained array

    virtual void            setupAttribPointerBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, size_t strideInBytes, size_t bufferOffsetInBytes) const = 0;
    virtual void            setupAttribPointerClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex) const = 0;
};


//==================================================================================================
//
// Strategy for specifying vertex attribute data as float, integers normalized
//
//==================================================================================================
class AttribSetupStrategyNormFloat
{
public:
    static void setupFromBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, size_t strideInBytes, size_t bufferOffsetInBytes);
    static void setupFromClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, const void* ptr);
};


//==================================================================================================
//
// Strategy for specifying vertex attribute data as float, integers converted directly
//
//==================================================================================================
class AttribSetupStrategyDirectFloat
{
public:
    static void setupFromBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, size_t strideInBytes, size_t bufferOffsetInBytes);
    static void setupFromClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, const void* ptr);
};


//==================================================================================================
//
// Strategy for specifying vertex attribute data as integer type
//
//==================================================================================================
class AttribSetupStrategyInt
{
public:
    static void setupFromBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, size_t strideInBytes, size_t bufferOffsetInBytes);
    static void setupFromClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex, uint compCount, cvfGLenum compTypeOpenGL, const void* ptr);
};


//==================================================================================================
//
// 
//
//==================================================================================================
template <typename T_ELEM_TYPE, VertexAttribute::ComponentType T_COMP_TYPE, int T_COMP_COUNT, typename T_SETUP_STRATEGY>
class VertexAttributeImpl : public VertexAttribute
{
public:
    VertexAttributeImpl(const char* attribName, Array<T_ELEM_TYPE>* valueArray)
    :   m_attribValues(valueArray), 
        m_name(attribName)
    {
        CVF_ASSERT(valueArray);
    }

    virtual const char*         name() const                { return m_name.ptr(); }
    virtual uint                componentCount() const      { return T_COMP_COUNT; }
    virtual ComponentType       componentType() const       { return T_COMP_TYPE; }
    virtual size_t              arrayDataByteCount() const  { return m_attribValues->size()*sizeof(T_ELEM_TYPE); }
    virtual const void*         arrayDataPtrVoid() const    { return m_attribValues->ptr(); }
    const Array<T_ELEM_TYPE>*   arrayPtr() const            { return m_attribValues.p(); }

    virtual void setupAttribPointerBufferObject(OpenGLContext* oglContext, uint vertexAttributeIndex, size_t strideInBytes, size_t bufferOffsetInBytes) const
    {
        T_SETUP_STRATEGY::setupFromBufferObject(oglContext, vertexAttributeIndex, T_COMP_COUNT, T_COMP_TYPE, strideInBytes, bufferOffsetInBytes);
    }

    virtual void setupAttribPointerClientMemory(OpenGLContext* oglContext, uint vertexAttributeIndex) const
    {
        T_SETUP_STRATEGY::setupFromClientMemory(oglContext, vertexAttributeIndex, T_COMP_COUNT, T_COMP_TYPE, m_attribValues->ptr());
    }

private:
    ref<Array<T_ELEM_TYPE> >  m_attribValues;   // Reference to the data array
    CharArray                 m_name;           // Name of the attribute
};


// For these, integer values are converted to float by normalization. [0,1] for unsigned integer types, [-1,1] for signed integer types.
typedef VertexAttributeImpl<float,     VertexAttribute::FLOAT, 1, AttribSetupStrategyNormFloat>     FloatVertexAttribute;
typedef VertexAttributeImpl<Vec2f,     VertexAttribute::FLOAT, 2, AttribSetupStrategyNormFloat>     Vec2fVertexAttribute;
typedef VertexAttributeImpl<Vec3f,     VertexAttribute::FLOAT, 3, AttribSetupStrategyNormFloat>     Vec3fVertexAttribute;
typedef VertexAttributeImpl<Color3f,   VertexAttribute::FLOAT, 3, AttribSetupStrategyNormFloat>     Color3fVertexAttribute;
typedef VertexAttributeImpl<Color3ub,  VertexAttribute::UBYTE, 3, AttribSetupStrategyNormFloat>     Color3ubVertexAttribute;

// Integer values converted directly to float
typedef VertexAttributeImpl<int,       VertexAttribute::INT,   1, AttribSetupStrategyDirectFloat>   IntVertexAttributeDirect;

// Only for integer values AND no conversion, requires OpenGL 3.0
typedef VertexAttributeImpl<int,       VertexAttribute::INT,   1, AttribSetupStrategyInt>           IntVertexAttributePure;
typedef VertexAttributeImpl<Color3ub,  VertexAttribute::UBYTE, 3, AttribSetupStrategyInt>           Color3ubVertexAttributePure;


}

