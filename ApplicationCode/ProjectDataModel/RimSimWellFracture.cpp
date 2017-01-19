/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimSimWellFracture.h"

#include "RiaApplication.h"

#include "RigMainGrid.h"
#include "RigTesselatorTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureDefinitionCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmUiItem.h"
#include "cafDisplayCoordTransform.h"

#include "cvfVector3.h"

#include <QToolBox>
#include <QList>
#include "RigGridBase.h"



CAF_PDM_SOURCE_INIT(RimSimWellFracture, "SimWellFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::RimSimWellFracture(void)
{
    CAF_PDM_InitObject("SimWellFracture", ":/FractureSymbol16x16.png", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::~RimSimWellFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSimWellFracture::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    return RimFracture::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
}


// --------------------------------------------------------------------------------------------------
// / 
// --------------------------------------------------------------------------------------------------
// std::vector<std::pair<size_t, size_t>> RimSimWellFracture::getFracturedCells()
// {
//     std::vector<std::pair<size_t, size_t>> cells;
// 
//     size_t gridindex = 0; //TODO! For now assuming only one grid
// 
// 
//     caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(this);
//     if (!objHandle) return cells;
// 
//     RimEclipseView* mainView = nullptr;
//     objHandle->firstAncestorOrThisOfType(mainView);
//     if (!mainView) return cells;
// 
//     const RigMainGrid* mainGrid = mainView->mainGrid(); 
//     if (!mainGrid) return cells;
// 
//     size_t cellIndex = mainGrid->cellIndexFromIJK(m_i - 1, m_j - 1, m_k - 1); // cellIndexFromIJK uses 0-based indexing 
// 
//     cells.push_back(std::make_pair(cellIndex, gridindex));
// 
//     return cells;
// }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSimWellFracture::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::setIJK(size_t i, size_t j, size_t k)
{
    cvf::Vec3d cellCenter = findCellCenterPosition(i, j, k);
    this->setAnchorPosition(cellCenter);

    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj) proj->createDisplayModelAndRedrawAllViews();
}
     
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    return RimFracture::defineUiOrdering(uiConfigName, uiOrdering);
/*
    uiOrdering.add(&name);

    RimFracture::defineUiOrdering(uiConfigName, uiOrdering);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fractures");
    geometryGroup->add(&m_fractureTemplate);
   
    geometryGroup->add(&m_i);
    geometryGroup->add(&m_j);
    geometryGroup->add(&m_k);
    geometryGroup->add(&ui_cellCenterPosition);

    uiOrdering.setForgetRemainingFields(true);
*/

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimSimWellFracture::findCellCenterPosition(size_t i, size_t j, size_t k) const
{
    cvf::Vec3d undef = cvf::Vec3d::UNDEFINED;

    const caf::PdmObjectHandle* objHandle = dynamic_cast<const caf::PdmObjectHandle*>(this);
    if (!objHandle) return undef;

    RimEclipseView* mainView = nullptr;
    objHandle->firstAncestorOrThisOfType(mainView);
    if (!mainView) return undef;

    const RigMainGrid* mainGrid = mainView->mainGrid();
    if (!mainGrid) return undef;

    size_t gridCellIndex = mainGrid->cellIndexFromIJK(i, j, k); // cellIndexFromIJK uses 0-based indexing 
    const RigCell& rigCell = mainGrid->cell(gridCellIndex);
    cvf::Vec3d center = rigCell.center();

    return center;
}

