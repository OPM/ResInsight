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

#include "RimGeoMechView.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaRegressionTestRunner.h"

#include "RicfCommandObject.h"

#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"

#include "Polygons/RimPolygonInViewCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechFaultReactivationResult.h"
#include "RimGeoMechPart.h"
#include "RimGeoMechPartCollection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGridCollection.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"
#include "RimSurfaceInViewCollection.h"
#include "RimTensorResults.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewLinker.h"
#include "RimViewNameConfig.h"
#include "RimWellMeasurementInView.h"
#include "RimWellMeasurementInViewCollection.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RiuViewer.h"

#include "RivGeoMechPartMgr.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivGeoMechVizLogic.h"
#include "RivSingleCellPartGenerator.h"
#include "RivTensorResultPartMgr.h"

#include "cafCadNavigation.h"
#include "cafCeetronPlusNavigation.h"
#include "cafDisplayCoordTransform.h"
#include "cafFrameAnimationControl.h"
#include "cafOverlayScalarMapperLegend.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfTransform.h"
#include "cvfViewport.h"
#include "cvfqtUtils.h"

CAF_PDM_SOURCE_INIT( RimGeoMechView, "GeoMechView" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView::RimGeoMechView()
    : m_currentInternalTimeStep( 0 )
    , m_currentDataFrameIndex( -1 )
{
    CAF_PDM_InitScriptableObject( "Geomechanical View", ":/3DViewGeoMech16x16.png", "", "The Geomechanical 3d View" );

    CAF_PDM_InitFieldNoDefault( &cellResult, "GridCellResult", "Color Result", ":/CellResult.png" );
    cellResult = new RimGeoMechCellColors();

    CAF_PDM_InitFieldNoDefault( &m_tensorResults, "TensorResults", "Tensor Results" );
    m_tensorResults = new RimTensorResults();

    CAF_PDM_InitFieldNoDefault( &m_faultReactivationResult, "FaultReactivationResult", "Fault Reactivation Result" );
    m_faultReactivationResult = new RimGeoMechFaultReactivationResult();

    CAF_PDM_InitFieldNoDefault( &m_propertyFilterCollection, "PropertyFilters", "Property Filters" );
    m_propertyFilterCollection = new RimGeoMechPropertyFilterCollection();

    CAF_PDM_InitFieldNoDefault( &m_partsCollection, "Parts", "Parts" );
    m_partsCollection = new RimGeoMechPartCollection();

    CAF_PDM_InitField( &m_showDisplacement, "ShowDisplacement", false, "Show Displacement" );
    CAF_PDM_InitField( &m_displacementScaling, "DisplacementScaling", 1.0, "Scaling Factor" );

    m_scaleTransform = new cvf::Transform();
    m_vizLogic       = new RivGeoMechVizLogic( this );
    m_tensorPartMgr  = new RivTensorResultPartMgr( this );

    nameConfig()->setCustomName( "GeoMech View" );
    nameConfig()->hideCaseNameField( false );
    nameConfig()->hideAggregationTypeField( true );
    nameConfig()->hidePropertyField( false );
    nameConfig()->hideSampleSpacingField( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView::~RimGeoMechView()
{
    m_geomechCase = nullptr;

    delete m_tensorResults;
    delete cellResult;
    delete m_propertyFilterCollection;
    delete m_faultReactivationResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::View3dContent RimGeoMechView::viewContent() const
{
    return RiaDefines::View3dContent::GEOMECH_DATA;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onLoadDataAndUpdate()
{
    caf::ProgressInfo progress( 7, "" );
    progress.setNextProgressIncrement( 5 );

    onUpdateScaleTransform();

    updateViewTreeItems( RiaDefines::ItemIn3dView::ALL );

    if ( m_geomechCase )
    {
        std::string                    errorMessage;
        RimGeoMechCase::CaseOpenStatus status = m_geomechCase->openGeoMechCase( &errorMessage );
        if ( status == RimGeoMechCase::CASE_OPEN_CANCELLED )
        {
            m_geomechCase = nullptr;
            return;
        }
        else if ( status == RimGeoMechCase::CASE_OPEN_ERROR )
        {
            if ( !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
            {
                QString displayMessage = errorMessage.empty() ? "Could not open the Odb file: \n" + m_geomechCase->gridFileName()
                                                              : QString::fromStdString( errorMessage );

                RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "File open error", displayMessage );
            }

            m_geomechCase = nullptr;
            return;
        }
    }
    progress.incrementProgress();

    progress.setProgressDescription( "Reading Current Result" );

    CVF_ASSERT( cellResult() != nullptr );
    m_geomechCase->geoMechData()->femPartResults()->setNormalizationAirGap( cellResult()->normalizationAirGap() );
    m_geomechCase->geoMechData()->femPartResults()->assertResultsLoaded( cellResult()->resultAddress() );
    progress.incrementProgress();
    progress.setProgressDescription( "Create Display model" );

    updateMdiWindowVisibility();

    geoMechPropertyFilterCollection()->loadAndInitializePropertyFilters();
    m_wellMeasurementCollection->syncWithChangesInWellMeasurementCollection();

    if ( m_surfaceCollection ) m_surfaceCollection->loadData();

    if ( m_partsCollection ) m_partsCollection->syncWithCase( m_geomechCase );

    if ( m_faultReactivationResult )
    {
        if ( m_geomechCase->gridFileName().toLower().endsWith( ".odb" ) )
        {
            m_faultReactivationResult->onLoadDataAndUpdate();
        }
    }

    scheduleCreateDisplayModelAndRedraw();

    progress.incrementProgress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechView::createAutoName() const
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

    if ( nameConfig()->addProperty() )
    {
        auto resultName = cellResultResultDefinition()->resultFieldName();
        if ( !resultName.isEmpty() ) generatedAutoTags.push_back( resultName );
    }

    if ( !generatedAutoTags.empty() )
    {
        autoName.push_back( generatedAutoTags.join( ", " ) );
    }
    return autoName.join( ": " );
}

//--------------------------------------------------------------------------------------------------
/// Create display model,
/// or at least empty scenes as frames that is delivered to the viewer
/// The real geometry generation is done inside RivReservoirViewGeometry and friends
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onCreateDisplayModel()
{
    if ( nativeOrOverrideViewer() == nullptr ) return;

    if ( !( m_geomechCase && m_geomechCase->geoMechData() && m_geomechCase->geoMechData()->femParts() ) ) return;

    const auto& theParts  = femParts();
    int         partCount = theParts->partCount();

    if ( partCount <= 0 ) return;

    for ( int i = 0; i < partCount; i++ )
    {
        theParts->part( i )->setEnabled( m_partsCollection()->isPartEnabled( i ) );
    }

    updateElementDisplacements();

    // Remove all existing animation frames from the viewer.
    // The parts are still cached in the RivReservoir geometry and friends

    nativeOrOverrideViewer()->removeAllFrames( isUsingOverrideViewer() );

    if ( isTimeStepDependentDataVisibleInThisOrComparisonView() )
    {
        // Create empty frames in the viewer, one per global timestep
        const int totalSteps = geoMechCase()->geoMechData()->femPartResults()->totalSteps();
        for ( int timeStepIndex = 0; timeStepIndex < totalSteps; timeStepIndex++ )
        {
            cvf::ref<cvf::Scene>          scene      = new cvf::Scene;
            cvf::ref<cvf::ModelBasicList> emptyModel = new cvf::ModelBasicList;
            emptyModel->setName( "EmptyModel" );
            scene->addModel( emptyModel.p() );
            nativeOrOverrideViewer()->addFrame( scene.p(), isUsingOverrideViewer() );
        }
    }

    // Set the Main scene in the viewer. Used when the animation is in "Stopped" state

    cvf::ref<cvf::Scene> mainScene = new cvf::Scene;

    // Grid model
    cvf::ref<cvf::ModelBasicList> mainSceneGridVizModel = new cvf::ModelBasicList;
    mainSceneGridVizModel->setName( "GridModel" );
    m_vizLogic->appendNoAnimPartsToModel( mainSceneGridVizModel.p() );

    mainSceneGridVizModel->updateBoundingBoxesRecursive();
    mainScene->addModel( mainSceneGridVizModel.p() );
    nativeOrOverrideViewer()->setMainScene( mainScene.p(), isUsingOverrideViewer() );

    // Well path model

    cvf::BoundingBox femBBox = femParts()->boundingBox();

    m_wellPathPipeVizModel->removeAllParts();
    addWellPathsToModel( m_wellPathPipeVizModel.p(), femBBox, ownerCase()->characteristicCellSize() );

    nativeOrOverrideViewer()->addStaticModelOnce( m_wellPathPipeVizModel.p(), isUsingOverrideViewer() );

    // Intersections

    appendIntersectionsToModel( cellFilterCollection()->hasActiveFilters(), propertyFilterCollection()->hasActiveDynamicFilters() );

    // Seismic sections

    cvf::ref<caf::DisplayCoordTransform> transform = displayCoordTransform();
    m_seismicVizModel->removeAllParts();
    m_seismicSectionCollection->appendPartsToModel( this, m_seismicVizModel.p(), transform.p(), femBBox );
    nativeOrOverrideViewer()->addStaticModelOnce( m_seismicVizModel.p(), isUsingOverrideViewer() );

    // Polygons
    appendPolygonPartsToModel( transform.p(), ownerCase()->allCellsBoundingBox() );

    // Surfaces

    m_surfaceVizModel->removeAllParts();
    if ( m_surfaceCollection )
    {
        m_surfaceCollection->appendPartsToModel( m_surfaceVizModel.p(), scaleTransform() );
        nativeOrOverrideViewer()->addStaticModelOnce( m_surfaceVizModel.p(), isUsingOverrideViewer() );
    }

    // If the animation was active before recreating everything, make viewer view current frame

    if ( isTimeStepDependentDataVisibleInThisOrComparisonView() )
    {
        if ( viewer() && !isUsingOverrideViewer() ) viewer()->setCurrentFrame( m_currentTimeStep );
    }
    else
    {
        onUpdateLegends();
        m_vizLogic->updateStaticCellColors( -1 );
        m_intersectionCollection->applySingleColorEffect();
        if ( m_surfaceCollection )
        {
            m_surfaceCollection->applySingleColorEffect();
        }

        m_overlayInfoConfig()->update3DInfo();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPropertyFilterCollection* RimGeoMechView::nativePropertyFilterCollection()
{
    return m_propertyFilterCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateElementDisplacements()
{
    auto [reload, rebuild] = m_partsCollection->needsReloadOrRebuildUpdate( m_currentTimeStep, m_showDisplacement, m_displacementScaling );

    if ( !rebuild ) return;

    if ( reload )
    {
        for ( auto part : m_partsCollection->parts() )
        {
            std::string             errmsg;
            std::vector<cvf::Vec3f> displacements;
            m_geomechCase->geoMechData()->readDisplacements( &errmsg,
                                                             part->partId(),
                                                             m_currentInternalTimeStep,
                                                             m_currentDataFrameIndex,
                                                             &displacements );
            part->setDisplacements( displacements );
        }
    }
    // store current settings so that we know if we need to rebuild later if any of them changes
    m_partsCollection->setCurrentDisplacementSettings( m_currentTimeStep, m_showDisplacement, m_displacementScaling );

    // tell geometry generator to regenerate grid
    m_vizLogic->scheduleGeometryRegenOfVisiblePartMgrs( m_currentTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimGeoMechView::viewerStepToTimeStepAndFrameIndex( int viewerTimeStep )
{
    // assuming callers check if the case etc. exists
    return m_geomechCase->geoMechData()->femPartResults()->stepListIndexToTimeStepAndDataFrameIndex( viewerTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onUpdateDisplayModelForCurrentTimeStep()
{
    onUpdateLegends();

    updateElementDisplacements();

    if ( isTimeStepDependentDataVisibleInThisOrComparisonView() )
    {
        if ( nativeOrOverrideViewer() )
        {
            cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
            if ( frameScene )
            {
                {
                    // Grid model
                    cvf::String name = "GridModel";
                    RimGeoMechView::removeModelByName( frameScene, name );

                    cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
                    frameParts->setName( name );
                    m_vizLogic->appendPartsToModel( m_currentTimeStep, frameParts.p() );
                    frameParts->updateBoundingBoxesRecursive();

                    frameScene->addModel( frameParts.p() );
                }

                // Well Paths
                {
                    cvf::String name = "WellPathMod";
                    RimGeoMechView::removeModelByName( frameScene, name );

                    cvf::ref<cvf::ModelBasicList> wellPathModelBasicList = new cvf::ModelBasicList;
                    wellPathModelBasicList->setName( name );

                    cvf::BoundingBox femBBox = femParts()->boundingBox();
                    addDynamicWellPathsToModel( wellPathModelBasicList.p(), femBBox, ownerCase()->characteristicCellSize() );

                    frameScene->addModel( wellPathModelBasicList.p() );
                }

                {
                    // Tensors
                    cvf::String name = "Tensor";
                    RimGeoMechView::removeModelByName( frameScene, name );

                    cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
                    frameParts->setName( name );
                    m_tensorPartMgr->appendDynamicGeometryPartsToModel( frameParts.p(),
                                                                        m_currentTimeStep,
                                                                        m_currentInternalTimeStep,
                                                                        m_currentDataFrameIndex );
                    frameParts->updateBoundingBoxesRecursive();

                    if ( frameParts->partCount() != 0 )
                    {
                        frameScene->addModel( frameParts.p() );
                    }
                }
            }
        }

        bool hasGeneralCellResult = cellResult()->hasResult();

        if ( hasGeneralCellResult )
            m_vizLogic->updateCellResultColor( m_currentTimeStep(), m_currentInternalTimeStep, m_currentDataFrameIndex, cellResult() );
        else
            m_vizLogic->updateStaticCellColors( m_currentTimeStep() );

        // Intersections
        if ( intersectionCollection()->shouldApplyCellFiltersToIntersections() && propertyFilterCollection()->hasActiveDynamicFilters() )
        {
            m_intersectionCollection->clearGeometry();
            appendIntersectionsForCurrentTimeStep();
        }

        m_intersectionCollection->updateCellResultColor( hasGeneralCellResult, m_currentTimeStep );

        if ( m_surfaceCollection )
        {
            m_surfaceCollection->updateCellResultColor( hasGeneralCellResult, m_currentTimeStep );
        }
    }
    else
    {
        m_vizLogic->updateStaticCellColors( -1 );

        m_intersectionCollection->updateCellResultColor( false, m_currentInternalTimeStep );
        if ( m_surfaceCollection ) m_surfaceCollection->updateCellResultColor( false, m_currentInternalTimeStep );

        nativeOrOverrideViewer()->animationControl()->slotPause(); // To avoid animation timer spinning in the background
    }

    m_overlayInfoConfig()->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onUpdateStaticCellColors()
{
    m_vizLogic->updateStaticCellColors( -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::setGeoMechCase( RimGeoMechCase* gmCase )
{
    m_geomechCase = gmCase;
    cellResult()->setGeoMechCase( gmCase );
    cellFilterCollection()->setCase( gmCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onUpdateLegends()
{
    if ( nativeOrOverrideViewer() )
    {
        if ( !isUsingOverrideViewer() )
        {
            nativeOrOverrideViewer()->removeAllColorLegends();
        }
        else
        {
            std::vector<RimLegendConfig*> legendConfs = legendConfigs();

            for ( auto legendConf : legendConfs )
            {
                nativeOrOverrideViewer()->removeColorLegend( legendConf->titledOverlayFrame() );
            }
        }

        updateLegendTextAndRanges( cellResult()->legendConfig(), m_currentTimeStep() );

        if ( cellResult()->hasResult() && cellResult()->legendConfig()->showLegend() )
        {
            nativeOrOverrideViewer()->addColorLegendToBottomLeftCorner( cellResult()->legendConfig->titledOverlayFrame(),
                                                                        isUsingOverrideViewer() );
        }

        for ( RimIntersectionResultDefinition* sepInterResDef : separateIntersectionResultsCollection()->intersectionResultsDefinitions() )
        {
            sepInterResDef->updateLegendRangesTextAndVisibility( "Intersection Results:\n", nativeOrOverrideViewer(), isUsingOverrideViewer() );
        }

        for ( RimIntersectionResultDefinition* sepInterResDef : separateSurfaceResultsCollection()->intersectionResultsDefinitions() )
        {
            sepInterResDef->updateLegendRangesTextAndVisibility( "Surface Results:\n", nativeOrOverrideViewer(), isUsingOverrideViewer() );
        }

        if ( tensorResults()->showTensors() )
        {
            updateTensorLegendTextAndRanges( m_tensorResults->arrowColorLegendConfig(), m_currentTimeStep() );

            if ( tensorResults()->vectorColors() == RimTensorResults::RESULT_COLORS && tensorResults()->arrowColorLegendConfig()->showLegend() )
            {
                nativeOrOverrideViewer()->addColorLegendToBottomLeftCorner( m_tensorResults->arrowColorLegendConfig->titledOverlayFrame(),
                                                                            isUsingOverrideViewer() );
            }
        }

        if ( m_wellMeasurementCollection->isChecked() )
        {
            for ( RimWellMeasurementInView* wellMeasurement : m_wellMeasurementCollection->measurements() )
            {
                if ( wellMeasurement->isChecked() && wellMeasurement->legendConfig()->showLegend() )
                {
                    wellMeasurement->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer(), isUsingOverrideViewer() );
                }
            }
        }

        if ( m_surfaceCollection && m_surfaceCollection->isChecked() )
        {
            m_surfaceCollection->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer(), isUsingOverrideViewer() );
        }

        if ( m_seismicSectionCollection->isChecked() )
        {
            m_seismicSectionCollection->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer(), isUsingOverrideViewer() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateTensorLegendTextAndRanges( RimRegularLegendConfig* legendConfig, int viewerTimeStep )
{
    if ( !m_geomechCase || !m_geomechCase->geoMechData() ) return;

    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
    double globalMin, globalMax;
    double globalPosClosestToZero, globalNegClosestToZero;

    RigGeoMechCaseData* gmCase = m_geomechCase->geoMechData();
    CVF_ASSERT( gmCase );

    RigFemResultPosEnum resPos       = RimTensorResults::resultPositionType();
    QString             resFieldName = tensorResults()->resultFieldName();

    RigFemResultAddress resVarAddress( resPos, resFieldName.toStdString(), "" );

    auto [timeStepIndex, frameIndex] = gmCase->femPartResults()->stepListIndexToTimeStepAndDataFrameIndex( viewerTimeStep );

    gmCase->femPartResults()->minMaxScalarValuesOverAllTensorComponents( resVarAddress, timeStepIndex, frameIndex, &localMin, &localMax );
    gmCase->femPartResults()->posNegClosestToZeroOverAllTensorComponents( resVarAddress,
                                                                          timeStepIndex,
                                                                          frameIndex,
                                                                          &localPosClosestToZero,
                                                                          &localNegClosestToZero );

    gmCase->femPartResults()->minMaxScalarValuesOverAllTensorComponents( resVarAddress, &globalMin, &globalMax );
    gmCase->femPartResults()->posNegClosestToZeroOverAllTensorComponents( resVarAddress, &globalPosClosestToZero, &globalNegClosestToZero );

    legendConfig->setClosestToZeroValues( globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero );
    legendConfig->setAutomaticRanges( globalMin, globalMax, localMin, localMax );

    QString legendTitle = "Tensors:\n" + RimTensorResults::uiFieldName( resFieldName );

    legendConfig->setTitle( legendTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateLegendTextAndRanges( RimRegularLegendConfig* legendConfig, int timeStepIndex )
{
    if ( !isTimeStepDependentDataVisible() )
    {
        return;
    }

    cellResult()->updateLegendTextAndRanges( legendConfig, "Cell Result:\n", timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::ref<RivGeoMechVizLogic> RimGeoMechView::vizLogic() const
{
    return m_vizLogic;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimTensorResults* RimGeoMechView::tensorResults() const
{
    return m_tensorResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTensorResults* RimGeoMechView::tensorResults()
{
    return m_tensorResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimLegendConfig*> RimGeoMechView::legendConfigs() const
{
    std::vector<RimLegendConfig*> absLegendConfigs;

    absLegendConfigs.push_back( cellResult()->legendConfig() );
    absLegendConfigs.push_back( tensorResults()->arrowColorLegendConfig() );

    for ( RimIntersectionResultDefinition* sepInterResDef : separateIntersectionResultsCollection()->intersectionResultsDefinitions() )
    {
        absLegendConfigs.push_back( sepInterResDef->regularLegendConfig() );
    }

    for ( RimIntersectionResultDefinition* sepInterResDef : separateSurfaceResultsCollection()->intersectionResultsDefinitions() )
    {
        absLegendConfigs.push_back( sepInterResDef->regularLegendConfig() );
    }

    for ( RimWellMeasurementInView* wellMeasurement : m_wellMeasurementCollection->measurements() )
    {
        absLegendConfigs.push_back( wellMeasurement->legendConfig() );
    }

    if ( m_surfaceCollection )
    {
        for ( auto legendConfig : m_surfaceCollection->legendConfigs() )
        {
            absLegendConfigs.push_back( legendConfig );
        }
    }

    for ( auto section : seismicSectionCollection()->seismicSections() )
    {
        absLegendConfigs.push_back( section->legendConfig() );
    }

    absLegendConfigs.erase( std::remove( absLegendConfigs.begin(), absLegendConfigs.end(), nullptr ), absLegendConfigs.end() );

    return absLegendConfigs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFemPartCollection* RimGeoMechView::femParts() const
{
    if ( m_geomechCase && m_geomechCase->geoMechData() )
    {
        return m_geomechCase->geoMechData()->femParts();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartCollection* RimGeoMechView::femParts()
{
    if ( m_geomechCase && m_geomechCase->geoMechData() )
    {
        return m_geomechCase->geoMechData()->femParts();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::convertCameraPositionFromOldProjectFiles()
{
    auto geoMechCase = this->geoMechCase();
    if ( geoMechCase )
    {
        // Up-cast to get access to public interface for camera functions
        RimCase*                  rimCase               = geoMechCase;
        RiuViewerToViewInterface* viewerToViewInterface = this;
        cvf::Vec3d                offset                = rimCase->displayModelOffset();
        auto                      diplayCoordTrans      = displayCoordTransform();

        {
            cvf::Mat4d cameraMx = cameraPosition().getInverted();

            cvf::Vec3d translation = cameraMx.translation();

            cvf::Vec3d translationDomainCoord = diplayCoordTrans->scaleToDomainSize( translation );
            translationDomainCoord -= offset;

            cvf::Vec3d newCameraTranslation = diplayCoordTrans->scaleToDisplaySize( translationDomainCoord );

            cameraMx.setTranslation( newCameraTranslation );

            viewerToViewInterface->setCameraPosition( cameraMx.getInverted() );
        }

        {
            cvf::Vec3d pointOfInterest = cameraPointOfInterest();

            cvf::Vec3d pointOfInterestDomain = diplayCoordTrans->scaleToDomainSize( pointOfInterest );
            pointOfInterestDomain -= offset;

            cvf::Vec3d newPointOfInterest = diplayCoordTrans->scaleToDisplaySize( pointOfInterestDomain );

            viewerToViewInterface->setCameraPointOfInterest( newPointOfInterest );
        }

        if ( viewer() )
        {
            viewer()->mainCamera()->setViewMatrix( cameraPosition() );
            viewer()->setPointOfInterest( cameraPointOfInterest() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechView::geoMechCase() const
{
    return m_geomechCase;
}

//--------------------------------------------------------------------------------------------------
/// Clamp the current timestep to actual possibilities
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onClampCurrentTimestep()
{
    int maxSteps = 0;

    if ( m_geomechCase )
    {
        maxSteps = m_geomechCase->geoMechData()->femPartResults()->totalSteps();
    }
    else
    {
        return;
    }
    if ( m_currentTimeStep >= maxSteps ) m_currentTimeStep = maxSteps - 1;
    if ( m_currentTimeStep < 0 ) m_currentTimeStep = 0;

    std::tie( m_currentInternalTimeStep, m_currentDataFrameIndex ) = viewerStepToTimeStepAndFrameIndex( m_currentTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGeoMechView::onTimeStepCountRequested()
{
    if ( m_geomechCase && m_geomechCase->geoMechData() && m_geomechCase->geoMechData()->femPartResults() )
    {
        return m_geomechCase->geoMechData()->femPartResults()->totalSteps();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechView::isTimeStepDependentDataVisible() const
{
    if ( cellResult()->hasResult() || geoMechPropertyFilterCollection()->hasActiveFilters() )
    {
        return true;
    }

    if ( hasVisibleTimeStepDependent3dWellLogCurves() )
    {
        return true;
    }

    if ( intersectionCollection()->hasAnyActiveSeparateResults() )
    {
        return true;
    }

    if ( surfaceInViewCollection() && surfaceInViewCollection()->hasAnyActiveSeparateResults() )
    {
        return true;
    }

    if ( m_wellMeasurementCollection->isChecked() )
    {
        return true;
    }

    if ( ( m_showDisplacement ) || m_partsCollection->isDisplacementsUsed() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Transform* RimGeoMechView::scaleTransform()
{
    return m_scaleTransform.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimGridView::fieldChangedByUi( changedField, oldValue, newValue );

    if ( ( changedField == &m_showDisplacement ) || ( ( changedField == &m_displacementScaling ) && m_showDisplacement() ) )
    {
        createDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::initAfterRead()
{
    RimGridView::initAfterRead();
    cellResult()->setGeoMechCase( m_geomechCase );

    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimGeoMechView::ownerCase() const
{
    return m_geomechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::scheduleGeometryRegen( RivCellSetEnum geometryType )
{
    m_vizLogic->scheduleGeometryRegen( geometryType );

    if ( isMasterView() )
    {
        RimViewLinker* viewLinker = assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->scheduleGeometryRegenForDepViews( geometryType );
        }
    }
    clearReservoirCellVisibilities();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::setOverridePropertyFilterCollection( RimGeoMechPropertyFilterCollection* pfc )
{
    m_overridePropertyFilterCollection = pfc;
    if ( m_overridePropertyFilterCollection )
    {
        m_propertyFilterCollection->isActive = m_overridePropertyFilterCollection->isActive;
    }
    m_propertyFilterCollection.uiCapability()->updateConnectedEditors();

    scheduleGeometryRegen( PROPERTY_FILTERED );
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection* RimGeoMechView::geoMechPropertyFilterCollection()
{
    if ( m_overridePropertyFilterCollection )
    {
        return m_overridePropertyFilterCollection;
    }
    else
    {
        return m_propertyFilterCollection;
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimGeoMechPropertyFilterCollection* RimGeoMechView::geoMechPropertyFilterCollection() const
{
    if ( m_overridePropertyFilterCollection )
    {
        return m_overridePropertyFilterCollection;
    }
    else
    {
        return m_propertyFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int viewerTimeStep )
{
    m_vizLogic->calculateCurrentTotalCellVisibility( totalVisibility, viewerTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateIconStateForFilterCollections()
{
    m_cellFilterCollection()->updateIconState();
    m_cellFilterCollection()->uiCapability()->updateConnectedEditors();

    // NB - notice that it is the filter collection managed by this view that the icon update applies to
    m_propertyFilterCollection()->updateIconState();
    m_propertyFilterCollection()->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel )
{
    CVF_ASSERT( xLabel && yLabel && zLabel );

    *xLabel = "E(x,1)";
    *yLabel = "N(y,2)";
    *zLabel = "Z(3)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechView::isUsingFormationNames() const
{
    if ( cellResult()->hasCategoryResult() ) return true; // Correct for now

    return geoMechPropertyFilterCollection()->isUsingFormationNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimGridView::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "View Name" );
    nameConfig()->uiOrdering( uiConfigName, *nameGroup );

    auto displacementGroup = uiOrdering.addNewGroup( "Displacements" );
    displacementGroup->add( &m_displacementScaling );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    uiTreeOrdering.add( m_gridCollection() );
    if ( m_partsCollection->shouldBeVisibleInTree() ) uiTreeOrdering.add( m_partsCollection() );

    uiTreeOrdering.add( cellResult() );
    uiTreeOrdering.add( m_tensorResults() );
    uiTreeOrdering.add( m_cellFilterCollection() );
    uiTreeOrdering.add( m_propertyFilterCollection() );

    if ( ( m_faultReactivationResult() != nullptr ) && ( m_faultReactivationResult->isValid() ) )
    {
        uiTreeOrdering.add( m_faultReactivationResult() );
    }

    addRequiredUiTreeObjects( uiTreeOrdering );

    uiTreeOrdering.add( m_intersectionCollection() );
    if ( surfaceInViewCollection() ) uiTreeOrdering.add( surfaceInViewCollection() );
    if ( seismicSectionCollection()->shouldBeVisibleInTree() ) uiTreeOrdering.add( seismicSectionCollection() );

    uiTreeOrdering.add( m_polygonInViewCollection );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition* RimGeoMechView::cellResultResultDefinition() const
{
    return cellResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimPropertyFilterCollection* RimGeoMechView::propertyFilterCollection() const
{
    return geoMechPropertyFilterCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimGeoMechPartCollection* RimGeoMechView::partsCollection() const
{
    return m_partsCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimGeoMechView::currentStepAndDataFrame() const
{
    return std::make_pair( m_currentInternalTimeStep, m_currentDataFrameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechView::displacementScaleFactor() const
{
    return m_displacementScaling;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechView::showDisplacements() const
{
    return m_showDisplacement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::setShowDisplacementsAndUpdate( bool show )
{
    m_showDisplacement = show;
    createDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::resetVizLogic()
{
    if ( m_vizLogic.notNull() )
    {
        m_vizLogic->resetPartMgrs();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::calculateCellVisibility( cvf::UByteArray* visibility, std::vector<RivCellSetEnum> geomTypes, int viewerTimeStep )
{
    CAF_ASSERT( m_vizLogic.notNull() );
    m_vizLogic->calculateCellVisibility( visibility, geomTypes, viewerTimeStep );
}
