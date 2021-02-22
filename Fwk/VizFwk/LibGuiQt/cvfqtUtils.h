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

#include "cvfString.h"
#include "cvfTextureImage.h"

#include <QtCore/QString>
#include <QImage>

namespace cvfqt {


//==================================================================================================
//
// Static helper class for Qt interop
//
//==================================================================================================
class Utils
{
public:
    static QString                  toQString(const cvf::String& cvfString);
    static cvf::String              toString(const QString& qtString);

    static std::vector<cvf::String> toStringVector(const QStringList& stringList);
    static QStringList              toQStringList(const std::vector<cvf::String>& stringVector);

    static QImage                   toQImage(const cvf::TextureImage& textureImage);
    static void                     toTextureImage(const QImage& qImage, cvf::TextureImage* textureImage);
    static void                     toTextureImageRegion(const QImage& qImage, const cvf::Vec2ui& srcPos, const cvf::Vec2ui& size, cvf::TextureImage* textureImage);
};

}
