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
#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimViewLink.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimViewLinkerCollection.h"

#include "RiuViewer.h"

#include "cvfCamera.h"
#include "cafFrameAnimationControl.h"
#include "cvfMatrix4.h"
#include "cafPdmUiTreeOrdering.h"



CAF_PDM_SOURCE_INIT(RimViewLinker, "RimViewLinker");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker::RimViewLinker(void)
{
    CAF_PDM_InitObject("Linked Views", ":/Reservoir1View.png", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_name, "Name", QString("View Group Name"), "View Group Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_mainView, "MainView", "Main View", "", "", "");
    m_mainView.uiCapability()->setUiChildrenHidden(true);
    m_mainView.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&viewLinks, "ManagedViews", "Managed Views", "", "", "");
    viewLinks.uiCapability()->setUiHidden(true);
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
    if (!isActive()) return;

    RimViewLink* sourceLinkedView = viewLinkFromView(sourceView);
    if (sourceLinkedView && !sourceLinkedView->syncTimeStep())
    {
        return;
    }

    if (m_mainView && m_mainView->viewer() && sourceView != m_mainView)
    {
        m_mainView->viewer()->setCurrentFrame(timeStep);
        m_mainView->viewer()->animationControl()->setCurrentFrameOnly(timeStep);
    }

    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        RimViewLink* managedViewConfig = viewLinks[i];
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
    if (!isActive()) return;

    RimView* rimView = m_mainView;
    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(rimView);
    if (masterEclipseView && masterEclipseView->cellResult())
    {
        RimEclipseResultDefinition* eclipseCellResultDefinition = masterEclipseView->cellResult();

        for (size_t i = 0; i < viewLinks.size(); i++)
        {
            RimViewLink* managedViewConfig = viewLinks[i];
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

        for (size_t i = 0; i < viewLinks.size(); i++)
        {
            RimViewLink* managedViewConfig = viewLinks[i];
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
    if (!isActive()) return;

    this->scheduleGeometryRegenForDepViews(RANGE_FILTERED);
    this->scheduleGeometryRegenForDepViews(RANGE_FILTERED_INACTIVE);
    this->scheduleCreateDisplayModelAndRedrawForDependentViews();

    #if 0
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        RimViewLink* managedViewConfig = viewLinks[i];
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
    #endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updatePropertyFilters()
{
    if (!isActive()) return;
    this->scheduleGeometryRegenForDepViews(RANGE_FILTERED);
    this->scheduleGeometryRegenForDepViews(RANGE_FILTERED_INACTIVE);
    this->scheduleCreateDisplayModelAndRedrawForDependentViews();

#if 0
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        RimViewLink* managedViewConfig = viewLinks[i];
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
#endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::configureOverrides()
{
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        RimViewLink* managedViewConfig = viewLinks[i];
        managedViewConfig->configureOverrides();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViewsForCameraSync(RimView* source, std::vector<RimView*>& views)
{
    if (!isActive()) return;

    if (source != m_mainView())
    {
        views.push_back(m_mainView());
    }

    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (viewLinks[i]->syncCamera && viewLinks[i]->managedView() && source != viewLinks[i]->managedView())
        {
            views.push_back(viewLinks[i]->managedView());
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

        displayName = rimCase->caseUserDescription() + ": " + view->name;
    }

    return displayName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    for (size_t cIdx = 0; cIdx < viewLinks.size(); ++cIdx)
    {
        PdmObjectHandle* childObject = viewLinks[cIdx];
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
RimViewLink* RimViewLinker::viewLinkFromView(RimView* view)
{
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (viewLinks[i]->managedView() == view) return viewLinks[i];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::setMainView(RimView* view)
{
    m_mainView = view;

    setNameAndIcon();
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

    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (viewLinks[i]->managedView())
        {
            views.push_back(viewLinks[i]->managedView());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::initAfterRead()
{
    setNameAndIcon();
    updateUiIcon();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimViewLinker::isActive()
{
    RimViewLinkerCollection* viewLinkerCollection = NULL;
    this->firstAnchestorOrThisOfType(viewLinkerCollection);
    
    if (!viewLinkerCollection->isActive()) return false;
    
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateUiIcon();
}

//--------------------------------------------------------------------------------------------------
/// Hande icon update locally as PdmUiItem::updateUiIconFromState works only for static icons
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateUiIcon()
{
    QPixmap icPixmap;
    icPixmap = m_originalIcon.pixmap(16, 16, QIcon::Normal);

    if (!m_isActive)
    {
        QIcon temp(icPixmap);
        icPixmap = temp.pixmap(16, 16, QIcon::Disabled);
    }

    QIcon newIcon(icPixmap);
    this->setUiIcon(newIcon);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::setNameAndIcon()
{
    m_name = displayNameForView(m_mainView);

    QIcon icon;
    if (m_mainView)
    {
        RimCase* rimCase = NULL;
        m_mainView->firstAnchestorOrThisOfType(rimCase);

        if (dynamic_cast<RimGeoMechCase*>(rimCase))
        {
            icon = QIcon(":/GeoMechCase48x48.png");
        }
        else if (dynamic_cast<RimEclipseResultCase*>(rimCase))
        {
            icon = QIcon(":/Case48x48.png");
        }
        else if (dynamic_cast<RimEclipseInputCase*>(rimCase))
        {
            icon = QIcon(":/EclipseInput48x48.png");
        }
    }

    m_originalIcon = icon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType)
{
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (  viewLinks[i]->syncVisibleCells() 
           || viewLinks[i]->syncPropertyFilters() 
           || viewLinks[i]->syncRangeFilters() 
           )
        {
            if (viewLinks[i]->managedView())
            {
                if (viewLinks[i]->syncVisibleCells()) {
                    viewLinks[i]->managedView()->scheduleGeometryRegen(OVERRIDDEN_CELL_VISIBILITY);
                }else{
                    viewLinks[i]->managedView()->scheduleGeometryRegen(geometryType);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::scheduleCreateDisplayModelAndRedrawForDependentViews()
{
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (viewLinks[i]->syncVisibleCells()
            || viewLinks[i]->syncPropertyFilters()
            || viewLinks[i]->syncRangeFilters()
            )
        {
            if (viewLinks[i]->managedView())
            {
                viewLinks[i]->managedView()->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }
}

