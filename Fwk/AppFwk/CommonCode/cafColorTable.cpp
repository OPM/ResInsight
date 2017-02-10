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


namespace caf {



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

    col3fArr.resize(m_colors.size());
    for (const auto& c : m_colors)
    {
        col3fArr.add(cvf::Color3f(c));
    }

    return col3fArr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3ub ColorTable::fromQColor(const QColor& color)
{
    return cvf::Color3ub(color.red(), color.green(), color.blue());
}

} // namespace caf
