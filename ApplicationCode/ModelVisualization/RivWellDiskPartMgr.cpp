/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RivWellDiskPartMgr.h"

#include "RiaGuiApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigSimWellData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"

#include "RivDiskGeometryGenerator.h"
#include "RivPartPriority.h"
#include "RivSectionFlattner.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivTextLabelSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellDiskPartMgr::RivWellDiskPartMgr( RimSimWellInView* well )
    : m_rimWell( well )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellDiskPartMgr::~RivWellDiskPartMgr() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellDiskPartMgr::buildWellDiskParts( size_t                            frameIndex,
                                             const caf::DisplayCoordTransform* displayXf,
                                             bool                              doFlatten,
                                             double                            xOffset )
{
    clearAllGeometry();

    if ( !viewWithSettings() ) return;

    RimSimWellInView* well = m_rimWell;

    double characteristicCellSize = viewWithSettings()->ownerCase()->characteristicCellSize();

    cvf::Vec3d whEndPos;
    cvf::Vec3d whStartPos;
    {
        well->wellHeadTopBottomPosition( static_cast<int>( frameIndex ), &whEndPos, &whStartPos );

        if ( doFlatten )
        {
            whEndPos.x()   = xOffset;
            whEndPos.y()   = 0.0;
            whStartPos.x() = xOffset;
            whStartPos.y() = 0.0;
            whEndPos       = displayXf->scaleToDisplaySize( whEndPos );
            whStartPos     = displayXf->scaleToDisplaySize( whStartPos );
            whEndPos.z() += characteristicCellSize;
        }
        else
        {
            whEndPos   = displayXf->transformToDisplayCoord( whEndPos );
            whStartPos = displayXf->transformToDisplayCoord( whStartPos );
            whEndPos.z() += characteristicCellSize;
        }
    }

    if ( !well->simWellData()->hasWellResult( frameIndex ) ) return;

    const RigWellResultFrame& wellResultFrame = well->simWellData()->wellResultFrame( frameIndex );

    double pipeRadius              = m_rimWell->pipeRadius();
    int    pipeCrossSectionVxCount = m_rimWell->pipeCrossSectionVertexCount();

    // Upper part of simulation well pipe is defined to use branch index 0
    cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo( m_rimWell, 0 );

    cvf::Vec3d diskPosition = whEndPos;
    diskPosition.z() += pipeRadius;

    // Well disk geometry
    double arrowLength = characteristicCellSize * simWellInViewCollection()->wellHeadScaleFactor() *
                         m_rimWell->wellHeadScaleFactor();

    cvf::Vec3d textPosition = diskPosition;
    textPosition.z() += 1.2 * arrowLength;

    cvf::Mat4f matr;

    double ijScaleFactor = arrowLength / 6;
    matr( 0, 0 ) *= ijScaleFactor;
    matr( 1, 1 ) *= ijScaleFactor;
    matr( 2, 2 ) *= arrowLength;

    matr.setTranslation( cvf::Vec3f( diskPosition ) );

    cvf::GeometryBuilderFaceList builder;
    RivDiskGeometryGenerator     gen;
    gen.setRelativeRadius( 2.5f );
    gen.setRelativeLength( 0.1f );
    gen.setNumSlices( pipeCrossSectionVxCount );
    gen.generate( &builder );

    cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();
    cvf::ref<cvf::UIntArray>  faceList = builder.faceList();

    size_t i;
    for ( i = 0; i < vertices->size(); i++ )
    {
        cvf::Vec3f v = vertices->get( i );
        v.transformPoint( matr );
        vertices->set( i, v );
    }

    cvf::ref<cvf::DrawableGeo> geo1 = new cvf::DrawableGeo;
    geo1->setVertexArray( vertices.p() );
    geo1->setFromFaceList( *faceList );
    geo1->computeNormals();

    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivWellDiskPartMgr: disk " + cvfqt::Utils::toString( well->name() ) );
        part->setDrawable( geo1.p() );

        cvf::Color4f color = cvf::Color4f( m_rimWell->wellPipeColor() );

        caf::SurfaceEffectGenerator surfaceGen( color, caf::PO_1 );
        if ( viewWithSettings() && viewWithSettings()->isLightingDisabled() )
        {
            surfaceGen.enableLighting( false );
        }
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect( eff.p() );
        part->setSourceInfo( sourceInfo.p() );

        m_wellDiskPart = part;
    }

    if ( well->showWellLabel() && !well->name().isEmpty() )
    {
        cvf::Font* font = RiaGuiApplication::instance()->defaultWellLabelFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont( font );
        drawableText->setCheckPosVisible( false );
        drawableText->setDrawBorder( false );
        drawableText->setDrawBackground( false );
        drawableText->setVerticalAlignment( cvf::TextDrawer::CENTER );
        drawableText->setTextColor( simWellInViewCollection()->wellLabelColor() );

        cvf::String cvfString = cvfqt::Utils::toString( m_rimWell->name() );

        cvf::Vec3f textCoord( textPosition );
        drawableText->addText( cvfString, textCoord );

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivWellDiskPartMgr: text " + cvfString );
        part->setDrawable( drawableText.p() );

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect( eff.p() );
        part->setPriority( RivPartPriority::PartType::Text );

        part->setSourceInfo( new RivTextLabelSourceInfo( m_rimWell, cvfString, textCoord ) );

        m_wellDiskLabelPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellDiskPartMgr::clearAllGeometry()
{
    m_wellDiskPart      = nullptr;
    m_wellDiskLabelPart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellDiskPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                            size_t                            frameIndex,
                                                            const caf::DisplayCoordTransform* displayXf )
{
    if ( m_rimWell.isNull() ) return;
    if ( !viewWithSettings() ) return;

    if ( !m_rimWell->isWellPipeVisible( frameIndex ) ) return;

    buildWellDiskParts( frameIndex, displayXf, false, 0.0 );

    if ( m_rimWell->showWellLabel() && m_wellDiskLabelPart.notNull() )
    {
        model->addPart( m_wellDiskLabelPart.p() );
    }

    if ( m_rimWell->showWellDisks() && m_wellDiskPart.notNull() )
    {
        model->addPart( m_wellDiskPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellDiskPartMgr::appendFlattenedDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                     size_t                            frameIndex,
                                                                     const caf::DisplayCoordTransform* displayXf,
                                                                     double                            xOffset )
{
    if ( m_rimWell.isNull() ) return;
    if ( !viewWithSettings() ) return;

    if ( !m_rimWell->isWellPipeVisible( frameIndex ) ) return;

    buildWellDiskParts( frameIndex, displayXf, true, xOffset );

    if ( m_rimWell->showWellLabel() && m_wellDiskLabelPart.notNull() )
    {
        model->addPart( m_wellDiskLabelPart.p() );
    }

    if ( m_rimWell->showWellDisks() && m_wellDiskPart.notNull() )
    {
        model->addPart( m_wellDiskPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RivWellDiskPartMgr::viewWithSettings()
{
    Rim3dView* view = nullptr;
    if ( m_rimWell ) m_rimWell->firstAncestorOrThisOfType( view );

    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInViewCollection* RivWellDiskPartMgr::simWellInViewCollection()
{
    RimSimWellInViewCollection* wellCollection = nullptr;
    if ( m_rimWell ) m_rimWell->firstAncestorOrThisOfType( wellCollection );

    return wellCollection;
}
