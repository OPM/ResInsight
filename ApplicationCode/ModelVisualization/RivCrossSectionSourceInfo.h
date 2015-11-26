/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
#include "cvfObject.h"
#include "cvfArray.h"

class RivCrossSectionGeometryGenerator;
class RimCrossSection;

class RivCrossSectionSourceInfo : public cvf::Object
{
public:
    RivCrossSectionSourceInfo(RivCrossSectionGeometryGenerator* geometryGenerator);

    const std::vector<size_t>& triangleToCellIndex() const;

    const RimCrossSection* crossSection() const;

private:
    cvf::cref<RivCrossSectionGeometryGenerator> m_crossSectionGeometryGenerator;
};
