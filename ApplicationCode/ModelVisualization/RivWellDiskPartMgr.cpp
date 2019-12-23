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
#include "cvfEffect.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRenderState_FF.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfTransform.h"
#include "cvfUniform.h"
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

    double pipeRadius = m_rimWell->pipeRadius();
    size_t numSectors = 100;

    // Upper part of simulation well pipe is defined to use branch index 0
    cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo( m_rimWell, 0 );

    // Well disk geometry
    double arrowLength = characteristicCellSize * simWellInViewCollection()->wellHeadScaleFactor() *
                         m_rimWell->wellHeadScaleFactor();

    cvf::Vec3d diskPosition = whEndPos;
    diskPosition.z() += pipeRadius + arrowLength;

    cvf::Vec3d textPosition = diskPosition;
    textPosition.z() += 2.0 * arrowLength;

    cvf::Mat4f matr;

    double ijScaleFactor = arrowLength / 6;
    matr( 0, 0 ) *= ijScaleFactor;
    matr( 1, 1 ) *= ijScaleFactor;
    matr( 2, 2 ) *= arrowLength;

    matr.setTranslation( cvf::Vec3f( diskPosition ) );

    cvf::GeometryBuilderFaceList builder;
    RivDiskGeometryGenerator     gen;
    gen.setRelativeRadius( 2.5f * ( m_rimWell->diskScale() ) );
    gen.setRelativeLength( 0.1f );
    gen.setNumSlices( numSectors );
    gen.generate( &builder );

    cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();
    cvf::ref<cvf::UIntArray>  faceList = builder.faceList();

    for ( size_t i = 0; i < vertices->size(); i++ )
    {
        cvf::Vec3f v = vertices->get( i );
        v.transformPoint( matr );
        vertices->set( i, v );
    }

    cvf::ref<cvf::DrawableGeo> geo1 = new cvf::DrawableGeo;
    geo1->setVertexArray( vertices.p() );
    geo1->setFromFaceList( *faceList );
    geo1->computeNormals();

    // Create the fixed function effect
    {
        m_fixedFuncEffect = new cvf::Effect;

        cvf::ref<cvf::RenderStateMaterial_FF> mat = new cvf::RenderStateMaterial_FF( cvf::Color3::BLUE );
        mat->enableColorMaterial( true );
        m_fixedFuncEffect->setRenderState( mat.p() );

        cvf::ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
        m_fixedFuncEffect->setRenderState( lighting.p() );
    }

    // Create effect with shader program
    {
        m_shaderEffect = new cvf::Effect;

        cvf::ShaderProgramGenerator gen( "PerVertexColor", cvf::ShaderSourceProvider::instance() );
        gen.addVertexCode( cvf::ShaderSourceRepository::vs_Standard );

        // TODO: settable?
        bool enableLighting = true;
        if ( enableLighting )
        {
            gen.addFragmentCode( cvf::ShaderSourceRepository::src_VaryingColorGlobalAlpha );
            gen.addFragmentCode( cvf::ShaderSourceRepository::light_Phong );
            gen.addFragmentCode( cvf::ShaderSourceRepository::fs_Standard );
        }
        else
        {
            gen.addFragmentCode( cvf::ShaderSourceRepository::fs_Unlit );
        }

        m_shaderProg = gen.generate();
        m_shaderProg->setDefaultUniform( new cvf::UniformFloat( "u_alpha", 1.0f ) );
        m_shaderProg->setDefaultUniform( new cvf::UniformFloat( "u_ambientIntensity", 100.0f ) );

        m_shaderEffect->setShaderProgram( m_shaderProg.p() );
    }

    cvf::ref<cvf::Effect> effectToUse = RiaGuiApplication::instance()->useShaders() ? m_shaderEffect : m_fixedFuncEffect;

    const cvf::Color3ub colorTable[] = {cvf::Color3ub( cvf::Color3::DARK_GREEN ),
                                        cvf::Color3ub( cvf::Color3::DARK_RED ),
                                        cvf::Color3ub( cvf::Color3::DARK_BLUE )};

    size_t                       vertexCount = geo1->vertexCount();
    cvf::ref<cvf::Color3ubArray> colorArray  = new cvf::Color3ubArray;
    colorArray->resize( vertexCount );
    colorArray->setAll( cvf::Color3::WHITE );
    CVF_ASSERT( vertexCount == numSectors * 3 );

    QString labelText;
    if ( !m_rimWell->isValidDisk() )
    {
        // Make invalid disks gray
        for ( size_t i = 0; i < numSectors * 3; i++ )
        {
            cvf::Color3ub c = cvf::Color3::GRAY;
            colorArray->set( i, c );
        }
        labelText = QString( "%1: N/A" ).arg( m_rimWell->name() );
    }
    else if ( m_rimWell->isSingleProperty() )
    {
        const double singleProperty = m_rimWell->singleProperty();
        // Set color for the triangle vertices
        for ( size_t i = 0; i < numSectors * 3; i++ )
        {
            cvf::Color3ub c = cvf::Color3::ORANGE_RED;
            colorArray->set( i, c );
        }
        labelText = QString( "%1: %2" ).arg( m_rimWell->name() ).arg( singleProperty, 0, 'f', 1 );
    }
    else
    {
        const double oil   = m_rimWell->oil();
        const double gas   = m_rimWell->gas();
        const double water = m_rimWell->water();

        const double total         = oil + gas + water;
        const double oilFraction   = oil / total;
        const double gasFraction   = gas / total;
        const double waterFraction = water / total;

        for ( size_t i = 0; i < numSectors; i++ )
        {
            int colorIdx = 0;

            // Find the color for this sector
            double lim = ( i + 1 ) / static_cast<double>( numSectors );
            if ( lim <= oilFraction )
            {
                colorIdx = 0;
            }
            else if ( lim <= oilFraction + gasFraction )
            {
                colorIdx = 1;
            }
            else
            {
                colorIdx = 2;
            }

            // Set color for the triangle vertices
            for ( int t = 0; t < 3; t++ )
            {
                cvf::Color3ub c = colorTable[colorIdx];
                colorArray->set( i * 3 + t, c );
            }
        }

        labelText = QString( "%1:\nO=%2\nG=%3\nW=%4" )
                        .arg( m_rimWell->name() )
                        .arg( oil, 0, 'f', 1 )
                        .arg( gas, 0, 'f', 1 )
                        .arg( water, 0, 'f', 1 );
    }
    geo1->setColorArray( colorArray.p() );

    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivWellDiskPartMgr: disk " + cvfqt::Utils::toString( well->name() ) );
        part->setDrawable( geo1.p() );

        part->setEffect( effectToUse.p() );
        part->setSourceInfo( sourceInfo.p() );

        m_wellDiskPart = part;
    }

    if ( well->showWellLabel() && well->showWellDisks() && !well->name().isEmpty() )
    {
        cvf::Font* font = RiaGuiApplication::instance()->defaultWellLabelFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont( font );
        drawableText->setCheckPosVisible( false );
        drawableText->setDrawBorder( false );
        drawableText->setDrawBackground( false );
        drawableText->setVerticalAlignment( cvf::TextDrawer::CENTER );
        drawableText->setTextColor( simWellInViewCollection()->wellLabelColor() );

        cvf::String cvfString = cvfqt::Utils::toString( labelText );

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
