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

#include "cvfBase.h"
#include "cvfVector3.h"

namespace cvf
{
//==================================================================================================
//
//==================================================================================================
class CellRange
{
public:
    CellRange();
    CellRange( cvf::Vec3st min, cvf::Vec3st max );
    CellRange( size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK );

    void setRange( const cvf::Vec3st& min, const cvf::Vec3st& max );
    void range( cvf::Vec3st& min, cvf::Vec3st& max ) const;

    bool normalize();

    bool isInRange( size_t i, size_t j, size_t k ) const;

private:
    cvf::Vec3st m_min;
    cvf::Vec3st m_max;
};

} // End namespace cvf
