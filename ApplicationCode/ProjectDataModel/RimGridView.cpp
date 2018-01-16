/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RiaApplication.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimGridCollection.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimPropertyFilterCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "RiuMainWindow.h"

#include "cvfModel.h"
#include "cvfScene.h"


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimGridView, "GenericGridView"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridView::RimGridView()
{

    CAF_PDM_InitFieldNoDefault(&m_rangeFilterCollection, "RangeFilters", "Range Filters", "", "", "");
    m_rangeFilterCollection.uiCapability()->setUiHidden(true);
    m_rangeFilterCollection = new RimCellRangeFilterCollection();

    CAF_PDM_InitFieldNoDefault(&m_overrideRangeFilterCollection, "RangeFiltersControlled", "Range Filters (controlled)", "", "", "");
    m_overrideRangeFilterCollection.uiCapability()->setUiHidden(true);
    m_overrideRangeFilterCollection.xmlCapability()->setIOWritable(false);
    m_overrideRangeFilterCollection.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_crossSectionCollection, "CrossSections", "Intersections", "", "", "");
    m_crossSectionCollection.uiCapability()->setUiHidden(true);
    m_crossSectionCollection = new RimIntersectionCollection();

    CAF_PDM_InitFieldNoDefault(&m_gridCollection, "GridCollection", "GridCollection", "", "", "");
    m_gridCollection.uiCapability()->setUiHidden(true);
    m_gridCollection = new RimGridCollection();

    m_previousGridModeMeshLinesWasFaults = false;

    CAF_PDM_InitFieldNoDefault(&m_overlayInfoConfig, "OverlayInfoConfig", "Info Box", "", "", "");
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView(this);
    m_overlayInfoConfig.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridView::~RimGridView(void)
{
    RimProject* proj = RiaApplication::instance()->project();

    if (proj && this->isMasterView())
    {
        delete proj->viewLinkerCollection->viewLinker();
        proj->viewLinkerCollection->viewLinker = NULL;

        proj->uiCapability()->updateConnectedEditors();
    }

    RimViewController* vController = this->viewController();
    if (proj && vController)
    {
        vController->setManagedView(NULL);
        vController->ownerViewLinker()->removeViewController(vController);
        delete vController;

        proj->uiCapability()->updateConnectedEditors();
    }

    delete this->m_overlayInfoConfig();

    delete m_rangeFilterCollection;
    delete m_overrideRangeFilterCollection;
    delete m_crossSectionCollection;
    delete m_gridCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::showGridCells(bool enableGridCells)
{
    if (!enableGridCells)
    {
        m_previousGridModeMeshLinesWasFaults = meshMode() == FAULTS_MESH;
        if (surfaceMode() != NO_SURFACE) surfaceMode.setValueWithFieldChanged(FAULTS);
        if (meshMode() != NO_MESH) meshMode.setValueWithFieldChanged(FAULTS_MESH);
    }
    else
    {
        if (surfaceMode() != NO_SURFACE) surfaceMode.setValueWithFieldChanged(SURFACE);
        if (meshMode() != NO_MESH) meshMode.setValueWithFieldChanged(m_previousGridModeMeshLinesWasFaults ? FAULTS_MESH : FULL_MESH);
    }

    m_gridCollection->isActive = enableGridCells;
    m_gridCollection->updateConnectedEditors();
    m_gridCollection->updateUiIconFromState(enableGridCells);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimGridView::currentTotalCellVisibility()
{
    if (m_currentReservoirCellVisibility.isNull())
    {
        m_currentReservoirCellVisibility = new cvf::UByteArray;
        this->calculateCurrentTotalCellVisibility(m_currentReservoirCellVisibility.p(), m_currentTimeStep());
    }

    return m_currentReservoirCellVisibility;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* RimGridView::crossSectionCollection() const
{
    return m_crossSectionCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection* RimGridView::rangeFilterCollection()
{
    if (this->viewController() && this->viewController()->isRangeFiltersControlled() && m_overrideRangeFilterCollection)
    {
        return m_overrideRangeFilterCollection;
    }
    else
    {
        return m_rangeFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimCellRangeFilterCollection* RimGridView::rangeFilterCollection() const
{
    if (this->viewController() && this->viewController()->isRangeFiltersControlled() && m_overrideRangeFilterCollection)
    {
        return m_overrideRangeFilterCollection;
    }
    else
    {
        return m_rangeFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGridView::hasOverridenRangeFilterCollection()
{
    return m_overrideRangeFilterCollection() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::setOverrideRangeFilterCollection(RimCellRangeFilterCollection* rfc)
{
    if (m_overrideRangeFilterCollection()) delete m_overrideRangeFilterCollection();

    m_overrideRangeFilterCollection = rfc;
    this->scheduleGeometryRegen(RANGE_FILTERED);
    this->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

    this->scheduleCreateDisplayModelAndRedraw();
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::replaceRangeFilterCollectionWithOverride()
{
    RimCellRangeFilterCollection* overrideRfc = m_overrideRangeFilterCollection;
    CVF_ASSERT(overrideRfc);

    RimCellRangeFilterCollection* currentRfc = m_rangeFilterCollection;
    if (currentRfc)
    {
        delete currentRfc;
    }

    // Must call removeChildObject() to make sure the object has no parent
    // No parent is required when assigning a object into a field
    m_overrideRangeFilterCollection.removeChildObject(overrideRfc);

    m_rangeFilterCollection = overrideRfc;

    this->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewController* RimGridView::viewController() const
{
    RimViewController* viewController = NULL;
    std::vector<caf::PdmObjectHandle*> reffingObjs;

    this->objectsWithReferringPtrFields(reffingObjs);
    for (size_t i = 0; i < reffingObjs.size(); ++i)
    {
        viewController = dynamic_cast<RimViewController*>(reffingObjs[i]);
        if (viewController) break;
    }

    return viewController;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimGridView::assosiatedViewLinker() const
{
    RimViewLinker* viewLinker = this->viewLinkerIfMasterView();
    if (!viewLinker)
    {
        RimViewController* viewController = this->viewController();
        if (viewController)
        {
            viewLinker = viewController->ownerViewLinker();
        }
    }

    return viewLinker;
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
void RimGridView::removeModelByName(cvf::Scene* scene, const cvf::String& modelName)
{
    std::vector<cvf::Model*> modelsToBeRemoved;
    for (cvf::uint i = 0; i < scene->modelCount(); i++)
    {
        if (scene->model(i)->name() == modelName)
        {
            modelsToBeRemoved.push_back(scene->model(i));
        }
    }

    for (size_t i = 0; i < modelsToBeRemoved.size(); i++)
    {
        scene->removeModel(modelsToBeRemoved[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::onTimeStepChanged()
{
    if (this->propertyFilterCollection() && this->propertyFilterCollection()->hasActiveDynamicFilters())
    {  
        m_currentReservoirCellVisibility = NULL; 
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if ( changedField == &scaleZ )
    {
        m_crossSectionCollection->updateIntersectionBoxGeometry();
    }

    Rim3dView::fieldChangedByUi(changedField, oldValue, newValue);

    if ( changedField == &scaleZ )
    {
        RimViewLinker* viewLinker = this->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateScaleZ(this, scaleZ);
            viewLinker->updateCamera(this);
        }
    }
    else if ( changedField == &m_currentTimeStep )
    {
        RimViewLinker* viewLinker = this->assosiatedViewLinker();
        if ( viewLinker )
        {
            viewLinker->updateTimeStep(this, m_currentTimeStep);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::selectOverlayInfoConfig()
{
    RiuMainWindow::instance()->selectAsCurrentItem(m_overlayInfoConfig);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimGridView::viewLinkerIfMasterView() const
{
    RimViewLinker* viewLinker = NULL;
    std::vector<caf::PdmObjectHandle*> reffingObjs;

    this->objectsWithReferringPtrFields(reffingObjs);

    for (size_t i = 0; i < reffingObjs.size(); ++i)
    {
        viewLinker = dynamic_cast<RimViewLinker*>(reffingObjs[i]);
        if (viewLinker) break;
    }

    return viewLinker;
}


