/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RivReservoirWellSpheresPartMgr.h"

#include "RivWellSpheresPartMgr.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

#include "cvfTransform.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirWellSpheresPartMgr::RivReservoirWellSpheresPartMgr(RimEclipseView* reservoirView)
{
    m_reservoirView = reservoirView;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirWellSpheresPartMgr::~RivReservoirWellSpheresPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirWellSpheresPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (m_reservoirView->wellCollection()->wellSphereVisibility == RimEclipseWellCollection::PIPES_FORCE_ALL_OFF) return;
    
    if (!m_reservoirView->wellCollection()->isActive()) return;

    if (m_reservoirView->wellCollection()->wells.size() != m_wellSpheresPartMgrs.size())
    {
        for (RimEclipseWell* rimWell : m_reservoirView->wellCollection()->wells())
        {
            RivWellSpheresPartMgr* wppmgr = new RivWellSpheresPartMgr(m_reservoirView, rimWell);
            m_wellSpheresPartMgrs.push_back(wppmgr);
        }
    }

    for (size_t i = 0; i < m_wellSpheresPartMgrs.size(); i++)
    {

        bool showGeo = false;
        
        
        if (m_reservoirView->wellCollection()->wells[i]->showWell())
        {
       
            if (m_reservoirView->wellCollection->wellSphereVisibility == RimEclipseWellCollection::PIPES_FORCE_ALL_ON)
            {
                showGeo = true;
            }
            
            else if (m_reservoirView->wellCollection->wellSphereVisibility == RimEclipseWellCollection::PIPES_INDIVIDUALLY && m_reservoirView->wellCollection()->wells[i]->showWellSpheres() )
            {
                showGeo = true;
            }
            
            else if (m_reservoirView->wellCollection->wellSphereVisibility == RimEclipseWellCollection::PIPES_OPEN_IN_VISIBLE_CELLS && m_reservoirView->wellCollection->wells[i]->visibleCellsInstersectsWell(frameIndex))
            {
                showGeo = true;
            }
            
        }


        if (showGeo)
        { 
            m_wellSpheresPartMgrs.at(i)->appendDynamicGeometryPartsToModel(model, frameIndex);
        }
    }
}

