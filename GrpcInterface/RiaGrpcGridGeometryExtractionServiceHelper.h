/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RigGridBase.h"

class RigGridCellFaultFaceVisibilityFilter : public cvf::CellFaceVisibilityFilter
{
public:
    explicit RigGridCellFaultFaceVisibilityFilter( const RigGridBase* const grid )
        : m_grid( grid )
    {
    }

    bool isFaceVisible( size_t                             i,
                        size_t                             j,
                        size_t                             k,
                        cvf::StructGridInterface::FaceType face,
                        const cvf::UByteArray*             cellVisibility ) const override;

private:
    const RigGridBase* const m_grid;
};
