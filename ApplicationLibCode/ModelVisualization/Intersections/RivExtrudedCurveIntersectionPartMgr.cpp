/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivExtrudedCurveIntersectionPartMgr.h"

#include "RiaGuiApplication.h"
#include "RiaOffshoreSphericalCoords.h"
#include "RiaPreferences.h"

#include "RigCaseCellResultsData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "Rim2dIntersectionView.h"
#include "RimAnnotationLineAppearance.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSurface.h"
#include "RimSurfaceIntersectionBand.h"
#include "RimSurfaceIntersectionCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuGeoMechXfTensorResultAccessor.h"

#include "RivExtrudedCurveIntersectionGeometryGenerator.h"
#include "RivExtrudedCurveIntersectionSourceInfo.h"
#include "RivIntersectionHexGridInterface.h"
#include "RivIntersectionResultsColoringTools.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivObjectSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPipeGeometryGenerator.h"
#include "RivPolylineGenerator.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivTernaryScalarMapper.h"
#include "RivTernaryTextureCoordsCreator.h"
#include "RivWellPathSourceInfo.h"

#include "cafTensor3.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryTools.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStatePoint.h"
#include "cvfRenderState_FF.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"

#include <functional>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivExtrudedCurveIntersectionPartMgr::RivExtrudedCurveIntersectionPartMgr( RimExtrudedCurveIntersection* rimCrossSection,
                                                                          bool                          isFlattened )
    : m_rimIntersection( rimCrossSection )
    , m_isFlattened( isFlattened )
{
    CVF_ASSERT( m_rimIntersection );

    m_intersectionFacesTextureCoords = new cvf::Vec2fArray;

    cvf::Vec3d flattenedPolylineStartPoint;

    std::vector<std::vector<cvf::Vec3d>> polyLines = m_rimIntersection->polyLines( &flattenedPolylineStartPoint );
    if ( !polyLines.empty() )
    {
        cvf::Vec3d                                direction = m_rimIntersection->extrusionDirection();
        cvf::ref<RivIntersectionHexGridInterface> hexGrid   = m_rimIntersection->createHexGridInterface();
        m_intersectionGenerator = new RivExtrudedCurveIntersectionGeometryGenerator( m_rimIntersection,
                                                                                     polyLines,
                                                                                     direction,
                                                                                     hexGrid.p(),
                                                                                     m_isFlattened,
                                                                                     flattenedPolylineStartPoint );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::applySingleColorEffect()
{
    if ( m_intersectionGenerator.isNull() ) return;

    caf::SurfaceEffectGenerator geometryEffgen( cvf::Color3f::OLIVE, caf::PO_1 );

    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if ( m_intersectionFaces.notNull() )
    {
        m_intersectionFaces->setEffect( geometryOnlyEffect.p() );
    }

    // Update mesh colors as well, in case of change
    RiaPreferences* prefs = RiaPreferences::current();

    if ( m_intersectionGridLines.notNull() )
    {
        cvf::ref<cvf::Effect>    eff;
        caf::MeshEffectGenerator CrossSectionEffGen( prefs->defaultGridLineColors() );
        eff = CrossSectionEffGen.generateCachedEffect();

        m_intersectionGridLines->setEffect( eff.p() );
    }

    if ( m_intersectionFaultGridLines.notNull() )
    {
        cvf::ref<cvf::Effect>    eff;
        caf::MeshEffectGenerator CrossSectionEffGen( prefs->defaultFaultGridLineColors() );
        eff = CrossSectionEffGen.generateCachedEffect();

        m_intersectionFaultGridLines->setEffect( eff.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::updateCellResultColor( size_t                   timeStepIndex,
                                                                 const cvf::ScalarMapper* explicitScalarColorMapper,
                                                                 const RivTernaryScalarMapper* explicitTernaryColorMapper )
{
    RivIntersectionResultsColoringTools::calculateIntersectionResultColors( timeStepIndex,
                                                                            !m_isFlattened,
                                                                            m_rimIntersection,
                                                                            m_intersectionGenerator.p(),
                                                                            explicitScalarColorMapper,
                                                                            explicitTernaryColorMapper,
                                                                            m_intersectionFaces.p(),
                                                                            m_intersectionFacesTextureCoords.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::calculateNodeOrElementNodeBasedGeoMechTextureCoords(
    cvf::Vec2fArray*                                 textureCoords,
    const std::vector<RivIntersectionVertexWeights>& vertexWeights,
    const std::vector<float>&                        resultValues,
    bool                                             isElementNodalResult,
    const RigFemPart*                                femPart,
    const cvf::ScalarMapper*                         mapper )
{
    textureCoords->resize( vertexWeights.size() );

    if ( resultValues.empty() )
    {
        textureCoords->setAll( cvf::Vec2f( 0.0, 1.0f ) );
    }
    else
    {
        cvf::Vec2f* rawPtr = textureCoords->ptr();

        int vxCount = static_cast<int>( vertexWeights.size() );

#pragma omp parallel for schedule( dynamic )
        for ( int triangleVxIdx = 0; triangleVxIdx < vxCount; ++triangleVxIdx )
        {
            float resValue = HUGE_VAL;

            int weightCount = vertexWeights[triangleVxIdx].size();
            if ( weightCount )
            {
                resValue = 0;

                for ( int wIdx = 0; wIdx < weightCount; ++wIdx )
                {
                    size_t resIdx;
                    if ( isElementNodalResult )
                    {
                        resIdx = vertexWeights[triangleVxIdx].vxId( wIdx );
                    }
                    else
                    {
                        resIdx = femPart->nodeIdxFromElementNodeResultIdx( vertexWeights[triangleVxIdx].vxId( wIdx ) );
                    }

                    resValue += resultValues[resIdx] * vertexWeights[triangleVxIdx].weight( wIdx );
                }
            }

            if ( resValue == HUGE_VAL || resValue != resValue ) // a != a is true for NAN's
            {
                rawPtr[triangleVxIdx][1] = 1.0f;
            }
            else
            {
                rawPtr[triangleVxIdx] = mapper->mapToTextureCoord( resValue );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::generatePartGeometry()
{
    if ( m_intersectionGenerator.isNull() ) return;

    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_intersectionGenerator->generateSurface();
        if ( geo.notNull() )
        {
            geo->computeNormals();

            if ( useBufferObjects )
            {
                geo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Cross Section" );
            part->setDrawable( geo.p() );

            // Set mapping from triangle face index to cell index
            cvf::ref<RivExtrudedCurveIntersectionSourceInfo> si =
                new RivExtrudedCurveIntersectionSourceInfo( m_intersectionGenerator.p(), m_rimIntersection );
            part->setSourceInfo( si.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellFaceBit );
            part->setPriority( RivPartPriority::PartType::Intersection );

            m_intersectionFaces = part;
        }
    }

    // Cell Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_intersectionGenerator->createMeshDrawable();
        if ( geoMesh.notNull() )
        {
            if ( useBufferObjects )
            {
                geoMesh->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Cross Section mesh" );
            part->setDrawable( geoMesh.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellMeshBit );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            part->setSourceInfo( new RivMeshLinesSourceInfo( m_rimIntersection ) );

            m_intersectionGridLines = part;
        }
    }

    // Fault Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_intersectionGenerator->createFaultMeshDrawable();
        if ( geoMesh.notNull() )
        {
            if ( useBufferObjects )
            {
                geoMesh->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Cross Section faultmesh" );
            part->setDrawable( geoMesh.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionFaultMeshBit );
            part->setPriority( RivPartPriority::PartType::FaultMeshLines );

            part->setSourceInfo( new RivMeshLinesSourceInfo( m_rimIntersection ) );

            m_intersectionFaultGridLines = part;
        }
    }
    createPolyLineParts( useBufferObjects );

    createExtrusionDirParts( useBufferObjects );

    if ( m_isFlattened ) createFaultLabelParts( m_intersectionGenerator->faultMeshLabelAndAnchorPositions() );

    applySingleColorEffect();

    createAnnotationSurfaceParts( useBufferObjects );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::createFaultLabelParts( const std::vector<std::pair<QString, cvf::Vec3d>>& labelAndAnchors )
{
    m_faultMeshLabels     = nullptr;
    m_faultMeshLabelLines = nullptr;

    if ( labelAndAnchors.empty() ) return;

    RimFaultInViewCollection* faultInViewColl = nullptr;

    if ( !m_rimIntersection->activeSeparateResultDefinition() )
    {
        RimEclipseView* eclipseView = nullptr;
        m_rimIntersection->firstAncestorOrThisOfType( eclipseView );
        if ( eclipseView )
        {
            faultInViewColl = eclipseView->faultCollection();

            if ( faultInViewColl && !faultInViewColl->showFaultLabel() ) return;
        }
    }

    cvf::Color3f faultLabelColor = RiaPreferences::current()->defaultWellLabelColor();
    if ( faultInViewColl ) faultLabelColor = faultInViewColl->faultLabelColor();

    cvf::Font* font = RiaGuiApplication::instance()->defaultSceneFont();

    std::vector<cvf::Vec3f> lineVertices;

    cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
    {
        drawableText->setFont( font );
        drawableText->setCheckPosVisible( false );
        drawableText->setDrawBorder( false );
        drawableText->setDrawBackground( false );
        drawableText->setVerticalAlignment( cvf::TextDrawer::BASELINE );
        drawableText->setTextColor( faultLabelColor );
    }

    cvf::BoundingBox bb                    = m_intersectionFaces->boundingBox();
    double           labelZOffset          = bb.extent().z() / 10;
    int              visibleFaultNameCount = 0;

    for ( const auto& labelAndAnchorPair : labelAndAnchors )
    {
        if ( faultInViewColl )
        {
            RimFaultInView* fault = faultInViewColl->findFaultByName( labelAndAnchorPair.first );
            if ( !( fault && fault->showFault() ) ) continue;
        }

        cvf::String cvfString = cvfqt::Utils::toString( labelAndAnchorPair.first );
        cvf::Vec3f  textCoord( labelAndAnchorPair.second );

        textCoord.z() += labelZOffset;
        drawableText->addText( cvfString, textCoord );

        lineVertices.emplace_back( labelAndAnchorPair.second );
        lineVertices.push_back( textCoord );
        visibleFaultNameCount++;
    }

    if ( visibleFaultNameCount == 0 ) return;

    // Labels part
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "Fault mesh label : text " );
        part->setDrawable( drawableText.p() );

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect( eff.p() );
        part->setPriority( RivPartPriority::PartType::Text );
        part->updateBoundingBox();

        m_faultMeshLabels = part;
    }

    // Lines to labels part
    {
        cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray;
        vertices->assign( lineVertices );
        cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
        geo->setVertexArray( vertices.p() );

        cvf::ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect( cvf::PT_LINES );
        primSet->setStartIndex( 0 );
        primSet->setIndexCount( vertices->size() );
        geo->addPrimitiveSet( primSet.p() );

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "Anchor lines for fault mesh labels" );
        part->setDrawable( geo.p() );

        part->updateBoundingBox();

        caf::MeshEffectGenerator gen( RiaPreferences::current()->defaultFaultGridLineColors() );
        cvf::ref<cvf::Effect>    eff = gen.generateCachedEffect();

        part->setEffect( eff.p() );
        m_faultMeshLabelLines = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::createPolyLineParts( bool useBufferObjects )
{
    // Highlight line

    m_highlightLineAlongPolyline = nullptr;
    m_highlightPointsForPolyline = nullptr;

    if ( m_rimIntersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE ||
         m_rimIntersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_AZIMUTHLINE )
    {
        {
            cvf::ref<cvf::DrawableGeo> polylineGeo = m_intersectionGenerator->createLineAlongPolylineDrawable();
            if ( polylineGeo.notNull() )
            {
                if ( useBufferObjects )
                {
                    polylineGeo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
                }

                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setName( "Cross Section Polyline" );
                part->setDrawable( polylineGeo.p() );

                part->updateBoundingBox();
                part->setPriority( RivPartPriority::PartType::Highlight );

                // Always show this part, also when mesh is turned off
                // part->setEnableMask(meshFaultBit);

                cvf::ref<cvf::Effect>    eff;
                caf::MeshEffectGenerator lineEffGen( cvf::Color3::MAGENTA );
                eff = lineEffGen.generateUnCachedEffect();

                cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
                depth->enableDepthTest( false );
                eff->setRenderState( depth.p() );

                part->setEffect( eff.p() );

                m_highlightLineAlongPolyline = part;
            }
        }

        cvf::ref<cvf::DrawableGeo> polylinePointsGeo = m_intersectionGenerator->createPointsFromPolylineDrawable();
        if ( polylinePointsGeo.notNull() )
        {
            if ( useBufferObjects )
            {
                polylinePointsGeo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Cross Section Polyline" );
            part->setDrawable( polylinePointsGeo.p() );

            part->updateBoundingBox();
            part->setPriority( RivPartPriority::PartType::Highlight );

            // Always show this part, also when mesh is turned off
            // part->setEnableMask(meshFaultBit);

            cvf::ref<cvf::Effect>    eff;
            caf::MeshEffectGenerator lineEffGen( cvf::Color3::MAGENTA );
            eff = lineEffGen.generateUnCachedEffect();

            cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
            depth->enableDepthTest( false );
            eff->setRenderState( depth.p() );

            cvf::ref<cvf::RenderStatePoint> pointRendState = new cvf::RenderStatePoint( cvf::RenderStatePoint::FIXED_SIZE );
            pointRendState->setSize( 5.0f );
            eff->setRenderState( pointRendState.p() );

            part->setEffect( eff.p() );

            m_highlightPointsForPolyline = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::createExtrusionDirParts( bool useBufferObjects )
{
    m_highlightLineAlongExtrusionDir = nullptr;
    m_highlightPointsForExtrusionDir = nullptr;

    if ( m_rimIntersection->direction() == RimExtrudedCurveIntersection::CrossSectionDirEnum::CS_TWO_POINTS )
    {
        {
            cvf::ref<cvf::DrawableGeo> polylineGeo = m_intersectionGenerator->createLineAlongExtrusionLineDrawable(
                m_rimIntersection->polyLinesForExtrusionDirection() );
            if ( polylineGeo.notNull() )
            {
                if ( useBufferObjects )
                {
                    polylineGeo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
                }

                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setName( "Cross Section Polyline" );
                part->setDrawable( polylineGeo.p() );

                part->updateBoundingBox();
                part->setPriority( RivPartPriority::PartType::Highlight );

                // Always show this part, also when mesh is turned off
                // part->setEnableMask(meshFaultBit);

                cvf::ref<cvf::Effect>    eff;
                caf::MeshEffectGenerator lineEffGen( cvf::Color3::MAGENTA );
                eff = lineEffGen.generateUnCachedEffect();

                cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
                depth->enableDepthTest( false );
                eff->setRenderState( depth.p() );

                part->setEffect( eff.p() );

                m_highlightLineAlongExtrusionDir = part;
            }
        }

        cvf::ref<cvf::DrawableGeo> polylinePointsGeo = m_intersectionGenerator->createPointsFromExtrusionLineDrawable(
            m_rimIntersection->polyLinesForExtrusionDirection() );
        if ( polylinePointsGeo.notNull() )
        {
            if ( useBufferObjects )
            {
                polylinePointsGeo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Cross Section Polyline" );
            part->setDrawable( polylinePointsGeo.p() );

            part->updateBoundingBox();
            part->setPriority( RivPartPriority::PartType::Highlight );

            // Always show this part, also when mesh is turned off
            // part->setEnableMask(meshFaultBit);

            cvf::ref<cvf::Effect>    eff;
            caf::MeshEffectGenerator lineEffGen( cvf::Color3::MAGENTA );
            eff = lineEffGen.generateUnCachedEffect();

            cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
            depth->enableDepthTest( false );
            eff->setRenderState( depth.p() );

            cvf::ref<cvf::RenderStatePoint> pointRendState = new cvf::RenderStatePoint( cvf::RenderStatePoint::FIXED_SIZE );
            pointRendState->setSize( 5.0f );
            eff->setRenderState( pointRendState.p() );

            part->setEffect( eff.p() );

            m_highlightPointsForExtrusionDir = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::createAnnotationSurfaceParts( bool useBufferObjects )
{
    m_annotationParts.clear();

    auto surfPolys = m_intersectionGenerator->transformedSurfaceIntersectionPolylines();

    for ( auto curve : m_rimIntersection->surfaceIntersectionCurves() )
    {
        if ( !curve->isChecked() ) continue;

        auto surface = curve->surface();
        if ( !surface ) continue;

        if ( surfPolys.count( surface ) == 0 ) continue;
        auto polylines = surfPolys[surface];

        auto part = createCurvePart( polylines,
                                     useBufferObjects,
                                     surface->userDescription(),
                                     curve->lineAppearance()->color(),
                                     curve->lineAppearance()->thickness() );

        if ( part.notNull() ) m_annotationParts.push_back( part.p() );
    }

    for ( auto band : m_rimIntersection->surfaceIntersectionBands() )
    {
        if ( !band->isChecked() ) continue;

        auto surface1 = band->surface1();
        auto surface2 = band->surface2();

        if ( !surface1 || !surface2 ) continue;

        if ( surfPolys.count( surface1 ) == 0 ) continue;
        if ( surfPolys.count( surface2 ) == 0 ) continue;

        auto polylineA = surfPolys[surface1];
        auto polylineB = surfPolys[surface2];

        // Create curve parts
        {
            auto part = createCurvePart( polylineA,
                                         useBufferObjects,
                                         surface1->userDescription(),
                                         band->lineAppearance()->color(),
                                         band->lineAppearance()->thickness() );

            if ( part.notNull() ) m_annotationParts.push_back( part.p() );
        }
        {
            auto part = createCurvePart( polylineB,
                                         useBufferObjects,
                                         surface2->userDescription(),
                                         band->lineAppearance()->color(),
                                         band->lineAppearance()->thickness() );

            if ( part.notNull() ) m_annotationParts.push_back( part.p() );
        }

        // Create a quad strip between the two polylines
        size_t pointCount = std::min( polylineA.size(), polylineB.size() );
        if ( pointCount > 1 )
        {
            cvf::GeometryBuilderDrawableGeo geoBuilder;

            for ( size_t i = 1; i < pointCount; i++ )
            {
                const auto& pA0 = polylineA[i - 1];
                const auto& pA1 = polylineA[i];
                const auto& pB0 = polylineB[i - 1];
                const auto& pB1 = polylineB[i];

                geoBuilder.addQuadByVertices( cvf::Vec3f( pA0 ), cvf::Vec3f( pA1 ), cvf::Vec3f( pB1 ), cvf::Vec3f( pB0 ) );
            }

            cvf::ref<cvf::DrawableGeo> geo = geoBuilder.drawableGeo();
            if ( geo.notNull() )
            {
                geo->computeNormals();

                if ( useBufferObjects )
                {
                    geo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
                }

                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setName( "Surface Intersection Band" );
                part->setDrawable( geo.p() );
                part->updateBoundingBox();
                part->setEnableMask( intersectionCellFaceBit );
                part->setPriority( RivPartPriority::PartType::Transparent );

                auto                        color = cvf::Color4f( band->bandColor(), band->bandOpacity() );
                caf::SurfaceEffectGenerator geometryEffgen( color, caf::PO_NEG_LARGE );

                cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateUnCachedEffect();

                {
                    cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;

                    polyOffset->enableFillMode( true );

                    // The factor value is defined by enums in
                    // EffectGenerator::createAndConfigurePolygonOffsetRenderState() Use a factor that is more negative
                    // than the existing enums
                    const double offsetFactor = -5;
                    polyOffset->setFactor( offsetFactor );

                    polyOffset->setUnits( band->polygonOffsetUnit() );

                    geometryOnlyEffect->setRenderState( polyOffset.p() );
                }

                part->setEffect( geometryOnlyEffect.p() );

                m_annotationParts.push_back( part.p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivExtrudedCurveIntersectionPartMgr::createCurvePart( const std::vector<cvf::Vec3d>& polylines,
                                                                          bool                useBufferObjects,
                                                                          const QString&      description,
                                                                          const cvf::Color3f& color,
                                                                          float               lineWidth )
{
    auto polylineGeo = RivPolylineGenerator::createLineAlongPolylineDrawable( polylines );
    if ( polylineGeo.notNull() )
    {
        if ( useBufferObjects )
        {
            polylineGeo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "Intersection " + description.toStdString() );
        part->setDrawable( polylineGeo.p() );

        part->updateBoundingBox();
        part->setPriority( RivPartPriority::PartType::FaultMeshLines );

        caf::MeshEffectGenerator lineEffGen( color );
        lineEffGen.setLineWidth( lineWidth );

        cvf::ref<cvf::Effect> eff = lineEffGen.generateUnCachedEffect();

        cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;
        polyOffset->enableFillMode( true );
        polyOffset->setFactor( -5 );
        const double maxOffsetFactor = -1000;
        polyOffset->setUnits( maxOffsetFactor );

        eff->setRenderState( polyOffset.p() );

        part->setEffect( eff.p() );
        return part;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::appendIntersectionFacesToModel( cvf::ModelBasicList* model,
                                                                          cvf::Transform*      scaleTransform )
{
    if ( m_intersectionFaces.isNull() )
    {
        generatePartGeometry();
    }

    if ( m_intersectionFaces.notNull() )
    {
        m_intersectionFaces->setTransform( scaleTransform );
        model->addPart( m_intersectionFaces.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::appendMeshLinePartsToModel( cvf::ModelBasicList* model,
                                                                      cvf::Transform*      scaleTransform )
{
    if ( m_intersectionGridLines.isNull() )
    {
        generatePartGeometry();
    }

    if ( m_intersectionGridLines.notNull() )
    {
        m_intersectionGridLines->setTransform( scaleTransform );
        model->addPart( m_intersectionGridLines.p() );
    }

    if ( m_intersectionFaultGridLines.notNull() )
    {
        m_intersectionFaultGridLines->setTransform( scaleTransform );
        model->addPart( m_intersectionFaultGridLines.p() );
    }

    if ( m_faultMeshLabelLines.notNull() )
    {
        m_faultMeshLabelLines->setTransform( scaleTransform );
        model->addPart( m_faultMeshLabelLines.p() );
    }

    if ( m_faultMeshLabels.notNull() )
    {
        m_faultMeshLabels->setTransform( scaleTransform );
        model->addPart( m_faultMeshLabels.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivExtrudedCurveIntersectionPartMgr::appendPolylinePartsToModel( Rim3dView&           view,
                                                                      cvf::ModelBasicList* model,
                                                                      cvf::Transform*      scaleTransform )
{
    auto* curr2dView = dynamic_cast<Rim2dIntersectionView*>( &view );

    if ( m_rimIntersection->inputPolyLineFromViewerEnabled() || ( curr2dView && curr2dView->showDefiningPoints() ) )
    {
        if ( m_highlightLineAlongPolyline.notNull() )
        {
            m_highlightLineAlongPolyline->setTransform( scaleTransform );
            model->addPart( m_highlightLineAlongPolyline.p() );
        }

        if ( m_highlightPointsForPolyline.notNull() )
        {
            m_highlightPointsForPolyline->setTransform( scaleTransform );
            model->addPart( m_highlightPointsForPolyline.p() );
        }
    }

    if ( m_rimIntersection->inputExtrusionPointsFromViewerEnabled() )
    {
        if ( m_highlightLineAlongExtrusionDir.notNull() )
        {
            m_highlightLineAlongExtrusionDir->setTransform( scaleTransform );
            model->addPart( m_highlightLineAlongExtrusionDir.p() );
        }

        if ( m_highlightPointsForExtrusionDir.notNull() )
        {
            m_highlightPointsForExtrusionDir->setTransform( scaleTransform );
            model->addPart( m_highlightPointsForExtrusionDir.p() );
        }
    }

    if ( m_rimIntersection->inputTwoAzimuthPointsFromViewerEnabled() || ( curr2dView && curr2dView->showDefiningPoints() ) )
    {
        if ( m_highlightLineAlongPolyline.notNull() )
        {
            m_highlightLineAlongPolyline->setTransform( scaleTransform );
            model->addPart( m_highlightLineAlongPolyline.p() );
        }

        if ( m_highlightPointsForPolyline.notNull() )
        {
            m_highlightPointsForPolyline->setTransform( scaleTransform );
            model->addPart( m_highlightPointsForPolyline.p() );
        }
    }

    for ( size_t i = 0; i < m_annotationParts.size(); i++ )
    {
        auto part = m_annotationParts[i];
        if ( part.notNull() )
        {
            part->setTransform( scaleTransform );
            model->addPart( part.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RivExtrudedCurveIntersectionPartMgr::unflattenTransformMatrix( const cvf::Vec3d& intersectionPointFlat ) const
{
    return m_intersectionGenerator->unflattenTransformMatrix( intersectionPointFlat );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RivExtrudedCurveIntersectionPartMgr::intersectionGeometryGenerator() const
{
    if ( m_intersectionGenerator.notNull() ) return m_intersectionGenerator.p();

    return nullptr;
}
