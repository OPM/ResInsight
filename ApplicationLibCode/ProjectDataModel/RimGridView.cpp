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
#include "RimAnnotationInViewCollection.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGridCollection.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimOilField.h"
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

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RiuMainWindow.h"

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
{
    CAF_PDM_InitFieldNoDefault( &m_overrideCellFilterCollection, "CellFiltersControlled", "Cell Filters (controlled)" );
    m_overrideCellFilterCollection.uiCapability()->setUiTreeHidden( true );
    m_overrideCellFilterCollection.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_intersectionCollection, "CrossSections", "Intersections" );
    m_intersectionCollection.uiCapability()->setUiTreeHidden( true );
    m_intersectionCollection = new RimIntersectionCollection();

    CAF_PDM_InitFieldNoDefault( &m_intersectionResultDefCollection, "IntersectionResultDefColl", "Intersection Results" );
    m_intersectionResultDefCollection.uiCapability()->setUiTreeHidden( true );
    m_intersectionResultDefCollection = new RimIntersectionResultsDefinitionCollection;

    CAF_PDM_InitFieldNoDefault( &m_surfaceResultDefCollection, "ReservoirSurfaceResultDefColl", "Surface Results" );
    m_surfaceResultDefCollection.uiCapability()->setUiTreeHidden( true );
    m_surfaceResultDefCollection = new RimIntersectionResultsDefinitionCollection;
    m_surfaceResultDefCollection->uiCapability()->setUiName( "Surface Results" );
    m_surfaceResultDefCollection->uiCapability()->setUiIcon( caf::IconProvider( ":/ReservoirSurface16x16.png" ) );

    CAF_PDM_InitFieldNoDefault( &m_gridCollection, "GridCollection", "GridCollection" );
    m_gridCollection.uiCapability()->setUiTreeHidden( true );
    m_gridCollection = new RimGridCollection();

    m_previousGridModeMeshLinesWasFaults = false;

    CAF_PDM_InitFieldNoDefault( &m_overlayInfoConfig, "OverlayInfoConfig", "Info Box" );
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView( this );
    m_overlayInfoConfig.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellMeasurementCollection, "WellMeasurements", "Well Measurements" );
    m_wellMeasurementCollection = new RimWellMeasurementInViewCollection;
    m_wellMeasurementCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceInViewCollection", "Surface Collection Field" );
    m_surfaceCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_seismicSectionCollection, "SeismicSectionCollection", "Seismic Collection Field" );
    m_seismicSectionCollection.uiCapability()->setUiTreeHidden( true );
    m_seismicSectionCollection = new RimSeismicSectionCollection();

    CAF_PDM_InitFieldNoDefault( &m_cellFilterCollection, "RangeFilters", "Cell Filter Collection Field" );
    m_cellFilterCollection = new RimCellFilterCollection();
    m_cellFilterCollection.uiCapability()->setUiTreeHidden( true );

    m_surfaceVizModel = new cvf::ModelBasicList;
    m_surfaceVizModel->setName( "SurfaceModel" );
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
        this->calculateCurrentTotalCellVisibility( m_currentReservoirCellVisibility.p(), m_currentTimeStep() );
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
    if ( this->viewController() && this->viewController()->isCellFiltersControlled() && m_overrideCellFilterCollection )
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
    if ( this->viewController() && this->viewController()->isCellFiltersControlled() && m_overrideCellFilterCollection )
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
RimAnnotationInViewCollection* RimGridView::annotationCollection() const
{
    return m_annotationCollection;
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

    this->scheduleGeometryRegen( RANGE_FILTERED );
    this->scheduleGeometryRegen( RANGE_FILTERED_INACTIVE );

    this->scheduleCreateDisplayModelAndRedraw();
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

    this->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridView::isGridVisualizationMode() const
{
    return this->m_gridCollection->isActive();
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

    RimProject* proj = nullptr;
    firstAncestorOrThisOfType( proj );
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
                                                    this->ownerCase()->displayModelOffset() );

                cvf::ref<cvf::Part> part = partGen.createPart( geomSelItem->m_color );
                part->setTransform( this->scaleTransform() );

                parts->push_back( part.p() );
            }
        }

        if ( items[i]->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT )
        {
            RiuEclipseSelectionItem* eclipseSelItem = static_cast<RiuEclipseSelectionItem*>( items[i] );

            if ( eclipseSelItem && eclipseSelItem->m_view == this )
            {
                CVF_ASSERT( eclipseSelItem->m_resultDefinition->eclipseCase() );
                CVF_ASSERT( eclipseSelItem->m_resultDefinition->eclipseCase()->eclipseCaseData() );

                RivSingleCellPartGenerator partGen( eclipseSelItem->m_resultDefinition->eclipseCase()->eclipseCaseData(),
                                                    eclipseSelItem->m_gridIndex,
                                                    eclipseSelItem->m_gridLocalCellIndex,
                                                    this->ownerCase()->displayModelOffset() );

                cvf::ref<cvf::Part> part = partGen.createPart( eclipseSelItem->m_color );
                part->setTransform( this->scaleTransform() );

                parts->push_back( part.p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridView::onClearReservoirCellVisibilitiesIfNecessary()
{
    if ( this->propertyFilterCollection() && this->propertyFilterCollection()->hasActiveDynamicFilters() )
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
void RimGridView::updateSurfacesInViewTreeItems()
{
    RimProject*           proj     = RimProject::current();
    RimSurfaceCollection* surfColl = proj->activeOilField()->surfaceCollection();

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

    this->updateConnectedEditors();
}
