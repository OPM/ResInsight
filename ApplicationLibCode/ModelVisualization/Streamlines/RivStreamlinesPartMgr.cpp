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
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimStreamlineInViewCollection.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigTracer.h"
#include "RigTracerPoint.h"
#include "RigWellResultPoint.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUIntScoped.h"
#include "cvfShaderProgram.h"

#include "cafPdmPointer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivStreamlinesPartMgr::RivStreamlinesPartMgr( RimEclipseView* reservoirView )
{
    m_rimReservoirView = reservoirView;
    m_count            = 0;
    m_currentT         = 0;
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
    m_streamlines.clear();

    RimStreamlineInViewCollection* streamlineCollection = m_rimReservoirView->streamlineCollection();

    if ( !streamlineCollection->isActive() ) return;

    CVF_ASSERT( model );
    if ( m_rimReservoirView.isNull() ) return;

    RimEclipseCase* eclipseCase = m_rimReservoirView->eclipseCase();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    for ( const RigTracer& tracer : streamlineCollection->tracers() )
    {
        Streamline streamline;
        for ( size_t i = 0; i < tracer.tracerPoints().size() - 1; i++ )
        {
            streamline.appendTracerPoint( tracer.tracerPoints()[i].position() );
            streamline.appendAbsVelocity( tracer.tracerPoints()[i].absValue() );
            streamline.appendPhase( tracer.tracerPoints()[i].phaseType() );
        }
        m_streamlines.push_back( streamline );
    }
    for ( Streamline& streamline : m_streamlines )
    {
        if ( streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::ANIMATION ||
             streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::MANUAL )
        {
            if ( streamline.countTracerPoints() > 1 )
            {
                model->addPart( createPart( *streamlineCollection, streamline ).p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::updateAnimation()
{
    RimStreamlineInViewCollection* streamlineCollection = m_rimReservoirView->streamlineCollection();
    if ( streamlineCollection &&
         ( streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::ANIMATION ||
           streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::MANUAL ) )
    {
        for ( Streamline& streamline : m_streamlines )
        {
            if ( streamline.getPart().notNull() && dynamic_cast<cvf::DrawableGeo*>( streamline.getPart()->drawable() ) )
            {
                auto primitiveSet = dynamic_cast<cvf::PrimitiveSetIndexedUIntScoped*>(
                    static_cast<cvf::DrawableGeo*>( streamline.getPart()->drawable() )->primitiveSet( 0 ) );

                if ( primitiveSet )
                {
                    size_t startIndex = 0;
                    size_t endIndex   = primitiveSet->indices()->size();

                    if ( streamlineCollection->visualizationMode() ==
                         RimStreamlineInViewCollection::VisualizationMode::ANIMATION )
                    {
                        streamline.incrementAnimationIndex( streamlineCollection->animationSpeed() );

                        startIndex = streamline.getAnimationIndex();

                        // Make sure we have an even animation index, as this index is used to define start of a line
                        // segment
                        startIndex /= 2;
                        startIndex *= 2;

                        endIndex = std::min( endIndex, startIndex + streamlineCollection->tracerLength() );
                    }
                    else
                    {
                        endIndex = std::min( endIndex, streamlineCollection->animationIndex() * 2 );
                    }

                    primitiveSet->setScope( startIndex, endIndex - startIndex );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::createPart( const RimStreamlineInViewCollection& streamlineCollection,
                                                       Streamline&                          streamline )
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<uint> indices;
    // Each point is used twice except the first and the last one
    indices.reserve( streamline.countTracerPoints() * 2 - 2 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( streamline.countTracerPoints() * 2 - 2 );

    uint count = 0;
    for ( size_t i = 0; i < streamline.countTracerPoints(); i++ )
    {
        vertices.push_back( cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamline.getTracerPoint( i ) ) ) );
        indices.push_back( count++ );
        if ( i > 0 && i < streamline.countTracerPoints() - 1 )
        {
            vertices.push_back( cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamline.getTracerPoint( i ) ) ) );
            indices.push_back( count++ );
        }
    }

    cvf::ref<cvf::PrimitiveSetIndexedUIntScoped> indexedUIntLine =
        new cvf::PrimitiveSetIndexedUIntScoped( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexedArrayLine = new cvf::UIntArray( indices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUIntLine->setIndices( indexedArrayLine.p(), 0, 0 );
    drawable->addPrimitiveSet( indexedUIntLine.p() );

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices );
    drawable->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>( drawable->textureCoordArray() );

    if ( lineTexCoords.isNull() )
    {
        lineTexCoords = new cvf::Vec2fArray;
    }

    cvf::ref<cvf::Effect> effect;

    const cvf::ScalarMapper* activeScalarMapper = streamlineCollection.legendConfig()->scalarMapper();
    createResultColorTextureCoords( lineTexCoords.p(), streamline, activeScalarMapper );

    caf::ScalarMapperMeshEffectGenerator meshEffGen( activeScalarMapper );
    effect = meshEffGen.generateCachedEffect();

    drawable->setTextureCoordArray( lineTexCoords.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );
    part->updateBoundingBox();
    streamline.setPart( part );

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::createResultColorTextureCoords( cvf::Vec2fArray*         textureCoords,
                                                            const Streamline&        streamline,
                                                            const cvf::ScalarMapper* mapper )
{
    CVF_ASSERT( textureCoords );
    CVF_ASSERT( mapper );

    RimStreamlineInViewCollection* streamlineCollection = m_rimReservoirView->streamlineCollection();
    CVF_ASSERT( streamlineCollection != nullptr );

    size_t vertexCount = streamline.countTracerPoints() * 2 - 2;
    if ( textureCoords->capacity() != vertexCount ) textureCoords->reserve( vertexCount );

    for ( size_t i = 0; i < streamline.countTracerPoints(); i++ )
    {
        cvf::Vec2f texCoord = mapper->mapToTextureCoord( streamline.getAbsVelocity( i ) );

        if ( streamlineCollection->colorMode() == RimStreamlineInViewCollection::ColorMode::PHASE_COLORS )
        {
            double phaseValue = 0.0;
            auto   phase      = streamline.getPhase( i );
            if ( phase == RiaDefines::PhaseType::GAS_PHASE )
            {
                phaseValue = 1.0;
            }
            else if ( phase == RiaDefines::PhaseType::OIL_PHASE )
            {
                phaseValue = 0.5;
            }
            else if ( phase == RiaDefines::PhaseType::WATER_PHASE )
            {
                phaseValue = 0.0;
            }

            texCoord.x() = phaseValue;
        }

        textureCoords->add( texCoord );
        if ( i > 0 && i < streamline.countTracerPoints() - 1 )
        {
            textureCoords->add( texCoord );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::setAlpha( cvf::ref<cvf::Part> part, float alpha )
{
    if ( part.notNull() )
    {
        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( alpha, alpha, alpha, 1 ), caf::PO_1 );
        surfaceGen.enableLighting( false );
        cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();
        part->setEffect( effect.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::appendTracerPoint( cvf::Vec3d point )
{
    tracerPoints.push_back( point );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::appendAbsVelocity( double velocity )
{
    absVelocities.push_back( velocity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::appendPhase( RiaDefines::PhaseType phase )
{
    dominantPhases.push_back( phase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::clear()
{
    tracerPoints.clear();
    absVelocities.clear();
    dominantPhases.clear();
    delete part.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::Streamline::getPart()
{
    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivStreamlinesPartMgr::Streamline::getTracerPoint( size_t index ) const
{
    return tracerPoints[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivStreamlinesPartMgr::Streamline::getAbsVelocity( size_t index ) const
{
    return absVelocities[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RivStreamlinesPartMgr::Streamline::getPhase( size_t index ) const
{
    return dominantPhases[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivStreamlinesPartMgr::Streamline::countTracerPoints() const
{
    return tracerPoints.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::setPart( cvf::ref<cvf::Part> part )
{
    this->part = part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivStreamlinesPartMgr::Streamline::getAnimationIndex() const
{
    return animIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::incrementAnimationIndex( size_t increment )
{
    animIndex += increment;

    // Make sure we have an even animation index, as this index is used to define start of a line segment
    animIndex /= 2;
    animIndex *= 2;

    if ( animIndex >= tracerPoints.size() * 2 - 2 )
    {
        animIndex = 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::setAnimationIndex( size_t index )
{
    animIndex = index;
    if ( animIndex >= tracerPoints.size() * 2 - 2 )
    {
        animIndex = tracerPoints.size() * 2 - 2;
    }
}
