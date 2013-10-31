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
#include "cvfTextureImage.h"

namespace cvf {

//==================================================================================================
///
/// \class cvf::TextureImage
/// \ingroup Render
///
/// RGBA ubyte 2D image designed to be directly compatible with OpenGL's texture functions.
/// The pixel at (0,0) is in the lower left corner of the image. Internally the image data is stored
/// as a 1-dimensional array of RGBA ubyte values with 4 ubyte values per pixel. 
/// The first element in the internal array (the pixel at position (0,0)) corresponds to the lower 
/// left corner of the texture image. Subsequent elements in the internal array progress left-to-right 
/// through the remaining pixels in the lowest row of the image, and then in successively higher rows 
/// of the texture image. The final element corresponds to the upper right corner of the texture image.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureImage::TextureImage()
:   m_width(0),
    m_height(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureImage::TextureImage(const TextureImage& img)
:   Object(),
    m_dataRgba(img.m_dataRgba),
    m_width(img.width()),
    m_height(img.height())
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const TextureImage& TextureImage::operator=(TextureImage rhs)
{
    // Copy-and-swap (copy already done since parameter is passed by value)
    rhs.swap(*this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Allocate image pixels
///
/// Allocates image of the specified size, but leaves all pixels uninitialized
//--------------------------------------------------------------------------------------------------
void TextureImage::allocate(uint width, uint height)
{
    uint numBytes = 4*width*height;
    CVF_ASSERT(numBytes > 0);

    m_dataRgba.resize(numBytes);

    m_width = width;
    m_height = height;
}


//--------------------------------------------------------------------------------------------------
/// Data must be 4 byte per pixel (r.g.b.a)
//--------------------------------------------------------------------------------------------------
void TextureImage::setData(const ubyte* rgbaData, uint width, uint height)
{
    CVF_ASSERT(width > 0 && height > 0);
    m_dataRgba.assign(rgbaData, width*height*4*sizeof(ubyte));

    m_width = width;
    m_height = height;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureImage::setFromRgb(const UByteArray& rgbData, uint width, uint height)
{
    CVF_ASSERT(width > 0 && height > 0);
    CVF_ASSERT(rgbData.size() == width*height*3);

    setFromRgb(rgbData.ptr(), width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureImage::setFromRgb(const ubyte* rgbData, uint width, uint height)
{
    CVF_ASSERT(rgbData);
    CVF_ASSERT(width > 0 && height > 0);

    const size_t numPixels = width*height;
    m_dataRgba.reserve(4*numPixels);
    m_width = width;
    m_height = height;

    for (size_t i = 0; i < numPixels; i++)
    {
        m_dataRgba.add(rgbData[3*i + 0]);
        m_dataRgba.add(rgbData[3*i + 1]);
        m_dataRgba.add(rgbData[3*i + 2]);
        m_dataRgba.add(255);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureImage::clear()
{
    m_width = 0;
    m_height = 0;
    m_dataRgba.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ubyte* TextureImage::ptr() 
{
    return m_dataRgba.ptr();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const ubyte* TextureImage::ptr() const
{
    return m_dataRgba.ptr();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<UByteArray> TextureImage::toRgb() const
{
    ref<UByteArray> rgbData = new UByteArray;

    if (m_width > 0 && m_height > 0)
    {
        rgbData->reserve(m_width*m_height*3);

        size_t numPixels = m_dataRgba.size()/4;
        size_t i;
        for (i = 0; i < numPixels; i++)
        {
            rgbData->add(m_dataRgba[4*i + 0]);
            rgbData->add(m_dataRgba[4*i + 1]);
            rgbData->add(m_dataRgba[4*i + 2]);
        }
    }

    return rgbData;
}


//--------------------------------------------------------------------------------------------------
/// Return the texture dimension i x direction
//--------------------------------------------------------------------------------------------------
uint TextureImage::width() const
{
    return m_width;
}


//--------------------------------------------------------------------------------------------------
/// Return the texture dimension i y direction
//--------------------------------------------------------------------------------------------------
uint TextureImage::height() const
{
    return m_height;
}


//--------------------------------------------------------------------------------------------------
/// Set the color value of a specific pixel
/// 
/// \warning Pixel position (0,0) is in the lower left corner of the image.
//--------------------------------------------------------------------------------------------------
void TextureImage::setPixel(uint x, uint y, const Color4ub& clr)
{
    CVF_TIGHT_ASSERT(x < m_width);
    CVF_TIGHT_ASSERT(y < m_height);

    ubyte* rgbaThisPixel = &m_dataRgba[4*(y*m_width + x)];
    rgbaThisPixel[0] = clr.r();
    rgbaThisPixel[1] = clr.g(); 
    rgbaThisPixel[2] = clr.b(); 
    rgbaThisPixel[3] = clr.a(); 
}


//--------------------------------------------------------------------------------------------------
/// Retrieve the color value of a specific pixel
/// 
/// \warning Pixel position (0,0) is in the lower left corner of the image.
//--------------------------------------------------------------------------------------------------
Color4ub TextureImage::pixel(uint x, uint y) const
{
    CVF_TIGHT_ASSERT(x < m_width);
    CVF_TIGHT_ASSERT(y < m_height);

    const uint idx = 4*(y*m_width + x);
    return Color4ub(m_dataRgba[idx], m_dataRgba[idx + 1], m_dataRgba[idx + 2], m_dataRgba[idx + 3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureImage::fill(const Color4ub& clr)
{
    size_t numBytes = m_dataRgba.size();

    size_t i;
    for (i = 0; i < numBytes; i += 4)
    {
        m_dataRgba[i]     = clr.r();
        m_dataRgba[i + 1] = clr.g();
        m_dataRgba[i + 2] = clr.b();
        m_dataRgba[i + 3] = clr.a();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureImage::flipVertical()
{
    if (m_dataRgba.size() > 0)
    {
        uint row;
        for (row = 0; row < m_height/2; row++)
        {
            uint col;
            for (col = 0; col < m_width; col++)
            {
                uint p1 = row*m_width + col;
                uint p2 = (m_height - row - 1)*m_width + col;

                swapPixels(p1, p2);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureImage::swapPixels(uint idx1, uint idx2)
{
    using std::swap;

    swap(m_dataRgba[idx1*4 + 0], m_dataRgba[idx2*4 + 0]);
    swap(m_dataRgba[idx1*4 + 1], m_dataRgba[idx2*4 + 1]);
    swap(m_dataRgba[idx1*4 + 2], m_dataRgba[idx2*4 + 2]);
    swap(m_dataRgba[idx1*4 + 3], m_dataRgba[idx2*4 + 3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextureImage::swap(TextureImage& other)
{
    using std::swap;

    m_dataRgba.swap(other.m_dataRgba);
    swap(m_width, other.m_width);
    swap(m_height, other.m_height);
}


}  // namespace cvf
