/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimGridView.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGridCollection.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimProject.h"
#include "RimPropertyFilterCollection.h"
#include "RimSeismicSectionCollection.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceInViewCollection.h"
#include "RimTextAnnotation.h"
#include "RimTools.h"
#include "RimViewController.h"
#include "RimViewNameConfig.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementInViewCollection.h"
#include "RimWellPathCollection.h"

#include "Polygons/RimPolygonInView.h"
#include "Polygons/RimPolygonInViewCollection.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "RivSingleCellPartGenerator.h"

#include "cvfModel.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"

#include "cafPdmUiTreeOrdering.h"

#include <set>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimGridView, "GenericGridView" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView::RimGridView()
    : cellVisibilityChanged( this )
{
    CAF_PDM_InitFieldNoDefault( &m_overrideCellFilterCollection, "CellFiltersControlled", "Cell Filters (controlled)" );
    m_overrideCellFilterCollection.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_intersectionCollection, "CrossSections", "Intersections" );
    m_intersectionCollection = new RimIntersectionCollection();

    CAF_PDM_InitFieldNoDefault( &m_intersectionResultDefCollection, "IntersectionResultDefColl", "Intersection Results" );
    m_intersectionResultDefCollection = new RimIntersectionResultsDefinitionCollection;

    CAF_PDM_InitFieldNoDefault( &m_surfaceResultDefCollection, "ReservoirSurfaceResultDefColl", "Surface Results" );
    m_surfaceResultDefCollection = new RimIntersectionResultsDefinitionCollection;
    m_surfaceResultDefCollection->uiCapability()->setUiName( "Surface Results" );
    m_surfaceResultDefCollection->uiCapability()->setUiIcon( caf::IconProvider( ":/ReservoirSurface16x16.png" ) );

    CAF_PDM_InitFieldNoDefault( &m_gridCollection, "GridCollection", "GridCollection" );
    m_gridCollection = new RimGridCollection();

    m_previousGridModeMeshLinesWasFaults = false;

    CAF_PDM_InitFieldNoDefault( &m_overlayInfoConfig, "OverlayInfoConfig", "Info Box" );
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView( this );

    CAF_PDM_InitFieldNoDefault( &m_wellMeasurementCollection, "WellMeasurements", "Well Measurements" );
    m_wellMeasurementCollection = new RimWellMeasurementInViewCollection;

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceInViewCollection", "Surface Collection Field" );

    CAF_PDM_InitFieldNoDefault( &m_seismicSectionCollection, "SeismicSectionCollection", "Seismic Collection Field" );
    m_seismicSectionCollection = new RimSeismicSectionCollection();

    CAF_PDM_InitFieldNoDefault( &m_polygonInViewCollection, "PolygonInViewCollection", "Polygon Collection Field" );
    m_polygonInViewCollection = new RimPolygonInViewCollection();

    CAF_PDM_InitFieldNoDefault( &m_cellFilterCollection, "RangeFilters", "Cell Filter Collection Field" );
    m_cellFilterCollection = new RimCellFilterCollection();

    m_surfaceVizModel = new cvf::ModelBasicList;
    m_surfaceVizModel->setName( "SurfaceModel" );

    m_intersectionVizModel = new cvf::ModelBasicList;
    m_intersectionVizModel->setName( "CrossSectionModel" );

    m_polygonVizModel = new cvf::ModelBasicList;
    m_polygonVizModel->setName( "PolygonModel" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::showGridCells( bool enableGridCells )
{
    m_gridCollection->setActive( enableGridCells );

    onCreateDisplayModel();
    updateDisplayModelVisibility();
    RiuMainWindow::instance()->refreshDrawStyleActions();
    RiuMainWindow::instance()->refreshAnimationActions();

    m_gridCollection->updateConnectedEditors();
    m_gridCollection->updateUiIconFromState( enableGridCells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimGridView::currentTotalCellVisibility()
{
    if ( m_currentReservoirCellVisibility.isNull() )
    {
        m_currentReservoirCellVisibility = new cvf::UByteArray;
        calculateCurrentTotalCellVisibility( m_currentReservoirCellVisibility.p(), m_currentTimeStep() );
        cellVisibilityChanged.send();
    }

    return m_currentReservoirCellVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* RimGridView::intersectionCollection() const
{
    return m_intersectionCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection* RimGridView::surfaceInViewCollection() const
{
    return m_surfaceCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSectionCollection* RimGridView::seismicSectionCollection() const
{
    return m_seismicSectionCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection* RimGridView::polygonInViewCollection() const
{
    return m_polygonInViewCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInViewCollection* RimGridView::measurementCollection() const
{
    return m_wellMeasurementCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection* RimGridView::separateIntersectionResultsCollection() const
{
    return m_intersectionResultDefCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection* RimGridView::separateSurfaceResultsCollection() const
{
    return m_surfaceResultDefCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::cellFiltersUpdated()
{
    updateViewFollowingCellFilterUpdates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterCollection* RimGridView::cellFilterCollection()
{
    if ( viewController() && viewController()->isCellFiltersControlled() && m_overrideCellFilterCollection )
    {
        return m_overrideCellFilterCollection;
    }
    else
    {
        return m_cellFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimCellFilterCollection* RimGridView::cellFilterCollection() const
{
    if ( viewController() && viewController()->isCellFiltersControlled() && m_overrideCellFilterCollection )
    {
        return m_overrideCellFilterCollection;
    }
    else
    {
        return m_cellFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridView::hasOverriddenCellFilterCollection()
{
    return m_overrideCellFilterCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::setOverrideCellFilterCollection( RimCellFilterCollection* rfc )
{
    if ( m_overrideCellFilterCollection() ) delete m_overrideCellFilterCollection();

    m_overrideCellFilterCollection = rfc;
    // Maintain a link in the active-selection
    if ( m_overrideCellFilterCollection )
    {
        m_cellFilterCollection->setActive( m_overrideCellFilterCollection->isActive() );
        m_cellFilterCollection()->uiCapability()->updateConnectedEditors();
    }

    scheduleGeometryRegen( RANGE_FILTERED );
    scheduleGeometryRegen( RANGE_FILTERED_INACTIVE );

    scheduleCreateDisplayModelAndRedraw();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::replaceCellFilterCollectionWithOverride()
{
    RimCellFilterCollection* overrideRfc = m_overrideCellFilterCollection;
    CVF_ASSERT( overrideRfc );

    RimCellFilterCollection* currentRfc = m_cellFilterCollection;
    if ( currentRfc )
    {
        delete currentRfc;
    }

    // Must call removeChildObject() to make sure the object has no parent
    // No parent is required when assigning a object into a field
    m_overrideCellFilterCollection.removeChild( overrideRfc );

    m_cellFilterCollection = overrideRfc;

    uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridView::isGridVisualizationMode() const
{
    return m_gridCollection->isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig* RimGridView::overlayInfoConfig() const
{
    return m_overlayInfoConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::updateViewFollowingCellFilterUpdates()
{
    showGridCells( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::initAfterRead()
{
    Rim3dView::initAfterRead();

    RimProject* proj = RimProject::current();
    if ( proj && proj->isProjectFileVersionEqualOrOlderThan( "2018.1.1" ) )
    {
        // For version prior to 2018.1.1 : Grid visualization mode was derived from surfaceMode and meshMode
        // Current : Grid visualization mode is directly defined by m_gridCollection->isActive
        // This change was introduced in https://github.com/OPM/ResInsight/commit/f7bfe8d0

        bool isGridVisualizationModeBefore_2018_1_1 =
            ( ( surfaceMode() == RimGridView::SURFACE ) || ( meshMode() == RiaDefines::MeshModeType::FULL_MESH ) );

        m_gridCollection->setActive( isGridVisualizationModeBefore_2018_1_1 );
        if ( !isGridVisualizationModeBefore_2018_1_1 )
        {
            // Was showing faults and intersections.
            // If was showing with mesh and/or surfaces, turn to full mesh/surf mode to show the mesh,
            // and to avoid a strange setup when dropping out into grid mode again
            if ( surfaceMode() != RimGridView::NO_SURFACE ) surfaceMode = RimGridView::SURFACE;
            if ( meshMode() != RiaDefines::MeshModeType::NO_MESH ) meshMode = RiaDefines::MeshModeType::FULL_MESH;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts )
{
    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();

    std::vector<RiuSelectionItem*> items;
    riuSelManager->selectedItems( items );

    for ( size_t i = 0; i < items.size(); i++ )
    {
        if ( items[i]->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT )
        {
            RiuGeoMechSelectionItem* geomSelItem = static_cast<RiuGeoMechSelectionItem*>( items[i] );

            if ( geomSelItem && geomSelItem->m_view == this && geomSelItem->m_resultDefinition->geoMechCase() )
            {
                RivSingleCellPartGenerator partGen( geomSelItem->m_resultDefinition->geoMechCase(),
                                                    geomSelItem->m_gridIndex,
                                                    geomSelItem->m_cellIndex,
                                                    ownerCase()->displayModelOffset() );

                cvf::ref<cvf::Part> part = partGen.createPart( geomSelItem->m_color );
                part->setTransform( scaleTransform() );

                parts->push_back( part.p() );
            }
        }

        if ( items[i]->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT )
        {
            RiuEclipseSelectionItem* eclipseSelItem = static_cast<RiuEclipseSelectionItem*>( items[i] );

            if ( eclipseSelItem && eclipseSelItem->m_view == this && eclipseSelItem->m_resultDefinition->eclipseCase() )
            {
                CVF_ASSERT( eclipseSelItem->m_resultDefinition->eclipseCase()->eclipseCaseData() );

                RivSingleCellPartGenerator partGen( eclipseSelItem->m_resultDefinition->eclipseCase()->eclipseCaseData(),
                                                    eclipseSelItem->m_gridIndex,
                                                    eclipseSelItem->m_gridLocalCellIndex,
                                                    ownerCase()->displayModelOffset() );

                cvf::ref<cvf::Part> part = partGen.createPart( eclipseSelItem->m_color );
                part->setTransform( scaleTransform() );

                parts->push_back( part.p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::appendPolygonPartsToModel( caf::DisplayCoordTransform* scaleTransform, const cvf::BoundingBox& boundingBox )
{
    m_polygonVizModel->removeAllParts();

    std::vector<RimPolygonInView*> polygonsInView;
    if ( m_polygonInViewCollection )
    {
        polygonsInView = m_polygonInViewCollection->visiblePolygonsInView();
    }

    if ( cellFilterCollection() && cellFilterCollection()->isActive() )
    {
        auto cellFilterPolygonsInView = cellFilterCollection()->enabledCellFilterPolygons();
        polygonsInView.insert( polygonsInView.end(), cellFilterPolygonsInView.begin(), cellFilterPolygonsInView.end() );
    }

    for ( RimPolygonInView* polygonInView : polygonsInView )
    {
        if ( polygonInView )
        {
            polygonInView->appendPartsToModel( m_polygonVizModel.p(), scaleTransform, boundingBox );
        }
    }

    nativeOrOverrideViewer()->addStaticModelOnce( m_polygonVizModel.p(), isUsingOverrideViewer() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::onClearReservoirCellVisibilitiesIfNecessary()
{
    if ( propertyFilterCollection() && propertyFilterCollection()->hasActiveDynamicFilters() )
    {
        m_currentReservoirCellVisibility = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_scaleZ )
    {
        m_intersectionCollection->updateIntersectionBoxGeometry();
    }

    Rim3dView::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCollection* RimGridView::gridCollection() const
{
    return m_gridCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::clearReservoirCellVisibilities()
{
    m_currentReservoirCellVisibility = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::addRequiredUiTreeObjects( caf::PdmUiTreeOrdering& uiTreeOrdering )
{
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( wellPathCollection )
    {
        const RimWellMeasurementCollection* measurementCollection = wellPathCollection->measurementCollection();
        if ( !measurementCollection->measurements().empty() )
        {
            uiTreeOrdering.add( &m_wellMeasurementCollection );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::selectOverlayInfoConfig()
{
    Riu3DMainWindowTools::selectAsCurrentItem( m_overlayInfoConfig );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::updateWellMeasurements()
{
    m_wellMeasurementCollection->syncWithChangesInWellMeasurementCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::updateViewTreeItems( RiaDefines::ItemIn3dView itemType )
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

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::appendIntersectionsForCurrentTimeStep()
{
    if ( nativeOrOverrideViewer() )
    {
        cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
        if ( frameScene )
        {
            cvf::String name = "IntersectionDynamicModel";
            Rim3dView::removeModelByName( frameScene, name );

            cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
            frameParts->setName( name );

            cvf::UByteArray visibility;

            calculateCellVisibility( &visibility, { PROPERTY_FILTERED, PROPERTY_FILTERED_WELL_CELLS }, m_currentTimeStep );

            m_intersectionCollection->appendDynamicPartsToModel( frameParts.p(), scaleTransform(), m_currentTimeStep, &visibility );

            frameParts->updateBoundingBoxesRecursive();

            frameScene->addModel( frameParts.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::appendIntersectionsToModel( bool cellFiltersActive, bool propertyFiltersActive )
{
    m_intersectionVizModel->removeAllParts();
    if ( m_intersectionCollection->isActive() )
    {
        m_intersectionCollection->clearGeometry();

        if ( m_intersectionCollection->shouldApplyCellFiltersToIntersections() && ( cellFiltersActive || propertyFiltersActive ) )
        {
            if ( !propertyFiltersActive )
            {
                cvf::UByteArray visibleCells;
                calculateCellVisibility( &visibleCells, { RANGE_FILTERED_WELL_CELLS, RANGE_FILTERED } );
                m_intersectionCollection->appendDynamicPartsToModel( m_intersectionVizModel.p(), scaleTransform(), currentTimeStep(), &visibleCells );
            }

            // NB! Geometry objects are recreated in appendDynamicPartsToModel(), always call
            // appendPartsToModel() after appendDynamicPartsToModel()
            m_intersectionCollection->appendPartsToModel( *this, m_intersectionVizModel.p(), scaleTransform() );
        }
        else
        {
            m_intersectionCollection->appendDynamicPartsToModel( m_intersectionVizModel.p(), scaleTransform(), currentTimeStep() );

            // NB! Geometry objects are recreated in appendDynamicPartsToModel(), always call
            // appendPartsToModel() after appendDynamicPartsToModel()
            m_intersectionCollection->appendPartsToModel( *this, m_intersectionVizModel.p(), scaleTransform() );
        }
        m_intersectionVizModel->updateBoundingBoxesRecursive();
        nativeOrOverrideViewer()->addStaticModelOnce( m_intersectionVizModel.p(), isUsingOverrideViewer() );
    }
}
