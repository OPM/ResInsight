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

#include "RivSeismicSectionPartMgr.h"

#include "RivPolylinePartMgr.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmObject.h"

#include "cvfBoundingBox.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSeismicSectionPartMgr::RivSeismicSectionPartMgr( RimSeismicSection* section )
    : m_section( section )
{
    CVF_ASSERT( section );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSeismicSectionPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                                           const cvf::BoundingBox&           boundingBox )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSeismicSectionPartMgr::appendPolylinePartsToModel( Rim3dView*                        view,
                                                           cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* transform,
                                                           const cvf::BoundingBox&           boundingBox )
{
    m_polylinePartMgr = new RivPolylinePartMgr( view, m_section.p(), m_section.p() );

    m_polylinePartMgr->appendDynamicGeometryPartsToModel( model, transform, boundingBox );
}
