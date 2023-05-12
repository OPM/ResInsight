/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RivFaultReactivationModelPartMgr.h"

#include "RiaGuiApplication.h"

#include "RivPolylinePartMgr.h"

#include "Rim3dView.h"
#include "RimFaultReactivationModel.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmObject.h"

#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFaultReactivationModelPartMgr::RivFaultReactivationModelPartMgr( RimFaultReactivationModel* frm )
    : m_frm( frm )
{
    CVF_ASSERT( frm );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFaultReactivationModelPartMgr::appendPolylinePartsToModel( Rim3dView*                        view,
                                                                   cvf::ModelBasicList*              model,
                                                                   const caf::DisplayCoordTransform* transform,
                                                                   const cvf::BoundingBox&           boundingBox )
{
    if ( m_polylinePartMgr.isNull() ) m_polylinePartMgr = new RivPolylinePartMgr( view, m_frm.p(), m_frm.p() );

    m_polylinePartMgr->appendDynamicGeometryPartsToModel( model, transform, boundingBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFaultReactivationModelPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                   const caf::DisplayCoordTransform* displayCoordTransform,
                                                                   const cvf::BoundingBox&           boundingBox )
{
}
