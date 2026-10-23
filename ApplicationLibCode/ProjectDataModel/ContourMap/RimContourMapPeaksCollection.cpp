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

#include "RimContourMapPeaksCollection.h"

#include "RimContourMapPeak.h"

#include "ContourMap/RigContourMapPeakFinder.h"
#include "ContourMap/RigContourMapProjection.h"
#include "ContourMap/RimContourMapProjection.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"

#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiOrdering.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimContourMapPeaksCollection, "RimContourMapPeaksCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapPeaksCollection::RimContourMapPeaksCollection()
{
    CAF_PDM_InitObject( "Contour Map Peaks", ":/WellTargetPoint16x16.png" );

    CAF_PDM_InitField( &m_peakCount, "PeakCount", 10, "Maximum Number of Peaks" );
    m_peakCount.setRange( 1, 20 );
    CAF_PDM_InitField( &m_minDistance, "MinDistance", 0.0, "Minimum Distance" );
    m_minDistance.setMinValue( 0.0 );
    CAF_PDM_InitField( &m_minProminence, "MinProminence", 0.0, "Minimum Prominence" );
    m_minProminence.uiCapability()->setUiToolTip( "Ignore peaks rising less than this value above the highest saddle to a higher peak" );

    CAF_PDM_InitFieldNoDefault( &m_resultSignature, "ResultSignature", "Result Signature" );
    m_resultSignature.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_peaks, "Peaks", "Peaks" );

    nameField()->uiCapability()->setUiReadOnly( true );
    setName( "Contour Map Peaks" );
}

