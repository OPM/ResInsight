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

#include "cvfPrimitiveSet.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class PrimitiveSetDirect : public PrimitiveSet
{
public:
    PrimitiveSetDirect(PrimitiveType primitiveType);

    virtual void    render(OpenGLContext* oglContext) const;
    virtual void    createUploadBufferObjectsGPU(OpenGLContext* oglContext);
    virtual void    releaseBufferObjectsGPU();

    void            setStartIndex(size_t startIndex);
    void            setIndexCount(size_t indexCount);

    virtual size_t  indexCount() const;
    virtual uint    index(size_t i) const;

private:
    size_t  m_startIndex;   // Starting index in the enabled (vertex attribute) arrays
    size_t  m_indexCount;   // Number of indices to be rendered.
};


}
