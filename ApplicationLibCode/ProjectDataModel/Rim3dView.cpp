/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "Rim3dView.h"

#include "RiaApplication.h"
#include "RiaFieldHandleTools.h"
#include "RiaGuiApplication.h"
#include "RiaOptionItemFactory.h"
#include "RiaPreferences.h"
#include "RiaPreferencesSystem.h"
#include "RiaViewRedrawScheduler.h"

#include "RicfCommandObject.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dWellLogCurve.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimGridView.h"
#include "RimLegendConfig.h"
#include "RimMainPlotCollection.h"
#include "RimMeasurement.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSeismicView.h"
#include "RimTools.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimViewManipulator.h"
#include "RimViewNameConfig.h"
#include "RimWellPathCollection.h"

#include "RivAnnotationsPartMgr.h"
#include "RivMeasurementPartMgr.h"
#include "RivWellPathsPartMgr.h"

#include "RiuMainWindow.h"
#include "RiuTimeStepChangedHandler.h"
#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafFrameAnimationControl.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfColor3.h"
#include "cafPdmUiComboBoxEditor.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfTransform.h"
#include "cvfViewport.h"

#include <climits>

namespace caf
{
template <>
void caf::AppEnum<Rim3dView::SurfaceModeType>::setUp()
{
    addItem( Rim3dView::SURFACE, "SURFACE", "All" );
    addItem( Rim3dView::FAULTS, "FAULTS", "Faults only" );
    addItem( Rim3dView::NO_SURFACE, "NO_SURFACE", "None" );
    setDefault( Rim3dView::SURFACE );
}

} // End namespace caf

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( Rim3dView, "View", "GenericView" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView::Rim3dView()
    : updateAnimations( this )
    , m_isCallingUpdateDisplayModelForCurrentTimestepAndRedraw( false )
    , m_animationIntervalMillisec( 50 )
    , m_animationTimerUsers( 0 )
{
    RiaPreferences* preferences = RiaPreferences::current();
    CVF_ASSERT( preferences );

    CAF_PDM_InitObject( "3d View" );

    CAF_PDM_InitScriptableField( &m_id, "Id", -1, "View ID" );
    m_id.registerKeywordAlias( "ViewId" );
    m_id.uiCapability()->setUiReadOnly( true );
    m_id.uiCapability()->setUiHidden( true );
    m_id.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );
    m_id.xmlCapability()->setCopyable( false );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "" );
    m_nameConfig = new RimViewNameConfig();

    CAF_PDM_InitField( &m_cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "" );
    m_cameraPosition.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_cameraPointOfInterest, "CameraPointOfInterest", cvf::Vec3d::ZERO, "" );
    m_cameraPointOfInterest.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableField( &isPerspectiveView, "PerspectiveProjection", true, "Perspective Projection" );

    double defaultScaleFactor = preferences->defaultScaleFactorZ();
    CAF_PDM_InitScriptableField( &m_scaleZ, "GridZScale", defaultScaleFactor, "Z Scale", "", "Scales the scene in the Z direction", "" );
    m_scaleZ.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitScriptableField( &m_backgroundColor, "BackgroundColor", defBackgColor, "Background" );
    m_backgroundColor.registerKeywordAlias( "ViewBackgroundColor" );

    CAF_PDM_InitField( &maximumFrameRate, "MaximumFrameRate", 10, "Maximum Frame Rate" );
    maximumFrameRate.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableField( &m_currentTimeStep, "CurrentTimeStep", 0, "Current Time Step" );
    m_currentTimeStep.uiCapability()->setUiHidden( true );

    caf::AppEnum<RiaDefines::MeshModeType> defaultMeshType = preferences->defaultMeshModeType();
    CAF_PDM_InitField( &meshMode, "MeshMode", defaultMeshType, "Grid Lines" );
    CAF_PDM_InitFieldNoDefault( &surfaceMode, "SurfaceMode", "Grid Surface" );

    CAF_PDM_InitScriptableField( &m_showGridBox, "ShowGridBox", RiaPreferences::current()->showGridBox(), "Show Grid Box" );

    CAF_PDM_InitScriptableField( &m_disableLighting,
                                 "DisableLighting",
                                 false,
                                 "Disable Results Lighting",
                                 "",
                                 "Disable light model for scalar result colors",
                                 "" );

    CAF_PDM_InitScriptableField( &m_showZScaleLabel, "ShowZScale", true, "Show Z Scale Label" );

    CAF_PDM_InitFieldNoDefault( &m_comparisonView, "ComparisonView", "Comparison View" );

    CAF_PDM_InitFieldNoDefault( &m_fontSize, "FontSize", "Font Size" );

    CAF_PDM_InitFieldNoDefault( &m_annotationStrategy, "AnnotationStrategy", "Annotation Strategy" );
    CAF_PDM_InitField( &m_annotationCountHint, "AnnotationCountHint", 5, "Annotation Count Hint" );
    CAF_PDM_InitField( &m_useCustomAnnotationStrategy,
                       "UseCustomAnnotationStrategy",
                       false,
                       "Use Custom Annotation Strategy",
                       "Specify the strategy to be applied on all screen space annotations." );

    m_seismicVizModel = new cvf::ModelBasicList;
    m_seismicVizModel->setName( "SeismicSectionModel" );

    m_highlightVizModel = new cvf::ModelBasicList;
    m_highlightVizModel->setName( "HighlightModel" );

    m_wellPathPipeVizModel = new cvf::ModelBasicList;
    m_wellPathPipeVizModel->setName( "WellPathPipeModel" );

    m_wellPathsPartManager   = new RivWellPathsPartMgr( this );
    m_annotationsPartManager = new RivAnnotationsPartMgr( this );
    m_measurementPartManager = new RivMeasurementPartMgr( this );

    this->setAs3DViewMdiWindow();

    // Every timer tick, send a signal for updating animations.
    // Any animation is supposed to connect to this signal
    // in order to having only one central animation driver.
    m_animationTimer = std::make_unique<QTimer>( new QTimer() );
    m_animationTimer->setInterval( m_animationIntervalMillisec );
    QObject::connect( m_animationTimer.get(), &QTimer::timeout, [this]() { updateAnimations.send(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView::~Rim3dView()
{
    // When a 3d view is destructed, make sure that all other views using this as a comparison view is reset and
    // redrawn. A crash was seen for test case
    // "\ResInsight-regression-test\ProjectFiles\ProjectFilesSmallTests\TestCase_CoViz-Simple" when a view used as
    // comparison view was deleted.

    if ( auto proj = RimProject::current() )
    {
        for ( auto v : proj->allViews() )
        {
            if ( v->activeComparisonView() == this )
            {
                v->setComparisonView( nullptr );
                v->scheduleCreateDisplayModelAndRedraw();
            }
        }

        if ( this->isMasterView() )
        {
            RimViewLinker* viewLinker = this->assosiatedViewLinker();
            viewLinker->setMasterView( nullptr );

            delete proj->viewLinkerCollection->viewLinker();
            proj->viewLinkerCollection->viewLinker = nullptr;

            proj->uiCapability()->updateConnectedEditors();
        }

        RimViewController* vController = this->viewController();
        if ( vController )
        {
            vController->setManagedView( nullptr );
            vController->ownerViewLinker()->removeViewController( vController );
            delete vController;

            proj->uiCapability()->updateConnectedEditors();
        }
    }

    if ( RiaApplication::instance()->activeReservoirView() == this )
    {
        RiaApplication::instance()->setActiveReservoirView( nullptr );
    }

    if ( m_viewer )
    {
        m_viewer->clearRimView();
    }

    // Make sure the object is disconnected from other objects before delete
    prepareForDelete();

    removeMdiWindowFromMdiArea();

    delete m_viewer;
    m_viewer = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int Rim3dView::id() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::requestAnimationTimer()
{
    m_animationTimerUsers++;
    if ( m_animationTimerUsers == 1 ) m_animationTimer->start();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::releaseAnimationTimer()
{
    m_animationTimerUsers--;
    CAF_ASSERT( m_animationTimerUsers >= 0 );
    if ( m_animationTimerUsers == 0 ) m_animationTimer->stop();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuViewer* Rim3dView::viewer() const
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuViewer* Rim3dView::nativeOrOverrideViewer() const
{
    if ( m_overrideViewer ) return m_overrideViewer;

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setOverrideViewer( RiuViewer* overrideViewer )
{
    m_overrideViewer = overrideViewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isUsingOverrideViewer() const
{
    return m_overrideViewer != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::hideComparisonViewField()
{
    m_comparisonView.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setName( const QString& name )
{
    m_nameConfig->setCustomName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dView::name() const
{
    return m_nameConfig->customName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dView::autoName() const
{
    return m_nameConfig->name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f Rim3dView::backgroundColor() const
{
    return m_backgroundColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* Rim3dView::createViewWidget( QWidget* mainWindowParent )
{
    // If parent widget is a live widget, the application will get OpenGL window issues if started on a non-primary
    // screen. Using nullptr as parent solves the issue.
    // https://github.com/OPM/ResInsight/issues/8192
    //
    m_viewer = new RiuViewer( nullptr );
    m_viewer->setOwnerReservoirView( this );

    cvf::String xLabel;
    cvf::String yLabel;
    cvf::String zLabel;

    this->defineAxisLabels( &xLabel, &yLabel, &zLabel );
    m_viewer->setAxisLabels( xLabel, yLabel, zLabel );

    updateZScaleLabel();
    return m_viewer->layoutWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateViewWidgetAfterCreation()
{
    if ( !m_viewer ) return;

    m_viewer->setDefaultPerspectiveNearPlaneDistance( 10 );

    onResetLegendsInViewer();

    m_viewer->updateNavigationPolicy();
    m_viewer->enablePerfInfoHud( RiaPreferencesSystem::current()->show3dInformation() );

    m_viewer->mainCamera()->setViewMatrix( m_cameraPosition );
    m_viewer->setPointOfInterest( m_cameraPointOfInterest() );
    m_viewer->enableParallelProjection( !isPerspectiveView() );

    m_viewer->mainCamera()->viewport()->setClearColor( cvf::Color4f( backgroundColor() ) );

    updateGridBoxData();
    updateAnnotationItems();
    createHighlightAndGridBoxDisplayModel();

    m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setId( int id )
{
    m_id                  = id;
    QString viewIdTooltip = QString( "View id: %1" ).arg( m_id );
    this->setUiToolTip( viewIdTooltip );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::assignIdIfNecessary()
{
    if ( m_id == -1 )
    {
        RimProject::current()->assignViewIdToView( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateMdiWindowTitle()
{
    if ( m_viewer )
    {
        auto title = autoName();

        if ( isMasterView() && assosiatedViewLinker() && assosiatedViewLinker()->isActive() )
        {
            title += " (Primary)";
        }
        else if ( viewController() && viewController()->isActive() )
        {
            title += " (Controlled)";
        }

        m_viewer->layoutWidget()->setWindowTitle( title );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker* Rim3dView::assosiatedViewLinker() const
{
    RimViewLinker* viewLinker = this->viewLinkerIfMasterView();
    if ( !viewLinker )
    {
        RimViewController* viewController = this->viewController();
        if ( viewController )
        {
            viewLinker = viewController->ownerViewLinker();
        }
    }

    return viewLinker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewController* Rim3dView::viewController() const
{
    std::vector<RimViewController*> objects = objectsWithReferringPtrFieldsOfType<RimViewController>();

    for ( auto v : objects )
    {
        if ( v )
        {
            return v;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::deleteViewWidget()
{
    // Earlier implementations has used m_viewer->deleteLater(). This caused issues triggered by 3D editors and
    // interaction with the event processing. deleteLater() will not be handeled by processEvents() if we are in the
    // state of processing UI events, ie in the process of handling a QAction

    delete m_viewer;
    m_viewer = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroupWithKeyword( "Viewer", "ViewGroup" );

    viewGroup->add( &m_fontSize );
    viewGroup->add( &m_backgroundColor );
    viewGroup->add( &m_showZScaleLabel );
    viewGroup->add( &m_showGridBox );
    viewGroup->add( &isPerspectiveView );
    viewGroup->add( &m_disableLighting );
    viewGroup->add( &m_comparisonView );

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup( "Grid Appearance" );
    gridGroup->add( &m_scaleZ );
    m_scaleZ.uiCapability()->setUiReadOnly( !this->isScaleZEditable() );
    gridGroup->add( &meshMode );
    gridGroup->add( &surfaceMode );

    caf::PdmUiGroup* annotationGroup = uiOrdering.addNewGroup( "Annotations" );
    annotationGroup->add( &m_useCustomAnnotationStrategy );
    annotationGroup->add( &m_annotationStrategy );
    annotationGroup->add( &m_annotationCountHint );
    m_annotationStrategy.uiCapability()->setUiReadOnly( !m_useCustomAnnotationStrategy );
    m_annotationCountHint.uiCapability()->setUiReadOnly(
        !m_useCustomAnnotationStrategy || ( m_annotationStrategy() != RivAnnotationTools::LabelPositionStrategy::COUNT_HINT ) );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage Rim3dView::snapshotWindowContent()
{
    if ( m_viewer )
    {
        return m_viewer->snapshotImage();
    }

    return QImage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::scheduleCreateDisplayModelAndRedraw()
{
    RiaViewRedrawScheduler::instance()->scheduleDisplayModelUpdateAndRedraw( this );
    if ( this->isMasterView() )
    {
        RimViewLinker* viewLinker = this->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->scheduleCreateDisplayModelAndRedrawForDependentViews();
        }
    }

    // Update  views using this as comparison
    std::set<Rim3dView*> containingViews = viewsUsingThisAsComparisonView();

    for ( auto view : containingViews )
    {
        RiaViewRedrawScheduler::instance()->scheduleDisplayModelUpdateAndRedraw( view );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<Rim3dView*> Rim3dView::viewsUsingThisAsComparisonView()
{
    std::set<Rim3dView*>              containingViews;
    std::vector<caf::PdmFieldHandle*> fieldsReferringToMe = referringPtrFields();
    for ( caf::PdmFieldHandle* field : fieldsReferringToMe )
    {
        if ( field->keyword() == m_comparisonView.keyword() )
        {
            Rim3dView* containingView = dynamic_cast<Rim3dView*>( field->ownerObject() );
            if ( containingView && containingView->activeComparisonView() == this )
            {
                containingViews.insert( containingView );
            }
        }
    }

    return containingViews;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> Rim3dView::validComparisonViews() const
{
    auto isIntersectionView = []( const Rim3dView* view ) { return dynamic_cast<const Rim2dIntersectionView*>( view ) != nullptr; };

    std::vector<Rim3dView*> validComparisonViews;
    for ( auto view : RimProject::current()->allViews() )
    {
        if ( dynamic_cast<RimSeismicView*>( view ) ) continue;

        bool isSameViewType = isIntersectionView( this ) == isIntersectionView( view );

        if ( view != this && isSameViewType )
        {
            validComparisonViews.push_back( view );
        }
    }

    return validComparisonViews;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isScaleZEditable()
{
    return ( this->viewsUsingThisAsComparisonView().empty() || ( this->viewController() && this->viewController()->isCameraLinked() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setComparisonView( Rim3dView* compView )
{
    m_comparisonView = compView;
    m_comparisonView.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isTimeStepDependentDataVisibleInThisOrComparisonView() const
{
    Rim3dView* otherView = activeComparisonView();
    if ( !otherView && isUsingOverrideViewer() )
    {
        otherView = dynamic_cast<Rim3dView*>( nativeOrOverrideViewer()->ownerReservoirView() );
    }

    return ( isTimeStepDependentDataVisible() || ( otherView && otherView->isTimeStepDependentDataVisible() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t Rim3dView::timeStepCount()
{
    return this->onTimeStepCountRequested();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dView::timeStepName( int frameIdx ) const
{
    if ( this->ownerCase() )
    {
        return this->ownerCase()->timeStepName( frameIdx );
    }
    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setCurrentTimeStep( int frameIndex )
{
    const int oldTimeStep = m_currentTimeStep;

    m_currentTimeStep = frameIndex;
    onClampCurrentTimestep();

    if ( m_currentTimeStep != oldTimeStep )
    {
        RiuTimeStepChangedHandler::instance()->handleTimeStepChanged( this );
        this->onClearReservoirCellVisibilitiesIfNecessary();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setCurrentTimeStepAndUpdate( int frameIndex )
{
    setCurrentTimeStep( frameIndex );
    updateDisplayModelForCurrentTimeStepAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateDisplayModelForCurrentTimeStepAndRedraw()
{
    if ( m_isCallingUpdateDisplayModelForCurrentTimestepAndRedraw ) return;

    if ( nativeOrOverrideViewer() )
    {
        this->onUpdateDisplayModelForCurrentTimeStep();
        appendAnnotationsToModel();
        appendMeasurementToModel();

        if ( Rim3dView* depView = prepareComparisonView() )
        {
            depView->onUpdateDisplayModelForCurrentTimeStep();
            depView->appendAnnotationsToModel();
            depView->appendMeasurementToModel();

            restoreComparisonView();
        }

        updateScreenSpaceModel();

        nativeOrOverrideViewer()->update();
    }

    m_isCallingUpdateDisplayModelForCurrentTimestepAndRedraw = true;

    std::set<Rim3dView*> containerViews = this->viewsUsingThisAsComparisonView();
    if ( !containerViews.empty() && !isUsingOverrideViewer() )
    {
        for ( auto view : containerViews )
        {
            view->updateDisplayModelForCurrentTimeStepAndRedraw();
        }
    }

    m_isCallingUpdateDisplayModelForCurrentTimestepAndRedraw = false;

    RimMainPlotCollection::current()->updateCurrentTimeStepInPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::createDisplayModelAndRedraw()
{
    if ( nativeOrOverrideViewer() )
    {
        this->onClampCurrentTimestep();

        onUpdateScaleTransform();

        onCreateDisplayModel();
        createHighlightAndGridBoxDisplayModel();
        updateDisplayModelVisibility();

        if ( m_cameraPosition().isIdentity() )
        {
            setDefaultView();
            m_cameraPosition        = nativeOrOverrideViewer()->mainCamera()->viewMatrix();
            m_cameraPointOfInterest = nativeOrOverrideViewer()->pointOfInterest();
        }

        if ( Rim3dView* depView = prepareComparisonView() )
        {
            depView->createDisplayModelAndRedraw();

            if ( isTimeStepDependentDataVisibleInThisOrComparisonView() )
            {
                // To make the override viewer see the new frame (skeletons) created by createDisplayModelAndRedraw
                // But avoid any call back down to this Rim3dView, instead do the update manually to not confuse the
                // m_currentTimeStep
                nativeOrOverrideViewer()->caf::Viewer::slotSetCurrentFrame( currentTimeStep() );
                depView->updateDisplayModelForCurrentTimeStepAndRedraw();
            }

            restoreComparisonView();
        }
        else if ( !isUsingOverrideViewer() && viewer() )
        {
            // Remove the comparison scene data when
            // we do not have a comparison view
            // and are not doing override generation
            viewer()->setMainScene( nullptr, true );
            viewer()->removeAllFrames( true );
        }

        updateScreenSpaceModel();
    }

    if ( RiuMainWindow::instance() )
    {
        RiuMainWindow::instance()->refreshAnimationActions();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::removeModelByName( cvf::Scene* scene, const cvf::String& modelName )
{
    std::vector<cvf::Model*> modelsToBeRemoved;
    for ( cvf::uint i = 0; i < scene->modelCount(); i++ )
    {
        if ( scene->model( i )->name() == modelName )
        {
            modelsToBeRemoved.push_back( scene->model( i ) );
        }
    }

    for ( size_t i = 0; i < modelsToBeRemoved.size(); i++ )
    {
        scene->removeModel( modelsToBeRemoved[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setDefaultView()
{
    if ( m_viewer )
    {
        m_viewer->setDefaultView( -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::endAnimation()
{
    this->onUpdateStaticCellColors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* Rim3dView::implementingPdmObject()
{
    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* Rim3dView::wellPathCollection() const
{
    return RimTools::wellPathCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dView::hasVisibleTimeStepDependent3dWellLogCurves() const
{
    if ( wellPathCollection() )
    {
        std::vector<Rim3dWellLogCurve*> wellLogCurves = wellPathCollection()->descendantsIncludingThisOfType<Rim3dWellLogCurve>();
        for ( const Rim3dWellLogCurve* curve : wellLogCurves )
        {
            if ( curve->showInView( this ) && curve->isShowingTimeDependentResult() )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setupBeforeSave()
{
    if ( m_viewer )
    {
        m_cameraPosition        = m_viewer->mainCamera()->viewMatrix();
        m_cameraPointOfInterest = m_viewer->pointOfInterest();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setMeshOnlyDrawstyle()
{
    meshMode.setValueWithFieldChanged( RiaDefines::MeshModeType::FULL_MESH );
    surfaceMode.setValueWithFieldChanged( NO_SURFACE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setMeshSurfDrawstyle()
{
    surfaceMode.setValueWithFieldChanged( SURFACE );
    meshMode.setValueWithFieldChanged( RiaDefines::MeshModeType::FULL_MESH );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setFaultMeshSurfDrawstyle()
{
    surfaceMode.setValueWithFieldChanged( SURFACE );
    meshMode.setValueWithFieldChanged( RiaDefines::MeshModeType::FAULTS_MESH );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setSurfOnlyDrawstyle()
{
    surfaceMode.setValueWithFieldChanged( SURFACE );
    meshMode.setValueWithFieldChanged( RiaDefines::MeshModeType::NO_MESH );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setSurfaceDrawstyle()
{
    if ( surfaceMode() != NO_SURFACE ) surfaceMode.setValueWithFieldChanged( SURFACE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::disableLighting( bool disable )
{
    m_disableLighting = disable;
    updateDisplayModelForCurrentTimeStepAndRedraw();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isLightingDisabled() const
{
    return m_disableLighting();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::onViewNavigationChanged()
{
    updateScreenSpaceModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dView::userDescriptionField()
{
    return m_nameConfig->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dView::backgroundColorField()
{
    return &m_backgroundColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &meshMode )
    {
        onCreateDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
        RiuMainWindow::instance()->refreshAnimationActions();
    }
    else if ( changedField == &isPerspectiveView )
    {
        if ( m_viewer ) m_viewer->enableParallelProjection( !isPerspectiveView() );
    }
    else if ( changedField == &m_scaleZ )
    {
        updateScaling();

        RiuMainWindow::instance()->updateScaleValue();

        RimViewLinker* viewLinker = this->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateScaleZ( this, scaleZ() );
            viewLinker->updateCamera( this );
        }
    }
    else if ( changedField == &surfaceMode )
    {
        onCreateDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
        RiuMainWindow::instance()->refreshAnimationActions();
    }
    else if ( changedField == &m_showGridBox )
    {
        createHighlightAndGridBoxDisplayModelAndRedraw();
    }
    else if ( changedField == &m_disableLighting )
    {
        onCreateDisplayModel();
        RiuMainWindow::instance()->refreshDrawStyleActions();
        RiuMainWindow::instance()->refreshAnimationActions();
    }
    else if ( changedField == m_nameConfig->nameField() )
    {
        updateMdiWindowTitle();

        if ( viewController() )
        {
            viewController()->updateDisplayNameAndIcon();
            viewController()->updateConnectedEditors();
        }
        else
        {
            if ( isMasterView() )
            {
                assosiatedViewLinker()->updateUiNameAndIcon();
                assosiatedViewLinker()->updateConnectedEditors();
            }
        }
    }
    else if ( changedField == &m_currentTimeStep )
    {
        if ( m_viewer )
        {
            m_viewer->update();
        }
    }
    else if ( changedField == &m_backgroundColor || changedField == &m_fontSize )
    {
        if ( changedField == &m_fontSize )
        {
            auto fontHolderChildren = descendantsOfType<caf::FontHolderInterface>();
            for ( auto fontHolder : fontHolderChildren )
            {
                fontHolder->updateFonts();
            }
        }
        this->applyBackgroundColorAndFontChanges();
        this->updateConnectedEditors();
    }
    else if ( changedField == &maximumFrameRate )
    {
        // !! Use cvf::UNDEFINED_INT or something if we end up with frame rate 0?
        // !! Should be able to specify legal range for number properties
        if ( m_viewer )
        {
            m_viewer->animationControl()->setTimeout( maximumFrameRate != 0 ? 1000 / maximumFrameRate : std::numeric_limits<int>::max() );
        }
    }
    else if ( changedField == &m_showZScaleLabel )
    {
        if ( m_viewer )
        {
            m_viewer->showZScaleLabel( m_showZScaleLabel() );
            m_viewer->update();
        }
    }
    else if ( changedField == &m_comparisonView )
    {
        createDisplayModelAndRedraw();
    }
    else if ( changedField == &m_annotationCountHint || changedField == &m_annotationStrategy || changedField == &m_useCustomAnnotationStrategy )
    {
        if ( m_viewer )
        {
            updateScreenSpaceModel();
            m_viewer->update();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::addWellPathsToModel( cvf::ModelBasicList*    wellPathModelBasicList,
                                     const cvf::BoundingBox& wellPathClipBoundingBox,
                                     double                  characteristicCellSize )
{
    cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

    m_wellPathsPartManager->appendStaticGeometryPartsToModel( wellPathModelBasicList,
                                                              transForm.p(),
                                                              characteristicCellSize,
                                                              wellPathClipBoundingBox );

    wellPathModelBasicList->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::addDynamicWellPathsToModel( cvf::ModelBasicList*    wellPathModelBasicList,
                                            const cvf::BoundingBox& wellPathClipBoundingBox,
                                            double                  characteristicCellSize )
{
    cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();

    size_t timeStepIndex = currentTimeStep();
    m_wellPathsPartManager->appendDynamicGeometryPartsToModel( wellPathModelBasicList,
                                                               timeStepIndex,
                                                               transForm.p(),
                                                               characteristicCellSize,
                                                               wellPathClipBoundingBox );

    wellPathModelBasicList->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::addAnnotationsToModel( cvf::ModelBasicList* annotationsModel )
{
    auto annotationCollections = descendantsIncludingThisOfType<RimAnnotationInViewCollection>();

    m_annotationsPartManager->clearGeometryCache();

    if ( !annotationCollections.empty() && annotationCollections.front()->isActive() )
    {
        cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();
        m_annotationsPartManager->appendGeometryPartsToModel( annotationsModel, transForm.p(), domainBoundingBox() );
    }

    annotationsModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::addMeasurementToModel( cvf::ModelBasicList* measureModel )
{
    if ( !this->ownerCase() ) return;

    RimMeasurement* measurement = RimProject::current()->measurement();

    if ( !measurement || measurement->pointsInDomainCoords().empty() )
    {
        m_measurementPartManager->clearGeometryCache();
    }
    else
    {
        cvf::ref<caf::DisplayCoordTransform> transForm = displayCoordTransform();
        cvf::Camera* mainOrComparisonCamera            = isUsingOverrideViewer() ? nativeOrOverrideViewer()->comparisonMainCamera()
                                                                                 : nativeOrOverrideViewer()->mainCamera();
        m_measurementPartManager->appendGeometryPartsToModel( mainOrComparisonCamera,
                                                              measureModel,
                                                              transForm.p(),
                                                              ownerCase()->allCellsBoundingBox() );
    }

    measureModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//---------------------------------------------------- ----------------------------------------------
bool Rim3dView::isMasterView() const
{
    RimViewLinker* viewLinker = this->assosiatedViewLinker();
    return viewLinker && this == viewLinker->masterView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox Rim3dView::domainBoundingBox()
{
    cvf::BoundingBox combinedDomainBBox;

    if ( viewer() && ownerCase() )
    {
        cvf::BoundingBox masterDomainBBox = isShowingActiveCellsOnly() ? ownerCase()->activeCellsBoundingBox()
                                                                       : ownerCase()->allCellsBoundingBox();
        combinedDomainBBox.add( masterDomainBBox );

        if ( Rim3dView* depView = activeComparisonView() )
        {
            viewer()->setComparisonViewEyePointOffset( RimViewManipulator::calculateEquivalentCamPosOffset( this, depView ) );

            RimCase* destinationOwnerCase = depView->ownerCase();

            if ( destinationOwnerCase )
            {
                cvf::BoundingBox depDomainBBox = depView->isShowingActiveCellsOnly() ? destinationOwnerCase->activeCellsBoundingBox()
                                                                                     : destinationOwnerCase->allCellsBoundingBox();

                combinedDomainBBox.add( depDomainBBox );
            }
        }
    }

    return combinedDomainBBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateGridBoxData()
{
    if ( viewer() && ownerCase() )
    {
        viewer()->updateGridBoxData( m_scaleZ(), ownerCase()->displayModelOffset(), backgroundColor(), domainBoundingBox(), fontSize() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateAnnotationItems()
{
    if ( nativeOrOverrideViewer() )
    {
        nativeOrOverrideViewer()->updateAnnotationItems();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::resetLegends()
{
    onResetLegendsInViewer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setScaleZAndUpdate( double scalingFactor )
{
    if ( scaleZ() != scalingFactor )
    {
        this->m_scaleZ.setValueWithFieldChanged( scalingFactor );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setScaleZ( double scalingFactor )
{
    m_scaleZ = scalingFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim3dView::scaleZ() const
{
    return m_scaleZ();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateScaling()
{
    if ( viewer() )
    {
        cvf::Vec3d poi = viewer()->pointOfInterest();
        cvf::Vec3d eye, dir, up;
        eye = viewer()->mainCamera()->position();
        dir = viewer()->mainCamera()->direction();
        up  = viewer()->mainCamera()->up();

        eye[2] = poi[2] * m_scaleZ() / this->scaleTransform()->worldTransform()( 2, 2 ) + ( eye[2] - poi[2] );
        poi[2] = poi[2] * m_scaleZ() / this->scaleTransform()->worldTransform()( 2, 2 );

        viewer()->mainCamera()->setFromLookAt( eye, eye + dir, up );
        viewer()->setPointOfInterest( poi );
    }

    if ( activeComparisonView() )
    {
        activeComparisonView()->setScaleZAndUpdate( m_scaleZ );
    }

    onUpdateScaleTransform();
    updateGridBoxData();
    updateZScaleLabel();

    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateZScaleLabel()
{
    if ( viewer() ) viewer()->setZScale( m_scaleZ() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::createMeasurementDisplayModelAndRedraw()
{
    appendMeasurementToModel();

    if ( Rim3dView* depView = prepareComparisonView() )
    {
        depView->appendMeasurementToModel();
        restoreComparisonView();
    }

    if ( nativeOrOverrideViewer() )
    {
        nativeOrOverrideViewer()->update();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::createHighlightAndGridBoxDisplayModelAndRedraw()
{
    createHighlightAndGridBoxDisplayModel();

    if ( Rim3dView* depView = prepareComparisonView() )
    {
        depView->createHighlightAndGridBoxDisplayModel();
        restoreComparisonView();
    }

    if ( nativeOrOverrideViewer() )
    {
        nativeOrOverrideViewer()->update();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::createHighlightAndGridBoxDisplayModel()
{
    if ( !nativeOrOverrideViewer() ) return;

    nativeOrOverrideViewer()->removeStaticModel( m_highlightVizModel.p() );

    m_highlightVizModel->removeAllParts();

    cvf::Collection<cvf::Part> parts;
    onCreatePartCollectionFromSelection( &parts );
    if ( !parts.empty() )
    {
        for ( size_t i = 0; i < parts.size(); i++ )
        {
            m_highlightVizModel->addPart( parts[i].p() );
        }

        m_highlightVizModel->updateBoundingBoxesRecursive();
        nativeOrOverrideViewer()->addStaticModelOnce( m_highlightVizModel.p(), isUsingOverrideViewer() );
    }

    updateGridBoxData();

    if ( viewer() ) viewer()->showGridBox( m_showGridBox() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setBackgroundColor( const cvf::Color3f& newBackgroundColor )
{
    m_backgroundColor = newBackgroundColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setShowGridBox( bool showGridBox )
{
    m_showGridBox = showGridBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::applyBackgroundColorAndFontChanges()
{
    if ( viewer() != nullptr )
    {
        viewer()->mainCamera()->viewport()->setClearColor( cvf::Color4f( backgroundColor() ) );
        viewer()->updateFonts( fontSize() );
    }
    updateGridBoxData();
    updateAnnotationItems();
    updateConnectedEditors();

    onUpdateLegends();
    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int Rim3dView::fontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultSceneFontSize(), m_fontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateFonts()
{
    applyBackgroundColorAndFontChanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::performAutoNameUpdate()
{
    updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateDisplayModelVisibility()
{
    if ( viewer() == nullptr || isUsingOverrideViewer() ) return;

    const cvf::uint uintSurfaceBit               = surfaceBit;
    const cvf::uint uintMeshSurfaceBit           = meshSurfaceBit;
    const cvf::uint uintFaultBit                 = faultBit;
    const cvf::uint uintMeshFaultBit             = meshFaultBit;
    const cvf::uint uintIntersectionCellFaceBit  = intersectionCellFaceBit;
    const cvf::uint uintIntersectionCellMeshBit  = intersectionCellMeshBit;
    const cvf::uint uintIntersectionFaultMeshBit = intersectionFaultMeshBit;

    // Initialize the mask to show everything except the the bits controlled here
    unsigned int mask = 0xffffffff & ~uintSurfaceBit & ~uintFaultBit & ~uintMeshSurfaceBit & ~uintMeshFaultBit &
                        ~uintIntersectionCellFaceBit & ~uintIntersectionCellMeshBit & ~uintIntersectionFaultMeshBit;

    // Then turn the appropriate bits on according to the user settings

    if ( surfaceMode == SURFACE )
    {
        mask |= uintSurfaceBit;
        mask |= uintFaultBit;
        mask |= intersectionCellFaceBit;
    }
    else if ( surfaceMode == FAULTS )
    {
        mask |= uintFaultBit;
        mask |= intersectionCellFaceBit;
    }

    if ( meshMode == RiaDefines::MeshModeType::FULL_MESH )
    {
        mask |= uintMeshSurfaceBit;
        mask |= uintMeshFaultBit;
        mask |= intersectionCellMeshBit;
        mask |= intersectionFaultMeshBit;
    }
    else if ( meshMode == RiaDefines::MeshModeType::FAULTS_MESH )
    {
        mask |= uintMeshFaultBit;
        mask |= intersectionFaultMeshBit;
    }

    viewer()->setEnableMask( mask, false );
    viewer()->setEnableMask( mask, true );

    this->onUpdateDisplayModelVisibility();

    viewer()->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dView::isShowingActiveCellsOnly()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::zoomAll()
{
    if ( m_viewer )
    {
        m_viewer->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<caf::DisplayCoordTransform> Rim3dView::displayCoordTransform() const
{
    cvf::ref<caf::DisplayCoordTransform> coordTrans = new caf::DisplayCoordTransform;

    cvf::Vec3d scale( 1.0, 1.0, m_scaleZ );
    coordTrans->setScale( scale );

    RimCase* rimCase = ownerCase();
    if ( rimCase )
    {
        coordTrans->setTranslation( rimCase->displayModelOffset() );
    }

    return coordTrans;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim3dView::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_comparisonView )
    {
        std::vector<Rim3dView*> views = validComparisonViews();
        for ( auto view : views )
        {
            RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName( view, &options );
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_fontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultSceneFontSize() );
    }
    else if ( fieldNeedingOptions == &m_scaleZ )
    {
        for ( auto scale : RiaDefines::viewScaleOptions() )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( scale ), scale ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Mat4d Rim3dView::cameraPosition() const
{
    return m_cameraPosition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d Rim3dView::cameraPointOfInterest() const
{
    return m_cameraPointOfInterest();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewNameConfig* Rim3dView::nameConfig() const
{
    return m_nameConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* Rim3dView::viewWidget()
{
    if ( m_viewer ) return m_viewer->layoutWidget();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setCameraPosition( const cvf::Mat4d& cameraPosition )
{
    m_cameraPosition = cameraPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setCameraPointOfInterest( const cvf::Vec3d& cameraPointOfInterest )
{
    m_cameraPointOfInterest = cameraPointOfInterest;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::forceShowWindowOn()
{
    m_showWindow.setValueWithFieldChanged( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int Rim3dView::currentTimeStep() const
{
    return m_currentTimeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::disableGridBoxField()
{
    m_showGridBox = false;

    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_showGridBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::disablePerspectiveProjectionField()
{
    isPerspectiveView = false;

    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &isPerspectiveView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::handleMdiWindowClosed()
{
    RimViewWindow::handleMdiWindowClosed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::setMdiWindowGeometry( const RimMdiWindowGeometry& windowGeometry )
{
    RimViewWindow::setMdiWindowGeometry( windowGeometry );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::appendAnnotationsToModel()
{
    if ( !nativeOrOverrideViewer() ) return;

    cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
    if ( frameScene )
    {
        cvf::String name = "Annotations";
        this->removeModelByName( frameScene, name );

        cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;
        model->setName( name );

        addAnnotationsToModel( model.p() );

        frameScene->addModel( model.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateScreenSpaceModel()
{
    if ( !m_viewer || !m_viewer->mainCamera() ) return;

    if ( m_screenSpaceModel.isNull() )
    {
        m_screenSpaceModel = new cvf::ModelBasicList;
        m_screenSpaceModel->setName( "ScreenSpaceModel" );
    }
    m_screenSpaceModel->removeAllParts();

    // Build annotation parts and put into screen space model
    cvf::Collection<cvf::Part> partCollection;
    if ( m_viewer->currentScene() ) m_viewer->currentScene()->allParts( &partCollection );

    RivAnnotationTools annoTool;
    if ( m_useCustomAnnotationStrategy )
    {
        annoTool.setOverrideLabelPositionStrategy( m_annotationStrategy() );
        annoTool.setCountHint( m_annotationCountHint() );
    }

    // The scaling factor is computed using the camera, and this does not work for the flat intersection view
    bool computeScalingFactor = ( viewContent() != RiaDefines::View3dContent::FLAT_INTERSECTION );
    annoTool.addAnnotationLabels( partCollection, m_viewer->mainCamera(), m_screenSpaceModel.p(), computeScalingFactor );

    nativeOrOverrideViewer()->addStaticModelOnce( m_screenSpaceModel.p(), isUsingOverrideViewer() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::appendMeasurementToModel()
{
    if ( !nativeOrOverrideViewer() ) return;

    const cvf::String name = "Measurement";

    cvf::Scene* scene = nativeOrOverrideViewer()->currentScene( isUsingOverrideViewer() );
    if ( scene )
    {
        Rim3dView::removeModelByName( scene, name );

        cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;
        model->setName( name );

        addMeasurementToModel( model.p() );

        scene->addModel( model.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* Rim3dView::activeComparisonView() const
{
    return m_comparisonView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* Rim3dView::prepareComparisonView()
{
    Rim3dView* depView = activeComparisonView();

    if ( !depView )
    {
        return nullptr;
    }

    if ( isUsingOverrideViewer() )
    {
        return nullptr;
    }

    if ( depView->scaleZ() != scaleZ() )
    {
        depView->setScaleZAndUpdate( scaleZ() );
    }

    viewer()->setComparisonViewEyePointOffset( RimViewManipulator::calculateEquivalentCamPosOffset( this, depView ) );

    depView->setOverrideViewer( viewer() );

    return depView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::restoreComparisonView()
{
    Rim3dView* depView = activeComparisonView();
    CVF_ASSERT( depView );

    depView->setOverrideViewer( nullptr );
    viewer()->setCurrentComparisonFrame( depView->currentTimeStep() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker* Rim3dView::viewLinkerIfMasterView() const
{
    std::vector<RimViewLinker*> objects = objectsWithReferringPtrFieldsOfType<RimViewLinker>();

    for ( auto viewLinker : objects )
    {
        if ( viewLinker )
        {
            return viewLinker;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::onResetLegendsInViewer()
{
    for ( auto legendConfig : legendConfigs() )
    {
        if ( legendConfig ) legendConfig->recreateLegend();
    }

    auto viewer = nativeOrOverrideViewer();
    if ( viewer ) viewer->removeAllColorLegends();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::onUpdateScaleTransform()
{
    if ( scaleTransform() )
    {
        cvf::Mat4d scale = cvf::Mat4d::IDENTITY;
        scale( 2, 2 )    = scaleZ();

        scaleTransform()->setLocalTransform( scale );

        if ( nativeOrOverrideViewer() ) nativeOrOverrideViewer()->updateCachedValuesInScene();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::updateViewTreeItems( RiaDefines::ItemIn3dView itemType )
{
    // default is to do nothing
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim3dView::characteristicCellSize() const
{
    if ( ownerCase() )
    {
        return ownerCase()->characteristicCellSize();
    }

    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection* Rim3dView::annotationCollection() const
{
    return m_annotationCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dView::synchronizeLocalAnnotationsFromGlobal()
{
    RimProject* proj = RimProject::current();
    if ( proj && proj->activeOilField() )
    {
        RimAnnotationCollection* annotColl = proj->activeOilField()->annotationCollection();
        if ( annotColl && annotationCollection() )
        {
            annotationCollection()->onGlobalCollectionChanged( annotColl );
        }
    }
}
