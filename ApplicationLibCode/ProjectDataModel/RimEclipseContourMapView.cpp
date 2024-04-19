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

#include "RimEclipseContourMapView.h"

#include "RicfCommandObject.h"

#include "RiuViewer.h"
#include "RivContourMapProjectionPartMgr.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimAnnotationInViewCollection.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimFaultInViewCollection.h"
#include "RimGridCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimViewLinker.h"
#include "RimViewNameConfig.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"

CAF_PDM_SOURCE_INIT( RimEclipseContourMapView, "RimContourMapView" );

const cvf::Mat4d RimEclipseContourMapView::sm_defaultViewMatrix = cvf::Mat4d( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1000, 0, 0, 0, 1 );

RimEclipseContourMapView::RimEclipseContourMapView()
    : m_cameraPositionLastUpdate( cvf::Vec3d::UNDEFINED )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Contour Map View",
                                                    ":/2DMap16x16.png",
                                                    "",
                                                    "",
                                                    "EclipseContourMap",
                                                    "A contour map for Eclipse cases" );

    CAF_PDM_InitFieldNoDefault( &m_contourMapProjection, "ContourMapProjection", "Contour Map Projection" );
    m_contourMapProjection = new RimEclipseContourMapProjection();

    CAF_PDM_InitField( &m_showAxisLines, "ShowAxisLines", true, "Show Axis Lines" );
    CAF_PDM_InitField( &m_showScaleLegend, "ShowScaleLegend", true, "Show Scale Legend" );

    setFaultVisParameters();

    setDefaultCustomName();

    m_contourMapProjectionPartMgr = new RivContourMapProjectionPartMgr( contourMapProjection(), this );

    setCameraPosition( sm_defaultViewMatrix );

    cellResult()->setTernaryEnabled( false );
    cellResult()->legendConfigChanged.connect( this, &RimEclipseContourMapView::onLegendConfigChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapProjection* RimEclipseContourMapView::contourMapProjection() const
{
    return m_contourMapProjection().p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::View3dContent RimEclipseContourMapView::viewContent() const
{
    return ( RiaDefines::View3dContent::ECLIPSE_DATA & RiaDefines::View3dContent::CONTOUR );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapView::createAutoName() const
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
        generatedAutoTags.push_back( cellResult()->resultVariable() );
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
void RimEclipseContourMapView::setDefaultCustomName()
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
void RimEclipseContourMapView::updatePickPointAndRedraw()
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
void RimEclipseContourMapView::initAfterRead()
{
    RimEclipseView::initAfterRead();

    disablePerspectiveProjectionField();
    setShowGridBox( false );
    meshMode.setValue( RiaDefines::MeshModeType::NO_MESH );
    surfaceMode.setValue( FAULTS );
    setFaultVisParameters();
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseContourMapView::isTimeStepDependentDataVisible() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::onCreateDisplayModel()
{
    RimEclipseView::onCreateDisplayModel();

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
void RimEclipseContourMapView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup( "Viewer" );
    viewGroup->add( userDescriptionField() );
    viewGroup->add( backgroundColorField() );
    viewGroup->add( &m_showAxisLines );
    viewGroup->add( &m_showScaleLegend );

    uiOrdering.add( &m_eclipseCase );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Contour Map Name" );
    nameConfig()->uiOrdering( uiConfigName, *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    uiTreeOrdering.add( m_contourMapProjection );
    uiTreeOrdering.add( cellResult() );
    cellResult()->uiCapability()->setUiReadOnly( m_contourMapProjection->isColumnResult() );
    uiTreeOrdering.add( wellCollection() );
    uiTreeOrdering.add( faultCollection() );
    uiTreeOrdering.add( annotationCollection() );
    uiTreeOrdering.add( m_cellFilterCollection() );
    uiTreeOrdering.add( nativePropertyFilterCollection() );

    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::onUpdateDisplayModelForCurrentTimeStep()
{
    static_cast<RimEclipsePropertyFilterCollection*>( nativePropertyFilterCollection() )->updateFromCurrentTimeStep();

    m_contourMapProjection->clearGeometry();
    updateGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::updateGeometry()
{
    caf::ProgressInfo progress( 100, "Generate Contour Map", true );

    updateVisibleGeometries();
    updateVisibleCellColors();

    { // Step 1: generate results and some minor updates. About 30% of the time.
        if ( m_contourMapProjection->isChecked() )
        {
            m_contourMapProjection->generateResultsIfNecessary( m_currentTimeStep() );
        }
        progress.setProgress( 30 );
    }

    onUpdateLegends(); // To make sure the scalar mappers are set up correctly

    { // Step 2: generate geometry. Takes about 60% of the time.
        createContourMapGeometry();
        progress.setProgress( 90 );
    }

    { // Step 3: generate drawables. Takes about 10% of the time.
        appendContourMapProjectionToModel();
        appendContourLinesToModel();
        appendPickPointVisToModel();
        progress.setProgress( 100 );
    }

    appendWellsAndFracturesToModel();

    m_overlayInfoConfig()->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::setFaultVisParameters()
{
    faultCollection()->setShouldApplyCellFiltersToFaults( true );
    faultCollection()->setShowOppositeFaultFaces( true );
    faultCollection()->setFaultResult( RimFaultInViewCollection::FAULT_NO_FACE_CULLING );
    faultResultSettings()->showCustomFaultResult = true;
    faultResultSettings()->customFaultResult()->setResultVariable( "None" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::createContourMapGeometry()
{
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        m_contourMapProjectionPartMgr->createProjectionGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::appendContourMapProjectionToModel()
{
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
        if ( frameScene )
        {
            cvf::String name = "ContourMapProjection";
            RimEclipseContourMapView::removeModelByName( frameScene, name );

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
void RimEclipseContourMapView::appendContourLinesToModel()
{
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
        if ( frameScene )
        {
            cvf::String name = "ContourMapLines";
            RimEclipseContourMapView::removeModelByName( frameScene, name );

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
void RimEclipseContourMapView::appendPickPointVisToModel()
{
    if ( nativeOrOverrideViewer() && m_contourMapProjection->isChecked() )
    {
        cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
        if ( frameScene )
        {
            cvf::String name = "ContourMapPickPoint";
            RimEclipseContourMapView::removeModelByName( frameScene, name );

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
void RimEclipseContourMapView::onUpdateLegends()
{
    if ( nativeOrOverrideViewer() )
    {
        if ( !isUsingOverrideViewer() )
        {
            nativeOrOverrideViewer()->removeAllColorLegends();
        }
        else if ( cellResult() && cellResult()->legendConfig() )
        {
            nativeOrOverrideViewer()->removeColorLegend( cellResult()->legendConfig()->titledOverlayFrame() );
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

        // Hide the scale widget if any 3D views are present, as the display of the scale widget is only working for
        // default rotation. The update is triggered in RimViewLinker::updateScaleWidgetVisibility()

        bool any3DViewsLinked = false;

        if ( auto viewLinker = assosiatedViewLinker() )
        {
            auto views = viewLinker->allViews();
            for ( auto v : views )
            {
                if ( dynamic_cast<RimEclipseContourMapView*>( v ) ) continue;
                any3DViewsLinked = true;
            }
        }

        nativeOrOverrideViewer()->showScaleLegend( any3DViewsLinked ? false : m_showScaleLegend() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::updateViewWidgetAfterCreation()
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
void RimEclipseContourMapView::updateViewFollowingCellFilterUpdates()
{
    m_contourMapProjection->setCheckState( true );
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::onLoadDataAndUpdate()
{
    RimEclipseView::onLoadDataAndUpdate();
    if ( nativeOrOverrideViewer() )
    {
        nativeOrOverrideViewer()->setView( cvf::Vec3d( 0, 0, -1 ), cvf::Vec3d( 0, 1, 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimEclipseView::fieldChangedByUi( changedField, oldValue, newValue );

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
caf::PdmFieldHandle* RimEclipseContourMapView::userDescriptionField()
{
    return nameConfig()->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RivCellSetEnum> RimEclipseContourMapView::allVisibleFaultGeometryTypes() const
{
    std::set<RivCellSetEnum> faultGeoTypes;
    // Normal eclipse views always shows faults for active and visible eclipse cells.
    if ( faultCollection()->isActive() )
    {
        faultGeoTypes = RimEclipseView::allVisibleFaultGeometryTypes();
    }
    return faultGeoTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimEclipseContourMapView::createViewWidget( QWidget* mainWindowParent )
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
void RimEclipseContourMapView::onViewNavigationChanged()
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
bool RimEclipseContourMapView::zoomChangeAboveTreshold( const cvf::Vec3d& currentCameraPosition ) const
{
    double       distance  = std::max( std::fabs( m_cameraPositionLastUpdate.z() ), std::fabs( currentCameraPosition.z() ) );
    const double threshold = 0.05 * distance;
    return std::fabs( m_cameraPositionLastUpdate.z() - currentCameraPosition.z() ) > threshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::scheduleGeometryRegen( RivCellSetEnum geometryType )
{
    if ( geometryType != VISIBLE_WELL_CELLS )
    {
        m_contourMapProjection->clearGeometry();
    }
    RimEclipseView::scheduleGeometryRegen( geometryType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::onLegendConfigChanged( const caf::SignalEmitter* emitter, RimLegendConfigChangeType changeType )
{
    using ChangeType = RimLegendConfigChangeType;
    if ( changeType == ChangeType::LEVELS || changeType == ChangeType::COLOR_MODE || changeType == ChangeType::RANGE ||
         changeType == ChangeType::ALL )
    {
        m_contourMapProjection->clearGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection* RimEclipseContourMapView::surfaceInViewCollection() const
{
    // Surfaces should not be shown in contour map.
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapView::zoomAll()
{
    setCameraPosition( sm_defaultViewMatrix );
    isPerspectiveView = false;

    // If a 3D view has been used as the primary linked view, a contour map can be rotated in 3D. Use the following
    // function to make sure view is reset to original state, with correct rotation and grid box configuration.
    updateViewWidgetAfterCreation();

    RimEclipseView::zoomAll();
}
