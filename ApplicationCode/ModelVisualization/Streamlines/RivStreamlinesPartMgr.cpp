/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RivStreamlinesPartMgr.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimStreamlineInViewCollection.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfShaderProgram.h"

#include "cafPdmPointer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivStreamlinesPartMgr::RivStreamlinesPartMgr( RimEclipseView* reservoirView )
{
    m_rimReservoirView = reservoirView;
    m_count            = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivStreamlinesPartMgr::~RivStreamlinesPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t timeStepIndex )
{
    CVF_ASSERT( model );
    if ( m_rimReservoirView.isNull() ) return;

    RimEclipseCase* eclipseCase = m_rimReservoirView->eclipseCase();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<StreamlineVisualization> streamlineVisualizations;

    RimStreamlineInViewCollection* streamlineCollection = nullptr;
    // m_rimReservoirView->streamlineCollection();

    const std::vector<RigCell>& cells = eclipseCase->mainGrid()->globalCellArray();
    std::vector<cvf::Vec3d>     tracerPoints;
    StreamlineVisualization     visualization;
    for ( size_t gcIdx = 0; gcIdx < std::max<size_t>( cells.size(), 10 ); ++gcIdx )
    {
        cvf::Vec3d cellCenter = cells[gcIdx].center();
        tracerPoints.push_back( displayCordXf->transformToDisplayCoord( cellCenter ) );
    }
    visualization.tracerPoints = tracerPoints;
    streamlineVisualizations.push_back( visualization );

    if ( !streamlineVisualizations.empty() )
    {
        m_part = createPart( *streamlineCollection, streamlineVisualizations );
        m_part->updateBoundingBox();
        model->addPart( m_part.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::updateAnimation()
{
    if ( m_part.notNull() )
    {
        if ( m_count == 1 )
        {
            cvf::ref<cvf::Effect>       effect;
            caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 0, 0, 1 ), caf::PO_1 );
            surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
            effect = surfaceGen.generateCachedEffect();
            m_part->setEffect( effect.p() );
        }
        else if ( m_count == 50 )
        {
            cvf::ref<cvf::Effect>       effect;
            caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 0, 1, 0, 1 ), caf::PO_1 );
            surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
            effect = surfaceGen.generateCachedEffect();
            m_part->setEffect( effect.p() );
        }
        else if ( m_count > 100 )
        {
            m_count = 0;
        }
        m_count++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part>
    RivStreamlinesPartMgr::createPart( const RimStreamlineInViewCollection&        streamlineCollection,
                                       const std::vector<StreamlineVisualization>& streamlineVisualizations ) const
{
    std::vector<uint>       tracerIndices;
    std::vector<cvf::Vec3f> vertices;

    // Better to reserve space already? - this would mean we need two for loops
    uint counter = 0;
    for ( StreamlineVisualization visualization : streamlineVisualizations )
    {
        for ( cvf::Vec3d tracerPoint : visualization.tracerPoints )
        {
            vertices.push_back( cvf::Vec3f( tracerPoint.x(), tracerPoint.y(), tracerPoint.z() ) );
            tracerIndices.push_back( counter++ );
        }
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntTracer =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexArrayTracer = new cvf::UIntArray( tracerIndices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUIntTracer->setIndices( indexArrayTracer.p() );
    drawable->addPrimitiveSet( indexedUIntTracer.p() );

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices );
    drawable->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>( drawable->textureCoordArray() );

    if ( lineTexCoords.isNull() )
    {
        lineTexCoords = new cvf::Vec2fArray;
    }

    const cvf::ScalarMapper* activeScalerMapper = nullptr;

    cvf::ref<cvf::Effect> effect;

    caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 0, 0, 1 ), caf::PO_1 );
    surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
    effect = surfaceGen.generateCachedEffect();

    drawable->setTextureCoordArray( lineTexCoords.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );

    return part;
}
