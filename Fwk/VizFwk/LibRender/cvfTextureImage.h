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
#include "cvfArray.h"
#include "cvfOpenGLTypes.h"

namespace cvf {

//==================================================================================================
//
// RGBA ubyte 2D image
//
//==================================================================================================
class TextureImage : public Object
{
public:
    TextureImage();
    TextureImage(const TextureImage& img);
    
    const TextureImage& operator=(TextureImage rhs);

    uint                width() const;
    uint                height() const;
 
    void                allocate(uint width, uint height);
    void                setData(const ubyte* rgbaData, uint width, uint height);
    void                setFromRgb(const UByteArray& rgbData, uint width, uint height);
    void                setFromRgb(const ubyte* rgbData, uint width, uint height);
    void                clear();

    ref<UByteArray>     toRgb() const;
    ubyte*              ptr();
    const ubyte*        ptr() const;

    void                setPixel(uint x, uint y, const Color4ub& clr);
    Color4ub            pixel(uint x, uint y) const;
    void                fill(const Color4ub& clr);

    void                flipVertical();

    void                swap(TextureImage& other);

private:
    void                swapPixels(uint idx1, uint idx2);

private:
    UByteArray  m_dataRgba;
    uint        m_width;
    uint        m_height;
};

}  // namespace cvf
