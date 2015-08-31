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

#include "RimManagedViewConfig.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimManagedViewCollection.h"
#include "RimProject.h"
#include "RimView.h"

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"


CAF_PDM_SOURCE_INIT(RimManagedViewConfig, "RimManagedViewConfig");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimManagedViewConfig::RimManagedViewConfig(void)
{
    CAF_PDM_InitObject("View Config", ":/chain.png", "", "");

    QString defaultName = "View Config : Empty view";
    CAF_PDM_InitField(&name, "Name", defaultName, "Managed View Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&managedView, "ManagedView", "Managed View", "", "", "");
    managedView.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitField(&syncCamera,          "SyncCamera", true,         "Sync Camera", "", "", "");
    CAF_PDM_InitField(&syncCellResult,      "SyncCellResult", true,     "Sync Cell Result", "", "", "");
    CAF_PDM_InitField(&syncTimeStep,        "SyncTimeStep", true,       "Sync Time Step", "", "", "");
    CAF_PDM_InitField(&syncRangeFilters,    "SyncRangeFilters", true,   "Sync Range Filters", "", "", "");
    CAF_PDM_InitField(&syncPropertyFilters, "SyncPropertyFilters", true,"Sync Property Filters", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimManagedViewConfig::~RimManagedViewConfig(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimManagedViewConfig::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &managedView)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimView*> views;
        proj->allVisibleViews(views);

        RimView* masterView = NULL;
        firstAnchestorOrThisOfType(masterView);

        for (size_t i = 0; i< views.size(); i++)
        {
            if (views[i] != masterView)
            {
                optionList.push_back(caf::PdmOptionItemInfo(displayNameForView(views[i]), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(views[i]))));
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
void RimManagedViewConfig::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &syncCamera || changedField == &syncTimeStep)
    {
        RimView* masterView = NULL;
        firstAnchestorOrThisOfType(masterView);

        masterView->viewer()->update();
    }
    else if (changedField == &syncCellResult)
    {
        // When cell result is activated, update cell result in managed views
        // Original result Will not be restored when cell result is disabled
        
        if (syncCellResult())
        {
            RimView* masterView = NULL;
            firstAnchestorOrThisOfType(masterView);

            masterView->managedViewCollection()->updateCellResult();
        }
    }
    else if (changedField == &syncRangeFilters)
    {
        configureOverridesUpdateDisplayModel();
    }
    else if (changedField == &syncPropertyFilters)
    {
        configureOverridesUpdateDisplayModel();
    }
    else if (changedField == &managedView)
    {
        configureOverridesUpdateDisplayModel();

        if (managedView)
        {
            if (syncCellResult())
            {
                RimView* masterView = NULL;
                firstAnchestorOrThisOfType(masterView);

                masterView->managedViewCollection()->updateCellResult();
            }

            name = displayNameForView(managedView);
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

            rimView->managedViewCollection()->configureOverrides();
        }

        updateDisplayName();
        name.uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewConfig::initAfterRead()
{
    configureOverrides();
    updateDisplayName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimManagedViewConfig::managedEclipseView()
{
    RimView* rimView = managedView;

    return dynamic_cast<RimEclipseView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimManagedViewConfig::managedGeoView()
{
    RimView* rimView = managedView;

    return dynamic_cast<RimGeoMechView*>(rimView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewConfig::configureOverridesUpdateDisplayModel()
{
    configureOverrides();

    if (managedView)
    {
        managedView->rangeFilterCollection()->updateDisplayModeNotifyManagedViews();
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewConfig::configureOverrides()
{
    RimView* masterView = NULL;
    firstAnchestorOrThisOfType(masterView);

    if (managedView)
    {
        if (syncRangeFilters)
        {
            managedView->setOverrideRangeFilterCollection(masterView->rangeFilterCollection());
        }
        else
        {
            managedView->setOverrideRangeFilterCollection(NULL);
        }

        RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(masterView);
        if (masterEclipseView)
        {
            RimEclipseView* eclipseView = managedEclipseView();
            if (eclipseView)
            {
                if (syncPropertyFilters)
                {
                    eclipseView->setOverridePropertyFilterCollection(masterEclipseView->propertyFilterCollection());
                }
                else
                {
                    eclipseView->setOverridePropertyFilterCollection(NULL);
                }
            }
        }

        RimGeoMechView* masterGeoView = dynamic_cast<RimGeoMechView*>(masterView);
        if (masterGeoView)
        {
            RimGeoMechView* geoView = managedGeoView();
            if (geoView)
            {
                if (syncPropertyFilters)
                {
                    geoView->setOverridePropertyFilterCollection(masterGeoView->propertyFilterCollection());
                }
                else
                {
                    geoView->setOverridePropertyFilterCollection(NULL);
                }
            }
        }

        // Propagate overrides in current view to managed views
        managedView->managedViewCollection()->configureOverrides();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimManagedViewConfig::displayNameForView(RimView* view)
{
    CVF_ASSERT(view);

    RimCase* rimCase = NULL;
    firstAnchestorOrThisOfType(rimCase);

    QString displayName = rimCase->caseUserDescription() + " : " + view->name;

    return displayName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewConfig::updateDisplayName()
{
    if (managedView)
    {
        name = displayNameForView(managedView);
    }
    else
    {
        name = "View Config : Empty view";
    }
}

