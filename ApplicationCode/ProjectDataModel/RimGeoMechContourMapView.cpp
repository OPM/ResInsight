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
#include "RimGeoMechContourMapView.h"
#include "RiuViewer.h"
#include "RivContourMapProjectionPartMgr.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCase.h"
#include "RimCellRangeFilterCollection.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGridCollection.h"
#include "RimScaleLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimViewNameConfig.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"

CAF_PDM_SOURCE_INIT( RimGeoMechContourMapView, "RimGeoMechContourMapView" );

const cvf::Mat4d RimGeoMechContourMapView::sm_defaultViewMatrix =
    cvf::Mat4d( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1000, 0, 0, 0, 1 );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView::RimGeoMechContourMapView()
    : m_cameraPositionLastUpdate( cvf::Vec3d::UNDEFINED )
{
    CAF_PDM_InitObject( "GeoMech Contour Map View", ":/2DMap16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_contourMapProjection, "ContourMapProjection", "Contour Map Projection", "", "", "" );
    m_contourMapProjection = new RimGeoMechContourMapProjection();

    CAF_PDM_InitField( &m_showAxisLines, "ShowAxisLines", true, "Show Axis Lines", "", "", "" );
    CAF_PDM_InitField( &m_showScaleLegend, "ShowScaleLegend", true, "Show Scale Legend", "", "", "" );

    m_gridCollection->setActive( false ); // This is also not added to the tree view, so cannot be enabled.

    setDefaultCustomName();

    m_contourMapProjectionPartMgr = new RivContourMapProjectionPartMgr( contourMapProjection(), this );

    ( (RiuViewerToViewInterface*)this )->setCameraPosition( sm_defaultViewMatrix );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapProjection* RimGeoMechContourMapView::contourMapProjection() const
{
    return m_contourMapProjection().p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechContourMapView::createAutoName() const
{
    QStringList autoName;

    if ( !nameConfig()->customName().isEmpty() )
    {
        autoName.push_back( nameConfig()->customName() );
    }

    QStringList generatedAutoTags;

    RimCase* ownerCase = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( ownerCase );

    if ( nameConfig()->addCaseName() )
    {
        generatedAutoTags.push_back( ownerCase->caseUserDescription() );
    }

    if ( nameConfig()->addAggregationType() )
    {
        generatedAutoTags.push_back( contourMapProjection()->resultAggregationText() );
    }

    if ( nameConfig()->addProperty() && !contourMapProjection()->isColumnResult() )
    {
        generatedAutoTags.push_back( cellResult()->resultFieldUiName() );
    }

    if ( nameConfig()->addSampleSpacing() )
    {
        generatedAutoTags.push_back( QString( "%1" ).arg( contourMapProjection()->sampleSpacingFactor(), 3, 'f', 2 ) );
    }

    if ( !generatedAutoTags.empty() )
    {
        autoName.push_back( generatedAutoTags.join( ", " ) );
    }
    return autoName.join( ": " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::setDefaultCustomName()
{
    nameConfig()->setCustomName( "Contour Map" );
    nameConfig()->hideCaseNameField( false );
    nameConfig()->hideAggregationTypeField( false );
    nameConfig()->hidePropertyField( false );
    nameConfig()->hideSampleSpacingField( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updatePickPointAndRedraw()
{
    appendPickPointVisToModel();
    if ( m_viewer )
    {
        m_viewer->update();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateCurrentTimeStepAndRedraw()
{
    m_contourMapProjection->clearGeometry();
    RimGeoMechView::updateCurrentTimeStepAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechContourMapView::isGridVisualizationMode() const
{
    return m_contourMapProjection->isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechContourMapView::isTimeStepDependentDataVisible() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::initAfterRead()
{
    RimGeoMechView::initAfterRead();

    m_gridCollection->setActive( false ); // This is also not added to the tree view, so cannot be enabled.
    disablePerspectiveProjectionField();
    setShowGridBox( false );
    meshMode.setValue( RiaDefines::NO_MESH );
    surfaceMode.setValue( FAULTS );
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::createDisplayModel()
{
    RimGeoMechView::createDisplayModel();

    if ( !this->isTimeStepDependentDataVisible() )
    {
        // Need to add geometry even if it hasn't happened during dynamic time step update.
        updateGeometry();
    }

    if ( this->viewer()->mainCamera()->viewMatrix() == sm_defaultViewMatrix )
    {
        this->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup( "Viewer" );
    viewGroup->add( this->userDescriptionField() );
    viewGroup->add( this->backgroundColorField() );
    viewGroup->add( &m_showAxisLines );
    viewGroup->add( &m_showScaleLegend );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Contour Map Name" );
    nameConfig()->uiOrdering( uiConfigName, *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                     QString                 uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    uiTreeOrdering.add( m_contourMapProjection );
    uiTreeOrdering.add( cellResult() );
    cellResult()->uiCapability()->setUiReadOnly( m_contourMapProjection->isColumnResult() );
    uiTreeOrdering.add( m_rangeFilterCollection() );
    uiTreeOrdering.add( nativePropertyFilterCollection() );

    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateCurrentTimeStep()
{
    updateGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateGeometry()
{
    caf::ProgressInfo progress( 100, "Generate Contour Map", true );

    { // Step 1: generate results. About 30% of the time.
        if ( m_contourMapProjection->isChecked() )
        {
            m_contourMapProjection->generateResultsIfNecessary( m_currentTimeStep() );
        }
        updateLegends();

        progress.setProgress( 30 );
    }

    { // Step 2: generate geometry. Takes about 60% of the time.
        createContourMapGeometry();
        progress.setProgress( 90 );
    }

    { // Step 3: generate drawables. About 10% of the time.
        appendContourMapProjectionToModel();
        appendContourLinesToModel();
        appendPickPointVisToModel();
        progress.setProgress( 100 );
    }
    m_overlayInfoConfig->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::createContourMapGeometry()
{
    if ( m_viewer && m_contourMapProjection->isChecked() )
    {
        m_contourMapProjectionPartMgr->createProjectionGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::appendContourMapProjectionToModel()
{
    if ( m_viewer && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = m_viewer->frame( m_currentTimeStep );
        if ( frameScene )
        {
            cvf::String name = "ContourMapProjection";
            this->removeModelByName( frameScene, name );

            cvf::ref<cvf::ModelBasicList> contourMapProjectionModelBasicList = new cvf::ModelBasicList;
            contourMapProjectionModelBasicList->setName( name );

            cvf::ref<caf::DisplayCoordTransform> transForm = this->displayCoordTransform();

            m_contourMapProjectionPartMgr->appendProjectionToModel( contourMapProjectionModelBasicList.p(),
                                                                    transForm.p() );
            contourMapProjectionModelBasicList->updateBoundingBoxesRecursive();
            frameScene->addModel( contourMapProjectionModelBasicList.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::appendContourLinesToModel()
{
    if ( m_viewer && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = m_viewer->frame( m_currentTimeStep );
        if ( frameScene )
        {
            cvf::String name = "ContourMapLines";
            this->removeModelByName( frameScene, name );

            cvf::ref<cvf::ModelBasicList> contourMapLabelModelBasicList = new cvf::ModelBasicList;
            contourMapLabelModelBasicList->setName( name );

            cvf::ref<caf::DisplayCoordTransform> transForm = this->displayCoordTransform();

            m_contourMapProjectionPartMgr->appendContourLinesToModel( viewer()->mainCamera(),
                                                                      contourMapLabelModelBasicList.p(),
                                                                      transForm.p() );
            contourMapLabelModelBasicList->updateBoundingBoxesRecursive();
            frameScene->addModel( contourMapLabelModelBasicList.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::appendPickPointVisToModel()
{
    if ( m_viewer && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = m_viewer->frame( m_currentTimeStep );
        if ( frameScene )
        {
            cvf::String name = "ContourMapPickPoint";
            this->removeModelByName( frameScene, name );

            cvf::ref<cvf::ModelBasicList> contourMapProjectionModelBasicList = new cvf::ModelBasicList;
            contourMapProjectionModelBasicList->setName( name );

            cvf::ref<caf::DisplayCoordTransform> transForm = this->displayCoordTransform();

            m_contourMapProjectionPartMgr->appendPickPointVisToModel( contourMapProjectionModelBasicList.p(),
                                                                      transForm.p() );
            contourMapProjectionModelBasicList->updateBoundingBoxesRecursive();
            frameScene->addModel( contourMapProjectionModelBasicList.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateLegends()
{
    if ( m_viewer )
    {
        m_viewer->removeAllColorLegends();

        if ( m_contourMapProjection && m_contourMapProjection->isChecked() )
        {
            RimRegularLegendConfig* projectionLegend = m_contourMapProjection->legendConfig();
            if ( projectionLegend )
            {
                m_contourMapProjection->updateLegend();
                if ( projectionLegend->showLegend() )
                {
                    m_viewer->addColorLegendToBottomLeftCorner( projectionLegend->titledOverlayFrame() );
                }
            }
        }

        m_viewer->showScaleLegend( m_showScaleLegend() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateViewWidgetAfterCreation()
{
    if ( m_viewer )
    {
        m_viewer->showAxisCross( false );
        m_viewer->showEdgeTickMarksXY( true, m_showAxisLines() );
        m_viewer->enableNavigationRotation( false );
    }

    Rim3dView::updateViewWidgetAfterCreation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateViewFollowingRangeFilterUpdates()
{
    m_contourMapProjection->setCheckState( true );
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::onLoadDataAndUpdate()
{
    RimGeoMechView::onLoadDataAndUpdate();
    if ( m_viewer )
    {
        m_viewer->setView( cvf::Vec3d( 0, 0, -1 ), cvf::Vec3d( 0, 1, 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    RimGeoMechView::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showAxisLines )
    {
        m_viewer->showEdgeTickMarksXY( true, m_showAxisLines() );
        scheduleCreateDisplayModelAndRedraw();
    }
    else if ( changedField == backgroundColorField() )
    {
        scheduleCreateDisplayModelAndRedraw();
    }
    else if ( changedField == &m_showScaleLegend )
    {
        updateLegends();
        scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGeoMechContourMapView::userDescriptionField()
{
    return nameConfig()->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGeoMechContourMapView::createViewWidget( QWidget* mainWindowParent )
{
    auto widget = Rim3dView::createViewWidget( mainWindowParent );

    if ( viewer() )
    {
        viewer()->showZScaleLabel( false );
        viewer()->hideZScaleCheckbox( true );
    }
    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::onViewNavigationChanged()
{
    cvf::Vec3d currentCameraPosition = viewer()->mainCamera()->position();
    if ( m_cameraPositionLastUpdate.isUndefined() || zoomChangeAboveTreshold( currentCameraPosition ) )
    {
        appendContourLinesToModel();
        m_cameraPositionLastUpdate = currentCameraPosition;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechContourMapView::zoomChangeAboveTreshold( const cvf::Vec3d& currentCameraPosition ) const
{
    double distance = std::max( std::fabs( m_cameraPositionLastUpdate.z() ), std::fabs( currentCameraPosition.z() ) );
    const double threshold = 0.05 * distance;
    return std::fabs( m_cameraPositionLastUpdate.z() - currentCameraPosition.z() ) > threshold;
}
