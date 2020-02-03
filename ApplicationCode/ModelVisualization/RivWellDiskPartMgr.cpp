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

#include "RiaColorTools.h"
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
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
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
void RivWellDiskPartMgr::buildWellDiskParts( size_t frameIndex, const caf::DisplayCoordTransform* displayXf )
{
    clearAllGeometry();

    if ( !viewWithSettings() ) return;

    RimSimWellInView* well = m_rimWell;

    double characteristicCellSize = viewWithSettings()->ownerCase()->characteristicCellSize();

    cvf::Vec3d whEndPos;
    cvf::Vec3d whStartPos;
    {
        well->wellHeadTopBottomPosition( static_cast<int>( frameIndex ), &whEndPos, &whStartPos );

        whEndPos = displayXf->transformToDisplayCoord( whEndPos );
        whEndPos.z() += characteristicCellSize;
    }

    if ( !well->simWellData()->hasWellResult( frameIndex ) ) return;

    auto productionType = well->simWellData()->wellResultFrame( frameIndex ).m_productionType;

    double       pipeRadius = m_rimWell->pipeRadius();
    unsigned int numSectors = 100;

    // Upper part of simulation well pipe is defined to use branch index 0
    cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo( m_rimWell, 0 );

    // Well disk geometry
    double arrowLength = characteristicCellSize * simWellInViewCollection()->wellHeadScaleFactor() *
                         m_rimWell->wellHeadScaleFactor();

    cvf::Vec3d diskPosition = whEndPos;
    diskPosition.z() += pipeRadius + arrowLength * 2.0;

    cvf::Vec3d textPosition = diskPosition;
    textPosition.z() += 0.1;

    double ijScaleFactor = arrowLength / 6;

    cvf::ref<cvf::DrawableGeo> geo1 = new cvf::DrawableGeo;
    {
        cvf::Mat4f matr;

        matr( 0, 0 ) *= ijScaleFactor;
        matr( 1, 1 ) *= ijScaleFactor;
        matr( 2, 2 ) *= ijScaleFactor;

        matr.setTranslation( cvf::Vec3f( diskPosition ) );

        cvf::GeometryBuilderFaceList builder;
        {
            RivDiskGeometryGenerator gen;
            gen.setRelativeRadius( 2.5f * ( m_rimWell->diskScale() ) );
            gen.setRelativeLength( 0.1f );
            gen.setNumSlices( numSectors );
            gen.generate( &builder );
        }

        cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();
        cvf::ref<cvf::UIntArray>  faceList = builder.faceList();

        for ( size_t i = 0; i < vertices->size(); i++ )
        {
            cvf::Vec3f v = vertices->get( i );
            v.transformPoint( matr );
            vertices->set( i, v );
        }

        geo1->setVertexArray( vertices.p() );
        geo1->setFromFaceList( *faceList );
        geo1->computeNormals();
    }

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

        gen.addFragmentCode( cvf::ShaderSourceRepository::src_VaryingColorGlobalAlpha );
        gen.addFragmentCode( cvf::ShaderSourceRepository::fs_Unlit );
        m_shaderProg = gen.generate();

        m_shaderProg->setDefaultUniform( new cvf::UniformFloat( "u_alpha", 1.0f ) );

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

    std::vector<std::pair<cvf::String, cvf::Vec3f>> labelsWithPosition;

    int numberPrecision = 2;

    double accumulatedPropertyValue = 0.0;

    QString         labelText = m_rimWell->name();
    RigWellDiskData diskData  = m_rimWell->wellDiskData();
    if ( diskData.isSingleProperty() )
    {
        // Set color for the triangle vertices
        for ( size_t i = 0; i < numSectors * 3; i++ )
        {
            cvf::Color3ub c = cvf::Color3::OLIVE;
            colorArray->set( i, c );
        }

        accumulatedPropertyValue = diskData.singlePropertyValue();

        if ( simWellInViewCollection()->showWellDiskQuantityLables() )
        {
            const double singleProperty = diskData.singlePropertyValue();
            labelText += QString( "\n%2" ).arg( singleProperty, 0, 'g', numberPrecision );
        }
    }
    else
    {
        const double oil   = diskData.oil();
        const double gas   = diskData.gas();
        const double water = diskData.water();

        const double total         = diskData.total();
        const double oilFraction   = oil / total;
        const double gasFraction   = gas / total;
        const double waterFraction = water / total;

        accumulatedPropertyValue = total;

        const double threshold = 1e-6;
        if ( total > threshold )
        {
            double aggregatedFraction = 0.0;

            {
                auto p = createTextAndLocation( oilFraction / 2.0, diskPosition, ijScaleFactor, oil, numberPrecision );
                labelsWithPosition.push_back( p );
                aggregatedFraction += oilFraction;
            }

            {
                auto p = createTextAndLocation( aggregatedFraction + gasFraction / 2.0,
                                                diskPosition,
                                                ijScaleFactor,
                                                gas,
                                                numberPrecision );
                labelsWithPosition.push_back( p );
                aggregatedFraction += gasFraction;
            }

            {
                auto p = createTextAndLocation( aggregatedFraction + waterFraction / 2.0,
                                                diskPosition,
                                                ijScaleFactor,
                                                water,
                                                numberPrecision );

                labelsWithPosition.push_back( p );
                aggregatedFraction += waterFraction;
            }
        }

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
    }
    geo1->setColorArray( colorArray.p() );

    double threshold = 0.1;
    if ( accumulatedPropertyValue > threshold )
    {
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivWellDiskPartMgr: disk " + cvfqt::Utils::toString( well->name() ) );
            part->setDrawable( geo1.p() );

            part->setEffect( effectToUse.p() );
            part->setSourceInfo( sourceInfo.p() );

            m_wellDiskPart = part;
        }

        // Add visual indicator for well type: producer or injector
        if ( productionType == RigWellResultFrame::PRODUCER )
        {
            const uint numPolysZDir = 1;
            float      bottomRadius = 0.5f;
            float      topRadius    = 0.5f;
            float      height       = 0.1f;
            float      topOffsetX   = 0.0f;
            float      topOffsetY   = 0.0f;

            cvf::GeometryBuilderFaceList builder;
            cvf::GeometryUtils::createObliqueCylinder( bottomRadius,
                                                       topRadius,
                                                       height,
                                                       topOffsetX,
                                                       topOffsetY,
                                                       20,
                                                       true,
                                                       true,
                                                       true,
                                                       numPolysZDir,
                                                       &builder );

            cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();
            cvf::ref<cvf::UIntArray>  faceList = builder.faceList();

            cvf::Mat4f matr;
            matr( 0, 0 ) *= ijScaleFactor;
            matr( 1, 1 ) *= ijScaleFactor;
            matr( 2, 2 ) *= ijScaleFactor;
            matr.setTranslation( cvf::Vec3f( diskPosition ) );

            for ( size_t i = 0; i < vertices->size(); i++ )
            {
                cvf::Vec3f v = vertices->get( i );
                v.transformPoint( matr );
                vertices->set( i, v );
            }

            caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( cvf::Color3::BLACK ), caf::PO_1 );
            surfaceGen.enableLighting( false );
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            cvf::ref<cvf::DrawableGeo> injectorGeo = new cvf::DrawableGeo;
            injectorGeo->setVertexArray( vertices.p() );
            injectorGeo->setFromFaceList( *faceList );
            injectorGeo->computeNormals();

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivWellDiskPartMgr: producer " + cvfqt::Utils::toString( well->name() ) );
            part->setDrawable( injectorGeo.p() );

            part->setEffect( eff.p() );
            part->setSourceInfo( sourceInfo.p() );

            m_wellDiskInjectorPart = part;
        }
        else if ( productionType == RigWellResultFrame::OIL_INJECTOR ||
                  productionType == RigWellResultFrame::GAS_INJECTOR ||
                  productionType == RigWellResultFrame::WATER_INJECTOR )
        {
            cvf::GeometryBuilderFaceList builder;
            cvf::Vec3f                   pos( 0.0, 0.0, 0.0 );

            // Construct a cross using to "bars"
            cvf::GeometryUtils::createBox( pos, 0.2f, 0.8f, 0.1f, &builder );
            cvf::GeometryUtils::createBox( pos, 0.8f, 0.2f, 0.1f, &builder );

            cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();
            cvf::ref<cvf::UIntArray>  faceList = builder.faceList();

            cvf::Mat4f matr;
            matr( 0, 0 ) *= ijScaleFactor;
            matr( 1, 1 ) *= ijScaleFactor;
            matr( 2, 2 ) *= ijScaleFactor;
            matr.setTranslation( cvf::Vec3f( diskPosition ) );

            for ( size_t i = 0; i < vertices->size(); i++ )
            {
                cvf::Vec3f v = vertices->get( i );
                v.transformPoint( matr );
                vertices->set( i, v );
            }

            cvf::Color4f                injectorMarkerColor = getWellInjectionColor( productionType );
            caf::SurfaceEffectGenerator surfaceGen( injectorMarkerColor, caf::PO_1 );
            surfaceGen.enableLighting( false );
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            cvf::ref<cvf::DrawableGeo> injectorGeo = new cvf::DrawableGeo;
            injectorGeo->setVertexArray( vertices.p() );
            injectorGeo->setFromFaceList( *faceList );
            injectorGeo->computeNormals();

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivWellDiskPartMgr: injector " + cvfqt::Utils::toString( well->name() ) );
            part->setDrawable( injectorGeo.p() );

            part->setEffect( eff.p() );
            part->setSourceInfo( sourceInfo.p() );

            m_wellDiskInjectorPart = part;
        }
    }

    bool showTextLabels = simWellInViewCollection()->showWellDiskQuantityLables() ||
                          ( well->showWellLabel() && well->showWellDisks() && !well->name().isEmpty() );

    if ( showTextLabels )
    {
        cvf::Font* font = RiaGuiApplication::instance()->defaultWellLabelFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont( font );
        drawableText->setCheckPosVisible( false );
        drawableText->setDrawBorder( false );

        drawableText->setDrawBackground( simWellInViewCollection()->showWellDiskLabelBackground() );
        drawableText->setVerticalAlignment( cvf::TextDrawer::CENTER );

        auto textColor = simWellInViewCollection()->wellLabelColor();
        drawableText->setTextColor( textColor );

        auto bgColor = RiaColorTools::contrastColor( textColor );
        drawableText->setBackgroundColor( bgColor );

        cvf::String cvfString = cvfqt::Utils::toString( labelText );

        cvf::Vec3f textCoord( textPosition );
        drawableText->addText( cvfString, textCoord );

        if ( simWellInViewCollection()->showWellDiskQuantityLables() )
        {
            for ( const auto& t : labelsWithPosition )
            {
                drawableText->addText( t.first, t.second );
            }
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivWellDiskPartMgr: text " + cvfString );
        part->setDrawable( drawableText.p() );

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect( eff.p() );
        part->setPriority( RivPartPriority::PartType::Text );

        m_wellDiskLabelPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::String, cvf::Vec3f> RivWellDiskPartMgr::createTextAndLocation( const double aggregatedFraction,
                                                                              cvf::Vec3d   diskPosition,
                                                                              double       ijScaleFactor,
                                                                              const double fraction,
                                                                              int          precision )
{
    double sinA = cvf::Math::sin( aggregatedFraction * 2.0 * cvf::PI_F );
    double cosA = cvf::Math::cos( aggregatedFraction * 2.0 * cvf::PI_F );

    cvf::Vec3f v = cvf::Vec3f( diskPosition );

    double radius = 2.5f * ( m_rimWell->diskScale() );
    radius *= ijScaleFactor;
    radius *= 1.1; // Put label outside the disk

    v.x() = v.x() + static_cast<float>( -sinA * radius );
    v.y() = v.y() + static_cast<float>( cosA * radius );

    auto        s    = QString::number( fraction, 'g', precision );
    cvf::String text = cvf::String( s.toStdString() );

    auto p = std::make_pair( text, v );

    return p;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellDiskPartMgr::clearAllGeometry()
{
    m_wellDiskPart         = nullptr;
    m_wellDiskLabelPart    = nullptr;
    m_wellDiskInjectorPart = nullptr;
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
    if ( !m_rimWell->isValidDisk() ) return;

    buildWellDiskParts( frameIndex, displayXf );

    if ( m_rimWell->showWellDisks() && m_wellDiskLabelPart.notNull() )
    {
        model->addPart( m_wellDiskLabelPart.p() );
    }

    if ( m_rimWell->showWellDisks() && m_wellDiskPart.notNull() )
    {
        model->addPart( m_wellDiskPart.p() );
    }

    if ( m_rimWell->showWellDisks() && m_wellDiskInjectorPart.notNull() )
    {
        model->addPart( m_wellDiskInjectorPart.p() );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color4f RivWellDiskPartMgr::getWellInjectionColor( RigWellResultFrame::WellProductionType productionType )
{
    if ( productionType == RigWellResultFrame::OIL_INJECTOR )
    {
        return cvf::Color4f( cvf::Color3::ORANGE );
    }
    else if ( productionType == RigWellResultFrame::GAS_INJECTOR )
    {
        return cvf::Color4f( cvf::Color3::RED );
    }
    else if ( productionType == RigWellResultFrame::WATER_INJECTOR )
    {
        return cvf::Color4f( cvf::Color3::BLUE );
    }

    return cvf::Color4f( cvf::Color3::BLACK );
}
