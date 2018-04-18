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
#include "RimCase.h"
#include "RimIntersection.h"
#include "RimGridView.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"
#include "RimGeoMechView.h"
#include "RimGeoMechCellColors.h"
#include "RimRegularLegendConfig.h"
#include "RimTernaryLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"

#include "RiuViewer.h"

#include "RivIntersectionPartMgr.h"
#include "RivTernarySaturationOverlayItem.h"
#include "RivSimWellPipesPartMgr.h"
#include "RivWellHeadPartMgr.h"
#include "RivWellPathPartMgr.h"

#include "cafDisplayCoordTransform.h"

#include "cvfModelBasicList.h"
#include "cvfTransform.h"
#include "cvfScene.h"
#include "cvfPart.h"

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
    m_legendConfig = new RimRegularLegendConfig();

    CAF_PDM_InitFieldNoDefault(&m_ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Legend Definition", "", "", "");
    m_ternaryLegendConfig.uiCapability()->setUiTreeHidden(true);
    m_ternaryLegendConfig.uiCapability()->setUiTreeChildrenHidden(true);
    m_ternaryLegendConfig.xmlCapability()->disableIO();
    m_ternaryLegendConfig = new RimTernaryLegendConfig();

    CAF_PDM_InitField(&m_showDefiningPoints, "ShowDefiningPoints", true, "Show Points", "", "", "");

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
RimIntersection* Rim2dIntersectionView::intersection() const
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
void Rim2dIntersectionView::update3dInfo()
{
    if (!m_viewer) return;

    QString overlayInfoText;

    RimEclipseView * eclView = nullptr;
    m_intersection->firstAncestorOrThisOfType(eclView);
    if (eclView && !eclView->overlayInfoConfig()->isActive())
    {
        m_viewer->showInfoText(false);
        m_viewer->showHistogram(false);
        m_viewer->showAnimationProgress(false);

        m_viewer->update();
        return;
    }
    if (eclView && eclView->overlayInfoConfig()->showCaseInfo())
    {
        overlayInfoText += "<b>--" + ownerCase()->caseUserDescription() + "--</b>";
    }

    RimGeoMechView * geoView = nullptr;
    m_intersection->firstAncestorOrThisOfType(geoView);
    if (geoView && geoView->overlayInfoConfig()->showCaseInfo())
    {
        overlayInfoText += "<b>--" + ownerCase()->caseUserDescription() + "--</b>";
    }

    overlayInfoText += "<p>";

    overlayInfoText += "<b>Z-scale:</b> " + QString::number(scaleZ()) + "<br> ";

    if (m_intersection->simulationWell())
    {
        overlayInfoText += "<b>Simulation Well:</b> " + m_intersection->simulationWell()->name() + "<br>";
    }
    else if (m_intersection->wellPath())
    {
        overlayInfoText += "<b>Well Path:</b> " + m_intersection->wellPath()->name() + "<br>";
    }
    else
    {
        overlayInfoText += "<b>Intersection:</b> " + m_intersection->name() + "<br>";
    }

    if (eclView)
    {
        if (eclView->overlayInfoConfig()->showAnimProgress())
        {
            m_viewer->showAnimationProgress(true);
        }
        else
        {
            m_viewer->showAnimationProgress(false);
        }

        if (eclView->overlayInfoConfig()->showResultInfo())
        {
            overlayInfoText += "<b>Cell Result:</b> " + eclView->cellResult()->resultVariableUiShortName() + "<br>";
        }
    }

    if (geoView)
    {
        if (geoView->overlayInfoConfig()->showAnimProgress())
        {
            m_viewer->showAnimationProgress(true);
        }
        else
        {
            m_viewer->showAnimationProgress(false);
        }

        if (geoView->overlayInfoConfig()->showResultInfo())
        {
            QString resultPos;
            QString fieldName = geoView->cellResultResultDefinition()->resultFieldUiName();
            QString compName = geoView->cellResultResultDefinition()->resultComponentUiName();

            switch (geoView->cellResultResultDefinition()->resultPositionType())
            {
            case RIG_NODAL:
                resultPos = "Nodal";
                break;

            case RIG_ELEMENT_NODAL:
                resultPos = "Element nodal";
                break;

            case RIG_INTEGRATION_POINT:
                resultPos = "Integration point";
                break;

            case RIG_ELEMENT:
                resultPos = "Element";
                break;
            default:
                break;
            }
            if (compName == "")
            {
                overlayInfoText += QString("<b>Cell result:</b> %1, %2<br>").arg(resultPos).arg(fieldName);
            }
            else
            {
                overlayInfoText += QString("<b>Cell result:</b> %1, %2, %3<br>").arg(resultPos).arg(fieldName).arg(compName);
            }
        }
    }


    overlayInfoText += "</p>";
    m_viewer->setInfoText(overlayInfoText);
    m_viewer->showInfoText(true);
    m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RivIntersectionPartMgr> Rim2dIntersectionView::flatIntersectionPartMgr() const
{
    return m_flatIntersectionPartMgr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim2dIntersectionView::isGridVisualizationMode() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d Rim2dIntersectionView::transformToUtm(const cvf::Vec3d& unscaledPointInFlatDomain) const
{
    cvf::Mat4d unflatXf = this->flatIntersectionPartMgr()->unflattenTransformMatrix(unscaledPointInFlatDomain);

    return unscaledPointInFlatDomain.getTransformedPoint(unflatXf);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<caf::DisplayCoordTransform> Rim2dIntersectionView::displayCoordTransform() const
{
   cvf::ref<caf::DisplayCoordTransform> dispTx = new caf::DisplayCoordTransform();
   dispTx->setScale(cvf::Vec3d(1.0, 1.0, scaleZ()));
   return dispTx;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim2dIntersectionView::showDefiningPoints() const
{
    return m_showDefiningPoints;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimLegendConfig*> Rim2dIntersectionView::legendConfigs() const
{
    std::vector<RimLegendConfig*> legendsIn3dView;

    Rim3dView* associated3dView = nullptr;
    this->firstAncestorOrThisOfType(associated3dView);

    if (associated3dView)
    {
        legendsIn3dView = associated3dView->legendConfigs();
    }

    return legendsIn3dView;
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
    return m_showWindow();
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

    m_flatIntersectionPartMgr = new RivIntersectionPartMgr(m_intersection(), true);


    m_intersectionVizModel->removeAllParts();
    
    m_flatIntersectionPartMgr->appendNativeCrossSectionFacesToModel(m_intersectionVizModel.p(), scaleTransform());
    m_flatIntersectionPartMgr->appendMeshLinePartsToModel(m_intersectionVizModel.p(), scaleTransform());
    m_flatIntersectionPartMgr->appendPolylinePartsToModel(*this, m_intersectionVizModel.p(), scaleTransform());

    m_flatIntersectionPartMgr->applySingleColorEffect();

    m_flatSimWellPipePartMgr = nullptr;
    m_flatWellHeadPartMgr = nullptr;

    if ( m_intersection->type() == RimIntersection::CS_SIMULATION_WELL
        && m_intersection->simulationWell() )
    {
        RimEclipseView* eclipseView = nullptr;
        m_intersection->firstAncestorOrThisOfType(eclipseView);

        if ( eclipseView )
        {
            m_flatSimWellPipePartMgr = new RivSimWellPipesPartMgr(m_intersection->simulationWell()); 
            m_flatWellHeadPartMgr = new RivWellHeadPartMgr(m_intersection->simulationWell());
        }
    }

    m_flatWellpathPartMgr = nullptr;
    if ( m_intersection->type() == RimIntersection::CS_WELL_PATH
        && m_intersection->wellPath() )
    {
        Rim3dView* settingsView = nullptr;
        m_intersection->firstAncestorOrThisOfType(settingsView);
        if ( settingsView )
        {
            m_flatWellpathPartMgr = new RivWellPathPartMgr(m_intersection->wellPath(), settingsView);
            m_flatWellpathPartMgr->appendFlattenedStaticGeometryPartsToModel(m_intersectionVizModel.p(),
                                                                             this->displayCoordTransform().p(),
                                                                             this->ownerCase()->characteristicCellSize(),
                                                                             this->ownerCase()->activeCellsBoundingBox());
        }
    }

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
void Rim2dIntersectionView::updateCurrentTimeStep()
{
    update3dInfo();
    updateLegends();

    if ( m_flatSimWellPipePartMgr.notNull() )
    {
        cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
        if (frameScene)
        {
            {
                cvf::String name = "SimWellPipeMod";
                Rim3dView::removeModelByName(frameScene, name);

                cvf::ref<cvf::ModelBasicList> simWellModelBasicList = new cvf::ModelBasicList;
                simWellModelBasicList->setName(name);

                m_flatSimWellPipePartMgr->appendFlattenedDynamicGeometryPartsToModel(simWellModelBasicList.p(),
                                                                                     m_currentTimeStep,
                                                                                     this->displayCoordTransform().p(),
                                                                                     m_intersection->extentLength(), 
                                                                                     m_intersection->branchIndex());

                for ( double offset : m_flatSimWellPipePartMgr->flattenedBranchWellHeadOffsets() )
                {
                    m_flatWellHeadPartMgr->appendFlattenedDynamicGeometryPartsToModel(simWellModelBasicList.p(),
                                                                                      m_currentTimeStep,
                                                                                      this->displayCoordTransform().p(), 
                                                                                      offset);
                }

                simWellModelBasicList->updateBoundingBoxesRecursive();
                frameScene->addModel(simWellModelBasicList.p());
                m_flatSimWellPipePartMgr->updatePipeResultColor(m_currentTimeStep);
            }
        }
    }

    if ( m_flatWellpathPartMgr.notNull() )
    {
        cvf::Scene* frameScene = m_viewer->frame(m_currentTimeStep);
        if (frameScene)
        {
            {
                cvf::String name = "WellPipeDynMod";
                Rim3dView::removeModelByName(frameScene, name);
                cvf::ref<cvf::ModelBasicList> dynWellPathModel = new cvf::ModelBasicList;
                dynWellPathModel->setName(name);

                m_flatWellpathPartMgr->appendFlattenedDynamicGeometryPartsToModel(dynWellPathModel.p(),
                                                                                  m_currentTimeStep,
                                                                                  this->displayCoordTransform().p(),
                                                                                  this->ownerCase()->characteristicCellSize(),
                                                                                  this->ownerCase()->activeCellsBoundingBox());
                dynWellPathModel->updateBoundingBoxesRecursive();
                frameScene->addModel(dynWellPathModel.p());
            }
        }
    }


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
    if (!m_viewer) return; 
    
    m_viewer->removeAllColorLegends();

    if (!hasResults()) return;

    RimEclipseView * eclView = nullptr;
    m_intersection->firstAncestorOrThisOfType(eclView);

    RimGeoMechView * geoView = nullptr;
    m_intersection->firstAncestorOrThisOfType(geoView);

    caf::TitledOverlayFrame* legend = nullptr;

    if (eclView)
    {
        m_legendConfig()->setUiValuesFromLegendConfig(eclView->cellResult()->legendConfig());
        m_ternaryLegendConfig()->setUiValuesFromLegendConfig(eclView->cellResult()->ternaryLegendConfig());
        eclView->cellResult()->updateLegendData(m_currentTimeStep(), m_legendConfig(), m_ternaryLegendConfig());

        if ( eclView->cellResult()->isTernarySaturationSelected() )
        {
            m_ternaryLegendConfig()->setTitle("Cell Result:\n");
            legend = m_ternaryLegendConfig()->legend();
        }
        else
        {
            m_legendConfig()->setTitle("Cell Result:\n" + eclView->cellResult()->resultVariableUiShortName());
            legend = m_legendConfig()->legend();
        }
    }

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
    m_viewer->showEdgeTickMarks(true);

    m_viewer->setMainScene(new cvf::Scene());
    m_viewer->enableNavigationRotation(false);

    m_ternaryLegendConfig()->recreateLegend();
    m_legendConfig()->recreateLegend();
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

    if (changedField == & m_intersection ||
        changedField == &m_showDefiningPoints)
    {
        this->loadDataAndUpdate();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim2dIntersectionView::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    Rim3dView::defineUiOrdering(uiConfigName, uiOrdering);
    uiOrdering.skipRemainingFields(true);

    if (m_intersection->hasDefiningPoints())
    {
        caf::PdmUiGroup* plGroup = uiOrdering.addNewGroup("Defining Points");
        plGroup->add(&m_showDefiningPoints);
    }
}
