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
#include "RiaPreferences.h"

#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCrossSectionCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGridCollection.h"
#include "RimLegendConfig.h"
#include "RimViewLinker.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "RivGeoMechPartMgr.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivGeoMechVizLogic.h"
#include "RivSingleCellPartGenerator.h"

#include "cafCadNavigation.h"
#include "cafCeetronPlusNavigation.h"
#include "cafFrameAnimationControl.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfModelBasicList.h"
#include "cvfOverlayScalarMapperLegend.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfViewport.h"
#include "cvfqtUtils.h"

#include <QMessageBox>


CAF_PDM_SOURCE_INIT(RimGeoMechView, "GeoMechView");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::RimGeoMechView(void)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);

    CAF_PDM_InitObject("Geomechanical View", ":/ReservoirView.png", "", "");

    CAF_PDM_InitFieldNoDefault(&cellResult, "GridCellResult", "Color Result", ":/CellResult.png", "", "");
    cellResult = new RimGeoMechCellColors();
    cellResult.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_propertyFilterCollection, "PropertyFilters", "Property Filters", "", "", "");
    m_propertyFilterCollection = new RimGeoMechPropertyFilterCollection();
    m_propertyFilterCollection.uiCapability()->setUiHidden(true);

    //this->cellResult()->setReservoirView(this);
    this->cellResult()->legendConfig()->setReservoirView(this);

    m_scaleTransform = new cvf::Transform();
    m_vizLogic = new RivGeoMechVizLogic(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::~RimGeoMechView(void)
{
    m_geomechCase = NULL;

    delete cellResult;
    delete m_propertyFilterCollection;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateViewerWidgetWindowTitle()
{
    if (m_viewer)
    {
        QString windowTitle;
        if (m_geomechCase.notNull())
        {
            windowTitle = QString("%1 - %2").arg(m_geomechCase->caseUserDescription()).arg(name);
        }
        else
        {
            windowTitle = name;
        }

        m_viewer->layoutWidget()->setWindowTitle(windowTitle);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::loadDataAndUpdate()
{
    caf::ProgressInfo progress(7, "");
    progress.setNextProgressIncrement(5);
    updateScaleTransform();

    if (m_geomechCase)
    {
        std::string errorMessage;
        if (!m_geomechCase->openGeoMechCase(&errorMessage))
        {
            QString displayMessage = errorMessage.empty() ? "Could not open the Odb file: \n" + m_geomechCase->caseFileName() : QString::fromStdString(errorMessage);

            QMessageBox::warning(RiuMainWindow::instance(), 
                            "File open error", 
                            displayMessage);
            m_geomechCase = NULL;
            return;
        }
    }
    progress.incrementProgress();

    progress.setProgressDescription("Reading Current Result");

    CVF_ASSERT(this->cellResult() != NULL);
    if (this->hasUserRequestedAnimation())
    {
        m_geomechCase->geoMechData()->femPartResults()->assertResultsLoaded(this->cellResult()->resultAddress());
    }
    progress.incrementProgress();
    progress.setProgressDescription("Create Display model");
   
    updateViewerWidget();

    this->geoMechPropertyFilterCollection()->loadAndInitializePropertyFilters();

    this->scheduleCreateDisplayModelAndRedraw();

    progress.incrementProgress();
}

//--------------------------------------------------------------------------------------------------
/// 
/// Todo: Work in progress
/// 
//--------------------------------------------------------------------------------------------------

void RimGeoMechView::updateScaleTransform()
{
    cvf::Mat4d scale = cvf::Mat4d::IDENTITY;
    scale(2, 2) = scaleZ();

    this->scaleTransform()->setLocalTransform(scale);

    if (m_viewer) m_viewer->updateCachedValuesInScene();
}

//--------------------------------------------------------------------------------------------------
/// Create display model,
/// or at least empty scenes as frames that is delivered to the viewer
/// The real geometry generation is done inside RivReservoirViewGeometry and friends
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::createDisplayModel()
{
   if (m_viewer.isNull()) return;

   if (!(m_geomechCase 
          && m_geomechCase->geoMechData() 
          && m_geomechCase->geoMechData()->femParts())) 
        return;

   int partCount = m_geomechCase->geoMechData()->femParts()->partCount();

   if (partCount <= 0) return;

   // Remove all existing animation frames from the viewer. 
   // The parts are still cached in the RivReservoir geometry and friends

   m_viewer->removeAllFrames();

   if (isTimeStepDependentDataVisible())
   {
       // Create empty frames in the viewer 

       int frameCount = geoMechCase()->geoMechData()->femPartResults()->frameCount();
       for (int frameIndex = 0; frameIndex < frameCount; frameIndex++)
       {
           cvf::ref<cvf::Scene> scene = new cvf::Scene;
           scene->addModel(new cvf::ModelBasicList);
           m_viewer->addFrame(scene.p());
       }
   }

   // Set the Main scene in the viewer. Used when the animation is in "Stopped" state

   cvf::ref<cvf::Scene> mainScene = new cvf::Scene;
   m_viewer->setMainScene(mainScene.p());

   // Grid model
   cvf::ref<cvf::ModelBasicList> mainSceneGridVizModel =  new cvf::ModelBasicList;
   mainSceneGridVizModel->setName("GridModel");
   m_vizLogic->appendNoAnimPartsToModel(mainSceneGridVizModel.p());
   mainSceneGridVizModel->updateBoundingBoxesRecursive();
   mainScene->addModel(mainSceneGridVizModel.p());

   // Well path model

   double characteristicCellSize = geoMechCase()->geoMechData()->femParts()->characteristicElementSize();
   cvf::BoundingBox femBBox = geoMechCase()->geoMechData()->femParts()->boundingBox();

   m_wellPathPipeVizModel->removeAllParts();
   addWellPathsToModel(m_wellPathPipeVizModel.p(),
                       cvf::Vec3d(0, 0, 0),
                       characteristicCellSize,
                       femBBox,
                       scaleTransform());

   m_viewer->addStaticModelOnce(m_wellPathPipeVizModel.p());


   // Cross sections

   m_crossSectionVizModel->removeAllParts();
   crossSectionCollection->appendPartsToModel(m_crossSectionVizModel.p(), scaleTransform());
   m_viewer->addStaticModelOnce(m_crossSectionVizModel.p());

   // If the animation was active before recreating everything, make viewer view current frame

   if (isTimeStepDependentDataVisible())
   {
        m_viewer->animationControl()->setCurrentFrameOnly(m_currentTimeStep);
        m_viewer->setCurrentFrame(m_currentTimeStep);
   }
   else
   {
       updateLegends();
       m_vizLogic->updateStaticCellColors(-1);
       m_overlayInfoConfig()->update3DInfo();
   }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateCurrentTimeStep()
{
    updateLegends();

    if (this->isTimeStepDependentDataVisible())
    {
        if (m_viewer)
        {
            cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
            if (frameScene)
            {
                // Grid model
                cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
                frameParts->setName("GridModel");
                m_vizLogic->appendPartsToModel(m_currentTimeStep, frameParts.p());
                frameParts->updateBoundingBoxesRecursive();

                this->removeModelByName(frameScene, frameParts->name());
                frameScene->addModel(frameParts.p());
            }
        }

        if (this->cellResult()->hasResult())
            m_vizLogic->updateCellResultColor(m_currentTimeStep(), this->cellResult());
        else
            m_vizLogic->updateStaticCellColors(m_currentTimeStep());

        if (this->cellResult()->hasResult())
            crossSectionCollection->updateCellResultColor(m_currentTimeStep);
        else
            crossSectionCollection->applySingleColorEffect();

    }
    else
    {
        m_vizLogic->updateStaticCellColors(-1);
        m_viewer->animationControl()->slotPause(); // To avoid animation timer spinning in the background
    }

    m_overlayInfoConfig()->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateStaticCellColors()
{
    m_vizLogic->updateStaticCellColors(-1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateDisplayModelVisibility()
{
    if (m_viewer.isNull()) return;

    const cvf::uint uintSurfaceBit      = surfaceBit;
    const cvf::uint uintMeshSurfaceBit  = meshSurfaceBit;
    const cvf::uint uintFaultBit        = faultBit;
    const cvf::uint uintMeshFaultBit    = meshFaultBit;
 
    // Initialize the mask to show everything except the the bits controlled here
    unsigned int mask = 0xffffffff & ~uintSurfaceBit & ~uintFaultBit & ~uintMeshSurfaceBit & ~uintMeshFaultBit ;

    // Then turn the appropriate bits on according to the user settings

    if (surfaceMode == SURFACE)
    {
         mask |= uintSurfaceBit;
         mask |= uintFaultBit;
    }
    else if (surfaceMode == FAULTS)
    {
        mask |= uintFaultBit;
    }

    if (meshMode == FULL_MESH)
    {
        mask |= uintMeshSurfaceBit;
        mask |= uintMeshFaultBit;
    }
    else if (meshMode == FAULTS_MESH)
    {
        mask |= uintMeshFaultBit;
    }

    m_viewer->setEnableMask(mask);
    m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::setGeoMechCase(RimGeoMechCase* gmCase)
{
    m_geomechCase = gmCase;
    cellResult()->setGeoMechCase(gmCase);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::resetLegendsInViewer()
{
    this->cellResult()->legendConfig->recreateLegend();

    m_viewer->removeAllColorLegends();
    m_viewer->addColorLegendToBottomLeftCorner(this->cellResult()->legendConfig->legend());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateLegends()
{
    if (m_viewer)
    {
        m_viewer->removeAllColorLegends();
    }

    if (!m_geomechCase || !m_viewer || !m_geomechCase->geoMechData()
        || !this->isTimeStepDependentDataVisible() 
        || !(cellResult()->resultAddress().isValid()) )
    {
        return;
    }

    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
    double globalMin, globalMax;
    double globalPosClosestToZero, globalNegClosestToZero;

    RigGeoMechCaseData* gmCase = m_geomechCase->geoMechData();
    CVF_ASSERT(gmCase);

    RigFemResultAddress resVarAddress = cellResult()->resultAddress();

    gmCase->femPartResults()->minMaxScalarValues(resVarAddress, m_currentTimeStep, &localMin, &localMax);
    gmCase->femPartResults()->posNegClosestToZero(resVarAddress, m_currentTimeStep, &localPosClosestToZero, &localNegClosestToZero);

    gmCase->femPartResults()->minMaxScalarValues(resVarAddress, &globalMin, &globalMax);
    gmCase->femPartResults()->posNegClosestToZero(resVarAddress, &globalPosClosestToZero, &globalNegClosestToZero);


    cellResult()->legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
    cellResult()->legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

    m_viewer->addColorLegendToBottomLeftCorner(cellResult()->legendConfig->legend());

    cvf::String legendTitle = cvfqt::Utils::toString(
        caf::AppEnum<RigFemResultPosEnum>(cellResult->resultPositionType()).uiText() + "\n"
        + cellResult->resultFieldUiName());

    if (!cellResult->resultComponentUiName().isEmpty())
    {
        legendTitle += ", " + cvfqt::Utils::toString(cellResult->resultComponentUiName());
    }

    if (cellResult->resultFieldName() == "SE" || cellResult->resultFieldName() == "ST" || cellResult->resultFieldName() == "POR-Bar")
    {
        legendTitle += " [Bar]";
    }

    cellResult()->legendConfig->legend()->setTitle(legendTitle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechView::geoMechCase()
{
    return m_geomechCase;
}

//--------------------------------------------------------------------------------------------------
/// Clamp the current timestep to actual possibilities
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::clampCurrentTimestep()
{
    int maxFrameCount = 0;
    
    if (m_geomechCase){
        maxFrameCount = m_geomechCase->geoMechData()->femPartResults()->frameCount();
    }

    if (m_currentTimeStep >= maxFrameCount ) m_currentTimeStep = maxFrameCount -1;
    if (m_currentTimeStep < 0 ) m_currentTimeStep = 0; 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechView::isTimeStepDependentDataVisible()
{
    return this->hasUserRequestedAnimation() && (this->cellResult()->hasResult() || this->geoMechPropertyFilterCollection()->hasActiveFilters());
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
void RimGeoMechView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimView::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &showWindow)
    {
        if (showWindow)
        {
            bool generateDisplayModel = (viewer() == NULL);
            updateViewerWidget();

            if (generateDisplayModel)
            {
                scheduleCreateDisplayModelAndRedraw();
            }
        }
        else
        {
            if (m_viewer)
            {
                this->setMdiWindowGeometry( RiuMainWindow::instance()->windowGeometryForViewer(m_viewer->layoutWidget()));

                RiuMainWindow::instance()->removeViewer(m_viewer->layoutWidget());
                delete m_viewer;
                m_viewer = NULL;
            }
        }

        this->updateUiIconFromToggleField();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::initAfterRead()
{
    this->cellResult()->setGeoMechCase(m_geomechCase);

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* RimGeoMechView::ownerCase()
{
    return m_geomechCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::scheduleGeometryRegen(RivCellSetEnum geometryType)
{
    m_vizLogic->scheduleGeometryRegen(geometryType);

    if (this->isMasterView())
    {
        RimViewLinker* viewLinker = this->assosiatedViewLinker();
        if (viewLinker)
        {
            viewLinker->scheduleGeometryRegenForDepViews(geometryType);
        }
    }
    m_currentReservoirCellVisibility = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::setOverridePropertyFilterCollection(RimGeoMechPropertyFilterCollection* pfc)
{
    m_overridePropertyFilterCollection = pfc;
    
    this->scheduleGeometryRegen(PROPERTY_FILTERED);
    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection* RimGeoMechView::geoMechPropertyFilterCollection()
{
    if (m_overridePropertyFilterCollection)
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
    if (m_overridePropertyFilterCollection)
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
void RimGeoMechView::calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility)
{
    m_vizLogic->calculateCurrentTotalCellVisibility(totalVisibility, m_currentTimeStep);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts)
{
    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    std::vector<RiuSelectionItem*> items;
    riuSelManager->selectedItems(items);
    for (size_t i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT)
        {
            RiuGeoMechSelectionItem* geomSelItem = static_cast<RiuGeoMechSelectionItem*>(items[i]);
            if (geomSelItem &&
                geomSelItem->m_view == this &&
                geomSelItem->m_view->geoMechCase())
            {
                RivSingleCellPartGenerator partGen(geomSelItem->m_view->geoMechCase(), geomSelItem->m_gridIndex, geomSelItem->m_cellIndex);
                cvf::ref<cvf::Part> part = partGen.createPart(geomSelItem->m_color);
                part->setTransform(this->scaleTransform());

                parts->push_back(part.p());
            }
        }
    }
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
void RimGeoMechView::axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel)
{
    CVF_ASSERT(xLabel && yLabel && zLabel);

    *xLabel = "E(x,1)";
    *yLabel = "N(y,2)";
    *zLabel = "Z(3)";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(m_overlayInfoConfig());
    uiTreeOrdering.add(m_gridCollection());

    uiTreeOrdering.add(cellResult());

    uiTreeOrdering.add(crossSectionCollection());
    
    uiTreeOrdering.add(m_rangeFilterCollection());
    uiTreeOrdering.add(m_propertyFilterCollection());
    
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimPropertyFilterCollection* RimGeoMechView::propertyFilterCollection() const
{
    return geoMechPropertyFilterCollection();
}

