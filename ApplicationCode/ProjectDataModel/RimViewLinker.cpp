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

#include "RimViewLinker.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimLinkedView.h"
#include "RimProject.h"
#include "RimView.h"

#include "RiuViewer.h"

#include "cvfCamera.h"
#include "cafFrameAnimationControl.h"
#include "cvfMatrix4.h"
#include "cafPdmUiTreeOrdering.h"



CAF_PDM_SOURCE_INIT(RimViewLinker, "RimLinkedViews");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker::RimViewLinker(void)
{
    CAF_PDM_InitObject("Linked Views", ":/Reservoir1View.png", "", "");

    CAF_PDM_InitField(&m_name, "Name", QString("View Group Name"), "View Group Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_mainView, "MainView", "Main View", "", "", "");
    m_mainView.uiCapability()->setUiChildrenHidden(true);
    m_mainView.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&linkedViews, "ManagedViews", "Managed Views", "", "", "");
    linkedViews.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker::~RimViewLinker(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateTimeStep(RimView* sourceView, int timeStep)
{
    RimLinkedView* sourceLinkedView = linkedViewFromView(sourceView);
    if (sourceLinkedView && !sourceLinkedView->syncTimeStep())
    {
        return;
    }

    if (m_mainView && m_mainView->viewer() && sourceView != m_mainView)
    {
        m_mainView->viewer()->setCurrentFrame(timeStep);
        m_mainView->viewer()->animationControl()->setCurrentFrameOnly(timeStep);
    }

    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        RimLinkedView* managedViewConfig = linkedViews[i];
        if (managedViewConfig->managedView() && managedViewConfig->managedView() != sourceView)
        {
            if (managedViewConfig->syncTimeStep() && managedViewConfig->managedView()->viewer())
            {
                managedViewConfig->managedView()->viewer()->setCurrentFrame(timeStep);
                managedViewConfig->managedView()->viewer()->animationControl()->setCurrentFrameOnly(timeStep);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCellResult()
{
    RimView* rimView = m_mainView;
    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(rimView);
    if (masterEclipseView && masterEclipseView->cellResult())
    {
        RimEclipseResultDefinition* eclipseCellResultDefinition = masterEclipseView->cellResult();

        for (size_t i = 0; i < linkedViews.size(); i++)
        {
            RimLinkedView* managedViewConfig = linkedViews[i];
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

        for (size_t i = 0; i < linkedViews.size(); i++)
        {
            RimLinkedView* managedViewConfig = linkedViews[i];
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
void RimViewLinker::updateRangeFilters()
{
    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        RimLinkedView* managedViewConfig = linkedViews[i];
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
void RimViewLinker::updatePropertyFilters()
{
    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        RimLinkedView* managedViewConfig = linkedViews[i];
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
void RimViewLinker::configureOverrides()
{
    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        RimLinkedView* managedViewConfig = linkedViews[i];
        managedViewConfig->configureOverrides();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViewsForCameraSync(RimView* source, std::vector<RimView*>& views)
{
    if (source != m_mainView())
    {
        views.push_back(m_mainView());
    }

    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        if (linkedViews[i]->syncCamera && linkedViews[i]->managedView() && source != linkedViews[i]->managedView())
        {
            views.push_back(linkedViews[i]->managedView());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::applyAllOperations()
{
    configureOverrides();

    updateCellResult();
    updateTimeStep(NULL, m_mainView->currentTimeStep());
    updateRangeFilters();
    updatePropertyFilters();
    updateScaleZ(m_mainView, m_mainView->scaleZ());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimViewLinker::displayNameForView(RimView* view)
{
    QString displayName = "None";

    if (view)
    {
        RimCase* rimCase = NULL;
        view->firstAnchestorOrThisOfType(rimCase);

        displayName = rimCase->caseUserDescription() + " : " + view->name;
    }

    return displayName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    for (size_t cIdx = 0; cIdx < linkedViews.size(); ++cIdx)
    {
        PdmObjectHandle* childObject = linkedViews[cIdx];
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
RimLinkedView* RimViewLinker::linkedViewFromView(RimView* view)
{
    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        if (linkedViews[i]->managedView() == view) return linkedViews[i];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::setMainView(RimView* view)
{
    m_mainView = view;

    initAfterRead();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RimViewLinker::mainView()
{
    return m_mainView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViews(std::vector<RimView*>& views)
{
    views.push_back(m_mainView());

    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        if (linkedViews[i]->managedView())
        {
            views.push_back(linkedViews[i]->managedView());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::initAfterRead()
{
    m_name = displayNameForView(m_mainView);

    QIcon icon;
    if (m_mainView)
    {
        RimCase* rimCase = NULL;
        m_mainView->firstAnchestorOrThisOfType(rimCase);

        icon = rimCase->uiCapability()->uiIcon();
    }

    this->setUiIcon(icon);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateScaleZ(RimView* source, double scaleZ)
{
    std::vector<RimView*> views;
    allViewsForCameraSync(source, views);

    // Make sure scale factors are identical
    for (size_t i = 0; i < views.size(); i++)
    {
        views[i]->setScaleZAndUpdate(scaleZ);
    }
}

