/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimGeneric3dView.h"

#include "Rim3dOverlayInfoConfig.h"
#include "Rim3dPropertiesInterface.h"
#include "RimAnnotationInViewCollection.h"
#include "RimLegendConfig.h"
#include "RimNameConfig.h"
#include "RimReachCircleAnnotation.h"
#include "RimReachCircleAnnotationInView.h"
#include "RimRegularLegendConfig.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceInViewCollection.h"
#include "RimTextAnnotation.h"
#include "RimTextAnnotationInView.h"
#include "RimTools.h"
#include "RimViewNameConfig.h"
#include "RimWellPathCollection.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"
#include "Polygons/RimPolygonInViewCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"

#include "cafDisplayCoordTransform.h"
#include "cvfBoundingBox.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfString.h"
#include "cvfTransform.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimGeneric3dView, "RimGeneric3dView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeneric3dView::RimGeneric3dView()
    : m_isDomainBoundingBoxCached( false )
{
    CAF_PDM_InitObject( "Generic View", ":/3DWindow.svg" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceInViewCollection", "Surface Collection Field" );

    CAF_PDM_InitFieldNoDefault( &m_polygonInViewCollection, "PolygonInViewCollection", "Polygon Collection Field" );
    m_polygonInViewCollection = new RimPolygonInViewCollection();
    m_polygonInViewCollection->uiCapability()->setUiIcon( caf::IconProvider( ":/PolylinesFromFile16x16.png" ) );

    CAF_PDM_InitFieldNoDefault( &m_annotationCollection, "AnnotationCollection", "Annotations" );
    m_annotationCollection = new RimAnnotationInViewCollection;

    CAF_PDM_InitFieldNoDefault( &m_overlayInfoConfig, "OverlayInfoConfig", "Info Box" );
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView( this );

    m_scaleTransform = new cvf::Transform();

    nameConfig()->hideCaseNameField( true );
    nameConfig()->hideAggregationTypeField( true );
    nameConfig()->hidePropertyField( true );
    nameConfig()->hideSampleSpacingField( true );

    meshMode.uiCapability()->setUiHidden( true );
    surfaceMode.uiCapability()->setUiHidden( true );
    hideComparisonViewField();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeneric3dView::~RimGeneric3dView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection* RimGeneric3dView::surfaceInViewCollection() const
{
    return m_surfaceCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection* RimGeneric3dView::polygonInViewCollection() const
{
    return m_polygonInViewCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::View3dContent RimGeneric3dView::viewContent() const
{
    return RiaDefines::View3dContent::DATA_OBJECTS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeneric3dView::isGridVisualizationMode() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimLegendConfig*> RimGeneric3dView::legendConfigs() const
{
    std::vector<RimLegendConfig*> legends;

    if ( m_surfaceCollection )
    {
        for ( auto legendConfig : m_surfaceCollection->legendConfigs() )
        {
            legends.push_back( legendConfig );
        }
    }

    legends.erase( std::remove( legends.begin(), legends.end(), nullptr ), legends.end() );

    return legends;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::scheduleGeometryRegen( RivCellSetEnum geometryType )
{
    // no need to do anything here, there are no cell based geometry sets in a case-less view
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeneric3dView::computeDomainBoundingBox() const
{
    cvf::BoundingBox bb;

    if ( auto* wellPathColl = RimWellPathCollection::instance() ) bb.add( wellPathColl->wellPathsBoundingBox() );

    if ( m_surfaceCollection() )
    {
        for ( auto* surfaceInView : m_surfaceCollection->visibleSurfacesInView() )
        {
            if ( auto* surface = surfaceInView->surface() )
            {
                if ( auto* propsInterface = dynamic_cast<Rim3dPropertiesInterface*>( surface ) )
                {
                    bb.add( propsInterface->boundingBoxInDomainCoords() );
                }
            }
        }
    }

    if ( m_polygonInViewCollection() )
    {
        for ( auto* polygonInView : m_polygonInViewCollection->visiblePolygonsInView() )
        {
            if ( auto* polygon = polygonInView->polygon() )
            {
                for ( const auto& point : polygon->pointsInDomainCoords() )
                    bb.add( point );
            }
        }
    }

    if ( m_annotationCollection() )
    {
        // Annotation part managers reject an invalid box, so annotations must widen it themselves
        for ( auto* inView : m_annotationCollection->globalTextAnnotations() )
        {
            if ( auto* annotation = inView->sourceAnnotation() )
            {
                bb.add( annotation->anchorPoint() );
                bb.add( annotation->labelPoint() );
            }
        }

        for ( auto* inView : m_annotationCollection->globalReachCircleAnnotations() )
        {
            if ( auto* annotation = inView->sourceAnnotation() ) bb.add( annotation->centerPoint() );
        }
    }

    if ( bb.isValid() )
    {
        // Avoid a degenerate bounding box where one axis is disproportionately small compared to the others
        // (e.g. surfaces or polygons located on a single horizontal plane give a near-zero Z extent, or a
        // single vertical well path gives a near-zero XY extent). A very thin axis gives the scene a
        // degenerate grid box and can confuse the camera framing, so each axis is padded to a minimum
        // fraction of the largest extent.
        const double maxExtent = std::max( { bb.extent().x(), bb.extent().y(), bb.extent().z() } );
        const double minExtent = maxExtent * 0.01;

        cvf::Vec3d min = bb.min();
        cvf::Vec3d max = bb.max();

        for ( int axis = 0; axis < 3; ++axis )
        {
            const double extent = max[axis] - min[axis];
            if ( extent < minExtent )
            {
                const double pad = 0.5 * ( minExtent - extent );
                min[axis] -= pad;
                max[axis] += pad;
            }
        }

        bb = cvf::BoundingBox( min, max );
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeneric3dView::cachedDomainBoundingBox() const
{
    if ( !m_isDomainBoundingBoxCached )
    {
        m_domainBoundingBox         = computeDomainBoundingBox();
        m_isDomainBoundingBoxCached = true;
    }

    return m_domainBoundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::invalidateDomainBoundingBox()
{
    m_isDomainBoundingBoxCached = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeneric3dView::domainBoundingBox()
{
    return cachedDomainBoundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeneric3dView::characteristicCellSize() const
{
    // No case to ask, so derive a length scale from the extent of the visualized data. The well pipe radius is
    // 0.1 (RimWellPathCollection::wellPathRadiusScaleFactor) times this value, so 1/100 of the horizontal extent
    // gives a pipe radius of ~1/1000 of the scene, which reads well on screen. Clamped to the same interval
    // RimEclipseCase::characteristicCellSize() uses.
    const auto bb = cachedDomainBoundingBox();
    if ( !bb.isValid() ) return 10.0;

    return std::clamp( 0.01 * std::max( bb.extent().x(), bb.extent().y() ), 10.0, 200.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::updateGridBoxData()
{
    if ( viewer() )
    {
        viewer()->updateGridBoxData( m_scaleZ(), cvf::Vec3d::ZERO, backgroundColor(), domainBoundingBox(), fontSize() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    Rim3dView::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "View Name" );
    nameConfig()->uiOrdering( uiConfigName, *nameGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    if ( surfaceInViewCollection() ) uiTreeOrdering.add( surfaceInViewCollection() );
    uiTreeOrdering.add( polygonInViewCollection() );
    uiTreeOrdering.add( annotationCollection() );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::onCreateDisplayModel()
{
    if ( nativeOrOverrideViewer() == nullptr ) return;

    invalidateDomainBoundingBox();
    const auto bb = domainBoundingBox();

    // Remove all existing frames from the viewer.
    nativeOrOverrideViewer()->removeAllFrames( isUsingOverrideViewer() );

    // Set the main scene in the viewer before adding the static models, since addStaticModelOnce() appends to the
    // frames of the existing scene.
    cvf::ref<cvf::Scene> mainScene = new cvf::Scene;
    nativeOrOverrideViewer()->setMainScene( mainScene.p(), isUsingOverrideViewer() );

    // Well path model

    auto* wellPathPipeVizModel = m_vizModels.findOrCreate( RivNamedVisualizationModels::wellPathPipeModelName() );
    wellPathPipeVizModel->removeAllParts();
    addWellPathsToModel( wellPathPipeVizModel, bb, characteristicCellSize() );
    nativeOrOverrideViewer()->addStaticModelOnce( wellPathPipeVizModel, isUsingOverrideViewer() );

    // Surfaces

    auto* surfaceVizModel = m_vizModels.findOrCreate( "SurfaceModel" );
    surfaceVizModel->removeAllParts();
    if ( m_surfaceCollection )
    {
        bool nativeOnly = true;
        m_surfaceCollection->appendPartsToModel( surfaceVizModel, scaleTransform(), nativeOnly );
        nativeOrOverrideViewer()->addStaticModelOnce( surfaceVizModel, isUsingOverrideViewer() );
    }

    // Polygons

    auto* polygonVizModel = m_vizModels.findOrCreate( "PolygonModel" );
    polygonVizModel->removeAllParts();
    if ( m_polygonInViewCollection )
    {
        cvf::ref<caf::DisplayCoordTransform> transform = displayCoordTransform();
        for ( auto* polygonInView : m_polygonInViewCollection->visiblePolygonsInView() )
        {
            if ( polygonInView ) polygonInView->appendPartsToModel( polygonVizModel, transform.p(), bb );
        }
        nativeOrOverrideViewer()->addStaticModelOnce( polygonVizModel, isUsingOverrideViewer() );
        polygonVizModel->updateBoundingBoxesRecursive();
    }

    // Annotations

    cvf::ref<cvf::ModelBasicList> annotationModel = new cvf::ModelBasicList;
    annotationModel->setName( "Annotations" );
    addAnnotationsToModel( annotationModel.p() );
    mainScene->addModel( annotationModel.p() );

    onUpdateLegends();
    if ( m_surfaceCollection )
    {
        m_surfaceCollection->applySingleColorEffect();
    }

    if ( bb.isValid() ) nativeOrOverrideViewer()->setPointOfInterest( bb.center() );

    m_overlayInfoConfig()->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel )
{
    *xLabel = "E(x)";
    *yLabel = "N(y)";
    *zLabel = "Z";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::onUpdateLegends()
{
    if ( !nativeOrOverrideViewer() ) return;

    if ( !isUsingOverrideViewer() )
    {
        nativeOrOverrideViewer()->removeAllColorLegends();
    }
    else
    {
        for ( auto legendConf : legendConfigs() )
        {
            nativeOrOverrideViewer()->removeColorLegend( legendConf->titledOverlayFrame() );
        }
    }

    if ( m_surfaceCollection && m_surfaceCollection->isChecked() )
    {
        m_surfaceCollection->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer(), isUsingOverrideViewer() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::onLoadDataAndUpdate()
{
    updateViewTreeItems( RiaDefines::ItemIn3dView::ALL );
    synchronizeLocalAnnotationsFromGlobal();
    onUpdateScaleTransform();

    updateDockWindowVisibility();

    // Surface data must load before the bounding box is computed, or boundingBoxInDomainCoords() returns an
    // empty box because the surface data has not been fetched yet.
    if ( m_surfaceCollection ) m_surfaceCollection->loadData( m_currentTimeStep );

    invalidateDomainBoundingBox();

    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::selectOverlayInfoConfig()
{
    Riu3DMainWindowTools::selectAsCurrentItem( m_overlayInfoConfig );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Transform* RimGeneric3dView::scaleTransform()
{
    return m_scaleTransform.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeneric3dView::createAutoName() const
{
    if ( !nameConfig()->customName().isEmpty() ) return nameConfig()->customName();

    return "Generic View";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::setDefaultView()
{
    if ( viewer() )
    {
        viewer()->setDefaultView( cvf::Vec3d::Y_AXIS, cvf::Vec3d::Z_AXIS );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeneric3dView::updateViewTreeItems( RiaDefines::ItemIn3dView itemType )
{
    auto bitmaskEnum = BitmaskEnum( itemType );

    if ( bitmaskEnum.AnyOf( RiaDefines::ItemIn3dView::SURFACE ) )
    {
        RimSurfaceCollection* surfColl = RimTools::surfaceCollection();

        if ( surfColl && surfColl->containsSurface() )
        {
            if ( !m_surfaceCollection() )
            {
                m_surfaceCollection = new RimSurfaceInViewCollection();
            }

            m_surfaceCollection->setSurfaceCollection( surfColl );
            m_surfaceCollection->updateFromSurfaceCollection();
        }
        else
        {
            delete m_surfaceCollection;
        }
    }

    if ( bitmaskEnum.AnyOf( RiaDefines::ItemIn3dView::POLYGON ) )
    {
        m_polygonInViewCollection->updateFromPolygonCollection();
    }

    invalidateDomainBoundingBox();

    updateConnectedEditors();
}
