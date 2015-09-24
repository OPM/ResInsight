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

#include "RigCaseData.h"

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
#include "cvfScene.h"
#include "cafFrameAnimationControl.h"
#include "cvfMatrix4.h"
#include "cafPdmUiTreeOrdering.h"



CAF_PDM_SOURCE_INIT(RimViewLinker, "ViewLinker");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker::RimViewLinker(void)
{
    CAF_PDM_InitObject("Linked Views", "", "", "");

    CAF_PDM_InitField(&m_name, "Name", QString("View Group Name"), "View Group Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_masterView, "MainView", "Main View", "", "", "");
    m_masterView.uiCapability()->setUiChildrenHidden(true);
    m_masterView.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&viewLinks, "ManagedViews", "Managed Views", "", "", "");
    viewLinks.uiCapability()->setUiHidden(true);
    viewLinks.uiCapability()->setUiChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker::~RimViewLinker(void)
{
    removeOverrides();

    viewLinks.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateTimeStep(RimView* sourceView, int timeStep)
{
    CVF_ASSERT(sourceView);

    if (!isActive()) return;

    if (masterView() != sourceView)
    {
        RimViewController* sourceViewLink = viewLinkForView(sourceView);
        CVF_ASSERT(sourceViewLink);

        if (!sourceViewLink->isActive() || !sourceViewLink->syncTimeStep())
        {
            return;
        }
    }

    if (m_masterView && m_masterView->viewer() && sourceView != m_masterView)
    {
        m_masterView->viewer()->setCurrentFrame(timeStep);
        m_masterView->viewer()->animationControl()->setCurrentFrameOnly(timeStep);
    }

    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        RimViewController* viewLink = viewLinks[i];
        if (!viewLink->isActive) continue;

        if (viewLink->managedView() && viewLink->managedView() != sourceView)
        {
            if (viewLink->syncTimeStep() && viewLink->managedView()->viewer())
            {
                viewLink->managedView()->viewer()->setCurrentFrame(timeStep);
                viewLink->managedView()->viewer()->animationControl()->setCurrentFrameOnly(timeStep);
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

    RimView* rimView = m_masterView;
    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>(rimView);
    if (masterEclipseView && masterEclipseView->cellResult())
    {
        RimEclipseResultDefinition* eclipseCellResultDefinition = masterEclipseView->cellResult();

        for (size_t i = 0; i < viewLinks.size(); i++)
        {
            RimViewController* viewLink = viewLinks[i];
            if (!viewLink->isActive) continue;

            if (viewLink->managedView())
            {
                RimView* rimView = viewLink->managedView();
                RimEclipseView* eclipeView = dynamic_cast<RimEclipseView*>(rimView);
                if (eclipeView)
                {
                    if (viewLink->syncCellResult())
                    {
                        eclipeView->cellResult()->setPorosityModel(eclipseCellResultDefinition->porosityModel());
                        eclipeView->cellResult()->setResultType(eclipseCellResultDefinition->resultType());
                        eclipeView->cellResult()->setResultVariable(eclipseCellResultDefinition->resultVariable());
                        eclipeView->scheduleCreateDisplayModelAndRedraw();
                    }
                    
                    eclipeView->cellResult()->updateIconState();
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
            RimViewController* viewLink = viewLinks[i];
            if (!viewLink->isActive) continue;

            if (viewLink->managedView())
            {
                RimView* rimView = viewLink->managedView();
                RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(rimView);
                if (geoView)
                {
                    if (viewLink->syncCellResult())
                    {
                        geoView->cellResult()->setResultAddress(geoMechResultDefinition->resultAddress());
                        geoView->scheduleCreateDisplayModelAndRedraw();
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
void RimViewLinker::updateOverrides()
{
    bool isViewlinkerActive = this->isActive();

    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        RimViewController* viewLink = viewLinks[i];
        if (isViewlinkerActive && viewLink->isActive)
        {
            viewLink->configureOverrides();
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
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (viewLinks[i]->managedView())
        {
            viewLinks[i]->removeOverrides();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViewsForCameraSync(RimView* source, std::vector<RimView*>& views)
{
    if (!isActive()) return;

    if (source != m_masterView())
    {
        views.push_back(m_masterView());
    }

    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (viewLinks[i]->isActive() && viewLinks[i]->syncCamera && viewLinks[i]->managedView() && source != viewLinks[i]->managedView())
        {
            views.push_back(viewLinks[i]->managedView());
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
    updateTimeStep(m_masterView, m_masterView->currentTimeStep());

    updateScaleZ(m_masterView, m_masterView->scaleZ());
    updateCamera(m_masterView);
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
void RimViewLinker::setMasterView(RimView* view)
{
    RimViewController* previousViewLink = RimViewLinker::viewLinkForView(view);

    // Remove the view as dependent view
    if (previousViewLink)
    {
        this->viewLinks.removeChildObject(previousViewLink);
        delete previousViewLink;
    }

    this->removeOverrides();

    m_masterView = view;

    updateUiNameAndIcon();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RimViewLinker::masterView()
{
    return m_masterView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViews(std::vector<RimView*>& views)
{
    views.push_back(m_masterView());

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
    updateUiNameAndIcon();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateScaleZ(RimView* sourceView, double scaleZ)
{
    if (!isActive()) return;

    if (masterView() != sourceView)
    {
        RimViewController* sourceViewLink = viewLinkForView(sourceView);
        CVF_ASSERT(sourceViewLink);

        if (!sourceViewLink->isActive() || !sourceViewLink->syncCamera())
        {
            return;
        }
    }

    std::vector<RimView*> views;
    allViewsForCameraSync(sourceView, views);

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
    for (size_t i = 0; i < viewLinks.size(); i++)
    {
        if (!viewLinks[i]->isActive) continue;

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
        if (!viewLinks[i]->isActive) continue;

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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::findNameAndIconFromView(QString* name, QIcon* icon, RimView* view)
{
    CVF_ASSERT(name && icon);

    *name = displayNameForView(view);

    if (view)
    {
        RimCase* rimCase = NULL;
        view->firstAnchestorOrThisOfType(rimCase);

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
RimViewController* RimViewLinker::viewLinkForView(const RimView* view)
{
    RimViewController* viewLink = NULL;
    std::vector<caf::PdmObjectHandle*> reffingObjs;

    view->objectsWithReferringPtrFields(reffingObjs);
    for (size_t i = 0; i < reffingObjs.size(); ++i)
    {
        viewLink = dynamic_cast<RimViewController*>(reffingObjs[i]);
        if (viewLink) break;
    }

    return viewLink;
}

//--------------------------------------------------------------------------------------------------
/// Return view linker if view is the main view (controlling) view
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimViewLinker::viewLinkerIfMainView(RimView* view)
{
    RimViewLinker* viewLinker = NULL;
    std::vector<caf::PdmObjectHandle*> reffingObjs;

    view->objectsWithReferringPtrFields(reffingObjs);

    for (size_t i = 0; i < reffingObjs.size(); ++i)
    {
        viewLinker = dynamic_cast<RimViewLinker*>(reffingObjs[i]);
        if (viewLinker) break;
    }

    return viewLinker;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimViewLinker::viewLinkerForMainOrControlledView(RimView* view)
{
    RimViewLinker* viewLinker = RimViewLinker::viewLinkerIfMainView(view);
    if (!viewLinker)
    {
        RimViewController* viewLink = RimViewLinker::viewLinkForView(view);
        if (viewLink)
        {
            viewLinker = viewLink->ownerViewLinker();
        }
    }

    return viewLinker;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCamera(RimView* sourceView)
{
    if (!sourceView->viewer()) return;
    
    if (!isActive()) return;

    RimViewController* viewLink = sourceView->controllingViewLink();
    if (viewLink)
    {
        if ((!viewLink->isActive() || !viewLink->syncCamera()))
        {
            return;
        }
    }

    std::vector<RimView*> viewsToUpdate;
    allViewsForCameraSync(sourceView, viewsToUpdate);

    cvf::Vec3d sourceCamUp;
    cvf::Vec3d sourceCamEye;
    cvf::Vec3d sourceCamViewRefPoint;
    sourceView->viewer()->mainCamera()->toLookAt(&sourceCamEye, &sourceCamViewRefPoint, &sourceCamUp);

    cvf::Vec3d sourceCamGlobalEye = sourceCamEye;
    cvf::Vec3d sourceCamGlobalViewRefPoint = sourceCamViewRefPoint;

    // Source bounding box in global coordinates including scaleZ
    cvf::BoundingBox sourceSceneBB = sourceView->viewer()->currentScene()->boundingBox();

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(sourceView);
    if (eclipseView
        && eclipseView->eclipseCase()
        && eclipseView->eclipseCase()->reservoirData()
        && eclipseView->eclipseCase()->reservoirData()->mainGrid())
    {
        cvf::Vec3d offset = eclipseView->eclipseCase()->reservoirData()->mainGrid()->displayModelOffset();
        offset.z() *= eclipseView->scaleZ();

        sourceCamGlobalEye += offset;
        sourceCamGlobalViewRefPoint += offset;

        cvf::Mat4d trans;
        trans.setTranslation(offset);
        sourceSceneBB.transform(trans);
    }

    // Propagate view matrix to all relevant views

    const cvf::Mat4d mat = sourceView->viewer()->mainCamera()->viewMatrix();
    for (size_t i = 0; i < viewsToUpdate.size(); i++)
    {
        if (viewsToUpdate[i] && viewsToUpdate[i]->viewer())
        {
            RiuViewer* destinationViewer = viewsToUpdate[i]->viewer();

            // Destination bounding box in global coordinates including scaleZ
            cvf::BoundingBox destSceneBB = destinationViewer->currentScene()->boundingBox();

            RimEclipseView* destEclipseView = dynamic_cast<RimEclipseView*>(viewsToUpdate[i]);
            if (destEclipseView
                && destEclipseView->eclipseCase()
                && destEclipseView->eclipseCase()->reservoirData()
                && destEclipseView->eclipseCase()->reservoirData()->mainGrid())
            {
                cvf::Vec3d destOffset = destEclipseView->eclipseCase()->reservoirData()->mainGrid()->displayModelOffset();
                destOffset.z() *= destEclipseView->scaleZ();

                cvf::Vec3d destinationCamEye = sourceCamGlobalEye - destOffset;
                cvf::Vec3d destinationCamViewRefPoint = sourceCamGlobalViewRefPoint - destOffset;

                cvf::Mat4d trans;
                trans.setTranslation(destOffset);
                destSceneBB.transform(trans);

                if (isBoundingBoxesOverlappingOrClose(sourceSceneBB, destSceneBB))
                {
                    destinationViewer->mainCamera()->setFromLookAt(destinationCamEye, destinationCamViewRefPoint, sourceCamUp);
                }
                else
                {
                    // Fallback using values from source camera
                    destinationViewer->mainCamera()->setFromLookAt(sourceCamEye, sourceCamViewRefPoint, sourceCamUp);
                }
            }
            else
            {
                if (isBoundingBoxesOverlappingOrClose(sourceSceneBB, destSceneBB))
                {
                    destinationViewer->mainCamera()->setFromLookAt(sourceCamGlobalEye, sourceCamGlobalViewRefPoint, sourceCamUp);
                }
                else
                {
                    // Fallback using values from source camera
                    destinationViewer->mainCamera()->setFromLookAt(sourceCamEye, sourceCamViewRefPoint, sourceCamUp);
                }
            }

            destinationViewer->update();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimViewLinker::isBoundingBoxesOverlappingOrClose(const cvf::BoundingBox& sourceBB, const cvf::BoundingBox& destBB)
{
    if (!sourceBB.isValid() || !destBB.isValid()) return false;

    if (sourceBB.intersects(destBB)) return true;

    double largestExtent = sourceBB.extent().length();
    if (destBB.extent().length() > largestExtent)
    {
        largestExtent = destBB.extent().length();
    }

    double centerDist = (sourceBB.center() - destBB.center()).length();
    if (centerDist < largestExtent * 5)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::addDependentView(RimView* view)
{
    CVF_ASSERT(view && view != m_masterView);
     
    RimViewController* viewLink = new RimViewController;
    this->viewLinks.push_back(viewLink);

    viewLink->setManagedView(view);

    viewLink->initAfterReadRecursively();
    viewLink->updateOptionSensitivity();
    viewLink->updateDisplayNameAndIcon();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinker::addViewControllers(caf::PdmUiTreeOrdering& uiTreeOrdering)
{
    for (size_t j = 0; j < viewLinks.size(); j++)
    {
        uiTreeOrdering.add(viewLinks()[j]);
    }
}

