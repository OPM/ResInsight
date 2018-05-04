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
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGridCollection.h"
#include "RimIntersectionCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimTensorResults.h"
#include "RimViewLinker.h"

#include "Riu3DMainWindowTools.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "RivGeoMechPartMgr.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivGeoMechVizLogic.h"
#include "RivSingleCellPartGenerator.h"
#include "RivTensorResultPartMgr.h"

#include "cafCadNavigation.h"
#include "cafCeetronPlusNavigation.h"
#include "cafFrameAnimationControl.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"
#include "cafOverlayScalarMapperLegend.h"

#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfTransform.h"
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

    CAF_PDM_InitObject("Geomechanical View", ":/3DView16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&cellResult, "GridCellResult", "Color Result", ":/CellResult.png", "", "");
    cellResult = new RimGeoMechCellColors();
    cellResult.uiCapability()->setUiHidden(true);


    CAF_PDM_InitFieldNoDefault(&m_tensorResults, "TensorResults", "Tensor Results", "", "", "");
    m_tensorResults = new RimTensorResults();
    m_tensorResults.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_propertyFilterCollection, "PropertyFilters", "Property Filters", "", "", "");
    m_propertyFilterCollection = new RimGeoMechPropertyFilterCollection();
    m_propertyFilterCollection.uiCapability()->setUiHidden(true);

    m_scaleTransform = new cvf::Transform();
    m_vizLogic = new RivGeoMechVizLogic(this);
    m_tensorPartMgr = new RivTensorResultPartMgr(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView::~RimGeoMechView(void)
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
    caf::ProgressInfo progress(7, "");
    progress.setNextProgressIncrement(5);
    updateScaleTransform();

    if (m_geomechCase)
    {
        std::string errorMessage;
        if (!m_geomechCase->openGeoMechCase(&errorMessage))
        {
            QString displayMessage = errorMessage.empty() ? "Could not open the Odb file: \n" + m_geomechCase->caseFileName() : QString::fromStdString(errorMessage);

            QMessageBox::warning(Riu3DMainWindowTools::mainWindowWidget(), 
                            "File open error", 
                            displayMessage);
            m_geomechCase = nullptr;
            return;
        }
    }
    progress.incrementProgress();

    progress.setProgressDescription("Reading Current Result");

    CVF_ASSERT(this->cellResult() != nullptr);
    if (this->hasUserRequestedAnimation())
    {
        m_geomechCase->geoMechData()->femPartResults()->assertResultsLoaded(this->cellResult()->resultAddress());
    }
    progress.incrementProgress();
    progress.setProgressDescription("Create Display model");
   
    updateMdiWindowVisibility();

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

   cvf::BoundingBox femBBox = femParts()->boundingBox();

   m_wellPathPipeVizModel->removeAllParts();
   addWellPathsToModel(m_wellPathPipeVizModel.p(), femBBox);

   m_viewer->addStaticModelOnce(m_wellPathPipeVizModel.p());


   // Cross sections

   m_crossSectionVizModel->removeAllParts();
   m_crossSectionCollection->appendPartsToModel(*this, m_crossSectionVizModel.p(), scaleTransform());
   m_viewer->addStaticModelOnce(m_crossSectionVizModel.p());

   // If the animation was active before recreating everything, make viewer view current frame

   if (isTimeStepDependentDataVisible())
   {
        m_viewer->setCurrentFrame(m_currentTimeStep);
   }
   else
   {
       updateLegends();
       m_vizLogic->updateStaticCellColors(-1);
       m_crossSectionCollection->applySingleColorEffect();

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
                {
                    // Grid model
                    cvf::String name = "GridModel";
                    this->removeModelByName(frameScene, name);

                    cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
                    frameParts->setName(name);
                    m_vizLogic->appendPartsToModel(m_currentTimeStep, frameParts.p());
                    frameParts->updateBoundingBoxesRecursive();

                    frameScene->addModel(frameParts.p());
                }

                // Well Paths
                {
                    cvf::String name = "WellPathMod";
                    this->removeModelByName(frameScene, name);

                    cvf::ref<cvf::ModelBasicList> wellPathModelBasicList = new cvf::ModelBasicList;
                    wellPathModelBasicList->setName(name);

                    cvf::BoundingBox femBBox = femParts()->boundingBox();
                    addDynamicWellPathsToModel(wellPathModelBasicList.p(), femBBox);

                    frameScene->addModel(wellPathModelBasicList.p());
                }

                {
                    // Tensors
                    cvf::String name = "Tensor";
                    this->removeModelByName(frameScene, name);

                    cvf::ref<cvf::ModelBasicList> frameParts = new cvf::ModelBasicList;
                    frameParts->setName(name);
                    m_tensorPartMgr->appendDynamicGeometryPartsToModel(frameParts.p(), m_currentTimeStep);
                    frameParts->updateBoundingBoxesRecursive();

                    if (frameParts->partCount() != 0)
                    {
                        frameScene->addModel(frameParts.p());
                    }
                }
            }
        }

        if (this->cellResult()->hasResult())
            m_vizLogic->updateCellResultColor(m_currentTimeStep(), this->cellResult());
        else
            m_vizLogic->updateStaticCellColors(m_currentTimeStep());

        if (this->cellResult()->hasResult())
        {
            m_crossSectionCollection->updateCellResultColor(m_currentTimeStep, 
                                                            this->cellResult()->legendConfig()->scalarMapper(), 
                                                            nullptr);
        }
        else
        {
            m_crossSectionCollection->applySingleColorEffect();
        }

    }
    else
    {
        m_vizLogic->updateStaticCellColors(-1);
        m_crossSectionCollection->applySingleColorEffect();

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
    m_viewer->addColorLegendToBottomLeftCorner(this->cellResult()->legendConfig->titledOverlayFrame());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateLegends()
{
    if ( m_viewer )
    {
        m_viewer->removeAllColorLegends();

        this->updateLegendTextAndRanges(cellResult()->legendConfig(), m_currentTimeStep());

        if (cellResult()->hasResult() && cellResult()->legendConfig()->showLegend())
        {
            m_viewer->addColorLegendToBottomLeftCorner(cellResult()->legendConfig->titledOverlayFrame());
        }

        if (tensorResults()->showTensors())
        {
            updateTensorLegendTextAndRanges(m_tensorResults->arrowColorLegendConfig(), m_currentTimeStep());

            if (tensorResults()->vectorColors() == RimTensorResults::RESULT_COLORS &&
                tensorResults()->arrowColorLegendConfig()->showLegend())
            {
                m_viewer->addColorLegendToBottomLeftCorner(m_tensorResults->arrowColorLegendConfig->titledOverlayFrame());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateTensorLegendTextAndRanges(RimRegularLegendConfig* legendConfig, int timeStepIndex)
{
    if (!m_geomechCase || !m_geomechCase->geoMechData()) return;

    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
    double globalMin, globalMax;
    double globalPosClosestToZero, globalNegClosestToZero;

    RigGeoMechCaseData* gmCase = m_geomechCase->geoMechData();
    CVF_ASSERT(gmCase);

    RigFemResultPosEnum resPos = tensorResults()->resultPositionType();
    QString resFieldName = tensorResults()->resultFieldName();

    RigFemResultAddress resVarAddress(resPos, resFieldName.toStdString(), "");

    gmCase->femPartResults()->minMaxScalarValuesOverAllTensorComponents(resVarAddress, timeStepIndex, &localMin, &localMax);
    gmCase->femPartResults()->posNegClosestToZeroOverAllTensorComponents(resVarAddress, timeStepIndex, &localPosClosestToZero, &localNegClosestToZero);
    
    gmCase->femPartResults()->minMaxScalarValuesOverAllTensorComponents(resVarAddress, &globalMin, &globalMax);
    gmCase->femPartResults()->posNegClosestToZeroOverAllTensorComponents(resVarAddress, &globalPosClosestToZero, &globalNegClosestToZero);

    legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
    legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

    QString legendTitle = "Tensors:\n" + RimTensorResults::uiFieldName(resFieldName);

    legendConfig->setTitle(legendTitle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::updateLegendTextAndRanges(RimRegularLegendConfig* legendConfig, int timeStepIndex)
{
    if (!m_geomechCase || !m_geomechCase->geoMechData()
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

    gmCase->femPartResults()->minMaxScalarValues(resVarAddress, timeStepIndex, &localMin, &localMax);
    gmCase->femPartResults()->posNegClosestToZero(resVarAddress, timeStepIndex, &localPosClosestToZero, &localNegClosestToZero);

    gmCase->femPartResults()->minMaxScalarValues(resVarAddress, &globalMin, &globalMax);
    gmCase->femPartResults()->posNegClosestToZero(resVarAddress, &globalPosClosestToZero, &globalNegClosestToZero);


    legendConfig->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
    legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

    if (cellResult()->hasCategoryResult())
    {
        std::vector<QString> fnVector;
        if (gmCase->femPartResults()->activeFormationNames())
        {
            fnVector = gmCase->femPartResults()->activeFormationNames()->formationNames(); 
        }
        legendConfig->setNamedCategoriesInverse(fnVector);
    }

    QString legendTitle = "Cell Results:\n" + caf::AppEnum<RigFemResultPosEnum>(cellResult->resultPositionType()).uiText() + 
        "\n" + cellResult->resultFieldUiName();

    if (!cellResult->resultComponentUiName().isEmpty())
    {
        legendTitle += ", " + cellResult->resultComponentUiName();
    }

    if (   cellResult->resultFieldName() == "SE" || cellResult->resultFieldName() == "ST" || cellResult->resultFieldName() == "POR-Bar" 
        || cellResult->resultFieldName() == "SM" || cellResult->resultFieldName() == "SEM" || cellResult->resultFieldName() == "Q" )
    {
        legendTitle += " [Bar]";
    }

    if (cellResult->resultFieldName() == "MODULUS")
    {
        legendTitle += " [GPa]";
    }

    legendConfig->setTitle(legendTitle);
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

    absLegendConfigs.push_back(cellResult()->legendConfig());
    absLegendConfigs.push_back(tensorResults()->arrowColorLegendConfig());

    return absLegendConfigs;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFemPartCollection* RimGeoMechView::femParts() const
{
    if (m_geomechCase && m_geomechCase->geoMechData())
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
    if (m_geomechCase && m_geomechCase->geoMechData())
    {
        return m_geomechCase->geoMechData()->femParts();
    }

    return nullptr;
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
bool RimGeoMechView::isTimeStepDependentDataVisible() const
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
    Rim3dView::fieldChangedByUi(changedField, oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::initAfterRead()
{
    RimGridView::initAfterRead();
    this->cellResult()->setGeoMechCase(m_geomechCase);

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
    m_currentReservoirCellVisibility = nullptr;
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
void RimGeoMechView::calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStep)
{
    m_vizLogic->calculateCurrentTotalCellVisibility(totalVisibility, timeStep);
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
bool RimGeoMechView::isUsingFormationNames() const
{
    if (cellResult()->hasCategoryResult()) return true; // Correct for now

    return geoMechPropertyFilterCollection()->isUsingFormationNames(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechView::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(m_overlayInfoConfig());
    uiTreeOrdering.add(m_gridCollection());

    uiTreeOrdering.add(cellResult());
    uiTreeOrdering.add(m_tensorResults());

    uiTreeOrdering.add(m_crossSectionCollection());
    
    uiTreeOrdering.add(m_rangeFilterCollection());
    uiTreeOrdering.add(m_propertyFilterCollection());

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition* RimGeoMechView::cellResultResultDefinition()
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