//--------------------------------------------------------------------------------------------------
/// Creates both the RimContourMapPeak metadata object and its single-point marker polygon (used to
/// visualize the peak's position as a sphere, reusing the existing polygon rendering/mirroring machinery).
//--------------------------------------------------------------------------------------------------
RimContourMapPeak*
    RimContourMapPeaksCollection::addPeak( int rank, double value, double prominence, const cvf::Vec3d& domainPosition, double sphereRadiusFactor )
{
    auto polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return nullptr;

    auto* peak = new RimContourMapPeak;
    peak->setValues( rank, value, prominence, domainPosition );

    auto* markerPolygon = polygonCollection->appendUserDefinedPolygon();
    markerPolygon->setPointsInDomainCoords( { domainPosition } );
    markerPolygon->setIsClosed( false );
    markerPolygon->setName( peak->name() );
    markerPolygon->setReadOnly( true );
    markerPolygon->setShowLines( false );
    markerPolygon->setShowSpheres( true );
    markerPolygon->setSphereRadiusFactor( sphereRadiusFactor );
    markerPolygon->setSphereColor( cvf::Color3f::DEEP_PINK );
    markerPolygon->coordinatesChanged.send();

    peak->setMarkerPolygon( markerPolygon );

    m_peaks.push_back( peak );

    return peak;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::clearPeaks()
{
    m_peaks.deleteChildren();
    m_resultSignature = "";
    updateOutdatedState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::updateVisualization()
{
    if ( auto* polygonCollection = RimTools::polygonCollection() )
    {
        polygonCollection->updateViewsAfterPolygonsChanged();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimContourMapPeak*> RimContourMapPeaksCollection::peaks() const
{
    return m_peaks.childrenByType();
}

//--------------------------------------------------------------------------------------------------
/// Deletes the existing peaks and recomputes new ones for the owning contour map projection, using
/// the current peakCount/minDistance field values.
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::computePeaks()
{
    auto* contourMapProjection = firstAncestorOrThisOfType<RimContourMapProjection>();
    if ( !contourMapProjection ) return;

    auto rigContourMapProjection = contourMapProjection->mapProjection();
    if ( !rigContourMapProjection ) return;

    RigContourMapPeakFinder::Settings settings;
    settings.maxPeaks         = std::max( 1, m_peakCount() );
    settings.minDistance      = std::max( 0.0, m_minDistance() );
    settings.minProminence    = std::max( 0.0, m_minProminence() );
    settings.excludeEdgePeaks = true;

    auto peaks = rigContourMapProjection->findPeaks( settings );

    // Rank the computed peaks by value (highest first), independent of the prominence-based criterion
    // used to select which peaks to keep.
    std::sort( peaks.begin(), peaks.end(), []( const auto& a, const auto& b ) { return a.z > b.z; } );

    clearPeaks();

    auto origin3d = rigContourMapProjection->origin3d();
    auto depth    = rigContourMapProjection->topDepthBoundingBox();

    // Sphere radius factor is multiplied by the view's characteristic cell size when rendered.
    const double sphereRadiusFactor = 0.3;

    for ( size_t i = 0; i < peaks.size(); ++i )
    {
        const auto& peak = peaks[i];

        cvf::Vec3d domainPoint( origin3d.x() + peak.x, origin3d.y() + peak.y, depth );
        addPeak( static_cast<int>( i + 1 ), peak.z, peak.prominence, domainPoint, sphereRadiusFactor );
    }

    m_resultSignature = currentResultSignature();
    updateOutdatedState();

    updateVisualization();

    Riu3DMainWindowTools::selectAsCurrentItem( this );
    Riu3DMainWindowTools::setExpanded( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::updateOutdatedState()
{
    QString newName = "Contour Map Peaks";
    if ( !m_peaks.empty() && m_resultSignature() != currentResultSignature() ) newName += " (Outdated)";

    if ( newName != name() )
    {
        setName( newName );
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// Identifies the result the peaks are computed from: result, time step and value filter.
//--------------------------------------------------------------------------------------------------
QString RimContourMapPeaksCollection::currentResultSignature() const
{
    auto* contourMapProjection = firstAncestorOrThisOfType<RimContourMapProjection>();
    if ( !contourMapProjection ) return {};

    QString signature = contourMapProjection->resultDescriptionText() + "|" + contourMapProjection->currentTimeStepName();

    if ( auto* rigContourMapProjection = contourMapProjection->mapProjection() )
    {
        if ( auto valueFilter = rigContourMapProjection->valueFilter() )
        {
            signature += QString( "|%1|%2" ).arg( valueFilter->first ).arg( valueFilter->second );
        }
    }

    return signature;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::initAfterRead()
{
    removePeaksWithMarkersSharedByOtherPeaks();
}

//--------------------------------------------------------------------------------------------------
/// When a contour map view is duplicated, the copied peaks reference the marker polygons of the
/// original peaks. Such peaks are removed from the copy without deleting the markers, so that one view
/// never deletes the markers of another view. The peaks can be recomputed in the copied view.
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::removePeaksWithMarkersSharedByOtherPeaks()
{
    for ( auto* peak : peaks() )
    {
        auto* marker = peak->markerPolygon();
        if ( !marker ) continue;

        const auto referringObjects = marker->objectsWithReferringPtrFields();

        const bool isShared = std::any_of( referringObjects.begin(),
                                           referringObjects.end(),
                                           [peak]( caf::PdmObjectHandle* obj )
                                           { return obj != peak && dynamic_cast<RimContourMapPeak*>( obj ) != nullptr; } );
        if ( !isShared ) continue;

        peak->releaseMarkerPolygon();
        m_peaks.removeChild( peak );
        delete peak;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_peakCount && m_peakCount() < 1 ) m_peakCount = 1;
    if ( changedField == &m_minDistance && m_minDistance() < 0.0 ) m_minDistance = 0.0;
    if ( changedField == &m_minProminence && m_minProminence() < 0.0 ) m_minProminence = 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapPeaksCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_peakCount );
    uiOrdering.add( &m_minDistance );
    uiOrdering.add( &m_minProminence );
    uiOrdering.addNewButton( "Compute", [this]() { computePeaks(); } );

    uiOrdering.skipRemainingFields();
}
