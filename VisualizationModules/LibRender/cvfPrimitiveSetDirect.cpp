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
#include "cvfAssert.h"
#include "cvfMath.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::PrimitiveSetDirect
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PrimitiveSetDirect::PrimitiveSetDirect(PrimitiveType primitiveType)
:   PrimitiveSet(primitiveType),
    m_startIndex(0),
    m_indexCount(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PrimitiveSetDirect::render(OpenGLContext* oglContext) const
{
    if (m_indexCount == 0)
    {
        return;
    }

    GLint first = static_cast<GLint>(m_startIndex);
    GLsizei count = static_cast<GLsizei>(m_indexCount);

    glDrawArrays(primitiveTypeOpenGL(), first, count);

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PrimitiveSetDirect::createUploadBufferObjectsGPU(OpenGLContext* /*oglContext*/)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PrimitiveSetDirect::releaseBufferObjectsGPU()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PrimitiveSetDirect::setStartIndex(size_t startIndex)
{
    m_startIndex = startIndex;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PrimitiveSetDirect::setIndexCount(size_t indexCount)
{
    m_indexCount = indexCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t PrimitiveSetDirect::indexCount() const
{
    return m_indexCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint PrimitiveSetDirect::index(size_t i) const
{
    CVF_TIGHT_ASSERT(i < m_indexCount);
    return static_cast<uint>(m_startIndex + i);
}


} // namespace cvf

