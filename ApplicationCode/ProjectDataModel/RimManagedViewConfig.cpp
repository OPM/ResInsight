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
        std::vector<RimView*> views;
        allVisibleViews(views);

        for (size_t i = 0; i< views.size(); i++)
        {
            optionList.push_back(caf::PdmOptionItemInfo(views[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(views[i]))));
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
void RimManagedViewConfig::allVisibleViews(std::vector<RimView*>& views)
{
    RimProject* proj = RiaApplication::instance()->project();

    RimView* masterView = NULL;
    firstAnchestorOrThisOfType(masterView);

    if (proj)
    {
        std::vector<RimCase*> cases;
        proj->allCases(cases);
        for (size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++)
        {
            RimCase* rimCase = cases[caseIdx];

            std::vector<RimView*> caseViews = rimCase->views();
            for (size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++)
            {
                if (caseViews[viewIdx]->viewer() && caseViews[viewIdx] != masterView)
                {
                    views.push_back(caseViews[viewIdx]);
                }
            }
        }
    }
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
        configureOverrides();

        if (managedView)
        {
            managedView->rangeFilterCollection()->updateUiUpdateDisplayModel();
        }
    }
    else if (changedField == &syncPropertyFilters)
    {
        configureOverrides();

        RimEclipseView* eclipseView = managedEclipseView();
        if (eclipseView)
        {
            eclipseView->propertyFilterCollection()->updateDisplayModel();
        }
    }
    else if (changedField == &managedView)
    {
        configureOverrides();

        if (managedView)
        {
            managedView->rangeFilterCollection()->updateUiUpdateDisplayModel();

            if (syncCellResult())
            {
                RimView* masterView = NULL;
                firstAnchestorOrThisOfType(masterView);

                masterView->managedViewCollection()->updateCellResult();
            }
        }

        PdmObjectHandle* prevValue = oldValue.value<caf::PdmPointer<PdmObjectHandle> >().rawPtr();
        if (prevValue)
        {
            RimView* rimView = dynamic_cast<RimView*>(prevValue);
            rimView->setOverrideRangeFilterCollection(NULL);
            rimView->rangeFilterCollection()->updateUiUpdateDisplayModel();

            RimEclipseView* rimEclipseView = dynamic_cast<RimEclipseView*>(rimView);
            if (rimEclipseView)
            {
                rimEclipseView->setOverridePropertyFilterCollection(NULL);
            }
        }
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
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimManagedViewConfig::initAfterRead()
{
    configureOverrides();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimManagedViewConfig::managedEclipseView()
{
    RimView* rimView = managedView;

    return dynamic_cast<RimEclipseView*>(rimView);
}

