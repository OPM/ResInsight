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

#include "cvfVector3.h"

#include <cstddef>
#include <string>

namespace caf
{
class VecIjk0;
class VecIjk1;

class VecIjk : public cvf::Vec3st
{
public:
    std::string toString() const;

protected:
    VecIjk( size_t i, size_t j, size_t k );
};

class VecIjk0 : public VecIjk
{
public:
    VecIjk0( size_t i, size_t j, size_t k );
    VecIjk1 toOneBased() const;

    static const VecIjk0 ZERO;
    static const VecIjk0 UNDEFINED;
};

class VecIjk1 : public VecIjk
{
public:
    VecIjk1( size_t i, size_t j, size_t k );
    VecIjk0 toZeroBased() const;
};

} // namespace caf
