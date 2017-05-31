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

#include <cstddef>
#include "cvfArray.h"
#include "cvfObject.h"
#include "cvfColor4.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivCellSetEnum.h"

#include <vector>

class RimGeoMechView;
class RimGeoMechCellColors;

namespace cvf
{
    class ModelBasicList;
}

class RivGeoMechVizLogic : public cvf::Object
{
public:

    explicit RivGeoMechVizLogic(RimGeoMechView * geomView);
    virtual ~RivGeoMechVizLogic();

    void                             appendNoAnimPartsToModel(cvf::ModelBasicList* model);
    void                             appendPartsToModel(int timeStepIndex, cvf::ModelBasicList* model);
    void                             updateCellResultColor(int timeStepIndex, RimGeoMechCellColors* cellResultColors);
    void                             updateStaticCellColors(int timeStepIndex);
    void                             scheduleGeometryRegen(RivCellSetEnum geometryType);
    void                             calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStepIndex);
private:

    std::vector<RivGeoMechPartMgrCache::Key>      keysToVisiblePartMgrs(int timeStepIndex);
    RivGeoMechPartMgr*                            getUpdatedPartMgr(RivGeoMechPartMgrCache::Key partMgrKey);
    void                                          scheduleRegenOfDirectlyDependentGeometry(RivCellSetEnum geometryType);

    cvf::ref<RivGeoMechPartMgrCache> m_partMgrCache;
    RimGeoMechView*                  m_geomechView;
};

