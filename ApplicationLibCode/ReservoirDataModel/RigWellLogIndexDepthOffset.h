/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "cvfObject.h"

#include <map>
#include <vector>

class RigWellLogIndexDepthOffset : public cvf::Object
{
public:
    RigWellLogIndexDepthOffset()           = default;
    ~RigWellLogIndexDepthOffset() override = default;

    void             setIndexOffsetDepth( int kIndex, double topDepth, double bottomDepth );
    double           getTopDepth( int kIndex ) const;
    double           getBottomDepth( int kIndex ) const;
    std::vector<int> sortedIndexes() const;

private:
    std::map<int, std::pair<double, double>> m_depthOffsets;
};
