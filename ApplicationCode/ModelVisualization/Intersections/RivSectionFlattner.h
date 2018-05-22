/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "cvfBase.h"
#include "cvfMatrix4.h"

#include <vector>


class RivSectionFlattner
{
public:
    static size_t  indexToNextValidPoint(const std::vector<cvf::Vec3d>& polyLine,
                                         const cvf::Vec3d extrDir,
                                         size_t idxToStartOfLineSegment);

    static std::vector<cvf::Mat4d> calculateFlatteningCSsForPolyline(const std::vector<cvf::Vec3d> & polyLine,
                                                                     const cvf::Vec3d& extrusionDir,
                                                                     const cvf::Vec3d& startOffset,
                                                                     cvf::Vec3d* endOffset);

private:

    static cvf::Mat4d calculateSectionLocalFlatteningCS(const cvf::Vec3d& p1,
                                                        const cvf::Vec3d& p2,
                                                        const cvf::Vec3d& extrusionDir);


};


