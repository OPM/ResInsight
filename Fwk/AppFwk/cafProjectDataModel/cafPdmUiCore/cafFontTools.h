//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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

#include <QList>

namespace caf
{
template <typename T>
class AppEnum;

class PdmOptionItemInfo;
} // namespace caf

namespace caf
{
//==================================================================================================
/// Tools for managing fonts in the application
//==================================================================================================

class FontTools
{
public:
    static const int MIN_FONT_SIZE;

    enum class FontSize
    {
        INVALID      = -1,
        FONT_SIZE_8  = 8,
        FONT_SIZE_10 = 10,
        FONT_SIZE_12 = 12,
        FONT_SIZE_14 = 14,
        FONT_SIZE_16 = 16,
        FONT_SIZE_24 = 24,
        FONT_SIZE_32 = 32,
        MAX_FONT_SIZE
    };
    typedef caf::AppEnum<FontSize> FontSizeEnum;

    enum class RelativeSize
    {
        XXSmall = -4,
        XSmall  = -2,
        Small   = -1,
        Medium  = 0,
        Large   = +2,
        XLarge  = +4,
        XXLarge = +8
    };
    typedef caf::AppEnum<RelativeSize> RelativeSizeEnum;

    static int absolutePointSize( FontSize normalPointSize, RelativeSize relativeSize = RelativeSize::Medium );
    static int pointSizeToPixelSize( FontSize pointSize );
    static int pointSizeToPixelSize( int pointSize );
    static int pixelSizeToPointSize( int pixelSize );

    static QList<caf::PdmOptionItemInfo> relativeSizeValueOptions( FontSize normalPointSize );
};

//==================================================================================================
/// Interface to implement for any PdmObject that has font sizes
//==================================================================================================
class FontHolderInterface
{
public:
    virtual int  fontSize() const = 0;
    virtual void updateFonts()    = 0;
};

} // namespace caf
