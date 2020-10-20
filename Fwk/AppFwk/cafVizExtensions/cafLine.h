//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2018- Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfVector3.h"

namespace caf
{
template <typename S>
class Line
{
public:
    Line();
    Line( const cvf::Vector3<S>& startPoint, const cvf::Vector3<S>& endPoint );
    Line( const Line& copyFrom );
    Line& operator=( const Line& copyFrom );

    const cvf::Vector3<S>& start() const;
    const cvf::Vector3<S>& end() const;
    cvf::Vector3<S>        vector() const;

    Line findLineBetweenNearestPoints( const Line& otherLine, bool* withinLineSegments = nullptr );

private:
    cvf::Vector3<S> m_start;
    cvf::Vector3<S> m_end;
};

} // namespace caf

#include "cafLine.inl"
