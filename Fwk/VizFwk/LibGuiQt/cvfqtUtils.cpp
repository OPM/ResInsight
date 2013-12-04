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
#include "cvfVector2.h"
#include "cvfqtUtils.h"

#include <QtCore/QStringList>

namespace cvfqt {



//==================================================================================================
///
/// \class cvfqt::Utils
/// \ingroup GuiQt
///
/// Static helper class for Qt interop
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString Utils::toQString(const cvf::String& cvfString)
{
    if (cvfString.isEmpty())
    {
        return QString();
    }

    if (sizeof(wchar_t) == 2)
    {
        const unsigned short* strPtr = reinterpret_cast<const unsigned short*>(cvfString.c_str());

        return QString::fromUtf16(strPtr);
    }
    else if (sizeof(wchar_t) == 4)
    {
        const unsigned int* strPtr = reinterpret_cast<const unsigned int*>(cvfString.c_str());

        return QString::fromUcs4(strPtr);
    }

    CVF_FAIL_MSG("Unexpected sizeof wchar_t");
    return QString();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::String Utils::toString(const QString& qtString)
{
    if (qtString.length() == 0)
    {
        return cvf::String();
    }

    if (sizeof(wchar_t) == 2)
    {
        const wchar_t* strPtr = reinterpret_cast<const wchar_t*>(qtString.utf16());

        return cvf::String(strPtr);
    }
    else if (sizeof(wchar_t) == 4)
    {
        QVector<uint> ucs4Str = qtString.toUcs4();
        ucs4Str.push_back(0);
        const wchar_t* strPtr = reinterpret_cast<const wchar_t*>(ucs4Str.data());

        return cvf::String(strPtr);
    }

    CVF_FAIL_MSG("Unexpected sizeof wchar_t");
    return cvf::String();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> Utils::toStringVector(const QStringList& stringList)
{
    std::vector<cvf::String> strVec;

    foreach (QString s, stringList)
    {
        strVec.push_back(toString(s));
    }

    return strVec;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList Utils::toQStringList(const std::vector<cvf::String>& stringVector)
{
    QStringList strList;

    foreach (cvf::String s, stringVector)
    {
        strList.push_back(toQString(s));
    }

    return strList;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage Utils::toQImage(const cvf::TextureImage& textureImage)
{
    const int width = static_cast<int>(textureImage.width());
    const int height = static_cast<int>(textureImage.height());
    if (width <= 0 || height <= 0)
    {
        return QImage();
    }

    const cvf::ubyte* rgbData = textureImage.ptr();
    QImage qimg(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            const int srcIdx = 4*y*width + 4*x;
            qimg.setPixel(x, height - y - 1, qRgba(rgbData[srcIdx], rgbData[srcIdx + 1], rgbData[srcIdx + 2], rgbData[srcIdx + 3]));
        }
    }

    return qimg;

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Utils::toTextureImage(const QImage& qImage, cvf::TextureImage* textureImage)
{
    CVF_ASSERT(textureImage);

    const int width = qImage.width();
    const int height = qImage.height();
    if (width <= 0 || height <= 0)
    {
        textureImage->clear();
        return;
    }

    return toTextureImageRegion(qImage, cvf::Vec2ui(0, 0), cvf::Vec2ui(static_cast<cvf::uint>(width), static_cast<cvf::uint>(height)), textureImage);
}


//--------------------------------------------------------------------------------------------------
/// Convert a region of a QImage to a TextureImage
/// 
/// \attention The source position \a srcPos is specified in QImage's coordinate system, where 
///            the pixel position (0,0) if the upper left corner.
//--------------------------------------------------------------------------------------------------
void Utils::toTextureImageRegion(const QImage& qImage, const cvf::Vec2ui& srcPos, const cvf::Vec2ui& size, cvf::TextureImage* textureImage)
{
    CVF_ASSERT(qImage.width() >= 0);
    CVF_ASSERT(qImage.height() >= 0);
    const cvf::uint qImgWidth = static_cast<cvf::uint>(qImage.width());
    const cvf::uint qImgHeight = static_cast<cvf::uint>(qImage.height());

    CVF_ASSERT(srcPos.x() < qImgWidth);
    CVF_ASSERT(srcPos.y() < qImgHeight);
    CVF_ASSERT(srcPos.x() + size.x() <= qImgWidth);
    CVF_ASSERT(srcPos.y() + size.y() <= qImgHeight);
    CVF_ASSERT(textureImage);

    if (size.x() < 1 || size.y() < 1)
    {
        textureImage->clear();
        return;
    }

    if (textureImage->width() != size.x() || textureImage->height() != size.y())
    {
        textureImage->allocate(size.x(), size.y());
    }


    const cvf::uint sizeX = size.x();
    const cvf::uint sizeY = size.y();
    const cvf::uint srcPosX = srcPos.x();
    const cvf::uint srcPosY = srcPos.y();
    cvf::ubyte* textureImageDataPtr = textureImage->ptr();

    // Check if QImage has format QImage::Format_ARGB32, and use a more optimized path
    if (qImage.format() == QImage::Format_ARGB32)
    {
        for (cvf::uint y = 0; y < sizeY; ++y)
        {
            const cvf::uint scanLineIdx = srcPosY + sizeY - y - 1;
            const QRgb* qWholeScanLine = reinterpret_cast<const QRgb*>(qImage.scanLine(scanLineIdx));
            const QRgb* qPixels = &qWholeScanLine[srcPosX];

            const cvf::uint dstStartIdx = 4*(y*sizeX);
            cvf::ubyte* dstRgba = &textureImageDataPtr[dstStartIdx];
            for (cvf::uint x = 0; x < sizeX; ++x)
            {
                QRgb qRgbaVal = qPixels[x];
                dstRgba[0] = qRed(qRgbaVal);
                dstRgba[1] = qGreen(qRgbaVal);
                dstRgba[2] = qBlue(qRgbaVal);
                dstRgba[3] = qAlpha(qRgbaVal);
                dstRgba += 4;
            }
        }
    }
    else
    {
        cvf::Color4ub cvfRgbVal;
        for (cvf::uint y = 0; y < sizeY; ++y)
        {
            const cvf::uint qImageYPos = srcPosY + sizeY - y - 1;
            for (cvf::uint x = 0; x < sizeX; ++x)
            {
                const QRgb qRgbaVal = qImage.pixel(srcPosX + x, qImageYPos);
                cvfRgbVal.r() = qRed(qRgbaVal);
                cvfRgbVal.g() = qGreen(qRgbaVal);
                cvfRgbVal.b() = qBlue(qRgbaVal);
                cvfRgbVal.a() = qAlpha(qRgbaVal); 
                textureImage->setPixel(x, y, cvfRgbVal);
            }
        }
    }
}



} // namespace cvfqt


