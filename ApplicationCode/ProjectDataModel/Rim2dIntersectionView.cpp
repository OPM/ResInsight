/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimIntersection.h"
#include "RimCase.h"
#include "RiuViewer.h"
#include "RimGridView.h"
#include "RivIntersectionPartMgr.h"
#include "RivTernarySaturationOverlayItem.h"

#include "cvfPart.h"
#include "cvfModelBasicList.h"
#include "cvfTransform.h"
#include "cvfScene.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"
#include "RimGeoMechView.h"
#include "RimGeoMechCellColors.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include <QDateTime>

CAF_PDM_SOURCE_INIT(Rim2dIntersectionView, "Intersection2dView"); 

const cvf::Mat4d defaultIntersectinoViewMatrix(1, 0, 0, 0,
                                               0, 0, 1, 0,
                                               0, -1, 0, 1000,
                                               0, 0, 0, 1);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionView::Rim2dIntersectionView(void)
{
    CAF_PDM_InitObject("Intersection View", ":/CrossSection16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_intersection,  "Intersection", "Intersection", ":/CrossSection16x16.png", "", "");
    m_intersection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_legendConfig,  "LegendDefinition", "Legend Definition", "", "", "");
    m_legendConfig.uiCapability()->setUiHidden(true);
    m_legendConfig.uiCapability()->setUiTreeChildrenHidden(true);
    m_legendConfig.xmlCapability()->disableIO();
    m_legendConfig = new RimLegendConfig();

    CAF_PDM_InitFieldNoDefault(&m_ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Legend Definition", "", "", "");
    m_ternaryLegendConfig.uiCapability()->setUiTreeHidden(true);
    m_ternaryLegendConfig.uiCapability()->setUiTreeChildrenHidden(true);
    m_ternaryLegendConfig.xmlCapability()->disableIO();
    m_ternaryLegendConfig = new RimTernaryLegendConfig();

    m_showWindow = false;
    m_scaleTransform = new cvf::Transform();
    m_intersectionVizModel = new cvf::ModelBasicList;

    hasUserRequestedAnimation = true;
    
    ((RiuViewerToViewInterface*)this)->setCameraPosition(defaultIntersectinoViewMatrix );

    disableGridBoxField();
    disablePerspectiveProjectionField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionView::~Rim2dIntersectionView(void)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::setVisible(bool isVisible)
{
    m_showWindow = isVisible;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::setIntersection(RimIntersection* intersection)
{
    CAF_ASSERT(intersection);

    m_intersection = intersection;
    Rim3dView * parentView = nullptr;
    intersection->firstAncestorOrThisOfTypeAsserted(parentView);
    name = parentView->name() + ": " + intersection->name();

    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersection* Rim2dIntersectionView::intersection()
{
    return m_intersection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim2dIntersectionView::isUsingFormationNames() const
{
  // Todo:

   return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::scheduleGeometryRegen(RivCellSetEnum geometryType)
{
    m_flatIntersectionPartMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* Rim2dIntersectionView::ownerCase() const
{
    RimCase* rimCase = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(rimCase);
    return rimCase;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim2dIntersectionView::isTimeStepDependentDataVisible() const
{
    if ( m_intersection() )
    {
        RimGridView * gridView = nullptr;
        m_intersection->firstAncestorOrThisOfTypeAsserted(gridView);
        return gridView->isTimeStepDependentDataVisible();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim2dIntersectionView::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                           bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options; 

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim2dIntersectionView::hasResults()
{
    if (!m_intersection()) return false;

    RimEclipseView * eclView = nullptr;
    m_intersection->firstAncestorOrThisOfType(eclView);
    if (eclView)
    {
        return (eclView->cellResult()->hasResult() || eclView->cellResult()->isTernarySaturationSelected());
    }

    RimGeoMechView * geoView = nullptr;
    m_intersection->firstAncestorOrThisOfType(geoView);
    if (geoView)
    {
        return geoView->cellResult()->hasResult();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int Rim2dIntersectionView::timeStepCount()
{
    if ( isTimeStepDependentDataVisible() )
    {
        return static_cast<int>( this->ownerCase()->timeStepStrings().size());
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim2dIntersectionView::isWindowVisible()
{
    if (m_showWindow())
    {
        Rim2dIntersectionViewCollection* viewColl = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(viewColl);
        return viewColl->isActive();
    }
    else 
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel)
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::createDisplayModel()
{
    if (m_viewer.isNull()) return;
    if (!m_intersection()) return;

    updateScaleTransform();

    m_viewer->removeAllFrames();

    int tsCount = this->timeStepCount();

    for (int i = 0; i < tsCount; ++i)
    {
        m_viewer->addFrame(new cvf::Scene());
    }


    if ( m_flatIntersectionPartMgr.isNull() || m_intersection() != m_flatIntersectionPartMgr->intersection())
    {
        m_flatIntersectionPartMgr = new RivIntersectionPartMgr(m_intersection(), true);
    }

    m_intersectionVizModel->removeAllParts();
    
    m_flatIntersectionPartMgr->appendNativeCrossSectionFacesToModel(m_intersectionVizModel.p(), scaleTransform());
    m_flatIntersectionPartMgr->appendMeshLinePartsToModel(m_intersectionVizModel.p(), scaleTransform());
    m_flatIntersectionPartMgr->appendPolylinePartsToModel(m_intersectionVizModel.p(), scaleTransform());
    m_flatIntersectionPartMgr->appendWellPipePartsToModel(m_intersectionVizModel.p(), scaleTransform());

    m_flatIntersectionPartMgr->applySingleColorEffect();

    m_viewer->addStaticModelOnce(m_intersectionVizModel.p());
    
    m_intersectionVizModel->updateBoundingBoxesRecursive();

    if ( this->hasUserRequestedAnimation() )
    {
        m_viewer->setCurrentFrame(m_currentTimeStep);
        updateCurrentTimeStep();
    }

    if ( this->viewer()->mainCamera()->viewMatrix() == defaultIntersectinoViewMatrix )
    {
        this->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts)
{
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::onTimeStepChanged()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::clampCurrentTimestep()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::updateCurrentTimeStep()
{
    updateLegends();

    if ((this->hasUserRequestedAnimation() && this->hasResults()))
    {
        m_flatIntersectionPartMgr->updateCellResultColor(m_currentTimeStep, 
                                                         m_legendConfig->scalarMapper(), 
                                                         m_ternaryLegendConfig()->scalarMapper());
    }
    else
    {
        m_flatIntersectionPartMgr->applySingleColorEffect();
    }

}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::updateLegends()
{
    if (m_viewer)
    {
        m_viewer->removeAllColorLegends();
    }

    if (!hasResults()) return ;

    cvf::OverlayItem* legend = nullptr;

    RimEclipseView * eclView = nullptr;
    m_intersection->firstAncestorOrThisOfType(eclView);
    if (eclView)
    {
        m_legendConfig()->setUiValuesFromLegendConfig(eclView->cellResult()->legendConfig());
        m_ternaryLegendConfig()->setUiValuesFromLegendConfig(eclView->cellResult()->ternaryLegendConfig());
        eclView->cellResult()->updateLegendData(m_currentTimeStep(), m_legendConfig(), m_ternaryLegendConfig());

        if ( eclView->cellResult()->isTernarySaturationSelected() )
        {
            m_ternaryLegendConfig()->setTitle("Cell Result:");
            legend = m_ternaryLegendConfig()->legend();
        }
        else
        {
            m_legendConfig()->setTitle("Cell Result:" + eclView->cellResult()->resultVariableUiShortName());
            legend = m_legendConfig()->legend();
        }
    }

    RimGeoMechView * geoView = nullptr;
    m_intersection->firstAncestorOrThisOfType(geoView);
    if (geoView)
    {
        m_legendConfig()->setUiValuesFromLegendConfig(geoView->cellResult()->legendConfig());
          
        geoView->updateLegendTextAndRanges(m_legendConfig(), m_currentTimeStep());
        legend = m_legendConfig()->legend();
    }

    if ( legend )
    {
        m_viewer->addColorLegendToBottomLeftCorner(legend);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::resetLegendsInViewer()
{
    m_viewer->showAxisCross(false);
    m_viewer->showAnimationProgress(true);
    m_viewer->showHistogram(false);
    m_viewer->showInfoText(false);

    m_viewer->setMainScene(new cvf::Scene());
    m_viewer->enableNavigationRotation(false);

    m_ternaryLegendConfig()->recreateLegend();
    m_legendConfig()->recreateLegend();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::updateStaticCellColors()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::updateScaleTransform()
{
    cvf::Mat4d scale = cvf::Mat4d::IDENTITY;
    scale(2, 2) = scaleZ();

    this->scaleTransform()->setLocalTransform(scale);

    if (m_viewer) m_viewer->updateCachedValuesInScene();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Transform* Rim2dIntersectionView::scaleTransform()
{
   return m_scaleTransform.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    this->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    Rim3dView::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == & m_intersection)
    {
        this->loadDataAndUpdate();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_intersection);

    Rim3dView::defineUiOrdering(uiConfigName, uiOrdering);
}
