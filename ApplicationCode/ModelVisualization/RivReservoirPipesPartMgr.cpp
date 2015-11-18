/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RivReservoirPipesPartMgr.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"

#include "RivWellHeadPartMgr.h"
#include "RivWellPipesPartMgr.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirPipesPartMgr::RivReservoirPipesPartMgr(RimEclipseView* reservoirView)
{
    m_reservoirView = reservoirView;

    m_scaleTransform = new cvf::Transform();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirPipesPartMgr::~RivReservoirPipesPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPipesPartMgr::clearGeometryCache()
{
    m_wellPipesPartMgrs.clear();
    m_wellHeadPartMgrs.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPipesPartMgr::scheduleGeometryRegen()
{
    for (size_t wIdx = 0; wIdx != m_wellPipesPartMgrs.size(); ++ wIdx)
    {
        m_wellPipesPartMgrs[wIdx]->scheduleGeometryRegen();
    }

    for (size_t wIdx = 0; wIdx != m_wellHeadPartMgrs.size(); ++ wIdx)
    {
        //m_wellHeadPartMgrs[wIdx]->scheduleGeometryRegen(scaleTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPipesPartMgr::setScaleTransform(cvf::Transform * scaleTransform)
{
    m_scaleTransform = scaleTransform;

    for (size_t wIdx = 0; wIdx != m_wellPipesPartMgrs.size(); ++ wIdx)
    {
        m_wellPipesPartMgrs[wIdx]->setScaleTransform(scaleTransform);
    }

    for (size_t wIdx = 0; wIdx != m_wellHeadPartMgrs.size(); ++ wIdx)
    {
        m_wellHeadPartMgrs[wIdx]->setScaleTransform(scaleTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPipesPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (!m_reservoirView->wellCollection()->isActive()) return;

    if (m_reservoirView->wellCollection()->wellPipeVisibility() == RimEclipseWellCollection::PIPES_FORCE_ALL_OFF) return;

    if (m_reservoirView->wellCollection()->wells.size() != m_wellPipesPartMgrs.size())
    {
        clearGeometryCache();

        for (size_t i = 0; i < m_reservoirView->wellCollection()->wells.size(); ++i)
        {
            RivWellPipesPartMgr * wppmgr = new RivWellPipesPartMgr(m_reservoirView, m_reservoirView->wellCollection()->wells[i]);
            m_wellPipesPartMgrs.push_back(wppmgr);
            wppmgr->setScaleTransform(m_scaleTransform.p());

            RivWellHeadPartMgr* wellHeadMgr = new RivWellHeadPartMgr(m_reservoirView, m_reservoirView->wellCollection()->wells[i]);
            m_wellHeadPartMgrs.push_back(wellHeadMgr);
            wellHeadMgr->setScaleTransform(m_scaleTransform.p());
        }
    }

    for (size_t wIdx = 0; wIdx != m_wellPipesPartMgrs.size(); ++ wIdx)
    {
        m_wellPipesPartMgrs[wIdx]->appendDynamicGeometryPartsToModel(model, frameIndex);
        m_wellHeadPartMgrs[wIdx]->appendDynamicGeometryPartsToModel(model, frameIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPipesPartMgr::updatePipeResultColor(size_t frameIndex)
{
    for (size_t wIdx = 0; wIdx != m_wellPipesPartMgrs.size(); ++ wIdx)
    {
        m_wellPipesPartMgrs[wIdx]->updatePipeResultColor( frameIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector< std::vector <cvf::Vec3d> >* RivReservoirPipesPartMgr::centerLineOfWellBranches(int wellIdx)
{
    if (wellIdx < static_cast<int>(m_wellPipesPartMgrs.size()))
    {
        return &(m_wellPipesPartMgrs[wellIdx]->centerLineOfWellBranches());
    }

    return NULL;
}

