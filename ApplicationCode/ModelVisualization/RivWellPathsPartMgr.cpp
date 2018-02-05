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
RivWellPathsPartMgr::RivWellPathsPartMgr(Rim3dView* view) : m_rimView(view) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellPathsPartMgr::~RivWellPathsPartMgr() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, double characteristicCellSize,
                                                           const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                           const caf::DisplayCoordTransform* displayCoordTransform)
{
    auto wellPathColl = wellPathCollection();

    if (!wellPathColl->isActive()) return;
    if (wellPathColl->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;

    buildPartManagers();

    for (auto& partMgr : m_wellPatshsPartMgrs)
    {
        partMgr->appendStaticGeometryPartsToModel(model, characteristicCellSize, wellPathClipBoundingBox, displayCoordTransform);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
void RivWellPathsPartMgr::appendStaticFracturePartsToModel(cvf::ModelBasicList* model, const Rim3dView* rimView)
{
    // Display of fractures is not supported in geomech view
    const RimEclipseView* eclView = dynamic_cast<const RimEclipseView*>(rimView);
    if (!eclView) return;

    auto wellPathColl = wellPathCollection();

    if (!wellPathColl->isActive()) return;
    if (wellPathColl->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;

    buildPartManagers();

    for (auto& partMgr : m_wellPatshsPartMgrs)
    {
        partMgr->appendStaticFracturePartsToModel(model, eclView);
    }
}
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, const QDateTime& timeStamp,
                                                            double                            characteristicCellSize,
                                                            const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                            const caf::DisplayCoordTransform* displayCoordTransform)
{
    auto wellPathColl = wellPathCollection();

    if (!wellPathColl->isActive()) return;
    if (wellPathColl->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;

    buildPartManagers();

    for (auto& partMgr : m_wellPatshsPartMgrs)
    {
        partMgr->appendDynamicGeometryPartsToModel(model, timeStamp, characteristicCellSize, wellPathClipBoundingBox,
                                                   displayCoordTransform);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivWellPathsPartMgr::segmentIndexFromTriangleIndex(size_t triangleIndex, RimWellPath* wellPath) const
{
    auto it = m_mapFromViewToIndex.find(wellPath);
    if (it == m_mapFromViewToIndex.end()) return -1;

    return it->second->segmentIndexFromTriangleIndex(triangleIndex);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::clearGeometryCache()
{
    m_wellPatshsPartMgrs.clear();
    m_mapFromViewToIndex.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::scheduleGeometryRegen() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathsPartMgr::buildPartManagers()
{
    RimProject* proj      = RiaApplication::instance()->project();
    auto        wellPaths = proj->allWellPaths();

    if (m_wellPatshsPartMgrs.size() != wellPaths.size())
    {
        clearGeometryCache();

        for (auto wellPath : wellPaths)
        {
            RivWellPathPartMgr* wppm = new RivWellPathPartMgr(wellPath, m_rimView);
            m_wellPatshsPartMgrs.push_back(wppm);
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
