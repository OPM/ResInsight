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

#include "RivWellPathsPartMgr.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "RivWellPathPartMgr.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellPathsPartMgr::RivWellPathsPartMgr(Rim3dView* view)
    : m_rimView(view)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellPathsPartMgr::~RivWellPathsPartMgr() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                                           double                            characteristicCellSize,
                                                           const cvf::BoundingBox&           wellPathClipBoundingBox)
{
    if (!isWellPathVisible()) return;

    createPartManagersIfRequired();

    for (auto& partMgr : m_wellPathsPartMgrs)
    {
        partMgr->appendStaticGeometryPartsToModel(model, displayCoordTransform, characteristicCellSize, wellPathClipBoundingBox);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::appendStaticFracturePartsToModel(cvf::ModelBasicList* model)
{
    if (!isWellPathVisible()) return;

    createPartManagersIfRequired();

    for (auto& partMgr : m_wellPathsPartMgrs)
    {
        partMgr->appendStaticFracturePartsToModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList*              model,
                                                            size_t                            timeStepIndex,
                                                            const caf::DisplayCoordTransform* displayCoordTransform,
                                                            double                            characteristicCellSize,
                                                            const cvf::BoundingBox&           wellPathClipBoundingBox)
{
    if (!isWellPathVisible()) return;

    createPartManagersIfRequired();

    for (auto& partMgr : m_wellPathsPartMgrs)
    {
        partMgr->appendDynamicGeometryPartsToModel(
            model, timeStepIndex, displayCoordTransform, characteristicCellSize, wellPathClipBoundingBox);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::clearGeometryCache()
{
    m_wellPathsPartMgrs.clear();
    m_mapFromViewToIndex.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::scheduleGeometryRegen() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::createPartManagersIfRequired()
{
    RimProject* proj      = RiaApplication::instance()->project();
    auto        wellPaths = proj->allWellPaths();

    if (m_wellPathsPartMgrs.size() != wellPaths.size())
    {
        clearGeometryCache();

        for (auto wellPath : wellPaths)
        {
            RivWellPathPartMgr* wppm = new RivWellPathPartMgr(wellPath, m_rimView);
            m_wellPathsPartMgrs.push_back(wppm);
            m_mapFromViewToIndex[wellPath] = wppm;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RivWellPathsPartMgr::wellPathCollection() const
{
    RimProject* proj = RiaApplication::instance()->project();

    return proj->activeOilField()->wellPathCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivWellPathsPartMgr::isWellPathVisible() const
{
    auto wellPathColl = wellPathCollection();

    if (!wellPathColl->isActive()) return false;
    if (wellPathColl->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return false;

    return true;
}
