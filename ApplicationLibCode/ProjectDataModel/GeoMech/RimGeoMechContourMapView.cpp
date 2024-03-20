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

#include "RicfCommandObject.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGridCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimViewNameConfig.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"

CAF_PDM_SOURCE_INIT( RimGeoMechContourMapView, "RimGeoMechContourMapView" );

const cvf::Mat4d RimGeoMechContourMapView::sm_defaultViewMatrix = cvf::Mat4d( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1000, 0, 0, 0, 1 );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView::RimGeoMechContourMapView()
    : m_cameraPositionLastUpdate( cvf::Vec3d::UNDEFINED )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "GeoMech Contour Map View",
                                                    ":/2DMap16x16.png",
                                                    "",
                                                    "",
                                                    "GeoMechContourMap",
                                                    "A contour map for GeoMech cases" );

    CAF_PDM_InitFieldNoDefault( &m_contourMapProjection, "ContourMapProjection", "Contour Map Projection" );
    m_contourMapProjection = new RimGeoMechContourMapProjection();

    CAF_PDM_InitField( &m_showAxisLines, "ShowAxisLines", true, "Show Axis Lines" );
    CAF_PDM_InitField( &m_showScaleLegend, "ShowScaleLegend", true, "Show Scale Legend" );

    m_gridCollection->setActive( false ); // This is also not added to the tree view, so cannot be enabled.

    setDefaultCustomName();

    m_contourMapProjectionPartMgr = new RivContourMapProjectionPartMgr( contourMapProjection(), this );

    ( (RiuViewerToViewInterface*)this )->setCameraPosition( sm_defaultViewMatrix );

    cellResult()->legendConfigChanged.connect( this, &RimGeoMechContourMapView::onLegendConfigChanged );
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
RiaDefines::View3dContent RimGeoMechContourMapView::viewContent() const
{
    return ( RiaDefines::View3dContent::GEOMECH_DATA & RiaDefines::View3dContent::CONTOUR );
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

    if ( nameConfig()->addCaseName() && ownerCase() )
    {
        generatedAutoTags.push_back( ownerCase()->caseUserDescription() );
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
    if ( nativeOrOverrideViewer() )
    {
        nativeOrOverrideViewer()->update();
    }
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
    meshMode.setValue( RiaDefines::MeshModeType::NO_MESH );
    surfaceMode.setValue( FAULTS );
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::onCreateDisplayModel()
{
    RimGeoMechView::onCreateDisplayModel();

    if ( !isTimeStepDependentDataVisible() )
    {
        // Need to add geometry even if it hasn't happened during dynamic time step update.
        updateGeometry();
    }

    if ( viewer()->mainCamera()->viewMatrix() == sm_defaultViewMatrix )
    {
        zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup( "Viewer" );
    viewGroup->add( userDescriptionField() );
    viewGroup->add( backgroundColorField() );
    viewGroup->add( &m_showAxisLines );
    viewGroup->add( &m_showScaleLegend );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Contour Map Name" );
    nameConfig()->uiOrdering( uiConfigName, *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    uiTreeOrdering.add( m_contourMapProjection );
    uiTreeOrdering.add( cellResult() );
    cellResult()->uiCapability()->setUiReadOnly( m_contourMapProjection->isColumnResult() );
    uiTreeOrdering.add( m_cellFilterCollection() );
    uiTreeOrdering.add( nativePropertyFilterCollection() );

    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::onUpdateDisplayModelForCurrentTimeStep()
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
            m_contourMapProjection->generateResultsIfNecessary( m_currentTimeStep );
        }
        onUpdateLegends();

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
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        m_contourMapProjectionPartMgr->createProjectionGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::appendContourMapProjectionToModel()
{
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
        if ( frameScene )
        {
            cvf::String name = "ContourMapProjection";
            RimGeoMechContourMapView::removeModelByName( frameScene, name );

            cvf::ref<cvf::ModelBasicList> contourMapProjectionModelBasicList = new cvf::ModelBasicList;
            contourMapProjectionModelBasicList->setName( name );

            cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

            m_contourMapProjectionPartMgr->appendProjectionToModel( contourMapProjectionModelBasicList.p(), transForm.p() );
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
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
        if ( frameScene )
        {
            cvf::String name = "ContourMapLines";
            RimGeoMechContourMapView::removeModelByName( frameScene, name );

            cvf::ref<cvf::ModelBasicList> contourMapLabelModelBasicList = new cvf::ModelBasicList;
            contourMapLabelModelBasicList->setName( name );

            cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

            m_contourMapProjectionPartMgr->appendContourLinesToModel( viewer()->mainCamera(), contourMapLabelModelBasicList.p(), transForm.p() );
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
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
        if ( frameScene )
        {
            cvf::String name = "ContourMapPickPoint";
            RimGeoMechContourMapView::removeModelByName( frameScene, name );

            cvf::ref<cvf::ModelBasicList> contourMapProjectionModelBasicList = new cvf::ModelBasicList;
            contourMapProjectionModelBasicList->setName( name );

            cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

            m_contourMapProjectionPartMgr->appendPickPointVisToModel( contourMapProjectionModelBasicList.p(), transForm.p() );
            contourMapProjectionModelBasicList->updateBoundingBoxesRecursive();
            frameScene->addModel( contourMapProjectionModelBasicList.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::onUpdateLegends()
{
    if ( nativeOrOverrideViewer() )
    {
        if ( !isUsingOverrideViewer() )
        {
            nativeOrOverrideViewer()->removeAllColorLegends();
        }
        else if ( m_contourMapProjection && m_contourMapProjection->legendConfig() )
        {
            nativeOrOverrideViewer()->removeColorLegend( m_contourMapProjection->legendConfig()->titledOverlayFrame() );
        }

        if ( m_contourMapProjection && m_contourMapProjection->isChecked() )
        {
            RimRegularLegendConfig* projectionLegend = m_contourMapProjection->legendConfig();
            if ( projectionLegend )
            {
                m_contourMapProjection->updateLegend();
                if ( projectionLegend->showLegend() )
                {
                    nativeOrOverrideViewer()->addColorLegendToBottomLeftCorner( projectionLegend->titledOverlayFrame(),
                                                                                isUsingOverrideViewer() );
                }
            }
        }

        nativeOrOverrideViewer()->showScaleLegend( m_showScaleLegend() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateViewWidgetAfterCreation()
{
    if ( viewer() )
    {
        viewer()->showAxisCross( false );
        viewer()->showEdgeTickMarksXY( true, m_showAxisLines() );
        viewer()->enableNavigationRotation( false );
    }

    Rim3dView::updateViewWidgetAfterCreation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::updateViewFollowingCellFilterUpdates()
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
    if ( nativeOrOverrideViewer() )
    {
        nativeOrOverrideViewer()->setView( cvf::Vec3d( 0, 0, -1 ), cvf::Vec3d( 0, 1, 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimGeoMechView::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showAxisLines )
    {
        viewer()->showEdgeTickMarksXY( true, m_showAxisLines() );
        scheduleCreateDisplayModelAndRedraw();
    }
    else if ( changedField == backgroundColorField() )
    {
        scheduleCreateDisplayModelAndRedraw();
    }
    else if ( changedField == &m_showScaleLegend )
    {
        onUpdateLegends();
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
    double       distance  = std::max( std::fabs( m_cameraPositionLastUpdate.z() ), std::fabs( currentCameraPosition.z() ) );
    const double threshold = 0.05 * distance;
    return std::fabs( m_cameraPositionLastUpdate.z() - currentCameraPosition.z() ) > threshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::scheduleGeometryRegen( RivCellSetEnum geometryType )
{
    m_contourMapProjection->clearGeometry();
    RimGeoMechView::scheduleGeometryRegen( geometryType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapView::onLegendConfigChanged( const caf::SignalEmitter* emitter, RimLegendConfigChangeType changeType )
{
    using ChangeType = RimLegendConfigChangeType;
    if ( changeType == ChangeType::LEVELS || changeType == ChangeType::COLOR_MODE || changeType == ChangeType::RANGE ||
         changeType == ChangeType::ALL )
    {
        m_contourMapProjection->clearGeometry();
    }
}
