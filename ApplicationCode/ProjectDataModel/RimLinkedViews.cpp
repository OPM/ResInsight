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

#include "RimLinkedViews.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimManagedViewConfig.h"
#include "RimProject.h"
#include "RimView.h"

#include "RiuViewer.h"

#include "cvfCamera.h"
#include "cvfMatrix4.h"
#include "cafPdmUiTreeOrdering.h"



CAF_PDM_SOURCE_INIT(RimLinkedViews, "RimLinkedViews");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLinkedViews::RimLinkedViews(void)
{
    CAF_PDM_InitObject("Linked Views", "", "", "");

    CAF_PDM_InitField(&m_name, "Name", QString("View Group Name"), "View Group Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_mainView, "MainView", "Main View", "", "", "");
    m_mainView.uiCapability()->setUiChildrenHidden(true);
    m_mainView.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&viewConfigs, "ManagedViews", "Managed Views", "", "", "");
    viewConfigs.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLinkedViews::~RimLinkedViews(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::updateTimeStep(RimView* sourceView, int timeStep)
{
    RimManagedViewConfig* sourceViewConfig = viewConfigForView(sourceView);
    if (sourceViewConfig && !sourceViewConfig->syncTimeStep())
    {
        return;
    }

    if (sourceView && sourceView != m_mainView)
    {
        m_mainView->viewer()->setCurrentFrame(timeStep);
    }
    else
    {
        m_mainView->viewer()->setCurrentFrame(timeStep);
    }

    for (size_t i = 0; i < viewConfigs.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = viewConfigs[i];
        if (managedViewConfig->managedView() && managedViewConfig->managedView() != sourceView)
        {
            if (managedViewConfig->syncTimeStep() && managedViewConfig->managedView()->viewer())
            {
                managedViewConfig->managedView()->viewer()->setCurrentFrame(timeStep);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::updateCellResult()
{
    RimView* rimView = m_mainView;
    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(rimView);
    if (masterEclipseView && masterEclipseView->cellResult())
    {
        RimEclipseResultDefinition* eclipseCellResultDefinition = masterEclipseView->cellResult();

        for (size_t i = 0; i < viewConfigs.size(); i++)
        {
            RimManagedViewConfig* managedViewConfig = viewConfigs[i];
            if (managedViewConfig->managedView())
            {
                if (managedViewConfig->syncCellResult())
                {
                    RimView* rimView = managedViewConfig->managedView();
                    RimEclipseView* eclipeView = dynamic_cast<RimEclipseView*>(rimView);
                    if (eclipeView)
                    {
                        eclipeView->cellResult()->setPorosityModel(eclipseCellResultDefinition->porosityModel());
                        eclipeView->cellResult()->setResultType(eclipseCellResultDefinition->resultType());
                        eclipeView->cellResult()->setResultVariable(eclipseCellResultDefinition->resultVariable());
                    }
                }
            }
        }
    }

    RimGeoMechView* masterGeoView = dynamic_cast<RimGeoMechView*>(rimView);
    if (masterGeoView && masterGeoView->cellResult())
    {
        RimGeoMechResultDefinition* geoMechResultDefinition = masterGeoView->cellResult();

        for (size_t i = 0; i < viewConfigs.size(); i++)
        {
            RimManagedViewConfig* managedViewConfig = viewConfigs[i];
            if (managedViewConfig->managedView())
            {
                if (managedViewConfig->syncCellResult())
                {
                    RimView* rimView = managedViewConfig->managedView();
                    RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
                    if (geoView)
                    {
                        geoView->cellResult()->setResultAddress(geoMechResultDefinition->resultAddress());
                        geoView->scheduleCreateDisplayModelAndRedraw();
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::updateRangeFilters()
{
    for (size_t i = 0; i < viewConfigs.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = viewConfigs[i];
        if (managedViewConfig->managedView())
        {
            if (managedViewConfig->syncRangeFilters())
            {
                RimView* rimView = managedViewConfig->managedView();
                RimEclipseView* eclipeView = dynamic_cast<RimEclipseView*>(rimView);
                if (eclipeView)
                {
                    eclipeView->scheduleGeometryRegen(RANGE_FILTERED);
                    eclipeView->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

                    eclipeView->scheduleCreateDisplayModelAndRedraw();
                }

                RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
                if (geoView)
                {
                    geoView->scheduleGeometryRegen(RANGE_FILTERED);
                    geoView->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

                    geoView->scheduleCreateDisplayModelAndRedraw();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::updatePropertyFilters()
{
    for (size_t i = 0; i < viewConfigs.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = viewConfigs[i];
        if (managedViewConfig->managedView())
        {
            if (managedViewConfig->syncPropertyFilters())
            {
                RimView* rimView = managedViewConfig->managedView();
                RimEclipseView* eclipeView = dynamic_cast<RimEclipseView*>(rimView);
                if (eclipeView)
                {
                    eclipeView->scheduleGeometryRegen(PROPERTY_FILTERED);

                    eclipeView->scheduleCreateDisplayModelAndRedraw();
                }

                RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
                if (geoView)
                {
                    geoView->scheduleGeometryRegen(PROPERTY_FILTERED);

                    geoView->scheduleCreateDisplayModelAndRedraw();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::configureOverrides()
{
    for (size_t i = 0; i < viewConfigs.size(); i++)
    {
        RimManagedViewConfig* managedViewConfig = viewConfigs[i];
        managedViewConfig->configureOverrides();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::allViewsForCameraSync(std::vector<RimView*>& views)
{
    views.push_back(m_mainView());

    for (size_t i = 0; i < viewConfigs.size(); i++)
    {
        if (viewConfigs[i]->syncCamera && viewConfigs[i]->managedView())
        {
            views.push_back(viewConfigs[i]->managedView());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::applyAllOperations()
{
    configureOverrides();

    updateCellResult();
    updateTimeStep(NULL, m_mainView->currentTimeStep());
    updateRangeFilters();
    updatePropertyFilters();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimLinkedViews::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_mainView)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimView*> views;
        proj->allVisibleViews(views);

        for (size_t i = 0; i < views.size(); i++)
        {
            optionList.push_back(caf::PdmOptionItemInfo(displayNameForView(views[i]), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(views[i]))));
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
QString RimLinkedViews::displayNameForView(RimView* view)
{
    RimCase* rimCase = NULL;
    view->firstAnchestorOrThisOfType(rimCase);

    QString displayName = rimCase->caseUserDescription() + " : " + view->name;

    return displayName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    for (size_t cIdx = 0; cIdx < viewConfigs.size(); ++cIdx)
    {
        PdmObjectHandle* childObject = viewConfigs[cIdx];
        if (childObject)
        {
            uiTreeOrdering.add(childObject);
        }
    }

    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimManagedViewConfig* RimLinkedViews::viewConfigForView(RimView* view)
{
    for (size_t i = 0; i < viewConfigs.size(); i++)
    {
        if (viewConfigs[i]->managedView() == view) return viewConfigs[i];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimLinkedViews::setMainView(RimView* view)
{
    m_mainView = view;

    m_name = displayNameForView(view);

    this->uiCapability()->setUiIcon(view->uiCapability()->uiIcon());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RimLinkedViews::mainView()
{
    return m_mainView;
}

