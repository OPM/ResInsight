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

#include "RigMainGrid.h"

#include "Rim3dView.h"
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
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimViewLinkerCollection.h"
#include "RimViewManipulator.h"

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"
#include "cvfCamera.h"
#include "cvfMatrix4.h"
#include "cvfScene.h"

CAF_PDM_SOURCE_INIT(RimViewLinker, "ViewLinker");
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker::RimViewLinker()
{
    // clang-format off
    CAF_PDM_InitObject("Linked Views", "", "", "");

    CAF_PDM_InitField(&m_name, "Name", QString("View Group Name"), "View Group Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_masterView, "MainView", "Main View", "", "", "");
    m_masterView.uiCapability()->setUiTreeChildrenHidden(true);
    m_masterView.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_viewControllers, "ManagedViews", "Managed Views", "", "", "");
    m_viewControllers.uiCapability()->setUiHidden(true);
    m_viewControllers.uiCapability()->setUiTreeChildrenHidden(true);
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker::~RimViewLinker()
{
    removeOverrides();

    m_viewControllers.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateTimeStep(RimGridView* sourceView, int timeStep)
{
    CVF_ASSERT(sourceView);

    if (!isActive()) return;

    if (masterView() != sourceView)
    {
        RimViewController* sourceViewLink = sourceView->viewController();
        CVF_ASSERT(sourceViewLink);

        if (!sourceViewLink->isTimeStepLinked())
        {
            return;
        }
    }

    if (m_masterView && m_masterView->viewer() && sourceView != m_masterView)
    {
        m_masterView->viewer()->setCurrentFrame(timeStep);
    }

    for (RimViewController* viewLink : m_viewControllers)
    {
        if (!viewLink->isTimeStepLinked()) continue;

        if (viewLink->managedView() && viewLink->managedView() != sourceView && viewLink->managedView()->viewer())
        {
            viewLink->managedView()->viewer()->setCurrentFrame(timeStep);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCellResult()
{
    Rim3dView*      rimView           = m_masterView;
    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(rimView);
    if (masterEclipseView && masterEclipseView->cellResult())
    {
        RimEclipseResultDefinition* eclipseCellResultDefinition = masterEclipseView->cellResult();

        for (RimViewController* viewLink : m_viewControllers)
        {
            if (viewLink->managedView())
            {
                Rim3dView*      managedView = viewLink->managedView();
                RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(managedView);
                if (eclipseView)
                {
                    if (viewLink->isResultColorControlled())
                    {
                        eclipseView->cellResult()->simpleCopy(eclipseCellResultDefinition);
                        eclipseView->cellResult()->loadResult();

                        if (viewLink->isLegendDefinitionsControlled())
                        {
                            eclipseView->cellResult()->legendConfig()->setUiValuesFromLegendConfig(
                                masterEclipseView->cellResult()->legendConfig());
                            eclipseView->cellResult()->legendConfig()->updateLegend();

                            eclipseView->cellResult()->ternaryLegendConfig()->setUiValuesFromLegendConfig(
                                masterEclipseView->cellResult()->ternaryLegendConfig());
                        }

                        eclipseView->scheduleCreateDisplayModelAndRedraw();
                        eclipseView->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
                    }

                    eclipseView->cellResult()->updateIconState();
                }
            }
        }
    }

    RimGeoMechView* masterGeoView = dynamic_cast<RimGeoMechView*>(rimView);
    if (masterGeoView && masterGeoView->cellResult())
    {
        RimGeoMechResultDefinition* geoMechResultDefinition = masterGeoView->cellResult();

        for (RimViewController* viewLink : m_viewControllers)
        {
            if (viewLink->managedView())
            {
                Rim3dView*      managedView = viewLink->managedView();
                RimGeoMechView* geoView     = dynamic_cast<RimGeoMechView*>(managedView);
                if (geoView)
                {
                    if (viewLink->isResultColorControlled())
                    {
                        geoView->cellResult()->setResultAddress(geoMechResultDefinition->resultAddress());

                        if (viewLink->isLegendDefinitionsControlled())
                        {
                            geoView->cellResult()->legendConfig()->setUiValuesFromLegendConfig(
                                masterGeoView->cellResult()->legendConfig());
                            geoView->cellResult()->legendConfig()->updateLegend();
                        }

                        geoView->scheduleCreateDisplayModelAndRedraw();
                        geoView->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
                    }

                    geoView->cellResult()->updateIconState();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateRangeFilters(RimCellRangeFilter* changedRangeFilter)
{
    for (RimViewController* viewLink : m_viewControllers)
    {
        viewLink->updateRangeFilterOverrides(changedRangeFilter);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateOverrides()
{
    for (RimViewController* viewLink : m_viewControllers)
    {
        if (viewLink->isActive())
        {
            viewLink->updateOverrides();
        }
        else
        {
            viewLink->removeOverrides();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::removeOverrides()
{
    for (const auto& viewController : m_viewControllers)
    {
        if (viewController->managedView())
        {
            viewController->removeOverrides();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViewsForCameraSync(const RimGridView* source, std::vector<RimGridView*>& views) const
{
    if (!isActive()) return;

    if (source != m_masterView())
    {
        views.push_back(m_masterView());
    }

    for (const auto& viewController : m_viewControllers)
    {
        if (viewController->managedView() && source != viewController->managedView())
        {
            if (viewController->isCameraLinked())
            {
                views.push_back(viewController->managedView());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateDependentViews()
{
    updateOverrides();
    updateCellResult();
    updateScaleZ(m_masterView, m_masterView->scaleZ());
    updateCamera(m_masterView);
    updateTimeStep(m_masterView, m_masterView->currentTimeStep());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimViewLinker::displayNameForView(RimGridView* view)
{
    QString displayName = "None";

    if (view)
    {
        RimCase* rimCase = nullptr;
        view->firstAncestorOrThisOfType(rimCase);
        if (rimCase)
        {
            displayName = rimCase->caseUserDescription() + ": " + view->name();
        }
    }

    return displayName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::setMasterView(RimGridView* view)
{
    RimViewController* previousViewController = view->viewController();

    // Remove the view as dependent view
    if (previousViewController)
    {
        delete previousViewController;
        this->m_viewControllers.removeChildObject(nullptr);
    }

    this->removeOverrides();

    m_masterView = view;

    updateUiNameAndIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimViewLinker::masterView() const
{
    return m_masterView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViews(std::vector<RimGridView*>& views) const
{
    views.push_back(m_masterView());

    for (const auto& viewController : m_viewControllers)
    {
        if (viewController->managedView())
        {
            views.push_back(viewController->managedView());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::initAfterRead()
{
    updateUiNameAndIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateScaleZ(RimGridView* sourceView, double scaleZ)
{
    if (!isActive()) return;

    if (masterView() != sourceView)
    {
        RimViewController* sourceViewLink = sourceView->viewController();
        CVF_ASSERT(sourceViewLink);

        if (!sourceViewLink->isCameraLinked())
        {
            return;
        }
    }

    std::vector<RimGridView*> views;
    allViewsForCameraSync(sourceView, views);

    // Make sure scale factors are identical
    for (auto& view : views)
    {
        view->setScaleZAndUpdate(scaleZ);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewLinker::isActive() const
{
    RimViewLinkerCollection* viewLinkerCollection = nullptr;
    this->firstAncestorOrThisOfType(viewLinkerCollection);

    if (!viewLinkerCollection)
    {
        // This will happen when the all linked views are about to be deleted
        // The viewLinker is taken out of the viewLinkerCollection, and no parent can be found
        // See RicDeleteAllLinkedViewsFeature
        return false;
    }

    return viewLinkerCollection->isActive();
}

//--------------------------------------------------------------------------------------------------
/// Hande icon update locally as PdmUiItem::updateUiIconFromState works only for static icons
//--------------------------------------------------------------------------------------------------
void RimViewLinker::applyIconEnabledState(caf::PdmObject* obj, const QIcon& icon, bool disable)
{
    QPixmap icPixmap;
    icPixmap = icon.pixmap(16, 16, QIcon::Normal);

    if (disable)
    {
        QIcon temp(icPixmap);
        icPixmap = temp.pixmap(16, 16, QIcon::Disabled);
    }

    QIcon newIcon(icPixmap);
    obj->setUiIcon(newIcon);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateUiNameAndIcon()
{
    RimViewLinker::findNameAndIconFromView(&m_name.v(), &m_originalIcon, m_masterView);
    RimViewLinker::applyIconEnabledState(this, m_originalIcon, false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType)
{
    for (const auto& viewController : m_viewControllers)
    {
        viewController->scheduleGeometryRegenForDepViews(geometryType);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::scheduleCreateDisplayModelAndRedrawForDependentViews()
{
    for (const auto& viewController : m_viewControllers)
    {
        viewController->scheduleCreateDisplayModelAndRedrawForDependentView();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::findNameAndIconFromView(QString* name, QIcon* icon, RimGridView* view)
{
    CVF_ASSERT(name && icon);

    *name = displayNameForView(view);

    if (view)
    {
        RimCase* rimCase = nullptr;
        view->firstAncestorOrThisOfType(rimCase);

        if (dynamic_cast<RimGeoMechCase*>(rimCase))
        {
            *icon = QIcon(":/GeoMechCase48x48.png");
        }
        else if (dynamic_cast<RimEclipseResultCase*>(rimCase))
        {
            *icon = QIcon(":/Case48x48.png");
        }
        else if (dynamic_cast<RimEclipseInputCase*>(rimCase))
        {
            *icon = QIcon(":/EclipseInput48x48.png");
        }
    }
    else
    {
        *icon = QIcon();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCursorPosition(const RimGridView* sourceView, const cvf::Vec3d& domainCoord)
{
    RimViewController* sourceViewLink = sourceView->viewController();
    if (sourceViewLink && !sourceViewLink->showCursor())
    {
        return;
    }

    std::vector<RimGridView*> viewsToUpdate;
    allViewsForCameraSync(sourceView, viewsToUpdate);

    for (Rim3dView* destinationView : viewsToUpdate)
    {
        if (destinationView == sourceView) continue;

        if (destinationView != m_masterView)
        {
            RimViewController* viewLink = destinationView->viewController();
            if (!viewLink) continue;

            if (!viewLink->showCursor()) continue;
        }

        RiuViewer* destinationViewer = destinationView->viewer();
        if (destinationViewer)
        {
            destinationViewer->setCursorPosition(domainCoord);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCamera(RimGridView* sourceView)
{
    if (!sourceView->viewer()) return;

    if (!isActive()) return;

    RimViewController* viewLink = sourceView->viewController();
    if (viewLink)
    {
        if (!viewLink->isCameraLinked())
        {
            return;
        }
    }

    std::vector<RimGridView*> viewsToUpdate;
    allViewsForCameraSync(sourceView, viewsToUpdate);

    RimViewManipulator::applySourceViewCameraOnDestinationViews(sourceView, viewsToUpdate);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::addDependentView(RimGridView* view)
{
    CVF_ASSERT(view && view != m_masterView);

    RimViewController* viewContr = new RimViewController;
    this->m_viewControllers.push_back(viewContr);

    viewContr->setManagedView(view);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewLinker::isFirstViewDependentOnSecondView(const RimGridView* firstView, const RimGridView* secondView) const
{
    for (const RimViewController* controller : m_viewControllers())
    {
        if (controller->masterView() == secondView && controller->managedView() == firstView)
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::addViewControllers(caf::PdmUiTreeOrdering& uiTreeOrdering) const
{
    for (const auto& viewController : m_viewControllers)
    {
        uiTreeOrdering.add(viewController);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::applyRangeFilterCollectionByUserChoice()
{
    for (const auto& viewController : m_viewControllers)
    {
        viewController->applyRangeFilterCollectionByUserChoice();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updatePropertyFilters(RimPropertyFilter* changedPropertyFilter)
{
    for (RimViewController* viewLink : m_viewControllers)
    {
        viewLink->updatePropertyFilterOverrides(changedPropertyFilter);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::removeViewController(RimViewController* viewController)
{
    m_viewControllers.removeChildObject(viewController);
}
