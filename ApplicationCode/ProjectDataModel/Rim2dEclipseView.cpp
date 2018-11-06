/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "Rim2dEclipseView.h"

#include "Riv2dGridProjectionPartMgr.h"
#include "RiuViewer.h"

#include "Rim2dGridProjection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimFaultInViewCollection.h"
#include "RimGridCollection.h"
#include "RimSimWellInViewCollection.h"

#include "cafPdmUiTreeOrdering.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"

CAF_PDM_SOURCE_INIT(Rim2dEclipseView, "Rim2dEclipseView");

const cvf::Mat4d defaultViewMatrix(1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 1000,
    0, 0, 0, 1);

Rim2dEclipseView::Rim2dEclipseView()
{
    CAF_PDM_InitObject("2d Contour Map", ":/2DMap16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_2dGridProjection, "Grid2dProjection", "2d Grid Projection", "", "", "");
    m_2dGridProjection = new Rim2dGridProjection();

    CAF_PDM_InitField(&m_showAxisLines, "ShowAxisLines", true, "Show Axis Lines", "", "", "");

    m_overlayInfoConfig->setIsActive(false);
    m_gridCollection->setActive(false); // This is also not added to the tree view, so cannot be enabled.
    wellCollection()->isActive = false;
    faultCollection()->showFaultCollection = false;    

    m_grid2dProjectionPartMgr = new Riv2dGridProjectionPartMgr(grid2dProjection(), this);
    
    ((RiuViewerToViewInterface*)this)->setCameraPosition(defaultViewMatrix);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dGridProjection* Rim2dEclipseView::grid2dProjection() const
{
    return m_2dGridProjection().p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::initAfterRead()
{
    m_gridCollection->setActive(false); // This is also not added to the tree view, so cannot be enabled.
    disablePerspectiveProjectionField();
    setShowGridBox(false);
    meshMode.setValue(NO_MESH);
    surfaceMode.setValue(FAULTS);
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::createDisplayModel()
{
    RimEclipseView::createDisplayModel();

    if (this->viewer()->mainCamera()->viewMatrix() == defaultViewMatrix)
    {
        this->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup("Viewer");
    viewGroup->add(this->userDescriptionField());
    viewGroup->add(this->backgroundColorField());
    viewGroup->add(&m_showAxisLines);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(m_overlayInfoConfig());
    uiTreeOrdering.add(m_2dGridProjection);
    uiTreeOrdering.add(cellResult());
    cellResult()->uiCapability()->setUiReadOnly(m_2dGridProjection->isSummationResult());
    uiTreeOrdering.add(wellCollection());
    uiTreeOrdering.add(faultCollection());
    uiTreeOrdering.add(m_rangeFilterCollection());
    uiTreeOrdering.add(nonOverridePropertyFilterCollection());

    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::updateCurrentTimeStep()
{
    if (m_2dGridProjection->isChecked())
    {
        m_2dGridProjection->generateResults();
    }

    static_cast<RimEclipsePropertyFilterCollection*>(nonOverridePropertyFilterCollection())->updateFromCurrentTimeStep();

    updateLegends(); // To make sure the scalar mappers are set up correctly

    if (m_viewer && m_2dGridProjection->isChecked())
    {
        cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);

        cvf::String name = "Grid2dProjection";
        this->removeModelByName(frameScene, name);

        cvf::ref<cvf::ModelBasicList> grid2dProjectionModelBasicList = new cvf::ModelBasicList;
        grid2dProjectionModelBasicList->setName(name);

        cvf::ref<caf::DisplayCoordTransform> transForm = this->displayCoordTransform();

        m_grid2dProjectionPartMgr->appendProjectionToModel(grid2dProjectionModelBasicList.p(), transForm.p());
        grid2dProjectionModelBasicList->updateBoundingBoxesRecursive();
        frameScene->addModel(grid2dProjectionModelBasicList.p());

        if (m_overlayInfoConfig->isActive())
        {
            m_overlayInfoConfig()->update3DInfo();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::updateLegends()
{
    if (m_viewer)
    {
        m_viewer->removeAllColorLegends();

        if (m_2dGridProjection && m_2dGridProjection->isChecked())
        {
            RimRegularLegendConfig* projectionLegend = m_2dGridProjection->legendConfig();
            if (projectionLegend)
            {
                m_2dGridProjection->updateLegend();
                if (projectionLegend->showLegend())
                {
                    m_viewer->addColorLegendToBottomLeftCorner(projectionLegend->titledOverlayFrame());
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::updateViewWidgetAfterCreation()
{
    if (m_viewer)
    {
        m_viewer->showAxisCross(false);
        m_viewer->showEdgeTickMarksXY(true, m_showAxisLines());
        m_viewer->enableNavigationRotation(false);
    }

    Rim3dView::updateViewWidgetAfterCreation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::updateViewFollowingRangeFilterUpdates()
{
    m_2dGridProjection->setCheckState(true);
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::onLoadDataAndUpdate()
{
    RimEclipseView::onLoadDataAndUpdate();
    if (m_viewer)
    {
        m_viewer->setView(cvf::Vec3d(0, 0, -1), cvf::Vec3d(0, 1, 0));
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim2dEclipseView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseView::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_showAxisLines)
    {
        m_viewer->showEdgeTickMarksXY(true, m_showAxisLines());
        scheduleCreateDisplayModelAndRedraw();
    }
    else if (changedField == backgroundColorField())
    {
        scheduleCreateDisplayModelAndRedraw();
    }
}
