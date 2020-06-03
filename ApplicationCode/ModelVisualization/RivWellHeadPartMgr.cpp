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

#include "RivWellHeadPartMgr.h"

#include "RiaGuiApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"

#include "RivPartPriority.h"
#include "RivPipeGeometryGenerator.h"
#include "RivSectionFlattner.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivTextLabelSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfArrowGenerator.h"
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
RivWellHeadPartMgr::RivWellHeadPartMgr( RimSimWellInView* well )
    : m_rimWell( well )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellHeadPartMgr::~RivWellHeadPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::buildWellHeadParts( size_t                            frameIndex,
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

    if ( wellResultFrame.m_isOpen )
    {
        // Use slightly larger well head arrow when well is open
        pipeRadius *= 1.1;
    }

    // Upper part of simulation well pipe is defined to use branch index 0
    cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo( m_rimWell, 0 );

    cvf::Vec3d arrowPosition = whEndPos;
    arrowPosition.z() += pipeRadius;

    // Well head pipe geometry
    {
        cvf::ref<cvf::Vec3dArray> wellHeadPipeCoords = new cvf::Vec3dArray;
        wellHeadPipeCoords->resize( 2 );
        wellHeadPipeCoords->set( 0, whStartPos );
        wellHeadPipeCoords->set( 1, whEndPos );

        cvf::ref<RivPipeGeometryGenerator> pipeGeomGenerator = new RivPipeGeometryGenerator;
        pipeGeomGenerator->setPipeCenterCoords( wellHeadPipeCoords.p() );
        pipeGeomGenerator->setCrossSectionVertexCount( pipeCrossSectionVxCount );

        pipeGeomGenerator->setRadius( pipeRadius );

        cvf::ref<cvf::DrawableGeo> pipeSurface        = pipeGeomGenerator->createPipeSurface();
        cvf::ref<cvf::DrawableGeo> centerLineDrawable = pipeGeomGenerator->createCenterLine();

        if ( pipeSurface.notNull() )
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivWellHeadPartMgr: surface " + cvfqt::Utils::toString( well->name() ) );
            part->setDrawable( pipeSurface.p() );

            caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( well->wellPipeColor() ), caf::PO_1 );
            if ( viewWithSettings() && viewWithSettings()->isLightingDisabled() )
            {
                surfaceGen.enableLighting( false );
            }

            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            part->setEffect( eff.p() );
            part->setSourceInfo( sourceInfo.p() );

            m_wellHeadPipeSurfacePart = part;
        }

        if ( centerLineDrawable.notNull() )
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivWellHeadPartMgr: centerline " + cvfqt::Utils::toString( well->name() ) );
            part->setDrawable( centerLineDrawable.p() );

            caf::MeshEffectGenerator meshGen( well->wellPipeColor() );
            cvf::ref<cvf::Effect>    eff = meshGen.generateCachedEffect();

            part->setEffect( eff.p() );
            part->setSourceInfo( sourceInfo.p() );

            m_wellHeadPipeCenterPart = part;
            part->updateBoundingBox();
            CVF_ASSERT( part->boundingBox().isValid() );
        }
    }

    double arrowLength =
        characteristicCellSize * simWellInViewCollection()->wellHeadScaleFactor() * m_rimWell->wellHeadScaleFactor();

    if ( wellResultFrame.m_isOpen )
    {
        // Use slightly larger well head arrow when well is open
        arrowLength = 1.1 * arrowLength;
    }

    cvf::Vec3d textPosition = arrowPosition;
    textPosition.z() += 1.2 * arrowLength;

    cvf::Mat4f matr;
    if ( wellResultFrame.m_productionType != RigWellResultFrame::PRODUCER )
    {
        matr = cvf::Mat4f::fromRotation( cvf::Vec3f( 1.0f, 0.0f, 0.0f ), cvf::Math::toRadians( 180.0f ) );
    }

    double ijScaleFactor = arrowLength / 6;
    if ( wellResultFrame.m_isOpen )
    {
        ijScaleFactor *= 1.1;
    }
    matr( 0, 0 ) *= ijScaleFactor;
    matr( 1, 1 ) *= ijScaleFactor;
    matr( 2, 2 ) *= arrowLength;

    if ( wellResultFrame.m_productionType != RigWellResultFrame::PRODUCER )
    {
        arrowPosition.z() += arrowLength;
    }

    matr.setTranslation( cvf::Vec3f( arrowPosition ) );

    cvf::GeometryBuilderFaceList builder;
    cvf::ArrowGenerator          gen;
    gen.setShaftRelativeRadius( 0.5f );
    gen.setHeadRelativeRadius( 1.0f );
    gen.setHeadRelativeLength( 0.4f );
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
        part->setName( "RivWellHeadPartMgr: arrow " + cvfqt::Utils::toString( well->name() ) );
        part->setDrawable( geo1.p() );

        cvf::Color4f headColor( cvf::Color3::GRAY );

        RimSimWellInViewCollection* wellColl = nullptr;
        if ( m_rimWell )
        {
            m_rimWell->firstAncestorOrThisOfType( wellColl );
        }

        if ( wellColl && wellColl->showConnectionStatusColors() )
        {
            if ( wellResultFrame.m_isOpen )
            {
                if ( wellResultFrame.m_productionType == RigWellResultFrame::PRODUCER )
                {
                    headColor = cvf::Color4f( cvf::Color3::GREEN );
                }
                else if ( wellResultFrame.m_productionType == RigWellResultFrame::OIL_INJECTOR )
                {
                    headColor = cvf::Color4f( cvf::Color3::ORANGE );
                }
                else if ( wellResultFrame.m_productionType == RigWellResultFrame::GAS_INJECTOR )
                {
                    headColor = cvf::Color4f( cvf::Color3::RED );
                }
                else if ( wellResultFrame.m_productionType == RigWellResultFrame::WATER_INJECTOR )
                {
                    headColor = cvf::Color4f( cvf::Color3::BLUE );
                }
            }
        }
        else
        {
            headColor = cvf::Color4f( m_rimWell->wellPipeColor() );
        }

        caf::SurfaceEffectGenerator surfaceGen( headColor, caf::PO_1 );
        if ( viewWithSettings() && viewWithSettings()->isLightingDisabled() )
        {
            surfaceGen.enableLighting( false );
        }
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect( eff.p() );
        part->setSourceInfo( sourceInfo.p() );

        m_wellHeadArrowPart = part;
    }

    // Show labels for well heads only when well disks are disabled:
    // well disk labels are preferred since they have more info.
    if ( well->showWellLabel() && !well->name().isEmpty() && !well->showWellDisks() )
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
        part->setName( "RivWellHeadPartMgr: text " + cvfString );
        part->setDrawable( drawableText.p() );

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect( eff.p() );
        part->setPriority( RivPartPriority::PartType::Text );

        part->setSourceInfo( new RivTextLabelSourceInfo( m_rimWell, cvfString, textCoord ) );

        m_wellHeadLabelPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::clearAllGeometry()
{
    m_wellHeadArrowPart       = nullptr;
    m_wellHeadLabelPart       = nullptr;
    m_wellHeadPipeCenterPart  = nullptr;
    m_wellHeadPipeSurfacePart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                            size_t                            frameIndex,
                                                            const caf::DisplayCoordTransform* displayXf )
{
    if ( m_rimWell.isNull() ) return;
    if ( !viewWithSettings() ) return;

    if ( !m_rimWell->isWellPipeVisible( frameIndex ) ) return;

    buildWellHeadParts( frameIndex, displayXf, false, 0.0 );

    // Always add pipe part of well head
    if ( m_wellHeadPipeCenterPart.notNull() ) model->addPart( m_wellHeadPipeCenterPart.p() );
    if ( m_wellHeadPipeSurfacePart.notNull() ) model->addPart( m_wellHeadPipeSurfacePart.p() );

    if ( m_rimWell->showWellLabel() && m_wellHeadLabelPart.notNull() )
    {
        model->addPart( m_wellHeadLabelPart.p() );
    }

    if ( m_rimWell->showWellHead() && m_wellHeadArrowPart.notNull() )
    {
        model->addPart( m_wellHeadArrowPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::appendFlattenedDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                     size_t                            frameIndex,
                                                                     const caf::DisplayCoordTransform* displayXf,
                                                                     double                            xOffset )
{
    if ( m_rimWell.isNull() ) return;
    if ( !viewWithSettings() ) return;

    if ( !m_rimWell->isWellPipeVisible( frameIndex ) ) return;

    buildWellHeadParts( frameIndex, displayXf, true, xOffset );

    // Always add pipe part of well head
    if ( m_wellHeadPipeCenterPart.notNull() ) model->addPart( m_wellHeadPipeCenterPart.p() );
    if ( m_wellHeadPipeSurfacePart.notNull() ) model->addPart( m_wellHeadPipeSurfacePart.p() );

    if ( m_rimWell->showWellLabel() && m_wellHeadLabelPart.notNull() )
    {
        model->addPart( m_wellHeadLabelPart.p() );
    }

    if ( m_rimWell->showWellHead() && m_wellHeadArrowPart.notNull() )
    {
        model->addPart( m_wellHeadArrowPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RivWellHeadPartMgr::viewWithSettings()
{
    Rim3dView* view = nullptr;
    if ( m_rimWell ) m_rimWell->firstAncestorOrThisOfType( view );

    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInViewCollection* RivWellHeadPartMgr::simWellInViewCollection()
{
    RimSimWellInViewCollection* wellCollection = nullptr;
    if ( m_rimWell ) m_rimWell->firstAncestorOrThisOfType( wellCollection );

    return wellCollection;
}
