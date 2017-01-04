/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RivReservoirFracturesPartMgr.h"

#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimEclipseWell.h"

#include "RivWellFracturesPartMgr.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirFracturesPartMgr::RivReservoirFracturesPartMgr(RimEclipseView* reservoirView)
{
    m_reservoirView = reservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirFracturesPartMgr::~RivReservoirFracturesPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFracturesPartMgr::clearGeometryCache()
{
    m_wellFracturesPartMgrs.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFracturesPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
//    if (m_reservoirView->wellCollection()->wellSphereVisibility == RimEclipseWellCollection::PIPES_FORCE_ALL_OFF) return;
    
    if (!m_reservoirView->wellCollection()->isActive()) return;

    if (m_reservoirView->wellCollection()->wells.size() != m_wellFracturesPartMgrs.size())
    {
        clearGeometryCache();

        for (RimEclipseWell* rimWell : m_reservoirView->wellCollection()->wells())
        {
            RivWellFracturesPartMgr* wppmgr = new RivWellFracturesPartMgr(rimWell);
            m_wellFracturesPartMgrs.push_back(wppmgr);
        }
    }

    for (size_t i = 0; i < m_wellFracturesPartMgrs.size(); i++)
    {
//        if (m_reservoirView->wellCollection()->wells[i]->isWellSpheresVisible(frameIndex))
        {
            m_wellFracturesPartMgrs.at(i)->appendDynamicGeometryPartsToModel(model, frameIndex);
        }
    }

    // Well path fractures

}

