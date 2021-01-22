/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RivCellFilterPartMgr.h"

#include "Rim3dView.h"
#include "RimCellFilterCollection.h"
#include "RimPolygonFilter.h"

#include "RivPolylinePartMgr.h"

#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivCellFilterPartMgr::RivCellFilterPartMgr( Rim3dView* view )
    : m_rimView( view )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivCellFilterPartMgr::~RivCellFilterPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivCellFilterPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                       const caf::DisplayCoordTransform* displayCoordTransform,
                                                       const cvf::BoundingBox&           boundingBox )
{
    createCellFilterPartManagers();

    for ( auto& partMgr : m_cellFilterPartMgrs )
    {
        partMgr->appendDynamicGeometryPartsToModel( model, displayCoordTransform, boundingBox );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivCellFilterPartMgr::createCellFilterPartManagers()
{
    std::vector<RimCellFilterCollection*> colls;
    m_rimView->descendantsIncludingThisOfType( colls );

    if ( colls.empty() ) return;
    auto coll = colls.front();

    clearGeometryCache();

    for ( auto filter : coll->filters() )
    {
        RimPolygonFilter* polyFilter = dynamic_cast<RimPolygonFilter*>( filter );
        if ( polyFilter )
        {
            RivPolylinePartMgr* ppm = new RivPolylinePartMgr( m_rimView, polyFilter, coll );
            m_cellFilterPartMgrs.push_back( ppm );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivCellFilterPartMgr::clearGeometryCache()
{
    m_cellFilterPartMgrs.clear();
}
