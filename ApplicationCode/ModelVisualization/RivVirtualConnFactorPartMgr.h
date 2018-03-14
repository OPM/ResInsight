/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <list>
#include <utility>
#include <vector>

namespace cvf
{
class Part;
class ModelBasicList;
class ScalarMapper;
} // namespace cvf

class RimWellPath;
class RimVirtualPerforationResults;

namespace caf
{
class DisplayCoordTransform;
}

//--------------------------------------------------------------------------------------------------
///
/// Based on RivWellSpheresPartMgr
///
//--------------------------------------------------------------------------------------------------
class RivVirtualConnFactorPartMgr : public cvf::Object
{
public:
    RivVirtualConnFactorPartMgr(RimWellPath* well, RimVirtualPerforationResults* virtualPerforationResult);
    ~RivVirtualConnFactorPartMgr();

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);

private:
    static cvf::ref<cvf::Part> createPart(std::vector<std::pair<cvf::Vec3f, double>>& centerColorPairs,
                                                        double                                            radius,
                                                        cvf::ScalarMapper*                                scalarMapper);

private:
    caf::PdmPointer<RimWellPath>                  m_rimWell;
    caf::PdmPointer<RimVirtualPerforationResults> m_virtualPerforationResult;
};
