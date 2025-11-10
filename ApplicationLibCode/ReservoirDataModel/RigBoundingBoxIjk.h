/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cvfVector3.h"

#include <optional>

//==================================================================================================
///
/// Value type representing an axis-aligned bounding box in IJK (integer) grid coordinates
///
//==================================================================================================
class RigBoundingBoxIjk
{
public:
    RigBoundingBoxIjk();
    RigBoundingBoxIjk( const cvf::Vec3st& min, const cvf::Vec3st& max );

    const cvf::Vec3st& min() const { return m_min; }
    const cvf::Vec3st& max() const { return m_max; }

    bool isValid() const;

    // Check if this box overlaps with another box
    bool overlaps( const RigBoundingBoxIjk& other ) const;

    // Compute intersection of this box with another box
    // Returns std::nullopt if boxes don't overlap
    std::optional<RigBoundingBoxIjk> intersection( const RigBoundingBoxIjk& other ) const;

    // Clamp this box to be within another box
    // Returns std::nullopt if boxes don't overlap
    std::optional<RigBoundingBoxIjk> clamp( const RigBoundingBoxIjk& bounds ) const;

private:
    cvf::Vec3st m_min;
    cvf::Vec3st m_max;
};
