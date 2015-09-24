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

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    QString defaultName = "View Config: Empty view";
    CAF_PDM_InitField(&name, "Name", defaultName, "Managed View Name", "", "", "");
    name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_managedView, "ManagedView", "Linked View", "", "", "");
    m_managedView.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitField(&syncCamera,          "SyncCamera", true,         "Camera", "", "", "");
    CAF_PDM_InitField(&syncTimeStep,        "SyncTimeStep", true,       "Time Step", "", "", "");
    CAF_PDM_InitField(&syncCellResult,      "SyncCellResult", false,    "Cell Result", "", "", "");
    
    CAF_PDM_InitField(&m_syncVisibleCells,    "SyncVisibleCells", false,  "Visible Cells", "", "", "");
    //syncVisibleCells.uiCapability()->setUiHidden(true); // For now

    CAF_PDM_InitField(&syncRangeFilters,    "SyncRangeFilters", true,   "Range Filters", "", "", "");
    CAF_PDM_InitField(&syncPropertyFilters, "SyncPropertyFilters", true,"Property Filters", "", "", "");
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
    if (changedField == &isActive)
    {
        updateDisplayNameAndIcon();
        if (syncCamera())       doSyncCamera();
        if (syncTimeStep())     doSyncTimeStep();
        if (syncCellResult())   doSyncCellResult();
        configureOverrides();
    }
    else if (changedField == &syncCamera && syncCamera())
    {
        doSyncCamera();
    }
    else if (changedField == &syncTimeStep && syncTimeStep())
    {
        doSyncTimeStep();
    }
    else if (changedField == &syncCellResult)
    {
        if (syncCellResult())
        {
            doSyncCellResult();
        }
        else
        {
            if (managedEclipseView())
            {
                managedEclipseView()->cellResult()->updateIconState();
            }
            else if (managedGeoView())
            {
                managedGeoView()->cellResult()->updateIconState();
            }
        }
    }
    else if (changedField == &syncRangeFilters)
    {
        configureOverrides();
    }
    else if (changedField == &syncPropertyFilters)
    {
        configureOverrides();
    }
    else if (changedField == &m_managedView)
    {
        configureOverrides();

        if (m_managedView)
        {
            RimViewLinker* viewLinker = ownerViewLinker();

            if (syncCellResult())
            {
                viewLinker->updateCellResult();
            }
            if (syncCamera())
            {
                viewLinker->updateCamera(m_managedView);
            }

            name = RimViewLinker::displayNameForView(m_managedView);
        }

        PdmObjectHandle* prevValue = oldValue.value<caf::PdmPointer<PdmObjectHandle> >().rawPtr();
        if (prevValue)
        {
            RimView* rimView = dynamic_cast<RimView*>(prevValue);
            rimView->setOverrideRangeFilterCollection(NULL);

            RimEclipseView* rimEclipseView = dynamic_cast<RimEclipseView*>(rimView);
            if (rimEclipseView)
            {
                rimEclipseView->setOverridePropertyFilterCollection(NULL);
            }

            RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
            if (geoView)
            {
                geoView->setOverridePropertyFilterCollection(NULL);
            }
        }

        updateOptionSensitivity();
        updateDisplayNameAndIcon();

        name.uiCapability()->updateConnectedEditors();
    }
    else if (&m_syncVisibleCells == changedField)
    {
        updateOptionSensitivity();
        configureOverrides();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::initAfterRead()
{
    configureOverrides();
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
void RimViewController::configureOverrides()
{
    RimViewLinkerCollection* viewLinkerColl = NULL;
    this->firstAnchestorOrThisOfType(viewLinkerColl);
    if (!viewLinkerColl->isActive()) return;

    RimViewLinker* viewLinker = NULL;
    this->firstAnchestorOrThisOfType(viewLinker);

    RimView* masterView = viewLinker->masterView();
    CVF_ASSERT(masterView);
    
    if (!masterView) return;

    if (m_managedView)
    {
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

        if (!isActive)
        {
            m_managedView->setOverrideRangeFilterCollection(NULL);
            if (manEclView) manEclView->setOverridePropertyFilterCollection(NULL);
            if (manGeoView) manGeoView->setOverridePropertyFilterCollection(NULL);
        }
        else if (syncVisibleCells())
        {
            m_managedView->setOverrideRangeFilterCollection(NULL);
            if (manEclView) manEclView->setOverridePropertyFilterCollection(NULL);
            if (manGeoView) manGeoView->setOverridePropertyFilterCollection(NULL);

        }
        else
        {
            if (syncRangeFilters)
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
                    if (syncPropertyFilters)
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
                    if (syncPropertyFilters)
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
        this->syncCellResult.uiCapability()->setUiReadOnly(true);
        this->syncCellResult = false;
        this->syncRangeFilters.uiCapability()->setUiReadOnly(true);
        this->syncRangeFilters = false;
        this->syncPropertyFilters.uiCapability()->setUiReadOnly(true);
        this->syncPropertyFilters = false;
    }
    else
    {
        this->syncCellResult.uiCapability()->setUiReadOnly(false);
        
        this->syncRangeFilters.uiCapability()->setUiReadOnly(false);
        this->syncPropertyFilters.uiCapability()->setUiReadOnly(false);
    }

    m_syncVisibleCells.uiCapability()->setUiReadOnly(!this->isVisibleCellsSyncPossible());
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    updateOptionSensitivity();
    uiOrdering.add(&m_managedView);

    caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup("Link Options");

    scriptGroup->add(&syncCamera);
    scriptGroup->add(&syncTimeStep);
    scriptGroup->add(&syncCellResult);

    caf::PdmUiGroup* visibleCells = uiOrdering.addNewGroup("Link Cell Filters");
    visibleCells->add(&m_syncVisibleCells);
    visibleCells->add(&syncRangeFilters);
    visibleCells->add(&syncPropertyFilters);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::removeOverrides()
{
    if (m_managedView)
    {
        RimEclipseView* manEclView = managedEclipseView();
        RimGeoMechView* manGeoView = managedGeoView();
        m_managedView->setOverrideRangeFilterCollection(NULL);
        if (manEclView) manEclView->setOverridePropertyFilterCollection(NULL);
        if (manGeoView) manGeoView->setOverridePropertyFilterCollection(NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::updateDisplayNameAndIcon()
{
    RimViewLinker::findNameAndIconFromView(&name.v(), &m_originalIcon, managedView());
    RimViewLinker::applyIconEnabledState(this, m_originalIcon, !isActive());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::doSyncCamera()
{
    RimViewLinkerCollection* viewLinkerColl = NULL;
    this->firstAnchestorOrThisOfType(viewLinkerColl);
    if (!viewLinkerColl->isActive()) return;

    RimViewLinker* viewLinker = NULL;
    this->firstAnchestorOrThisOfType(viewLinker);
    viewLinker->updateScaleZ(viewLinker->masterView(), viewLinker->masterView()->scaleZ());
    viewLinker->updateCamera(viewLinker->masterView());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::doSyncTimeStep()
{
    RimViewLinkerCollection* viewLinkerColl = NULL;
    this->firstAnchestorOrThisOfType(viewLinkerColl);
    if (!viewLinkerColl->isActive()) return;

    if (m_managedView)
    {
        RimViewLinker* linkedViews = NULL;
        this->firstAnchestorOrThisOfType(linkedViews);
        linkedViews->updateTimeStep(m_managedView, m_managedView->currentTimeStep());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewController::doSyncCellResult()
{
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
bool RimViewController::isVisibleCellsSyncPossible()
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
bool RimViewController::syncVisibleCells()
{
    if (isVisibleCellsSyncPossible())
    {
        return m_syncVisibleCells();
    }
    else
    {
        return false;
    }
}

