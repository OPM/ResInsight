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

#include "RivCellSetEnum.h"
#include "RivGeoMechPartMgrCache.h"
#include "cvfArray.h"
#include "cvfColor4.h"
#include "cvfObject.h"
#include <cstddef>

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
    explicit RivGeoMechVizLogic( RimGeoMechView* geomView );
    ~RivGeoMechVizLogic() override;

    void appendNoAnimPartsToModel( cvf::ModelBasicList* model );
    void appendPartsToModel( int viewerStepIndex, cvf::ModelBasicList* model );
    void updateCellResultColor( int viewerStepIndex, int timeStepIndex, int frameIndex, RimGeoMechCellColors* cellResultColors );
    void updateStaticCellColors( int viewerStepIndex );
    void scheduleGeometryRegen( RivCellSetEnum geometryType );
    void scheduleGeometryRegenOfVisiblePartMgrs( int viewerStepIndex );

    void calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int viewerStepIndex );
    void calculateCellVisibility( cvf::UByteArray* totalVisibility, std::vector<RivCellSetEnum> geomTypes, int viewerStepIndex );

    void resetPartMgrs();

    std::vector<RivGeoMechPartMgrCache::Key> keysToVisiblePartMgrs( int viewerStepIndex ) const;
    const cvf::ref<RivGeoMechPartMgrCache>   partMgrCache() const;

    static cvf::Color3f staticCellColor();

private:
    RivGeoMechPartMgr* getUpdatedPartMgr( RivGeoMechPartMgrCache::Key partMgrKey );
    void               scheduleRegenOfDirectlyDependentGeometry( RivCellSetEnum geometryType );

    cvf::ref<RivGeoMechPartMgrCache> m_partMgrCache;
    RimGeoMechView*                  m_geomechView;
};
