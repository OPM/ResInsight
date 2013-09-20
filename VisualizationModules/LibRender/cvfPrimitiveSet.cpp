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
#include "cvfPrimitiveSet.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::PrimitiveSet
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PrimitiveSet::PrimitiveSet(PrimitiveType primitiveType)
    : m_primitiveType(primitiveType)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PrimitiveType PrimitiveSet::primitiveType() const
{
    return m_primitiveType;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum PrimitiveSet::primitiveTypeOpenGL() const
{
    switch (m_primitiveType)
    {
        case PT_POINTS:         return GL_POINTS;
        case PT_LINES:          return GL_LINES;
        case PT_LINE_LOOP:      return GL_LINE_LOOP;
        case PT_LINE_STRIP:     return GL_LINE_STRIP;
        case PT_TRIANGLES:      return GL_TRIANGLES;
        case PT_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
        case PT_TRIANGLE_FAN:   return GL_TRIANGLE_FAN;
        default:                CVF_FAIL_MSG("Unhandled primitive type");
    }

    return UNDEFINED_UINT;
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of triangles in this primitive group
//--------------------------------------------------------------------------------------------------
size_t PrimitiveSet::triangleCount() const
{
    if (m_primitiveType != PT_TRIANGLES)
    {
        return 0;
    }

    CVF_ASSERT(indexCount()%3 == 0);

    return indexCount()/3;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t PrimitiveSet::faceCount() const
{
    switch (m_primitiveType)
    {
        case PT_POINTS:         return indexCount();
        case PT_LINES:          return indexCount()/2;
        case PT_LINE_LOOP:      return indexCount();
        case PT_LINE_STRIP:     return indexCount() - 1;
        case PT_TRIANGLES:      return triangleCount();
        case PT_TRIANGLE_STRIP: return indexCount() - 2;
        case PT_TRIANGLE_FAN:   return indexCount() - 2;
        default:                CVF_FAIL_MSG("Unhandled primitive type");
    }

    return 0;
}


//--------------------------------------------------------------------------------------------------
/// Gets connectivity for one face in this primitive set
/// 
/// The indices array will be populated with the connectivities for the specified face. 
/// The connectivity table will have 1, 2, 3 or 4 entries. If this primitive set consists 
/// of a triangle fan, triangles will be returned. In a similar fashion, if the primitive set
/// consist of a quad strip, quads will be returned.
//--------------------------------------------------------------------------------------------------
void PrimitiveSet::getFaceIndices(size_t indexOfFace, UIntArray* indices) const
{
    CVF_TIGHT_ASSERT(indexOfFace < faceCount());
    CVF_TIGHT_ASSERT(indices);

    indices->setSizeZero();

    if (m_primitiveType == PT_POINTS)
    {
        indices->reserve(1);
        indices->add(index(indexOfFace));
    }
    else if (m_primitiveType == PT_LINES)
    {
        indices->reserve(2);
        indices->add(index(2*indexOfFace));
        indices->add(index(2*indexOfFace + 1));
    }
    else if (m_primitiveType == PT_LINE_LOOP)
    {
        indices->reserve(2);

        if (indexOfFace == faceCount() - 1)
        {
            indices->add(index(indexOfFace));
            indices->add(index(0));
        }
        else
        {
            indices->add(index(indexOfFace));
            indices->add(index(indexOfFace + 1));
        }
    }
    else if (m_primitiveType == PT_LINE_STRIP)
    {
        indices->reserve(2);
        indices->add(index(indexOfFace));
        indices->add(index(indexOfFace + 1));
    }
    else if (m_primitiveType == PT_TRIANGLES)
    {
        indices->reserve(3);

        const size_t baseIdx = 3*indexOfFace;
        indices->add(index(baseIdx));
        indices->add(index(baseIdx + 1));
        indices->add(index(baseIdx + 2));
    }
    else if (m_primitiveType == PT_TRIANGLE_FAN)
    {
        indices->reserve(3);
        indices->add(index(0));
        indices->add(index(indexOfFace + 1));
        indices->add(index(indexOfFace + 2));
    }
    else if (m_primitiveType == PT_TRIANGLE_STRIP)
    {
        indices->reserve(3);

        if (indexOfFace % 2 == 0)
        {
            indices->add(index(indexOfFace));
            indices->add(index(indexOfFace + 1));
            indices->add(index(indexOfFace + 2));
        }
        else
        {
            indices->add(index(indexOfFace + 1));
            indices->add(index(indexOfFace));
            indices->add(index(indexOfFace + 2));
        }
    }
    else
    {
        CVF_FAIL_MSG("Not implemented");
    }
}


} // namespace cvf

