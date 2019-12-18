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

#include "cafColorTable.h"

#include <QColor>

#include <algorithm>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable::ColorTable(const std::vector<cvf::Color3ub>& colors)
    : m_colors(colors)
{
    CVF_ASSERT(m_colors.size() > 0);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable::ColorTable(const cvf::Color3ubArray& colors)
    : m_colors(colors.begin(), colors.end())
{
    CVF_ASSERT(m_colors.size() > 0);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f ColorTable::cycledColor3f(size_t itemIndex) const
{
    return cvf::Color3f(cycledColor3ub(itemIndex));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3ub ColorTable::cycledColor3ub(size_t itemIndex) const
{
    size_t modIndex = itemIndex % m_colors.size();

    return m_colors[modIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor ColorTable::cycledQColor(size_t itemIndex) const
{
    cvf::Color3ub col = cycledColor3ub(itemIndex);
    return QColor(col.r(), col.g(), col.b());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3ubArray ColorTable::color3ubArray() const
{
    return cvf::Color3ubArray(m_colors);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3fArray ColorTable::color3fArray() const
{
    cvf::Color3fArray col3fArr;

    col3fArr.reserve(m_colors.size());
    for (const auto& c : m_colors)
    {
        col3fArr.add(cvf::Color3f(c));
    }

    return col3fArr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t ColorTable::size() const
{
    return m_colors.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ColorTable ColorTable::inverted() const
{
    std::vector<cvf::Color3ub> invertedColors = m_colors;
    std::reverse(invertedColors.begin(), invertedColors.end());
    return ColorTable(invertedColors);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3ub ColorTable::fromQColor(const QColor& color)
{
    return cvf::Color3ub(color.red(), color.green(), color.blue());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3ubArray ColorTable::interpolateColorArray(const cvf::Color3ubArray& colorArray, size_t targetColorCount)
{
    size_t inputColorCount = colorArray.size();
    CVF_ASSERT(inputColorCount > 1);
    CVF_ASSERT(targetColorCount > 1);

    cvf::Color3ubArray colors;
    colors.reserve(targetColorCount);

    const size_t inputColorsMaxIdx  = inputColorCount - 1;
    const size_t outputColorsMaxIdx = targetColorCount - 1;

    for (size_t outputLevelIdx = 0; outputLevelIdx < outputColorsMaxIdx; outputLevelIdx++)
    {
        double dblInputLevelIndex = inputColorsMaxIdx * (outputLevelIdx / static_cast<double>(outputColorsMaxIdx));

        const size_t inputLevelIndex = static_cast<size_t>(dblInputLevelIndex);
        CVF_ASSERT(inputLevelIndex < inputColorsMaxIdx);

        double t = dblInputLevelIndex - inputLevelIndex;
        CVF_ASSERT(t >= 0 && t <= 1.0);

        cvf::Color3ub c1 = colorArray[inputLevelIndex];
        cvf::Color3ub c2 = colorArray[inputLevelIndex + 1];

        int r = static_cast<int>(c1.r() + t * (c2.r() - c1.r()) + 0.5);
        int g = static_cast<int>(c1.g() + t * (c2.g() - c1.g()) + 0.5);
        int b = static_cast<int>(c1.b() + t * (c2.b() - c1.b()) + 0.5);

        r = cvf::Math::clamp(r, 0, 255);
        g = cvf::Math::clamp(g, 0, 255);
        b = cvf::Math::clamp(b, 0, 255);

        cvf::Color3ub col((cvf::ubyte)r, (cvf::ubyte)g, (cvf::ubyte)b);
        colors.add(col);
    }

    colors.add(colorArray[colorArray.size() - 1]);

    return colors;
}

} // namespace caf
