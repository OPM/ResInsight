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

#include "RimDataView.h"

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

#include "WellPath/RimWellPathInView.h"
#include "WellPath/RimWellPathInViewCollection.h"

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

CAF_PDM_SOURCE_INIT( RimDataView, "RimDataView", "DataView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataView::RimDataView()
    : m_isDomainBoundingBoxCached( false )
{
    CAF_PDM_InitObject( "Data View", ":/3DWindow.svg" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceInViewCollection", "Surface Collection Field" );

    CAF_PDM_InitFieldNoDefault( &m_polygonInViewCollection, "PolygonInViewCollection", "Polygon Collection Field" );
    m_polygonInViewCollection = new RimPolygonInViewCollection();
    m_polygonInViewCollection->uiCapability()->setUiIcon( caf::IconProvider( ":/PolylinesFromFile16x16.png" ) );

    CAF_PDM_InitFieldNoDefault( &m_wellPathInViewCollection, "WellPathInViewCollection", "Well Path Collection Field" );
    m_wellPathInViewCollection = new RimWellPathInViewCollection();

    CAF_PDM_InitFieldNoDefault( &m_annotationCollection, "AnnotationCollection", "Annotations" );
    m_annotationCollection = new RimAnnotationInViewCollection;

    CAF_PDM_InitFieldNoDefault( &m_overlayInfoConfig, "OverlayInfoConfig", "Info Box" );
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView( this );

    m_scaleTransform = new cvf::Transform();

    m_surfaceVizModel = new cvf::ModelBasicList;
    m_surfaceVizModel->setName( "SurfaceModel" );

    m_polygonVizModel = new cvf::ModelBasicList;
    m_polygonVizModel->setName( "PolygonModel" );

    meshMode.uiCapability()->setUiHidden( true );
    surfaceMode.uiCapability()->setUiHidden( true );
    hideComparisonViewField();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataView::~RimDataView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection* RimDataView::surfaceInViewCollection() const
{
    return m_surfaceCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection* RimDataView::polygonInViewCollection() const
{
    return m_polygonInViewCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathInViewCollection* RimDataView::wellPathInViewCollection() const
{
    return m_wellPathInViewCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataView::isWellPathVisibleInView( const RimWellPath* wellPath ) const
{
    if ( !m_wellPathInViewCollection() ) return true;

    return m_wellPathInViewCollection->isWellPathVisible( wellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimDataView::ownerCase() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::View3dContent RimDataView::viewContent() const
{
    return RiaDefines::View3dContent::DATA_OBJECTS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataView::isGridVisualizationMode() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataView::isUsingFormationNames() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimLegendConfig*> RimDataView::legendConfigs() const
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
void RimDataView::scheduleGeometryRegen( RivCellSetEnum geometryType )
{
    // no need to do anything here, there are no cell based geometry sets in a case-less view
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimDataView::computeDomainBoundingBox() const
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

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimDataView::cachedDomainBoundingBox() const
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
void RimDataView::invalidateDomainBoundingBox()
{
    m_isDomainBoundingBoxCached = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimDataView::domainBoundingBox()
{
    return cachedDomainBoundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDataView::characteristicCellSize() const
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
void RimDataView::updateGridBoxData()
{
    if ( viewer() )
    {
        viewer()->updateGridBoxData( m_scaleZ(), cvf::Vec3d::ZERO, backgroundColor(), domainBoundingBox(), fontSize() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( userDescriptionField() );

    Rim3dView::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    if ( surfaceInViewCollection() ) uiTreeOrdering.add( surfaceInViewCollection() );
    uiTreeOrdering.add( polygonInViewCollection() );
    uiTreeOrdering.add( wellPathInViewCollection() );
    uiTreeOrdering.add( annotationCollection() );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::onCreateDisplayModel()
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

    m_wellPathPipeVizModel->removeAllParts();
    addWellPathsToModel( m_wellPathPipeVizModel.p(), bb, characteristicCellSize() );
    nativeOrOverrideViewer()->addStaticModelOnce( m_wellPathPipeVizModel.p(), isUsingOverrideViewer() );

    // Surfaces

    m_surfaceVizModel->removeAllParts();
    if ( m_surfaceCollection )
    {
        bool nativeOnly = true;
        m_surfaceCollection->appendPartsToModel( m_surfaceVizModel.p(), scaleTransform(), nativeOnly );
        nativeOrOverrideViewer()->addStaticModelOnce( m_surfaceVizModel.p(), isUsingOverrideViewer() );
    }

    // Polygons

    m_polygonVizModel->removeAllParts();
    if ( m_polygonInViewCollection )
    {
        cvf::ref<caf::DisplayCoordTransform> transform = displayCoordTransform();
        for ( auto* polygonInView : m_polygonInViewCollection->visiblePolygonsInView() )
        {
            if ( polygonInView ) polygonInView->appendPartsToModel( m_polygonVizModel.p(), transform.p(), bb );
        }
        nativeOrOverrideViewer()->addStaticModelOnce( m_polygonVizModel.p(), isUsingOverrideViewer() );
        m_polygonVizModel->updateBoundingBoxesRecursive();
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
void RimDataView::onUpdateDisplayModelForCurrentTimeStep()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::onClampCurrentTimestep()
{
    m_currentTimeStep = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimDataView::onTimeStepCountRequested()
{
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataView::isTimeStepDependentDataVisible() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel )
{
    *xLabel = "E(x)";
    *yLabel = "N(y)";
    *zLabel = "Z";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts )
{
    // no action needed, might be needed if we want to hilite something later
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::onUpdateStaticCellColors()
{
    // no action needed
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::onUpdateLegends()
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
void RimDataView::onLoadDataAndUpdate()
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
void RimDataView::selectOverlayInfoConfig()
{
    Riu3DMainWindowTools::selectAsCurrentItem( m_overlayInfoConfig );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Transform* RimDataView::scaleTransform()
{
    return m_scaleTransform.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDataView::createAutoName() const
{
    if ( !nameConfig()->customName().isEmpty() ) return nameConfig()->customName();

    return "Data View";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::setDefaultView()
{
    if ( viewer() )
    {
        viewer()->setDefaultView( cvf::Vec3d::Y_AXIS, cvf::Vec3d::Z_AXIS );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataView::updateViewTreeItems( RiaDefines::ItemIn3dView itemType )
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

    if ( bitmaskEnum.AnyOf( RiaDefines::ItemIn3dView::WELL_PATH ) )
    {
        m_wellPathInViewCollection->updateFromWellPathCollection();
    }

    invalidateDomainBoundingBox();

    updateConnectedEditors();
}
