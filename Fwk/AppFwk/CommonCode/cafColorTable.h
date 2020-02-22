//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cvfArray.h"
#include "cvfBase.h"

#include <vector>

class QColor;

namespace caf
{
//==================================================================================================
//
//
//
//==================================================================================================
class ColorTable
{
public:
    explicit ColorTable(const std::vector<cvf::Color3ub>& colors);
    explicit ColorTable(const cvf::Color3ubArray& colors);

    cvf::Color3f  cycledColor3f(size_t itemIndex) const;
    cvf::Color3ub cycledColor3ub(size_t itemIndex) const;
    QColor        cycledQColor(size_t itemIndex) const;

    cvf::Color3ubArray color3ubArray() const;
    cvf::Color3fArray  color3fArray() const;

    size_t size() const;

    ColorTable inverted() const;

    static cvf::Color3ub      fromQColor(const QColor& color);
    static cvf::Color3ubArray interpolateColorArray(const cvf::Color3ubArray& colorArray, size_t targetColorCount);

private:
    const std::vector<cvf::Color3ub> m_colors;
};

} // namespace caf
