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

#include "cvfqtUtils.h"

#include <QtCore/QVector>

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
QString	Utils::toQString(const cvf::String& ceeString)
{
    if (ceeString.isEmpty())
    {
        return QString();
    }

    if (sizeof(wchar_t) == 2)
    {
        const unsigned short* strPtr = reinterpret_cast<const unsigned short*>(ceeString.c_str());

        return QString::fromUtf16(strPtr);
    }
    else if (sizeof(wchar_t) == 4)
    {
        const unsigned int* strPtr = reinterpret_cast<const unsigned int*>(ceeString.c_str());

        return QString::fromUcs4(strPtr);
    }

    CVF_FAIL_MSG("Unexpected sizeof wchar_t");
    return QString();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::String Utils::fromQString(const QString& qtString)
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
void Utils::copyFromQImage(cvf::TextureImage* textureImage, const QImage& qtImage)
{
    CVF_ASSERT(textureImage);

    if (qtImage.isNull())
    {
        return;
    }

    if (((int)textureImage->height()) != qtImage.height() || ((int)textureImage->width() != qtImage.width()))
    {
        textureImage->allocate(qtImage.width(), qtImage.height());
    }

    int height = textureImage->height();
    int width = textureImage->width();

    // Check if QImage has format QImage::Format_ARGB32, and perform a direct memory copy of image data
    if (qtImage.format() == QImage::Format_ARGB32)
    {
        cvf::ubyte* dataPtr = const_cast<cvf::ubyte*>(textureImage->ptr());

        int negy = 0;
        uint idx = 0;
        QRgb qtRgbaVal = 0;

        // This loop is a candidate for multi-threading. Testing with OpenMP has so far indicated
        // quite large variance in performance (Windows Intel i7 with 8 cores).
        // When this function is called from the paint event,
        // the user experience is considered better when the paint time is consistent.
        for (int y = 0 ; y < height; ++y)
        {
            negy = height - 1 - y;
            const uchar* s = qtImage.scanLine(negy);

            for (int x = 0 ; x < width; ++x)
            {
                qtRgbaVal = ((QRgb*)s)[x]; // Taken from QImage::pixel(int x, int y)

                idx = 4*(y*width + x);
                dataPtr[idx]     = qRed(qtRgbaVal);
                dataPtr[idx + 1] = qGreen(qtRgbaVal); 
                dataPtr[idx + 2] = qBlue(qtRgbaVal); 
                dataPtr[idx + 3] = qAlpha(qtRgbaVal); 
            }
        }
    }
    else
    {
        for (int y = 0 ; y < height; ++y)
        {
            int negy =  height - 1 - y;
            QRgb qtRgbaVal;
            cvf::Color4ub cvfRgbVal;
            for (int x = 0 ; x < width; ++x)
            {
                qtRgbaVal = qtImage.pixel(x, negy);
                cvfRgbVal.set(qRed(qtRgbaVal), qGreen(qtRgbaVal), qBlue(qtRgbaVal), qAlpha(qtRgbaVal)); 
                textureImage->setPixel(x, y, cvfRgbVal);
            }
        }
    }
}


} // namespace cvfqt


