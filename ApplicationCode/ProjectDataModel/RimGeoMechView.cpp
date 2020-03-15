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

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGridCollection.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimRegularLegendConfig.h"
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

#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimGeoMechView, "GeoMechView" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView::RimGeoMechView( void )
{
    CAF_PDM_InitScriptableObject( "Geomechanical View", ":/3DViewGeoMech16x16.png", "", "The Geomechanical 3d View" );

    CAF_PDM_InitFieldNoDefault( &cellResult, "GridCellResult", "Color Result", ":/CellResult.png", "", "" );
    cellResult = new RimGeoMechCellColors();
    cellResult.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_tensorResults, "TensorResults", "Tensor Results", "", "", "" );
    m_tensorResults = new RimTensorResults();
    m_tensorResults.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_propertyFilterCollection, "PropertyFilters", "Property Filters", "", "", "" );
    m_propertyFilterCollection = new RimGeoMechPropertyFilterCollection();
    m_propertyFilterCollection.uiCapability()->setUiHidden( true );

    m_scaleTransform = new cvf::Transform();
    m_vizLogic       = new RivGeoMechVizLogic( this );
    m_tensorPartMgr  = new RivTensorResultPartMgr( this );

    nameConfig()->setCustomName( "GeoMech View" );
    nameConfig()->hideCaseNameField( false );
    nameConfig()->hideAggregationTypeField( true );
    nameConfig()->hidePropertyField( false );
    nameConfig()->hideSampleSpacingField( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView::~RimGeoMechView( void )
{
    m_geomechCase = nullptr;

    delete m_tensorResults;
    delete cellResult;
    delete m_propertyFilterCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onLoadDataAndUpdate()
{
    caf::ProgressInfo progress( 7, "" );
    progress.setNextProgressIncrement( 5 );

    onUpdateScaleTransform();

    this->updateSurfacesInViewTreeItems();

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
                QString displayMessage = errorMessage.empty()
                                             ? "Could not open the Odb file: \n" + m_geomechCase->gridFileName()
                                             : QString::fromStdString( errorMessage );

                if ( RiaGuiApplication::isRunning() )
                {
                    QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(), "File open error", displayMessage );
                }
                RiaLogging::error( displayMessage );
            }

            m_geomechCase = nullptr;
            return;
        }
    }
    progress.incrementProgress();

    progress.setProgressDescription( "Reading Current Result" );

    CVF_ASSERT( this->cellResult() != nullptr );
    if ( this->hasUserRequestedAnimation() )
    {
        m_geomechCase->geoMechData()->femPartResults()->setNormalizationAirGap( this->cellResult()->normalizationAirGap() );
        m_geomechCase->geoMechData()->femPartResults()->assertResultsLoaded( this->cellResult()->resultAddress() );
    }
    progress.incrementProgress();
    progress.setProgressDescription( "Create Display model" );

    updateMdiWindowVisibility();

    this->geoMechPropertyFilterCollection()->loadAndInitializePropertyFilters();
    m_wellMeasurementCollection->syncWithChangesInWellMeasurementCollection();

    this->scheduleCreateDisplayModelAndRedraw();

    progress.incrementProgress();
}

//--------------------------------------------------------------------------------------------------
///
/// Todo: Work in progress
///
//--------------------------------------------------------------------------------------------------

