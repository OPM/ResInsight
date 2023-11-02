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

#include "RigBasicPlane.h"
#include "RigFaultReactivationModel.h"

#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"
#include "RivPolylinePartMgr.h"

#include "Rim3dView.h"
#include "RimFaultReactivationModel.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
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
                                                                   cvf::ModelBasicList*              vizModel,
                                                                   const caf::DisplayCoordTransform* transform,
                                                                   const cvf::BoundingBox&           boundingBox )
{
    if ( m_polylinePartMgr.isNull() ) m_polylinePartMgr = new RivPolylinePartMgr( view, m_frm.p(), m_frm.p() );

    m_polylinePartMgr->appendDynamicGeometryPartsToModel( vizModel, transform, boundingBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFaultReactivationModelPartMgr::appendMeshPartsToModel( Rim3dView*                        view,
                                                               cvf::ModelBasicList*              vizModel,
                                                               const caf::DisplayCoordTransform* transform,
                                                               const cvf::BoundingBox&           boundingBox )
{
    // auto model2d = m_frm->model();
    // if ( model2d.notNull() && model2d->isValid() && m_frm->isChecked() && m_frm->showModel() )
    //{
    //     for ( auto gridPart : m_frm->model()->allGridParts() )
    //     {
    //         auto& lines = m_frm->model()->meshLines( gridPart );

    //        std::vector<std::vector<cvf::Vec3d>> displayPoints;
    //        for ( const auto& pts : lines )
    //        {
    //            displayPoints.push_back( transform->transformToDisplayCoords( pts ) );
    //        }

    //        cvf::ref<cvf::DrawableGeo> drawableGeo = RivPolylineGenerator::createSetOfLines( displayPoints );
    //        cvf::ref<cvf::Part>        part        = new cvf::Part;
    //        part->setName( "FaultReactMeshLines" );
    //        part->setDrawable( drawableGeo.p() );

    //        caf::MeshEffectGenerator effgen( cvf::Color3::LIGHT_GRAY );
    //        effgen.setLineWidth( 1.5 );
    //        effgen.setLineStipple( false );
    //        cvf::ref<cvf::Effect> eff = effgen.generateCachedEffect();

    //        part->setEffect( eff.p() );
    //        part->setPriority( RivPartPriority::PartType::MeshLines );

    //        vizModel->addPart( part.p() );
    //    }
    //}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFaultReactivationModelPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              vizModel,
                                                                   const caf::DisplayCoordTransform* displayCoordTransform,
                                                                   const cvf::BoundingBox&           boundingBox )
{
    if ( !m_canUseShaders ) return;

    // auto theModel = m_frm->model();
    // if ( theModel->isValid() && m_frm->showModel() )
    //{
    //     for ( auto part : theModel->allModelParts() )
    //     {
    //         cvf::Vec3dArray displayPoints;
    //         displayPoints.reserve( theModel->rect( part ).size() );

    //        for ( auto& vOrg : theModel->rect( part ) )
    //        {
    //            displayPoints.add( displayCoordTransform->transformToDisplayCoord( vOrg ) );
    //        }

    //        cvf::ref<cvf::Part> quadPart = createSingleTexturedQuadPart( displayPoints, theModel->texture( part ), false );

    //        vizModel->addPart( quadPart.p() );
    //    }
    //}
}
