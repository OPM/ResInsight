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
#include "RimIntersection.h"
#include "RimCase.h"
#include "RiuViewer.h"
#include "RimGridView.h"
#include "RivIntersectionPartMgr.h"

#include "cvfPart.h"
#include "cvfModelBasicList.h"
#include "cvfTransform.h"
#include "cvfScene.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT(Rim2dIntersectionView, "Intersection2dView"); 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionView::Rim2dIntersectionView(void)
{
    CAF_PDM_InitObject("Intersection View", ":/CrossSection16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_intersection,  "Intersection", "Intersection", ":/CrossSection16x16.png", "", "");

    m_scaleTransform = new cvf::Transform();
    m_intersectionVizModel = new cvf::ModelBasicList;
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
void Rim2dIntersectionView::setIntersection(RimIntersection* intersection)
{
    m_intersection = intersection;
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
QList<caf::PdmOptionItemInfo> Rim2dIntersectionView::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                           bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options; 
    if (fieldNeedingOptions == &m_intersection)
    {
        std::vector<RimIntersection*> intersections; 
         
        this->ownerCase()->descendantsIncludingThisOfType(intersections);

        for (auto intersection : intersections)
        {
            options.push_back(caf::PdmOptionItemInfo(intersection->name(), intersection));
        }
    }

    return options;
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
    
    m_intersectionVizModel->removeAllParts();
    
    m_intersection()->intersectionPartMgr()->appendNativeCrossSectionFacesToModel(m_intersectionVizModel.p(), scaleTransform());
    m_intersection()->intersectionPartMgr()->appendMeshLinePartsToModel(m_intersectionVizModel.p(), scaleTransform());

    m_viewer->addStaticModelOnce(m_intersectionVizModel.p());

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
void Rim2dIntersectionView::updateDisplayModelVisibility()
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
void Rim2dIntersectionView::resetLegendsInViewer()
{
    m_viewer->showAxisCross(false);
    m_viewer->showAnimationProgress(false);
    m_viewer->setMainScene(new cvf::Scene());
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