void RimGeoMechView::onUpdateScaleTransform()
{
    cvf::Mat4d scale = cvf::Mat4d::IDENTITY;
    scale( 2, 2 )    = scaleZ();

    this->scaleTransform()->setLocalTransform( scale );

    if ( nativeOrOverrideViewer() ) nativeOrOverrideViewer()->updateCachedValuesInScene();
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

    RimCase* ownerCase = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( ownerCase );

    if ( nameConfig()->addCaseName() )
    {
        generatedAutoTags.push_back( ownerCase->caseUserDescription() );
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

    int partCount = m_geomechCase->geoMechData()->femParts()->partCount();

    if ( partCount <= 0 ) return;

    // Remove all existing animation frames from the viewer.
    // The parts are still cached in the RivReservoir geometry and friends

    nativeOrOverrideViewer()->removeAllFrames( isUsingOverrideViewer() );

    if ( isTimeStepDependentDataVisibleInThisOrComparisonView() )
    {
        // Create empty frames in the viewer

        int frameCount = geoMechCase()->geoMechData()->femPartResults()->frameCount();
        for ( int frameIndex = 0; frameIndex < frameCount; frameIndex++ )
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
    addWellPathsToModel( m_wellPathPipeVizModel.p(), femBBox );

    nativeOrOverrideViewer()->addStaticModelOnce( m_wellPathPipeVizModel.p(), isUsingOverrideViewer() );

    // Cross sections

    m_intersectionVizModel->removeAllParts();
    m_intersectionCollection->rebuildGeometry();
    m_intersectionCollection->appendPartsToModel( *this, m_intersectionVizModel.p(), scaleTransform() );
    nativeOrOverrideViewer()->addStaticModelOnce( m_intersectionVizModel.p(), isUsingOverrideViewer() );

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
void RimGeoMechView::onUpdateDisplayModelForCurrentTimeStep()
{
    onUpdateLegends();

    if ( this->isTimeStepDependentDataVisibleInThisOrComparisonView() )
    {
        if ( nativeOrOverrideViewer() )
        {
            cvf::Scene* frameScene = nativeOrOverrideViewer()->frame( m_currentTimeStep, isUsingOverrideViewer() );
            if ( frameScene )
            {
                {
                    // Grid model
                    cvf::String name = "GridModel";
                    this->removeModelByName( frameScene, name );

                    cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
                    frameParts->setName( name );
                    m_vizLogic->appendPartsToModel( m_currentTimeStep, frameParts.p() );
                    frameParts->updateBoundingBoxesRecursive();

                    frameScene->addModel( frameParts.p() );
                }

                // Well Paths
                {
                    cvf::String name = "WellPathMod";
                    this->removeModelByName( frameScene, name );

                    cvf::ref<cvf::ModelBasicList> wellPathModelBasicList = new cvf::ModelBasicList;
                    wellPathModelBasicList->setName( name );

                    cvf::BoundingBox femBBox = femParts()->boundingBox();
                    addDynamicWellPathsToModel( wellPathModelBasicList.p(), femBBox );

                    frameScene->addModel( wellPathModelBasicList.p() );
                }

                {
                    // Tensors
                    cvf::String name = "Tensor";
                    this->removeModelByName( frameScene, name );

                    cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
                    frameParts->setName( name );
                    m_tensorPartMgr->appendDynamicGeometryPartsToModel( frameParts.p(), m_currentTimeStep );
                    frameParts->updateBoundingBoxesRecursive();

                    if ( frameParts->partCount() != 0 )
                    {
                        frameScene->addModel( frameParts.p() );
                    }
                }
            }
        }

        if ( this->cellResult()->hasResult() )
            m_vizLogic->updateCellResultColor( m_currentTimeStep(), this->cellResult() );
        else
            m_vizLogic->updateStaticCellColors( m_currentTimeStep() );

        bool hasGeneralCellResult = this->cellResult()->hasResult();

        m_intersectionCollection->updateCellResultColor( hasGeneralCellResult, m_currentTimeStep );
        if ( m_surfaceCollection )
        {
            m_surfaceCollection->updateCellResultColor( hasGeneralCellResult, m_currentTimeStep );
        }
    }
    else
    {
        m_vizLogic->updateStaticCellColors( -1 );

        m_intersectionCollection->updateCellResultColor( false, m_currentTimeStep );
        if ( m_surfaceCollection ) m_surfaceCollection->updateCellResultColor( false, m_currentTimeStep );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::onResetLegendsInViewer()
{
    this->cellResult()->legendConfig->recreateLegend();

    for ( RimIntersectionResultDefinition* sepInterResDef :
          this->separateIntersectionResultsCollection()->intersectionResultsDefinitions() )
    {
        sepInterResDef->regularLegendConfig()->recreateLegend();
        sepInterResDef->ternaryLegendConfig()->recreateLegend();
    }

    for ( RimIntersectionResultDefinition* sepInterResDef :
          this->separateSurfaceResultsCollection()->intersectionResultsDefinitions() )
    {
        sepInterResDef->regularLegendConfig()->recreateLegend();
        sepInterResDef->ternaryLegendConfig()->recreateLegend();
    }

    for ( RimWellMeasurementInView* wellMeasurement : m_wellMeasurementCollection->measurements() )
    {
        wellMeasurement->legendConfig()->recreateLegend();
    }

    nativeOrOverrideViewer()->removeAllColorLegends();
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
            std::vector<RimLegendConfig*> legendConfs = this->legendConfigs();

            for ( auto legendConf : legendConfs )
            {
                nativeOrOverrideViewer()->removeColorLegend( legendConf->titledOverlayFrame() );
            }
        }

        this->updateLegendTextAndRanges( cellResult()->legendConfig(), m_currentTimeStep() );

        if ( cellResult()->hasResult() && cellResult()->legendConfig()->showLegend() )
        {
            nativeOrOverrideViewer()->addColorLegendToBottomLeftCorner( cellResult()->legendConfig->titledOverlayFrame(),
                                                                        isUsingOverrideViewer() );
        }

        for ( RimIntersectionResultDefinition* sepInterResDef :
              this->separateIntersectionResultsCollection()->intersectionResultsDefinitions() )
        {
            sepInterResDef->updateLegendRangesTextAndVisibility( "Intersection Results:\n",
                                                                 nativeOrOverrideViewer(),
                                                                 isUsingOverrideViewer() );
        }

        for ( RimIntersectionResultDefinition* sepInterResDef :
              this->separateSurfaceResultsCollection()->intersectionResultsDefinitions() )
        {
            sepInterResDef->updateLegendRangesTextAndVisibility( "Surface Results:\n",
                                                                 nativeOrOverrideViewer(),
                                                                 isUsingOverrideViewer() );
        }

        if ( tensorResults()->showTensors() )
        {
            updateTensorLegendTextAndRanges( m_tensorResults->arrowColorLegendConfig(), m_currentTimeStep() );

            if ( tensorResults()->vectorColors() == RimTensorResults::RESULT_COLORS &&
                 tensorResults()->arrowColorLegendConfig()->showLegend() )
            {
                nativeOrOverrideViewer()
                    ->addColorLegendToBottomLeftCorner( m_tensorResults->arrowColorLegendConfig->titledOverlayFrame(),
                                                        isUsingOverrideViewer() );
            }
        }

        if ( m_wellMeasurementCollection->isChecked() )
        {
            for ( RimWellMeasurementInView* wellMeasurement : m_wellMeasurementCollection->measurements() )
            {
                if ( wellMeasurement->isChecked() && wellMeasurement->legendConfig()->showLegend() )
                {
                    wellMeasurement->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer(),
                                                                          isUsingOverrideViewer() );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateTensorLegendTextAndRanges( RimRegularLegendConfig* legendConfig, int timeStepIndex )
{
    if ( !m_geomechCase || !m_geomechCase->geoMechData() ) return;

    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
    double globalMin, globalMax;
    double globalPosClosestToZero, globalNegClosestToZero;

    RigGeoMechCaseData* gmCase = m_geomechCase->geoMechData();
    CVF_ASSERT( gmCase );

    RigFemResultPosEnum resPos       = tensorResults()->resultPositionType();
    QString             resFieldName = tensorResults()->resultFieldName();

    RigFemResultAddress resVarAddress( resPos, resFieldName.toStdString(), "" );

    gmCase->femPartResults()->minMaxScalarValuesOverAllTensorComponents( resVarAddress, timeStepIndex, &localMin, &localMax );
    gmCase->femPartResults()->posNegClosestToZeroOverAllTensorComponents( resVarAddress,
                                                                          timeStepIndex,
                                                                          &localPosClosestToZero,
                                                                          &localNegClosestToZero );

    gmCase->femPartResults()->minMaxScalarValuesOverAllTensorComponents( resVarAddress, &globalMin, &globalMax );
    gmCase->femPartResults()->posNegClosestToZeroOverAllTensorComponents( resVarAddress,
                                                                          &globalPosClosestToZero,
                                                                          &globalNegClosestToZero );

    legendConfig->setClosestToZeroValues( globalPosClosestToZero,
                                          globalNegClosestToZero,
                                          localPosClosestToZero,
                                          localNegClosestToZero );
    legendConfig->setAutomaticRanges( globalMin, globalMax, localMin, localMax );

    QString legendTitle = "Tensors:\n" + RimTensorResults::uiFieldName( resFieldName );

    legendConfig->setTitle( legendTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateLegendTextAndRanges( RimRegularLegendConfig* legendConfig, int timeStepIndex )
{
    if ( !this->isTimeStepDependentDataVisible() )
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

    for ( RimIntersectionResultDefinition* sepInterResDef :
          this->separateIntersectionResultsCollection()->intersectionResultsDefinitions() )
    {
        absLegendConfigs.push_back( sepInterResDef->regularLegendConfig() );
    }

    for ( RimIntersectionResultDefinition* sepInterResDef :
          this->separateSurfaceResultsCollection()->intersectionResultsDefinitions() )
    {
        absLegendConfigs.push_back( sepInterResDef->regularLegendConfig() );
    }

    for ( RimWellMeasurementInView* wellMeasurement : m_wellMeasurementCollection->measurements() )
    {
        absLegendConfigs.push_back( wellMeasurement->legendConfig() );
    }

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
        auto                      diplayCoordTrans      = this->displayCoordTransform();

        {
            cvf::Mat4d cameraMx = this->cameraPosition().getInverted();

            cvf::Vec3d translation = cameraMx.translation();

            cvf::Vec3d translationDomainCoord = diplayCoordTrans->scaleToDomainSize( translation );
            translationDomainCoord -= offset;

            cvf::Vec3d newCameraTranslation = diplayCoordTrans->scaleToDisplaySize( translationDomainCoord );

            cameraMx.setTranslation( newCameraTranslation );

            viewerToViewInterface->setCameraPosition( cameraMx.getInverted() );
        }

        {
            cvf::Vec3d pointOfInterest = this->cameraPointOfInterest();

            cvf::Vec3d pointOfInterestDomain = diplayCoordTrans->scaleToDomainSize( pointOfInterest );
            pointOfInterestDomain -= offset;

            cvf::Vec3d newPointOfInterest = diplayCoordTrans->scaleToDisplaySize( pointOfInterestDomain );

            viewerToViewInterface->setCameraPointOfInterest( newPointOfInterest );
        }

        if ( viewer() )
        {
            viewer()->mainCamera()->setViewMatrix( this->cameraPosition() );
            viewer()->setPointOfInterest( this->cameraPointOfInterest() );
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
    int maxFrameCount = 0;

    if ( m_geomechCase )
    {
        maxFrameCount = m_geomechCase->geoMechData()->femPartResults()->frameCount();
    }

    if ( m_currentTimeStep >= maxFrameCount ) m_currentTimeStep = maxFrameCount - 1;
    if ( m_currentTimeStep < 0 ) m_currentTimeStep = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGeoMechView::onTimeStepCountRequested()
{
    if ( m_geomechCase )
    {
        return m_geomechCase->geoMechData()->femPartResults()->frameCount();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechView::isTimeStepDependentDataVisible() const
{
    if ( this->hasUserRequestedAnimation() &&
         ( this->cellResult()->hasResult() || this->geoMechPropertyFilterCollection()->hasActiveFilters() ) )
    {
        return true;
    }

    if ( this->hasVisibleTimeStepDependent3dWellLogCurves() )
    {
        return true;
    }

    if ( this->intersectionCollection()->hasAnyActiveSeparateResults() )
    {
        return true;
    }

    if ( this->surfaceInViewCollection() && this->surfaceInViewCollection()->hasAnyActiveSeparateResults() )
    {
        return true;
    }

    if ( m_wellMeasurementCollection->isChecked() )
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
void RimGeoMechView::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    RimGridView::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::initAfterRead()
{
    RimGridView::initAfterRead();
    this->cellResult()->setGeoMechCase( m_geomechCase );

    this->updateUiIconFromToggleField();
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

    if ( this->isMasterView() )
    {
        RimViewLinker* viewLinker = this->assosiatedViewLinker();
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

    this->scheduleGeometryRegen( PROPERTY_FILTERED );
    this->scheduleCreateDisplayModelAndRedraw();
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
void RimGeoMechView::calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int timeStep )
{
    m_vizLogic->calculateCurrentTotalCellVisibility( totalVisibility, timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateIconStateForFilterCollections()
{
    m_rangeFilterCollection()->updateIconState();
    m_rangeFilterCollection()->uiCapability()->updateConnectedEditors();

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    uiTreeOrdering.add( m_gridCollection() );

    uiTreeOrdering.add( cellResult() );
    uiTreeOrdering.add( m_tensorResults() );
    uiTreeOrdering.add( &m_intersectionResultDefCollection );
    uiTreeOrdering.add( &m_surfaceResultDefCollection );
    uiTreeOrdering.add( &m_wellMeasurementCollection );

    uiTreeOrdering.add( m_intersectionCollection() );
    if ( surfaceInViewCollection() ) uiTreeOrdering.add( surfaceInViewCollection() );

    uiTreeOrdering.add( m_rangeFilterCollection() );
    uiTreeOrdering.add( m_propertyFilterCollection() );

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
