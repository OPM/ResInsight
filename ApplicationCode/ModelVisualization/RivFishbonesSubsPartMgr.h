/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cvfBase.h"
#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cvfCollection.h"
#include "cvfColor3.h"


namespace cvf
{
     class ModelBasicList;
     class DrawableGeo;
     class Part;
     class Transform;
}

namespace caf
{
    class DisplayCoordTransform;
}

class RimFishbonesMultipleSubs;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivFishbonesSubsPartMgr : public cvf::Object
{
public:
    RivFishbonesSubsPartMgr(RimFishbonesMultipleSubs* subs);
    ~RivFishbonesSubsPartMgr();

    void        appendGeometryPartsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize);
    void        clearGeometryCache();

private:
    void        buildParts(caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize);


private:
    caf::PdmPointer<RimFishbonesMultipleSubs> m_rimFishbonesSubs;
    cvf::Collection<cvf::Part>                m_parts;
};
