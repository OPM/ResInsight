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

#include "RiaGuiApplication.h"

#include "RivPartPriority.h"
#include "RivPolylinePartMgr.h"
#include "RivSeismicSectionSourceInfo.h"

#include "Rim3dView.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicAlphaMapper.h"
#include "RimSeismicSection.h"

#include "RigTexturedSection.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmObject.h"

#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScalarMapper.h"

#include <zgyaccess/seismicslice.h>

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
void RivSeismicSectionPartMgr::appendPolylinePartsToModel( Rim3dView*                        view,
                                                           cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* transform,
                                                           const cvf::BoundingBox&           boundingBox )
{
    if ( m_polylinePartMgr.isNull() ) m_polylinePartMgr = new RivPolylinePartMgr( view, m_section.p(), m_section.p() );

    m_polylinePartMgr->appendDynamicGeometryPartsToModel( model, transform, boundingBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSeismicSectionPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                                           const cvf::BoundingBox&           boundingBox )
{
    if ( !m_canUseShaders ) return;

    auto texSection = m_section->texturedSection();

    for ( int i = 0; i < (int)texSection->partsCount(); i++ )
    {
        auto& part = texSection->part( i );

        cvf::Vec3dArray displayPoints;
        displayPoints.reserve( part.rect.size() );

        for ( auto& vOrg : part.rect )
        {
            displayPoints.add( displayCoordTransform->transformToDisplayCoord( vOrg ) );
        }

        if ( part.texture.isNull() )
        {
            if ( ( part.sliceData == nullptr ) || part.sliceData.get()->isEmpty() ) continue;

            part.texture = createImageFromData( part.sliceData.get() );
        }

        cvf::ref<cvf::Part> quadPart = createSingleTexturedQuadPart( displayPoints, part.texture, m_section->isTransparent() );

        cvf::ref<RivSeismicSectionSourceInfo> si = new RivSeismicSectionSourceInfo( m_section, i );
        quadPart->setSourceInfo( si.p() );

        model->addPart( quadPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::TextureImage* RivSeismicSectionPartMgr::createImageFromData( ZGYAccess::SeismicSliceData* data )
{
    const int width = data->width();
    const int depth = data->depth();

    cvf::TextureImage* textureImage = new cvf::TextureImage();
    textureImage->allocate( width, depth );

    auto   legend = m_section->legendConfig();
    float* pData  = data->values();

    if ( ( legend == nullptr ) || ( pData == nullptr ) )
    {
        textureImage->fill( cvf::Color4ub( 0, 0, 0, 0 ) );
        return textureImage;
    }

    const bool isTransparent = m_section->isTransparent();

    auto alphaMapper = m_section->alphaValueMapper();
    auto colorMapper = legend->scalarMapper();

    for ( int i = 0; i < width; i++ )
    {
        for ( int j = depth - 1; j >= 0; j-- )
        {
            auto rgb = colorMapper->mapToColor( *pData );

            cvf::ubyte uAlpha = 255;
            if ( isTransparent ) uAlpha = alphaMapper->alphaValue( *pData );

            textureImage->setPixel( i, j, cvf::Color4ub( rgb, uAlpha ) );

            pData++;
        }
    }

    return textureImage;
}
