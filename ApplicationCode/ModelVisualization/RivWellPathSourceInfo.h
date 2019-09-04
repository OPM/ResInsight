/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmPointer.h"


#include "cvfObject.h"
#include "cvfVector3.h"

class RimWellPath;
class RivPipeGeometryGenerator;

//==================================================================================================
///  
//==================================================================================================
class RivWellPathSourceInfo : public cvf::Object
{
public:
    explicit RivWellPathSourceInfo(RimWellPath* wellPath, RivPipeGeometryGenerator* pipeGeomGenerator);
    ~RivWellPathSourceInfo() override;

    RimWellPath* wellPath() const;

    size_t segmentIndex(size_t triangleIndex) const;
    double measuredDepth(size_t triangleIndex, const cvf::Vec3d& globalIntersectionInDomain) const;
    cvf::Vec3d closestPointOnCenterLine(size_t triangleIndex, const cvf::Vec3d& globalIntersectionInDomain) const;

private:
    void normalizedIntersection(size_t triangleIndex, 
                                const cvf::Vec3d& globalIntersectionInDomain,
                                size_t* firstSegmentIndex, 
                                double* normalizedSegmentIntersection) const;

private:
    caf::PdmPointer<RimWellPath> m_wellPath;
    cvf::ref<RivPipeGeometryGenerator> m_pipeGeomGenerator;
};
