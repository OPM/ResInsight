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

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"


CAF_PDM_SOURCE_INIT(RimViewLink, "RimViewLink");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLink::RimViewLink(void)
{
    CAF_PDM_InitObject("View Config", ":/ReservoirView.png", "", "");

    QString defaultName = "View Config: Empty view";
    CAF_PDM_InitField(&name, "Name", defaultName, "Managed View Name", "", "", "");
    name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_managedView, "ManagedView", "Linked View", "", "", "");
    m_managedView.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitField(&syncCamera,          "SyncCamera", true,         "Camera", "", "", "");
    CAF_PDM_InitField(&syncTimeStep,        "SyncTimeStep", true,       "Time Step", "", "", "");
    CAF_PDM_InitField(&syncCellResult,      "SyncCellResult", false,     "Cell Result", "", "", "");
    
    CAF_PDM_InitField(&syncVisibleCells,    "SyncVisibleCells", true,   "Visible Cells", "", "", "");
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
    if (changedField == &syncCamera && syncCamera())
    {
        RimViewLinker* linkedViews = NULL;
        this->firstAnchestorOrThisOfType(linkedViews);
        linkedViews->updateScaleZ(linkedViews->mainView(), linkedViews->mainView()->scaleZ());

        if (m_managedView && m_managedView->viewer())
        {
            m_managedView->viewer()->navigationPolicyUpdate();
        }
    }
    else if (changedField == &syncTimeStep && syncTimeStep())
    {
        if (m_managedView)
        {
            RimViewLinker* linkedViews = NULL;
            this->firstAnchestorOrThisOfType(linkedViews);
            linkedViews->updateTimeStep(m_managedView, m_managedView->currentTimeStep());
        }
    }
    else if (changedField == &syncCellResult && syncCellResult())
    {
        RimViewLinker* linkedViews = NULL;
        this->firstAnchestorOrThisOfType(linkedViews);
        linkedViews->updateCellResult();
    }
    else if (changedField == &syncRangeFilters)
    {
        configureOverridesUpdateDisplayModel();
    }
    else if (changedField == &syncPropertyFilters)
    {
        configureOverridesUpdateDisplayModel();
    }
    else if (changedField == &m_managedView)
    {
        configureOverridesUpdateDisplayModel();

        if (m_managedView)
        {
            RimViewLinker* linkedViews = NULL;
            this->firstAnchestorOrThisOfType(linkedViews);

            if (syncCellResult())
            {
                linkedViews->updateCellResult();
            }
            if (syncCamera())
            {
                m_managedView->notifyCameraHasChanged();
            }

            name = RimViewLinker::displayNameForView(m_managedView);
        }

        PdmObjectHandle* prevValue = oldValue.value<caf::PdmPointer<PdmObjectHandle> >().rawPtr();
        if (prevValue)
        {
            RimView* rimView = dynamic_cast<RimView*>(prevValue);
            rimView->setOverrideRangeFilterCollection(NULL);
            rimView->rangeFilterCollection()->updateDisplayModeNotifyManagedViews();

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
            linkedViews->configureOverrides();
        }

        updateOptionSensitivity();
        updateDisplayNameAndIcon();

        name.uiCapability()->updateConnectedEditors();
    }
    else if (&syncVisibleCells == changedField)
    {
        updateOptionSensitivity();
        configureOverridesUpdateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::initAfterRead()
{
    configureOverrides();
    updateDisplayNameAndIcon();
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
void RimViewLink::configureOverridesUpdateDisplayModel()
{
    configureOverrides();

    // This update scheduling actually schedules update in the master view (not the managed view). Is that intentional ? JJS
    /*
    if (m_managedView)
    {
        m_managedView->rangeFilterCollection()->updateDisplayModeNotifyManagedViews();
    }

    RimEclipseView* eclipseView = managedEclipseView();
    if (eclipseView)
    {
        eclipseView->propertyFilterCollection()->updateDisplayModelNotifyManagedViews();
    }

    RimGeoMechView* geoView = managedGeoView();
    if (geoView)
    {
        geoView->propertyFilterCollection()->updateDisplayModelNotifyManagedViews();
    }
    */
    // Todo : Notify the managed view of the possible change of visualCellsOverride 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::configureOverrides()
{
    RimViewLinker* viewLinker = NULL;
    this->firstAnchestorOrThisOfType(viewLinker);

    RimView* masterView = viewLinker->mainView();
    CVF_ASSERT(masterView);
    
    if (!masterView) return;

    if (m_managedView)
    {
        RimEclipseView* manEclView = managedEclipseView();
        RimGeoMechView* manGeoView = managedGeoView();

        if (syncVisibleCells)
        {
            m_managedView->setOverrideRangeFilterCollection(NULL);
            if (manEclView) manEclView->setOverridePropertyFilterCollection(NULL);
            if (manGeoView) manGeoView->setOverridePropertyFilterCollection(NULL);

            // Todo: set up the managed view with the visible cell override.
            // Create Cell-mapping if necessary
            // Set VisibleCellsOverrider object on the managed view, 
            // with master view and mapper object
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

    bool hideCapabilities = false;
    if (eclipseMasterView && !managedEclipseView())
    {
        hideCapabilities = true;
    }
    if (geoMasterView && !managedGeoView())
    {
        hideCapabilities = true;
    }

    if (hideCapabilities)
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
        bool filterSyncBlocked = syncVisibleCells();
        
        this->syncRangeFilters.uiCapability()->setUiReadOnly(filterSyncBlocked);
        this->syncPropertyFilters.uiCapability()->setUiReadOnly(filterSyncBlocked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLink::updateDisplayNameAndIcon()
{
    if (m_managedView)
    {
        name = RimViewLinker::displayNameForView(m_managedView);
    }
    else
    {
        name = "View Config: Empty view";
    }

    QIcon icon;
    if (m_managedView)
    {
        RimCase* rimCase = NULL;
        m_managedView->firstAnchestorOrThisOfType(rimCase);

        icon = rimCase->uiCapability()->uiIcon();
    }

    this->setUiIcon(icon);
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
    uiOrdering.add(&m_managedView);

    caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup("Link Options");

    scriptGroup->add(&syncCamera);
    scriptGroup->add(&syncTimeStep);
    scriptGroup->add(&syncCellResult);

    caf::PdmUiGroup* visibleCells = uiOrdering.addNewGroup("Link Cell Filters");
    visibleCells->add(&syncVisibleCells);
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

