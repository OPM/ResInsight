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

#include "RivReservoirSimWellsPartMgr.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"

#include "RivSimWellPipesPartMgr.h"
#include "RivWellConnectionsPartMgr.h"
#include "RivWellDiskPartMgr.h"
#include "RivWellHeadPartMgr.h"
#include "RivWellSpheresPartMgr.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

#include "cvfTransform.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivReservoirSimWellsPartMgr::RivReservoirSimWellsPartMgr( RimEclipseView* reservoirView )
{
    m_reservoirView = reservoirView;

    m_scaleTransform = new cvf::Transform();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivReservoirSimWellsPartMgr::~RivReservoirSimWellsPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReservoirSimWellsPartMgr::clearGeometryCache()
{
    m_wellPipesPartMgrs.clear();
    m_wellHeadPartMgrs.clear();
    m_wellSpheresPartMgrs.clear();
    m_wellDiskPartMgrs.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReservoirSimWellsPartMgr::scheduleGeometryRegen()
{
    m_wellSpheresPartMgrs.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReservoirSimWellsPartMgr::setScaleTransform( cvf::Transform* scaleTransform )
{
    m_scaleTransform = scaleTransform;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReservoirSimWellsPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t frameIndex )
{
    if ( !m_reservoirView->wellCollection()->isActive() ) return;

    if ( m_reservoirView->wellCollection()->wells.size() != m_wellPipesPartMgrs.size() )
    {
        clearGeometryCache();

        for ( size_t i = 0; i < m_reservoirView->wellCollection()->wells.size(); ++i )
        {
            RivSimWellPipesPartMgr* wppmgr = new RivSimWellPipesPartMgr( m_reservoirView->wellCollection()->wells[i] );
            m_wellPipesPartMgrs.push_back( wppmgr );

            RivWellHeadPartMgr* wellHeadMgr = new RivWellHeadPartMgr( m_reservoirView->wellCollection()->wells[i] );
            m_wellHeadPartMgrs.push_back( wellHeadMgr );
        }
    }

    for ( size_t wIdx = 0; wIdx != m_wellPipesPartMgrs.size(); ++wIdx )
    {
        m_wellPipesPartMgrs[wIdx]->appendDynamicGeometryPartsToModel( model,
                                                                      frameIndex,
                                                                      m_reservoirView->displayCoordTransform().p() );
        m_wellHeadPartMgrs[wIdx]->appendDynamicGeometryPartsToModel( model,
                                                                     frameIndex,
                                                                     m_reservoirView->displayCoordTransform().p() );
    }

    // Well disks
    if ( m_reservoirView->wellCollection()->wells.size() != m_wellDiskPartMgrs.size() )
    {
        m_wellDiskPartMgrs.clear();

        for ( size_t i = 0; i < m_reservoirView->wellCollection()->wells.size(); ++i )
        {
            RivWellDiskPartMgr* wellDiskMgr = new RivWellDiskPartMgr( m_reservoirView->wellCollection()->wells[i] );
            m_wellDiskPartMgrs.push_back( wellDiskMgr );
        }
    }

    for ( size_t wIdx = 0; wIdx != m_wellDiskPartMgrs.size(); ++wIdx )
    {
        m_wellDiskPartMgrs[wIdx]->appendDynamicGeometryPartsToModel( model,
                                                                     frameIndex,
                                                                     m_reservoirView->displayCoordTransform().p() );
    }

    // Well spheres

    if ( m_reservoirView->wellCollection()->wells.size() != m_wellSpheresPartMgrs.size() )
    {
        m_wellSpheresPartMgrs.clear();

        for ( RimSimWellInView* rimWell : m_reservoirView->wellCollection()->wells() )
        {
            RivWellSpheresPartMgr* wppmgr = new RivWellSpheresPartMgr( m_reservoirView, rimWell );
            m_wellSpheresPartMgrs.push_back( wppmgr );
        }
    }

    for ( size_t wIdx = 0; wIdx < m_wellSpheresPartMgrs.size(); wIdx++ )
    {
        if ( m_reservoirView->wellCollection()->wells[wIdx]->isWellSpheresVisible( frameIndex ) )
        {
            m_wellSpheresPartMgrs[wIdx]->appendDynamicGeometryPartsToModel( model, frameIndex );
        }
    }

    // Well Connection Arrows
    if ( m_reservoirView->wellCollection()->showWellCommunicationLines() )
    {
        for ( RimSimWellInView* rimWell : m_reservoirView->wellCollection()->wells() )
        {
            cvf::ref<RivWellConnectionsPartMgr> wppmgr = new RivWellConnectionsPartMgr( m_reservoirView, rimWell );
            wppmgr->appendDynamicGeometryPartsToModel( model, frameIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivReservoirSimWellsPartMgr::updatePipeResultColor( size_t frameIndex )
{
    for ( size_t wIdx = 0; wIdx != m_wellPipesPartMgrs.size(); ++wIdx )
    {
        m_wellPipesPartMgrs[wIdx]->updatePipeResultColor( frameIndex );
    }
}
