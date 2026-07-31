/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RivSurfaceIntersectionCurveTools.h"

#include "Surface/RigSurface.h"
#include "Surface/RigSurfaceResampler.h"

#include "Rim3dView.h"
#include "RimAnnotationLineAppearance.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimIntersection.h"
#include "RimSurface.h"
#include "RimSurfaceIntersectionBand.h"
#include "RimSurfaceIntersectionCollection.h"
#include "RimSurfaceIntersectionCurve.h"

#include "RivAnnotationSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfPart.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfTransform.h"

#include <algorithm>
#include <cmath>

namespace
{
// Distance between the points the footprint is resampled to before the surface is looked up
const double maxLineSegmentLength = 1.0;

//--------------------------------------------------------------------------------------------------
/// Move a point onto the pillar at the depth of the point, and report whether the depth is within the
/// pillar at all. The surface lookup falls back to a search in the XY plane when the pillar misses the
/// surface triangles, and that fallback keeps the XY of the top of the pillar and ignores the extent
/// of the pillar. Without this the point ends up beside a tilted pillar, or outside a pillar that is
/// bounded by the extent of the intersection.
//--------------------------------------------------------------------------------------------------
bool projectOntoPillar( const cvf::Vec3d& pillarTop, const cvf::Vec3d& pillarBottom, cvf::Vec3d& point )
{
    const double pillarHeight = pillarTop.z() - pillarBottom.z();
    if ( std::fabs( pillarHeight ) < 1.0e-9 ) return true;

    const double t = ( pillarTop.z() - point.z() ) / pillarHeight;
    if ( t < 0.0 || t > 1.0 ) return false;

    point = pillarTop + t * ( pillarBottom - pillarTop );

    return true;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RivSurfaceCurtainPolyline::validRuns() const
{
    std::vector<std::vector<cvf::Vec3d>> runs;

    std::vector<cvf::Vec3d> currentRun;
    for ( size_t i = 0; i < points.size(); i++ )
    {
        if ( valid[i] )
        {
            currentRun.push_back( points[i] );
        }
        else if ( currentRun.size() > 1 )
        {
            runs.push_back( currentRun );
            currentRun.clear();
        }
        else
        {
            currentRun.clear();
        }
    }

    if ( currentRun.size() > 1 ) runs.push_back( currentRun );

    return runs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurface*> RivSurfaceIntersectionCurveTools::referencedSurfaces( const RimSurfaceIntersectionCollection* surfaceIntersections )
{
    if ( !surfaceIntersections ) return {};

    std::vector<RimSurface*> surfaces;

    auto appendSurface = [&surfaces]( RimSurface* surface )
    {
        if ( !surface ) return;
        if ( std::find( surfaces.begin(), surfaces.end(), surface ) != surfaces.end() ) return;

        surfaces.push_back( surface );
    };

    for ( auto curve : surfaceIntersections->surfaceIntersectionCurves() )
    {
        appendSurface( curve->surface() );
    }

    for ( auto band : surfaceIntersections->surfaceIntersectionBands() )
    {
        appendSurface( band->surface1() );
        appendSurface( band->surface2() );
    }

    return surfaces;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RimSurface*, RivSurfaceCurtainPolyline>
    RivSurfaceIntersectionCurveTools::computeSurfaceCurtainPolylines( const std::vector<RimSurface*>& surfaces,
                                                                      const RimIntersectionCurtain&   curtain,
                                                                      const std::function<cvf::Vec3d( const cvf::Vec3d&, size_t )>& pointTransform )
{
    std::map<RimSurface*, RivSurfaceCurtainPolyline> surfacePolylines;

    if ( !curtain.isValid() ) return surfacePolylines;

    const auto resampledTrace = RigSurfaceResampler::computeResampledPolylineWithSegmentInfo( curtain.trace, maxLineSegmentLength );
    if ( resampledTrace.empty() ) return surfacePolylines;

    // Both ends of a pillar are interpolated using the position of the resampled point within its trace
    // segment, so a resampled pillar stays on the curtain
    std::vector<std::pair<cvf::Vec3d, cvf::Vec3d>> resampledPillars;
    std::vector<size_t>                            segmentIndices;
    resampledPillars.reserve( resampledTrace.size() );
    segmentIndices.reserve( resampledTrace.size() );

    for ( const auto& [point, segmentIndex] : resampledTrace )
    {
        if ( segmentIndex + 1 >= curtain.pillars.size() ) continue;

        const auto& traceStart = curtain.trace[segmentIndex];
        const auto& traceEnd   = curtain.trace[segmentIndex + 1];

        const double segmentLength = ( traceEnd - traceStart ).length();
        const double t             = segmentLength > 0.0 ? std::clamp( ( point - traceStart ).length() / segmentLength, 0.0, 1.0 ) : 0.0;

        const auto& pillarStart = curtain.pillars[segmentIndex];
        const auto& pillarEnd   = curtain.pillars[segmentIndex + 1];

        const cvf::Vec3d top    = pillarStart.first + t * ( pillarEnd.first - pillarStart.first );
        const cvf::Vec3d bottom = pillarStart.second + t * ( pillarEnd.second - pillarStart.second );

        resampledPillars.emplace_back( top, bottom );
        segmentIndices.push_back( segmentIndex );
    }

    for ( auto rimSurface : surfaces )
    {
        if ( !rimSurface ) continue;

        rimSurface->loadDataIfRequired();
        auto surface = rimSurface->surfaceData();
        if ( !surface ) continue;

        RivSurfaceCurtainPolyline curtainPolyline;
        curtainPolyline.points.reserve( resampledPillars.size() );
        curtainPolyline.valid.reserve( resampledPillars.size() );

        bool anyValidPoint = false;

        for ( size_t i = 0; i < resampledPillars.size(); i++ )
        {
            const auto& [pillarTop, pillarBottom] = resampledPillars[i];

            cvf::Vec3d intersectionPoint;
            bool       isValid = RigSurfaceResampler::findClosestPointOnSurface( surface, pillarTop, pillarBottom, intersectionPoint );

            if ( isValid ) isValid = projectOntoPillar( pillarTop, pillarBottom, intersectionPoint );

            // A point is always appended, also when the surface was not hit, to keep the two curves of a band index aligned
            curtainPolyline.points.push_back( pointTransform( intersectionPoint, segmentIndices[i] ) );
            curtainPolyline.valid.push_back( isValid );

            anyValidPoint = anyValidPoint || isValid;
        }

        if ( anyValidPoint ) surfacePolylines[rimSurface] = curtainPolyline;
    }

    return surfacePolylines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Collection<cvf::Part>
    RivSurfaceIntersectionCurveTools::createAnnotationParts( const RimSurfaceIntersectionCollection*                 surfaceIntersections,
                                                             const std::map<RimSurface*, RivSurfaceCurtainPolyline>& surfacePolylines,
                                                             cvf::Transform*                                         scaleTransform )
{
    cvf::Collection<cvf::Part> parts;

    if ( !surfaceIntersections ) return parts;

    auto appendParts = [&parts]( cvf::Collection<cvf::Part>& partsToAppend )
    {
        for ( size_t i = 0; i < partsToAppend.size(); i++ )
        {
            parts.push_back( partsToAppend.at( i ) );
        }
    };

    for ( auto curve : surfaceIntersections->surfaceIntersectionCurves() )
    {
        if ( !curve->isChecked() ) continue;

        auto surface = curve->surface();
        if ( !surface ) continue;

        auto it = surfacePolylines.find( surface );
        if ( it == surfacePolylines.end() ) continue;

        auto curveParts = createCurveParts( it->second,
                                            surface->fullName(),
                                            curve->lineAppearance()->color(),
                                            curve->lineAppearance()->thickness(),
                                            scaleTransform );
        appendParts( curveParts );
    }

    for ( auto band : surfaceIntersections->surfaceIntersectionBands() )
    {
        if ( !band->isChecked() ) continue;

        auto surface1 = band->surface1();
        auto surface2 = band->surface2();
        if ( !surface1 || !surface2 ) continue;

        auto it1 = surfacePolylines.find( surface1 );
        auto it2 = surfacePolylines.find( surface2 );
        if ( it1 == surfacePolylines.end() || it2 == surfacePolylines.end() ) continue;

        const auto& polylineA = it1->second;
        const auto& polylineB = it2->second;

        auto curvePartsA =
            createCurveParts( polylineA, surface1->fullName(), band->lineAppearance()->color(), band->lineAppearance()->thickness(), scaleTransform );
        appendParts( curvePartsA );

        auto curvePartsB =
            createCurveParts( polylineB, surface2->fullName(), band->lineAppearance()->color(), band->lineAppearance()->thickness(), scaleTransform );
        appendParts( curvePartsB );

        auto bandPart = createBandPart( polylineA, polylineB, band->bandColor(), band->bandOpacity(), band->polygonOffsetUnit() );
        if ( bandPart.notNull() ) parts.push_back( bandPart.p() );
    }

    return parts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Collection<cvf::Part> RivSurfaceIntersectionCurveTools::createAnnotationParts( const RimIntersection* intersection,
                                                                                    cvf::Transform*        scaleTransform )
{
    if ( !intersection || !intersection->supportsSurfaceIntersectionCurves() ) return {};

    const auto surfaces = referencedSurfaces( intersection->surfaceIntersectionCollection() );
    if ( surfaces.empty() ) return {};

    const auto curtain = intersection->surfaceCurtain();
    if ( !curtain.isValid() ) return {};

    // The visualization parts are built in display coordinates
    cvf::Vec3d displayOffset( 0.0, 0.0, 0.0 );
    {
        auto gridView = intersection->firstAncestorOrThisOfType<RimGridView>();
        if ( gridView && gridView->ownerCase() ) displayOffset = gridView->ownerCase()->displayModelOffset();
    }

    auto pointTransform = [displayOffset]( const cvf::Vec3d& point, size_t segmentIndex ) { return point - displayOffset; };

    const auto surfacePolylines = computeSurfaceCurtainPolylines( surfaces, curtain, pointTransform );

    return createAnnotationParts( intersection->surfaceIntersectionCollection(), surfacePolylines, scaleTransform );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Collection<cvf::Part> RivSurfaceIntersectionCurveTools::createCurveParts( const RivSurfaceCurtainPolyline& polyline,
                                                                               const QString&                   description,
                                                                               const cvf::Color3f&              color,
                                                                               float                            lineWidth,
                                                                               cvf::Transform*                  scaleTransform )
{
    cvf::Collection<cvf::Part> parts;

    std::vector<cvf::Vec3d> allDisplayCoords;

    for ( const auto& run : polyline.validRuns() )
    {
        auto part = createCurvePart( run, color, lineWidth );
        if ( part.isNull() ) continue;

        part->setName( "Intersection " + description.toStdString() );
        parts.push_back( part.p() );

        if ( scaleTransform )
        {
            // The polylines are defined in the display coordinate system without Z-scaling. The z-scaling is applied to the
            // visualization parts using Part::setTransform(Transform* transform)
            // The annotation objects are defined by display coordinates, so apply the Z-scaling to the coordinates

            const auto& mat = scaleTransform->worldTransform();
            for ( const auto& p : run )
            {
                allDisplayCoords.push_back( p.getTransformedPoint( mat ) );
            }
        }
    }

    if ( !parts.empty() && !allDisplayCoords.empty() )
    {
        // Add annotation info to be used to display label in Rim3dView::onViewNavigationChanged()
        // Set the source info on one part only, as this data is only used for display of labels
        auto annoObj = new RivAnnotationSourceInfo( description.toStdString(), allDisplayCoords );
        annoObj->setLabelPositionStrategyHint( RivAnnotationTools::LabelPositionStrategy::RIGHT );
        annoObj->setShowColor( true );
        annoObj->setColor( color );

        parts[0]->setSourceInfo( annoObj );
    }

    return parts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part>
    RivSurfaceIntersectionCurveTools::createCurvePart( const std::vector<cvf::Vec3d>& polyline, const cvf::Color3f& color, float lineWidth )
{
    auto polylineGeo = RivPolylineGenerator::createLineAlongPolylineDrawable( polyline );
    if ( polylineGeo.isNull() ) return nullptr;

    polylineGeo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );

    cvf::ref<cvf::Part> part = new cvf::Part;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivSurfaceIntersectionCurveTools::createBandPart( const RivSurfaceCurtainPolyline& polylineA,
                                                                      const RivSurfaceCurtainPolyline& polylineB,
                                                                      const cvf::Color3f&              color,
                                                                      float                            opacity,
                                                                      double                           polygonOffsetUnit )
{
    const size_t pointCount = std::min( polylineA.points.size(), polylineB.points.size() );
    if ( pointCount < 2 ) return nullptr;

    cvf::GeometryBuilderDrawableGeo geoBuilder;

    bool anyQuad = false;
    for ( size_t i = 1; i < pointCount; i++ )
    {
        // A quad requires both surfaces to be present at both ends of the segment
        if ( !polylineA.valid[i - 1] || !polylineA.valid[i] ) continue;
        if ( !polylineB.valid[i - 1] || !polylineB.valid[i] ) continue;

        const auto& pA0 = polylineA.points[i - 1];
        const auto& pA1 = polylineA.points[i];
        const auto& pB0 = polylineB.points[i - 1];
        const auto& pB1 = polylineB.points[i];

        geoBuilder.addQuadByVertices( cvf::Vec3f( pA0 ), cvf::Vec3f( pA1 ), cvf::Vec3f( pB1 ), cvf::Vec3f( pB0 ) );
        anyQuad = true;
    }

    if ( !anyQuad ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = geoBuilder.drawableGeo();
    if ( geo.isNull() ) return nullptr;

    geo->computeNormals();
    geo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName( "Surface Intersection Band" );
    part->setDrawable( geo.p() );
    part->updateBoundingBox();
    part->setEnableMask( intersectionCellFaceBit );
    part->setPriority( RivPartPriority::PartType::Transparent );

    caf::SurfaceEffectGenerator geometryEffgen( cvf::Color4f( color, opacity ), caf::PO_NEG_LARGE );

    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateUnCachedEffect();

    {
        cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;

        polyOffset->enableFillMode( true );

        // The factor value is defined by enums in EffectGenerator::createAndConfigurePolygonOffsetRenderState()
        // Use a factor that is more negative than the existing enums
        const double offsetFactor = -5;
        polyOffset->setFactor( offsetFactor );

        polyOffset->setUnits( polygonOffsetUnit );

        geometryOnlyEffect->setRenderState( polyOffset.p() );
    }

    part->setEffect( geometryOnlyEffect.p() );

    return part;
}
