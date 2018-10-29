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

#include "Riu3DMainWindowTools.h"

#include "cvfModel.h"
#include "cvfScene.h"
#include "RiuMainWindow.h"


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
        proj->viewLinkerCollection->viewLinker = nullptr;

        proj->uiCapability()->updateConnectedEditors();
    }

    RimViewController* vController = this->viewController();
    if (proj && vController)
    {
        vController->setManagedView(nullptr);
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
    m_gridCollection->setActive(enableGridCells);

    createDisplayModel();
    updateDisplayModelVisibility();
    RiuMainWindow::instance()->refreshDrawStyleActions();
    RiuMainWindow::instance()->refreshAnimationActions();

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
void RimGridView::rangeFiltersUpdated()
{
    updateViewFollowingRangeFilterUpdates();
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
    return m_overrideRangeFilterCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::setOverrideRangeFilterCollection(RimCellRangeFilterCollection* rfc)
{
    if (m_overrideRangeFilterCollection()) delete m_overrideRangeFilterCollection();

    m_overrideRangeFilterCollection = rfc;
    // Maintain a link in the active-selection
    if (m_overrideRangeFilterCollection)
    {
        m_rangeFilterCollection->isActive = m_overrideRangeFilterCollection->isActive;
        m_rangeFilterCollection()->uiCapability()->updateConnectedEditors();
    }

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
    std::vector<RimViewController*> objects;
    this->objectsWithReferringPtrFieldsOfType(objects);

    for (auto v : objects)
    {
        if (v)
        {
            return v;
        }
    }

    return nullptr;
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
void RimGridView::updateViewFollowingRangeFilterUpdates()
{
    showGridCells(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::initAfterRead()
{
    RimViewWindow::initAfterRead();

    RimProject* proj = nullptr;
    firstAncestorOrThisOfType(proj);
    if (proj && proj->isProjectFileVersionEqualOrOlderThan("2018.1.1"))
    {
        // For version prior to 2018.1.1 : Grid visualization mode was derived from surfaceMode and meshMode
        // Current : Grid visualization mode is directly defined by m_gridCollection->isActive
        // This change was introduced in https://github.com/OPM/ResInsight/commit/f7bfe8d0

        bool isGridVisualizationModeBefore_2018_1_1 = ((surfaceMode() == RimGridView::SURFACE) || (meshMode() == RimGridView::FULL_MESH));

        m_gridCollection->setActive(isGridVisualizationModeBefore_2018_1_1);
        if (!isGridVisualizationModeBefore_2018_1_1)
        {
            // Was showing faults and intersections. 
            // If was showing with mesh and/or surfaces, turn to full mesh/surf mode to show the mesh, 
            // and to avoid a strange setup when dropping out into grid mode again
            if (surfaceMode() != RimGridView::NO_SURFACE) surfaceMode = RimGridView::SURFACE;
            if (meshMode() != RimGridView::NO_MESH) meshMode = RimGridView::FULL_MESH;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::onTimeStepChanged()
{
    if (this->propertyFilterCollection() && this->propertyFilterCollection()->hasActiveDynamicFilters())
    {  
        m_currentReservoirCellVisibility = nullptr; 
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
RimGridCollection* RimGridView::gridCollection() const
{
    return m_gridCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridView::selectOverlayInfoConfig()
{
    Riu3DMainWindowTools::selectAsCurrentItem(m_overlayInfoConfig);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimGridView::viewLinkerIfMasterView() const
{
    std::vector<RimViewLinker*> objects;
    this->objectsWithReferringPtrFieldsOfType(objects);

    for (auto viewLinker : objects)
    {
        if (viewLinker)
        {
            return viewLinker;
        }
    }

    return nullptr;
}


