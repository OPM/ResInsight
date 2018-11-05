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

#include "RimViewController.h"

#include "RiaApplication.h"
#include "RiaOptionItemFactory.h"

#include "RigCaseToCaseCellMapper.h"
#include "RigCaseToCaseRangeFilterMapper.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"

#include "Rim2dEclipseView.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"

#include <QMessageBox>

CAF_PDM_SOURCE_INIT(RimViewController, "ViewController");
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewController::RimViewController()
{
    // clang-format off

    CAF_PDM_InitObject("View Link", "", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    QString defaultName = "View Config: Empty view";
    CAF_PDM_InitField(&m_name, "Name", defaultName, "Managed View Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_managedView, "ManagedView", "Linked View", "", "", "");
    m_managedView.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_syncCamera,            "SyncCamera", true,             "Camera", "", "", "");
    CAF_PDM_InitField(&m_showCursor,            "ShowCursor", true,             "   Show Cursor", "", "", "");
    CAF_PDM_InitField(&m_syncTimeStep,          "SyncTimeStep", true,           "Time Step", "", "", "");
    CAF_PDM_InitField(&m_syncCellResult,        "SyncCellResult", false,        "Cell Result", "", "", "");
    CAF_PDM_InitField(&m_syncLegendDefinitions, "SyncLegendDefinitions", true,  "   Legend Definition", "", "", "");
    
    CAF_PDM_InitField(&m_syncVisibleCells,    "SyncVisibleCells", false,  "Visible Cells", "", "", "");
    /// We do not support this. Consider to remove sometime
    m_syncVisibleCells.uiCapability()->setUiHidden(true);
    m_syncVisibleCells.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_syncRangeFilters,    "SyncRangeFilters", false,   "Range Filters", "", "", "");
    CAF_PDM_InitField(&m_syncPropertyFilters, "SyncPropertyFilters", false,"Property Filters", "", "", "");

    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewController::~RimViewController()
{
    this->removeOverrides();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimViewController::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                       bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_managedView)
    {
        RimProject*               proj = RiaApplication::instance()->project();
        std::vector<RimGridView*> views;
        proj->allNotLinkedViews(views);

        // Add currently linked view to list
        if (this->managedView())
        {
            views.push_back(this->managedView());
        }

        RimViewLinker* viewLinker = nullptr;
        this->firstAncestorOrThisOfType(viewLinker);
        CVF_ASSERT(viewLinker);

        for (RimGridView* view : views)
        {
            if (view != viewLinker->masterView())
            {
                RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName(view, &options);
            }
        }

        if (!options.empty())
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    updateDisplayNameAndIcon();
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue)
{
    if (changedField == &m_isActive)
    {
        if (!m_isActive)
        {
            applyRangeFilterCollectionByUserChoice();
        }

        updateOverrides();
        updateResultColorsControl();
        updateCameraLink();
        updateDisplayNameAndIcon();
        updateTimeStepLink();
    }
    else if (changedField == &m_syncCamera)
    {
        updateCameraLink();
    }
    else if (changedField == &m_syncTimeStep)
    {
        updateTimeStepLink();
    }
    else if (changedField == &m_showCursor)
    {
        if (!m_showCursor && m_managedView && m_managedView->viewer())
        {
            m_managedView->viewer()->setCursorPosition(cvf::Vec3d::UNDEFINED);
        }
    }
    else if (changedField == &m_syncCellResult)
    {
        updateResultColorsControl();
        if (managedEclipseView())
        {
            managedEclipseView()->cellResult()->updateIconState();
        }
        else if (managedGeoView())
        {
            managedGeoView()->cellResult()->updateIconState();
        }
    }
    else if (changedField == &m_syncLegendDefinitions)
    {
        updateLegendDefinitions();
    }
    else if (changedField == &m_syncRangeFilters)
    {
        if (!m_syncRangeFilters)
        {
            applyRangeFilterCollectionByUserChoice();
        }
        updateOverrides();
    }
    else if (changedField == &m_syncPropertyFilters)
    {
        updateOverrides();
    }
    else if (changedField == &m_managedView)
    {
        PdmObjectHandle* prevValue           = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
        RimGridView*     previousManagedView = dynamic_cast<RimGridView*>(prevValue);
        RimViewController::removeOverrides(previousManagedView);

        setManagedView(m_managedView());

        m_name.uiCapability()->updateConnectedEditors();
    }
    else if (&m_syncVisibleCells == changedField)
    {
        updateOptionSensitivity();
        updateOverrides();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimViewController::managedEclipseView() const
{
    RimGridView* rimView = m_managedView;

    return dynamic_cast<RimEclipseView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimViewController::managedGeoView() const
{
    RimGridView* rimView = m_managedView;

    return dynamic_cast<RimGeoMechView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateOverrides()
{
    RimViewLinker* viewLinker = ownerViewLinker();

    RimGridView* masterView = viewLinker->masterView();

    CVF_ASSERT(masterView);

    if (m_managedView)
    {
        RimEclipseView* manEclView = managedEclipseView();
        RimGeoMechView* manGeoView = managedGeoView();

        if (isVisibleCellsOveridden())
        {
            if (manEclView) manEclView->setOverridePropertyFilterCollection(nullptr);
            if (manGeoView) manGeoView->setOverridePropertyFilterCollection(nullptr);
            m_managedView->scheduleGeometryRegen(OVERRIDDEN_CELL_VISIBILITY);
            m_managedView->scheduleCreateDisplayModelAndRedraw();
        }
        else
        {
            RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(masterView);
            if (masterEclipseView)
            {
                if (manEclView)
                {
                    if (isPropertyFilterOveridden())
                    {
                        manEclView->setOverridePropertyFilterCollection(masterEclipseView->eclipsePropertyFilterCollection());
                    }
                    else
                    {
                        manEclView->setOverridePropertyFilterCollection(nullptr);
                    }
                }
            }

            RimGeoMechView* masterGeoView = dynamic_cast<RimGeoMechView*>(masterView);
            if (masterGeoView)
            {
                if (manGeoView)
                {
                    if (isPropertyFilterOveridden())
                    {
                        manGeoView->setOverridePropertyFilterCollection(masterGeoView->geoMechPropertyFilterCollection());
                    }
                    else
                    {
                        manGeoView->setOverridePropertyFilterCollection(nullptr);
                    }
                }
            }
        }

        this->updateRangeFilterOverrides(nullptr);

        if (manGeoView)
        {
            manGeoView->updateIconStateForFilterCollections();
        }

        if (manEclView)
        {
            manEclView->updateIconStateForFilterCollections();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::removeOverrides()
{
    removeOverrides(m_managedView);

    RimEclipseView* manEclView = managedEclipseView();
    RimGeoMechView* manGeoView = managedGeoView();

    if (manGeoView)
    {
        manGeoView->updateIconStateForFilterCollections();
    }

    if (manEclView)
    {
        manEclView->updateIconStateForFilterCollections();
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::removeOverrides(RimGridView* view)
{
    if (view)
    {
        RimEclipseView* manEclView = dynamic_cast<RimEclipseView*>(view);
        RimGeoMechView* manGeoView = dynamic_cast<RimGeoMechView*>(view);

        if (manEclView) manEclView->setOverridePropertyFilterCollection(nullptr);
        if (manGeoView) manGeoView->setOverridePropertyFilterCollection(nullptr);

        view->setOverrideRangeFilterCollection(nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateOptionSensitivity()
{
    RimGridView* mainView = nullptr;

    {
        RimViewLinker* linkedViews = nullptr;
        firstAncestorOrThisOfType(linkedViews);
        CVF_ASSERT(linkedViews);

        if (linkedViews)
        {
            mainView = linkedViews->masterView();
        }
        CVF_ASSERT(mainView);
    }

    RimEclipseView* eclipseMasterView = dynamic_cast<RimEclipseView*>(mainView);
    RimGeoMechView* geoMasterView     = dynamic_cast<RimGeoMechView*>(mainView);

    bool isMasterAndDependentViewDifferentType = false;
    if (eclipseMasterView && !managedEclipseView())
    {
        isMasterAndDependentViewDifferentType = true;
    }

    if (geoMasterView && !managedGeoView())
    {
        isMasterAndDependentViewDifferentType = true;
    }

    if (isMasterAndDependentViewDifferentType)
    {
        this->m_syncCellResult.uiCapability()->setUiReadOnly(true);
        this->m_syncCellResult = false;

        this->m_syncLegendDefinitions.uiCapability()->setUiReadOnly(true);
        this->m_syncLegendDefinitions = false;
    }
    else
    {
        this->m_syncCellResult.uiCapability()->setUiReadOnly(false);

        if (this->m_syncCellResult)
        {
            this->m_syncLegendDefinitions.uiCapability()->setUiReadOnly(false);
        }
        else
        {
            this->m_syncLegendDefinitions.uiCapability()->setUiReadOnly(true);
        }
    }

    if (isCameraControlPossible())
    {
        this->m_syncCamera.uiCapability()->setUiReadOnly(false);
    }
    else
    {
        this->m_syncCamera.uiCapability()->setUiReadOnly(true);
        this->m_syncCamera = false;        
    }

    if (isPropertyFilterControlPossible())
    {
        this->m_syncPropertyFilters.uiCapability()->setUiReadOnly(false);
    }
    else
    {
        this->m_syncPropertyFilters.uiCapability()->setUiReadOnly(true);
        this->m_syncPropertyFilters = false;
    }

    if (isRangeFilterControlPossible())
    {
        this->m_syncRangeFilters.uiCapability()->setUiReadOnly(false);
    }
    else
    {
        this->m_syncRangeFilters.uiCapability()->setUiReadOnly(true);
        this->m_syncRangeFilters = false;
    }

    if (m_syncCamera)
    {
        this->m_showCursor.uiCapability()->setUiReadOnly(false);
    }
    else
    {
        this->m_showCursor.uiCapability()->setUiReadOnly(true);
        this->m_showCursor = false;
    }

    m_syncVisibleCells.uiCapability()->setUiReadOnly(!this->isMasterAndDepViewDifferentType());

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimViewController::managedView() const
{
    return m_managedView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::setManagedView(RimGridView* view)
{
    m_managedView = view;

    updateOptionSensitivity();
    updateDefaultOptions();
    updateOverrides();
    updateResultColorsControl();
    updateCameraLink();
    updateDisplayNameAndIcon();
    updateTimeStepLink();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    updateOptionSensitivity();
    uiOrdering.add(&m_managedView);

    caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup("Link Options");

    scriptGroup->add(&m_syncCamera);
    scriptGroup->add(&m_showCursor);
    scriptGroup->add(&m_syncTimeStep);
    scriptGroup->add(&m_syncCellResult);
    scriptGroup->add(&m_syncLegendDefinitions);

    caf::PdmUiGroup* visibleCells = uiOrdering.addNewGroup("Link Cell Filters");
    visibleCells->add(&m_syncVisibleCells);
    visibleCells->add(&m_syncRangeFilters);
    visibleCells->add(&m_syncPropertyFilters);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateDisplayNameAndIcon()
{
    RimViewLinker::findNameAndIconFromView(&m_name.v(), &m_originalIcon, managedView());
    RimViewLinker::applyIconEnabledState(this, m_originalIcon, !m_isActive());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateCameraLink()
{
    if (!this->isCameraLinked()) return;
    if (m_managedView)
    {
        RimViewLinker* viewLinker = this->ownerViewLinker();

        viewLinker->updateScaleZ(viewLinker->masterView(), viewLinker->masterView()->scaleZ());
        viewLinker->updateCamera(viewLinker->masterView());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateTimeStepLink()
{
    if (!this->isTimeStepLinked()) return;

    if (m_managedView)
    {
        RimViewLinker* viewLinker = this->ownerViewLinker();

        viewLinker->updateTimeStep(viewLinker->masterView(), viewLinker->masterView()->currentTimeStep());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateResultColorsControl()
{
    if (!this->isResultColorControlled()) return;

    RimViewLinker* viewLinker = ownerViewLinker();
    viewLinker->updateCellResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateLegendDefinitions()
{
    if (!this->isLegendDefinitionsControlled()) return;

    RimViewLinker* viewLinker = ownerViewLinker();
    viewLinker->updateCellResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateDefaultOptions()
{
    m_syncCellResult = isCellResultControlAdvisable();
    m_syncRangeFilters = isRangeFilterControlAdvisable();
    m_syncPropertyFilters = isPropertyFilterControlAdvisable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimViewController::ownerViewLinker() const
{
    RimViewLinker* viewLinker = nullptr;
    this->firstAncestorOrThisOfType(viewLinker);

    return viewLinker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCaseToCaseCellMapper* RimViewController::cellMapper()
{
    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(masterView());
    RimEclipseView* dependEclipseView = managedEclipseView();
    RimGeoMechView* masterGeomechView = dynamic_cast<RimGeoMechView*>(masterView());
    RimGeoMechView* dependGeomechView = managedGeoView();

    RigMainGrid* masterEclGrid = nullptr;
    RigMainGrid* dependEclGrid = nullptr;
    RigFemPart*  masterFemPart = nullptr;
    RigFemPart*  dependFemPart = nullptr;

    if (masterEclipseView)
    {
        masterEclGrid = masterEclipseView->mainGrid();
    }

    if (dependEclipseView)
    {
        dependEclGrid = dependEclipseView->mainGrid();
    }

    if (masterGeomechView && masterGeomechView->geoMechCase()->geoMechData() && masterGeomechView->femParts()->partCount())
    {
        masterFemPart = masterGeomechView->femParts()->part(0);
    }

    if (dependGeomechView && dependGeomechView->geoMechCase()->geoMechData() && dependGeomechView->femParts()->partCount())
    {
        dependFemPart = dependGeomechView->femParts()->part(0);
    }

    // If we have the correct mapping already, return it.
    if (m_caseToCaseCellMapper.notNull())
    {
        if (masterEclGrid == m_caseToCaseCellMapper->masterGrid() && dependEclGrid == m_caseToCaseCellMapper->dependentGrid() &&
            masterFemPart == m_caseToCaseCellMapper->masterFemPart() &&
            dependFemPart == m_caseToCaseCellMapper->dependentFemPart())
        {
            return m_caseToCaseCellMapper.p();
        }
        else
        {
            m_caseToCaseCellMapper = nullptr;
        }
    }

    // Create the mapping if needed

    if (m_caseToCaseCellMapper.isNull())
    {
        if (masterEclGrid && dependFemPart)
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper(masterEclGrid, dependFemPart);
        }
        else if (masterEclGrid && dependEclGrid)
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper(masterEclGrid, dependEclGrid);
        }
        else if (masterFemPart && dependFemPart)
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper(masterFemPart, dependFemPart);
        }
        else if (masterFemPart && dependEclGrid)
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper(masterFemPart, dependEclGrid);
        }
    }

    return m_caseToCaseCellMapper.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimViewController::masterView() const
{
    return ownerViewLinker()->masterView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCameraControlPossible() const
{
    Rim2dEclipseView* contourMapMasterView  = dynamic_cast<Rim2dEclipseView*>(masterView());
    Rim2dEclipseView* contourMapManagedView = dynamic_cast<Rim2dEclipseView*>(managedEclipseView());
    return !(contourMapMasterView || contourMapManagedView);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isMasterAndDepViewDifferentType() const
{
    RimEclipseView* eclipseMasterView = dynamic_cast<RimEclipseView*>(masterView());
    RimGeoMechView* geoMasterView     = dynamic_cast<RimGeoMechView*>(masterView());

    bool isMasterAndDependentViewDifferentType = false;
    if (eclipseMasterView && !managedEclipseView())
    {
        isMasterAndDependentViewDifferentType = true;
    }
    if (geoMasterView && !managedGeoView())
    {
        isMasterAndDependentViewDifferentType = true;
    }

    return isMasterAndDependentViewDifferentType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::scheduleCreateDisplayModelAndRedrawForDependentView() const
{
    if (!this->isActive()) return;

    if (this->isVisibleCellsOveridden() || this->isRangeFiltersControlled() || this->isPropertyFilterOveridden() ||
        this->isResultColorControlled())
    {
        if (this->managedView())
        {
            this->managedView()->scheduleCreateDisplayModelAndRedraw();
        }
    }

    if (this->isResultColorControlled() && this->managedView())
    {
        this->managedView()->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType) const
{
    if (!this->isActive()) return;

    if (this->isVisibleCellsOveridden() || this->isRangeFiltersControlled() || this->isPropertyFilterOveridden() ||
        this->isResultColorControlled())
    {
        if (this->managedView())
        {
            if (this->isVisibleCellsOveridden())
            {
                this->managedView()->scheduleGeometryRegen(OVERRIDDEN_CELL_VISIBILITY);
            }

            this->managedView()->scheduleGeometryRegen(geometryType);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isActive() const
{
    return ownerViewLinker()->isActive() && this->m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCameraLinked() const
{
    if (ownerViewLinker()->isActive() && this->m_isActive())
    {
        return m_syncCamera;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::showCursor() const
{
    return m_showCursor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isTimeStepLinked() const
{
    if (ownerViewLinker()->isActive() && this->m_isActive())
    {
        return m_syncTimeStep;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isResultColorControlled() const
{
    if (ownerViewLinker()->isActive() && this->m_isActive())
    {
        return m_syncCellResult;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isLegendDefinitionsControlled() const
{
    if (ownerViewLinker()->isActive() && this->m_isActive())
    {
        return m_syncLegendDefinitions;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isVisibleCellsOveridden() const
{
    if (isMasterAndDepViewDifferentType())
    {
        if (ownerViewLinker()->isActive() && this->m_isActive())
        {
            return m_syncVisibleCells();
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isRangeFilterControlPossible() const
{
    return true;
#if 0
    if (!isMasterAndDepViewDifferentType()) return true;

    // Make sure the cases are in the same domain
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(masterView());
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(masterView());
    if (!geomView) geomView = managedGeoView();
    if (!eclipseView) eclipseView = managedEclipseView();

    if (eclipseView && geomView)
    {
        if (eclipseView->eclipseCase()->reservoirData() && geomView->geoMechCase()->geoMechData())
        {
            RigMainGrid* eclGrid = eclipseView->eclipseCase()->reservoirData()->mainGrid();
            RigFemPart* femPart = geomView->femParts()->part(0);
            
            if (eclGrid && femPart)
            {
                cvf::BoundingBox fembb = femPart->boundingBox();
                cvf::BoundingBox eclbb = eclGrid->boundingBox();
                return fembb.contains(eclbb.min()) && fembb.contains(eclbb.max());
            }
        }
    }
    return false;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isRangeFilterMappingApplicable() const
{
    if (!isMasterAndDepViewDifferentType()) return false;

    // Make sure the cases are in the same domain
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(masterView());
    RimGeoMechView* geomView    = dynamic_cast<RimGeoMechView*>(masterView());
    if (!geomView) geomView = managedGeoView();
    if (!eclipseView) eclipseView = managedEclipseView();

    if (eclipseView && geomView)
    {
        if (eclipseView->eclipseCase()->eclipseCaseData() && geomView->geoMechCase() && geomView->geoMechCase()->geoMechData())
        {
            RigMainGrid* eclGrid = eclipseView->mainGrid();
            RigFemPart*  femPart = geomView->femParts()->part(0);

            if (eclGrid && femPart)
            {
                cvf::BoundingBox fembb = femPart->boundingBox();
                cvf::BoundingBox eclbb = eclGrid->boundingBox();
                return fembb.contains(eclbb.min()) && fembb.contains(eclbb.max());
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCellResultControlAdvisable() const
{
    bool contourMapMasterView = dynamic_cast<Rim2dEclipseView*>(masterView()) != nullptr;
    bool contourMapManagedView = dynamic_cast<Rim2dEclipseView*>(managedEclipseView()) != nullptr;
    return !isMasterAndDepViewDifferentType() && contourMapMasterView != contourMapManagedView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isRangeFilterControlAdvisable() const
{
    bool contourMapMasterView  = dynamic_cast<Rim2dEclipseView*>(masterView()) != nullptr;
    bool contourMapManagedView = dynamic_cast<Rim2dEclipseView*>(managedEclipseView()) != nullptr;
    return isRangeFilterControlPossible() && contourMapMasterView != contourMapManagedView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isPropertyFilterControlAdvisable() const
{
    bool contourMapMasterView = dynamic_cast<Rim2dEclipseView*>(masterView()) != nullptr;
    bool contourMapManagedView = dynamic_cast<Rim2dEclipseView*>(managedEclipseView()) != nullptr;
    return isPropertyFilterControlPossible() && contourMapMasterView != contourMapManagedView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isRangeFiltersControlled() const
{
    if (!isRangeFilterControlPossible()) return false;

    if (ownerViewLinker()->isActive() && this->m_isActive())
    {
        return m_syncRangeFilters;
    }
    else
    {
        return false;
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isPropertyFilterControlPossible() const
{
    // The cases need to be the same
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(masterView());

    if (geomView)
    {
        RimGeoMechView* depGeomView = managedGeoView();
        if (depGeomView && geomView->geoMechCase() == depGeomView->geoMechCase())
        {
            return true;
        }
    }

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(masterView());

    if (eclipseView)
    {
        RimEclipseView* depEclipseView = managedEclipseView();
        if (depEclipseView && eclipseView->eclipseCase() == depEclipseView->eclipseCase())
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isPropertyFilterOveridden() const
{
    if (!isPropertyFilterControlPossible()) return false;

    if (ownerViewLinker()->isActive() && this->m_isActive())
    {
        return m_syncPropertyFilters;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateRangeFilterOverrides(RimCellRangeFilter* changedRangeFilter)
{
    if (!m_managedView) return;

    if (!isRangeFiltersControlled())
    {
        m_managedView->setOverrideRangeFilterCollection(nullptr);

        return;
    }

    // Todo: Optimize by only mapping the changed range filter, if needed.

    {
        // Copy the rangeFilterCollection

        RimCellRangeFilterCollection* sourceFilterCollection = masterView()->rangeFilterCollection();
        QString                       xmlRangeFilterCollCopy = sourceFilterCollection->writeObjectToXmlString();
        PdmObjectHandle*              objectCopy =
            PdmXmlObjectHandle::readUnknownObjectFromXmlString(xmlRangeFilterCollCopy, caf::PdmDefaultObjectFactory::instance());
        RimCellRangeFilterCollection* overrideRangeFilterColl = dynamic_cast<RimCellRangeFilterCollection*>(objectCopy);

        // Convert the range filter to fit in the managed view if needed
        if (isRangeFilterMappingApplicable())
        {
            RimEclipseView* eclipseMasterView = dynamic_cast<RimEclipseView*>(masterView());
            RimGeoMechView* geoMasterView     = dynamic_cast<RimGeoMechView*>(masterView());
            RimEclipseView* depEclView        = managedEclipseView();
            RimGeoMechView* depGeomView       = managedGeoView();

            if (eclipseMasterView && depGeomView)
            {
                if (eclipseMasterView->mainGrid())
                {
                    RigMainGrid* srcEclGrid = eclipseMasterView->mainGrid();
                    RigFemPart*  dstFemPart = depGeomView->femParts()->part(0);
                    for (size_t rfIdx = 0; rfIdx < sourceFilterCollection->rangeFilters().size(); ++rfIdx)
                    {
                        RimCellRangeFilter* srcRFilter = sourceFilterCollection->rangeFilters[rfIdx];
                        RimCellRangeFilter* dstRFilter = overrideRangeFilterColl->rangeFilters[rfIdx];
                        RigCaseToCaseRangeFilterMapper::convertRangeFilterEclToFem(
                            srcRFilter, srcEclGrid, dstRFilter, dstFemPart);
                    }
                }
            }
            else if (geoMasterView && depEclView)
            {
                if (depEclView->mainGrid())
                {
                    RigFemPart*  srcFemPart = geoMasterView->femParts()->part(0);
                    RigMainGrid* dstEclGrid = depEclView->mainGrid();
                    for (size_t rfIdx = 0; rfIdx < sourceFilterCollection->rangeFilters().size(); ++rfIdx)
                    {
                        RimCellRangeFilter* srcRFilter = sourceFilterCollection->rangeFilters[rfIdx];
                        RimCellRangeFilter* dstRFilter = overrideRangeFilterColl->rangeFilters[rfIdx];
                        RigCaseToCaseRangeFilterMapper::convertRangeFilterFemToEcl(
                            srcRFilter, srcFemPart, dstRFilter, dstEclGrid);
                    }
                }
            }
        }

        m_managedView->setOverrideRangeFilterCollection(overrideRangeFilterColl);
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updatePropertyFilterOverrides(RimPropertyFilter* changedPropertyFilter)
{
    updateOverrides();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::applyRangeFilterCollectionByUserChoice()
{
    if (!m_managedView) return;

    if (!m_managedView->hasOverridenRangeFilterCollection())
    {
        return;
    }

    bool restoreOriginal = askUserToRestoreOriginalRangeFilterCollection(m_managedView->name());
    if (restoreOriginal)
    {
        m_managedView->setOverrideRangeFilterCollection(nullptr);
    }
    else
    {
        m_managedView->replaceRangeFilterCollectionWithOverride();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::askUserToRestoreOriginalRangeFilterCollection(const QString& viewName)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();

    QMessageBox msgBox(activeView->viewer()->layoutWidget());
    msgBox.setIcon(QMessageBox::Question);

    QString questionText;
    questionText = QString("The range filters in the view \"%1\" are about to be unlinked.").arg(viewName);

    msgBox.setText(questionText);
    msgBox.setInformativeText("Do you want to keep the range filters from the master view?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes)
    {
        return false;
    }
    else
    {
        return true;
    }
}
