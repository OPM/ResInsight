//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2022- Ceetron Solutions AS
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

#include <QIcon>

namespace caf
{
class UiIconFactory
{
public:
    static const QIcon stepDownIcon();
    static const QIcon stepUpIcon();

    static const QIcon twoStateChainIcon();
    static const QIcon twoStateWhiteChainIcon();

    static const QIcon informationIcon();

private:
    static int iconWidth();
    static int iconHeight();

    static const QIcon createTwoStateIcon( const char*  onStateSvgData,
                                           const char*  offStateSvgData,
                                           unsigned int width,
                                           unsigned int height );

    static const QIcon   createIcon( const unsigned char* data, unsigned int width, unsigned int height );
    static const QIcon   createSvgIcon( const char* data, unsigned int width, unsigned int height );
    static const QPixmap createPixmap( const char* svgData, unsigned int width, unsigned int height );
};

} // namespace caf
