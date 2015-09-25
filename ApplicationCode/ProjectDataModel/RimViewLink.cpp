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

#include "RimViewLink.h"

#include "RiaApplication.h"

#include "RigCaseData.h"
#include "RigCaseToCaseCellMapper.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimCase.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT(RimViewController, "ViewController");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewController::RimViewController(void)
{
    CAF_PDM_InitObject("View Link", "", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    QString defaultName = "View Config: Empty view";
    CAF_PDM_InitField(&m_name, "Name", defaultName, "Managed View Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_managedView, "ManagedView", "Linked View", "", "", "");
    m_managedView.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitField(&m_syncCamera,          "SyncCamera", true,         "Camera", "", "", "");
    CAF_PDM_InitField(&m_syncTimeStep,        "SyncTimeStep", true,       "Time Step", "", "", "");
    CAF_PDM_InitField(&m_syncCellResult,      "SyncCellResult", false,    "Cell Result", "", "", "");
    
    CAF_PDM_InitField(&m_syncVisibleCells,    "SyncVisibleCells", false,  "Visible Cells", "", "", "");
    //syncVisibleCells.uiCapability()->setUiHidden(true); // For now

    CAF_PDM_InitField(&m_syncRangeFilters,    "SyncRangeFilters", true,   "Range Filters", "", "", "");
    CAF_PDM_InitField(&m_syncPropertyFilters, "SyncPropertyFilters", true,"Property Filters", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewController::~RimViewController(void)
{
    this->removeOverrides();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimViewController::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_managedView)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimView*> views;
        proj->allNotLinkedViews(views);

        // Add currently linked view to list
        if (this->managedView())
        {
            views.push_back(this->managedView());
        }

        RimViewLinker* linkedViews = NULL;
        this->firstAnchestorOrThisOfType(linkedViews);

        for (size_t i = 0; i< views.size(); i++)
        {
            if (views[i] != linkedViews->masterView())
            {
                RimCase* rimCase = NULL;
                views[i]->firstAnchestorOrThisOfType(rimCase);
                QIcon icon;
                if (rimCase)
                {
                    icon = rimCase->uiCapability()->uiIcon();
                }

                optionList.push_back(caf::PdmOptionItemInfo(RimViewLinker::displayNameForView(views[i]),
                    QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(views[i])),
                    false,
                    icon));
            }
        }

        if (optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }

    return optionList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_isActive)
    {
        updateCameraLink();
        updateTimeStepLink();
        updateResultColorsControl();
        updateOverrides();
        updateDisplayNameAndIcon();
    }
    else if (changedField == &m_syncCamera)
    {
        updateCameraLink();
    }
    else if (changedField == &m_syncTimeStep )
    {
        updateTimeStepLink();
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
    else if (changedField == &m_syncRangeFilters)
    {
        updateOverrides();
    }
    else if (changedField == &m_syncPropertyFilters)
    {
        updateOverrides();
    }
    else if (changedField == &m_managedView)
    {
        PdmObjectHandle* prevValue = oldValue.value<caf::PdmPointer<PdmObjectHandle> >().rawPtr();
        RimView* previousManagedView = dynamic_cast<RimView*>(prevValue);
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
void RimViewController::initAfterRead()
{
    updateOverrides();
    updateDisplayNameAndIcon();
    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimViewController::managedEclipseView()
{
    RimView* rimView = m_managedView;

    return dynamic_cast<RimEclipseView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimViewController::managedGeoView()
{
    RimView* rimView = m_managedView;

    return dynamic_cast<RimGeoMechView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::updateOverrides()
{
    RimViewLinker* viewLinker = ownerViewLinker();
    
    RimView* masterView = viewLinker->masterView();

    CVF_ASSERT(masterView);
    
    if (m_managedView)
    {
        RimEclipseView* manEclView = managedEclipseView();
        RimGeoMechView* manGeoView = managedGeoView();

        if (isVisibleCellsOveridden())
        {
            m_managedView->setOverrideRangeFilterCollection(NULL);
            if (manEclView) manEclView->setOverridePropertyFilterCollection(NULL);
            if (manGeoView) manGeoView->setOverridePropertyFilterCollection(NULL);
            m_managedView->scheduleGeometryRegen(OVERRIDDEN_CELL_VISIBILITY);
            m_managedView->scheduleCreateDisplayModelAndRedraw();
        }
        else
        {
            if (isRangeFilterOveridden())
            {
                m_managedView->setOverrideRangeFilterCollection(masterView->rangeFilterCollection());
            }
            else
            {
                m_managedView->setOverrideRangeFilterCollection(NULL);
            }

            RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(masterView);
            if (masterEclipseView)
            {

                if (manEclView)
                {
                    if (isPropertyFilterOveridden())
                    {
                        manEclView->setOverridePropertyFilterCollection(masterEclipseView->propertyFilterCollection());
                    }
                    else
                    {
                        manEclView->setOverridePropertyFilterCollection(NULL);
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
                        manGeoView->setOverridePropertyFilterCollection(masterGeoView->propertyFilterCollection());
                    }
                    else
                    {
                        manGeoView->setOverridePropertyFilterCollection(NULL);
                    }
                }
            }
        }
        
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
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::removeOverrides(RimView* view)
{
    if (view)
    {
        RimEclipseView* manEclView = dynamic_cast<RimEclipseView*>(view);
        RimGeoMechView* manGeoView = dynamic_cast<RimGeoMechView*>(view);

        view->setOverrideRangeFilterCollection(NULL);
        if (manEclView) manEclView->setOverridePropertyFilterCollection(NULL);
        if (manGeoView) manGeoView->setOverridePropertyFilterCollection(NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::updateOptionSensitivity()
{
    RimViewLinker* linkedViews = NULL;
    firstAnchestorOrThisOfType(linkedViews);
    CVF_ASSERT(linkedViews);

    RimView* mainView = linkedViews->masterView();
    CVF_ASSERT(mainView);

    RimEclipseView* eclipseMasterView = dynamic_cast<RimEclipseView*>(mainView);
    RimGeoMechView* geoMasterView = dynamic_cast<RimGeoMechView*>(mainView);

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
        this->m_syncRangeFilters.uiCapability()->setUiReadOnly(true);
        this->m_syncRangeFilters = false;
        this->m_syncPropertyFilters.uiCapability()->setUiReadOnly(true);
        this->m_syncPropertyFilters = false;
    }
    else
    {
        this->m_syncCellResult.uiCapability()->setUiReadOnly(false);
        
        this->m_syncRangeFilters.uiCapability()->setUiReadOnly(false);
        this->m_syncPropertyFilters.uiCapability()->setUiReadOnly(false);
    }

    m_syncVisibleCells.uiCapability()->setUiReadOnly(!this->isMasterAndDepViewDifferentType());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RimViewController::managedView()
{
    return m_managedView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::setManagedView(RimView* view)
{
    m_managedView = view;
    updateCameraLink();
    updateTimeStepLink();
    updateResultColorsControl();
    updateOverrides();
    updateOptionSensitivity();
    updateDisplayNameAndIcon();
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
    scriptGroup->add(&m_syncTimeStep);
    scriptGroup->add(&m_syncCellResult);

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
        RimViewLinker* viewLinker  = this->ownerViewLinker();

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
RimViewLinker* RimViewController::ownerViewLinker()
{
    RimViewLinker* viewLinker = NULL;
    this->firstAnchestorOrThisOfType(viewLinker);
    CVF_ASSERT(viewLinker);

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

    RigMainGrid* masterEclGrid = NULL;
    RigMainGrid* dependEclGrid = NULL;
    RigFemPart*  masterFemPart = NULL;
    RigFemPart*  dependFemPart = NULL;

    if (masterEclipseView && masterEclipseView->eclipseCase()->reservoirData())
    {
        masterEclGrid = masterEclipseView->eclipseCase()->reservoirData()->mainGrid();
    }

    if (dependEclipseView && dependEclipseView->eclipseCase()->reservoirData())
    {
        dependEclGrid = dependEclipseView->eclipseCase()->reservoirData()->mainGrid();
    }

    if (masterGeomechView && masterGeomechView->geoMechCase()->geoMechData()
        && masterGeomechView->geoMechCase()->geoMechData()->femParts()->partCount())
    {
        masterFemPart = masterGeomechView->geoMechCase()->geoMechData()->femParts()->part(0);
    }

    if (dependGeomechView &&  dependGeomechView->geoMechCase()->geoMechData()
        && dependGeomechView->geoMechCase()->geoMechData()->femParts()->partCount())
    {
        dependFemPart = dependGeomechView->geoMechCase()->geoMechData()->femParts()->part(0);
    }

    // If we have the correct mapping already, return it.
    if (m_caseToCaseCellMapper.notNull())
    {
        if (   masterEclGrid == m_caseToCaseCellMapper->masterGrid()
            && dependEclGrid == m_caseToCaseCellMapper->dependentGrid()
            && masterFemPart == m_caseToCaseCellMapper->masterFemPart()
            && dependFemPart == m_caseToCaseCellMapper->dependentFemPart())
            {
                return m_caseToCaseCellMapper.p();
            }
            else
            {
                m_caseToCaseCellMapper = NULL;
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
RimView* RimViewController::masterView()
{
    return ownerViewLinker()->masterView();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimViewController::isMasterAndDepViewDifferentType()
{
    RimEclipseView* eclipseMasterView = dynamic_cast<RimEclipseView*>(masterView());
    RimGeoMechView* geoMasterView = dynamic_cast<RimGeoMechView*>(masterView());

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
void RimViewController::scheduleCreateDisplayModelAndRedrawForDependentView()
{
    if (!this->isActive()) return;

    if (this->isVisibleCellsOveridden()
        || this->isRangeFilterOveridden()
        || this->isPropertyFilterOveridden()
        ||  this->isResultColorControlled()
        )
    {
        if (this->managedView())
        {
            this->managedView()->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType)
{
    if (!this->isActive()) return;

    if (   this->isVisibleCellsOveridden()
        || this->isRangeFilterOveridden()
        || this->isPropertyFilterOveridden()
        || this->isResultColorControlled()
        )
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
bool RimViewController::isActive()
{
    return ownerViewLinker()->isActive() && this->m_isActive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCameraLinked()
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
bool RimViewController::isTimeStepLinked()
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
bool RimViewController::isResultColorControlled()
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
bool RimViewController::isVisibleCellsOveridden()
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
bool RimViewController::isRangeFilterOveridden()
{
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
bool RimViewController::isPropertyFilterOveridden()
{
   if (ownerViewLinker()->isActive() && this->m_isActive())
    {
        return m_syncPropertyFilters;
    }
    else
    {
        return false;
    }
}

