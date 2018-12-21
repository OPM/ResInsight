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

#include "RimContourMapView.h"

#include "RivContourMapProjectionPartMgr.h"
#include "RiuViewer.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCase.h"
#include "RimCellRangeFilterCollection.h"
#include "RimContourMapNameConfig.h"
#include "RimContourMapProjection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimFaultInViewCollection.h"
#include "RimGridCollection.h"
#include "RimSimWellInViewCollection.h"

#include "cafPdmUiTreeOrdering.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"

CAF_PDM_SOURCE_INIT(RimContourMapView, "RimContourMapView");

const cvf::Mat4d defaultViewMatrix(1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 1000,
    0, 0, 0, 1);

RimContourMapView::RimContourMapView()
{
    CAF_PDM_InitObject("Contour Map View", ":/2DMap16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_contourMapProjection, "ContourMapProjection", "Contour Map Projection", "", "", "");
    m_contourMapProjection = new RimContourMapProjection();

    CAF_PDM_InitField(&m_showAxisLines, "ShowAxisLines", true, "Show Axis Lines", "", "", "");

    m_gridCollection->setActive(false); // This is also not added to the tree view, so cannot be enabled.
    setFaultVisParameters();

    CAF_PDM_InitFieldNoDefault(&m_nameConfig, "NameConfig", "", "", "", "");
    m_nameConfig = new RimContourMapNameConfig(this);

    m_contourMapProjectionPartMgr = new RivContourMapProjectionPartMgr(contourMapProjection(), this);
    
    ((RiuViewerToViewInterface*)this)->setCameraPosition(defaultViewMatrix);

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection* RimContourMapView::contourMapProjection() const
{
    return m_contourMapProjection().p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimContourMapView::createAutoName() const
{
    QStringList autoName;

    if (!m_nameConfig->customName().isEmpty())
    {
        autoName.push_back(m_nameConfig->customName());
    }

    QStringList generatedAutoTags;

    RimCase* ownerCase = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(ownerCase);

    if (m_nameConfig->addCaseName())
    {
        generatedAutoTags.push_back(ownerCase->caseUserDescription());
    }

    if (m_nameConfig->addAggregationType())
    {
        generatedAutoTags.push_back(contourMapProjection()->resultAggregationText());
    }

    if (m_nameConfig->addProperty() && !contourMapProjection()->isColumnResult())
    {
        generatedAutoTags.push_back(cellResult()->resultVariable());
    }

    if (m_nameConfig->addSampleSpacing())
    {
        generatedAutoTags.push_back(QString("%1").arg(contourMapProjection()->sampleSpacingFactor(), 3, 'f', 2));
    }

    if (!generatedAutoTags.empty())
    {
        autoName.push_back(generatedAutoTags.join(", "));
    }
    return autoName.join(": ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::initAfterRead()
{
    m_gridCollection->setActive(false); // This is also not added to the tree view, so cannot be enabled.
    disablePerspectiveProjectionField();
    setShowGridBox(false);
    meshMode.setValue(NO_MESH);
    surfaceMode.setValue(FAULTS);
    setFaultVisParameters();
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::createDisplayModel()
{
    RimEclipseView::createDisplayModel();
    
    if (!this->isTimeStepDependentDataVisible())
    {
        // Need to add geometry even if it hasn't happened during dynamic time step update.
        updateGeometry();
    }

    if (this->viewer()->mainCamera()->viewMatrix() == defaultViewMatrix)
    {
        this->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* viewGroup = uiOrdering.addNewGroup("Viewer");
    viewGroup->add(this->userDescriptionField());
    viewGroup->add(this->backgroundColorField());
    viewGroup->add(&m_showAxisLines);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Contour Map Name");
    m_nameConfig->uiOrdering(uiConfigName, *nameGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(m_overlayInfoConfig());
    uiTreeOrdering.add(m_contourMapProjection);
    uiTreeOrdering.add(cellResult());
    cellResult()->uiCapability()->setUiReadOnly(m_contourMapProjection->isColumnResult());
    uiTreeOrdering.add(wellCollection());
    uiTreeOrdering.add(faultCollection());
    uiTreeOrdering.add(m_rangeFilterCollection());
    uiTreeOrdering.add(nativePropertyFilterCollection());

    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::updateCurrentTimeStep()
{
    static_cast<RimEclipsePropertyFilterCollection*>(nativePropertyFilterCollection())->updateFromCurrentTimeStep();
    updateGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::updateGeometry()
{
    this->updateVisibleGeometriesAndCellColors();

    if (m_contourMapProjection->isChecked())
    {
        m_contourMapProjection->generateResults();
    }
    updateLegends(); // To make sure the scalar mappers are set up correctly

    appendWellsAndFracturesToModel();

    appendContourMapProjectionToModel();

    appendPickPointVisToModel();

     m_overlayInfoConfig()->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::setFaultVisParameters()
{
    faultCollection()->setShowFaultsOutsideFilter(false);
    faultCollection()->showOppositeFaultFaces = true;
    faultCollection()->faultResult            = RimFaultInViewCollection::FAULT_NO_FACE_CULLING;
    faultResultSettings()->showCustomFaultResult = true;
    faultResultSettings()->customFaultResult()->setResultVariable("None");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::appendContourMapProjectionToModel()
{
    if (m_viewer && m_contourMapProjection->isChecked())
    {
        cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
        if (frameScene)
        {
            cvf::String name = "ContourMapProjection";
            this->removeModelByName(frameScene, name);

            cvf::ref<cvf::ModelBasicList> contourMapProjectionModelBasicList = new cvf::ModelBasicList;
            contourMapProjectionModelBasicList->setName(name);

            cvf::ref<caf::DisplayCoordTransform> transForm = this->displayCoordTransform();

            m_contourMapProjectionPartMgr->appendProjectionToModel(contourMapProjectionModelBasicList.p(), transForm.p());
            contourMapProjectionModelBasicList->updateBoundingBoxesRecursive();
            frameScene->addModel(contourMapProjectionModelBasicList.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::appendPickPointVisToModel()
{
    if (m_viewer && m_contourMapProjection->isChecked())
    {
        cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
        if (frameScene)
        {
            cvf::String name = "ContourMapPickPoint";
            this->removeModelByName(frameScene, name);

            cvf::ref<cvf::ModelBasicList> contourMapProjectionModelBasicList = new cvf::ModelBasicList;
            contourMapProjectionModelBasicList->setName(name);

            cvf::ref<caf::DisplayCoordTransform> transForm = this->displayCoordTransform();

            m_contourMapProjectionPartMgr->appendPickPointVisToModel(contourMapProjectionModelBasicList.p(), transForm.p());
            contourMapProjectionModelBasicList->updateBoundingBoxesRecursive();
            frameScene->addModel(contourMapProjectionModelBasicList.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::updateLegends()
{
    if (m_viewer)
    {
        m_viewer->removeAllColorLegends();

        if (m_contourMapProjection && m_contourMapProjection->isChecked())
        {
            RimRegularLegendConfig* projectionLegend = m_contourMapProjection->legendConfig();
            if (projectionLegend)
            {
                m_contourMapProjection->updateLegend();
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
void RimContourMapView::updateViewWidgetAfterCreation()
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
void RimContourMapView::updateViewFollowingRangeFilterUpdates()
{
    m_contourMapProjection->setCheckState(true);
    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapView::onLoadDataAndUpdate()
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
void RimContourMapView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimContourMapView::userDescriptionField()
{
    return m_nameConfig()->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RivCellSetEnum> RimContourMapView::allVisibleFaultGeometryTypes() const
{
    std::set<RivCellSetEnum> faultGeoTypes;
    // Normal eclipse views always shows faults for active and visible eclipse cells.
    if (faultCollection()->showFaultCollection())
    {
        faultGeoTypes = RimEclipseView::allVisibleFaultGeometryTypes();
    }
    return faultGeoTypes;
}
