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

#include "RimCase.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"
#include "RimEclipseCase.h"
#include "RigCaseData.h"
#include "RimGeoMechCase.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
#include "RigCaseToCaseCellMapper.h"


CAF_PDM_SOURCE_INIT(RimViewLink, "RimViewLink");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLink::RimViewLink(void)
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
RimViewLink::~RimViewLink(void)
{
    this->removeOverrides();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimViewLink::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
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
            if (views[i] != linkedViews->mainView())
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
void RimViewLink::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isActive)
    {
        updateUiIconFromActiveState();
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
    else if (changedField == &syncCellResult && syncCellResult())
    {
        doSyncCellResult();
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

            RimViewLinker* linkedViews = NULL;
            this->firstAnchestorOrThisOfType(linkedViews);
            linkedViews->configureOverrides(); // Should not be neccesary? JJS
        }

        updateOptionSensitivity();
        updateDisplayNameAndIcon();
        updateUiIconFromActiveState();

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
void RimViewLink::initAfterRead()
{
    configureOverrides();
    updateDisplayNameAndIcon();
    updateUiIconFromActiveState();
    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimViewLink::managedEclipseView()
{
    RimView* rimView = m_managedView;

    return dynamic_cast<RimEclipseView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimViewLink::managedGeoView()
{
    RimView* rimView = m_managedView;

    return dynamic_cast<RimGeoMechView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::configureOverrides()
{
    RimViewLinkerCollection* viewLinkerColl = NULL;
    this->firstAnchestorOrThisOfType(viewLinkerColl);
    if (!viewLinkerColl->isActive()) return;

    RimViewLinker* viewLinker = NULL;
    this->firstAnchestorOrThisOfType(viewLinker);

    RimView* masterView = viewLinker->mainView();
    CVF_ASSERT(masterView);
    
    if (!masterView) return;

    if (m_managedView)
    {
        RimEclipseView* manEclView = managedEclipseView();
        RimGeoMechView* manGeoView = managedGeoView();

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
void RimViewLink::updateOptionSensitivity()
{
    RimViewLinker* linkedViews = NULL;
    firstAnchestorOrThisOfType(linkedViews);
    CVF_ASSERT(linkedViews);

    RimView* mainView = linkedViews->mainView();
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
RimView* RimViewLink::managedView()
{
    return m_managedView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::setManagedView(RimView* view)
{
    m_managedView = view;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
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
void RimViewLink::removeOverrides()
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
void RimViewLink::updateUiIconFromActiveState()
{
    RimViewLinker::applyIconEnabledState(this, m_originalIcon, !isActive());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::updateDisplayNameAndIcon()
{
    RimViewLinker::findNameAndIconFromView(&name.v(), &m_originalIcon, managedView());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::doSyncCamera()
{
    RimViewLinkerCollection* viewLinkerColl = NULL;
    this->firstAnchestorOrThisOfType(viewLinkerColl);
    if (!viewLinkerColl->isActive()) return;

    RimViewLinker* viewLinker = NULL;
    this->firstAnchestorOrThisOfType(viewLinker);
    viewLinker->updateScaleZ(viewLinker->mainView(), viewLinker->mainView()->scaleZ());
    viewLinker->updateCamera(viewLinker->mainView());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::doSyncTimeStep()
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
void RimViewLink::doSyncCellResult()
{
    RimViewLinker* viewLinker = ownerViewLinker();
    viewLinker->updateCellResult();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimViewLink::ownerViewLinker()
{
    RimViewLinker* viewLinker = NULL;
    this->firstAnchestorOrThisOfType(viewLinker);
    CVF_ASSERT(viewLinker);

    return viewLinker;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigCaseToCaseCellMapper* RimViewLink::cellMapper()
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
RimView* RimViewLink::masterView()
{
    return ownerViewLinker()->mainView();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimViewLink::isVisibleCellsSyncPossible()
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
bool RimViewLink::syncVisibleCells()
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

