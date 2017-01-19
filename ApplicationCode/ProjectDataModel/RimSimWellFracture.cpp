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

#include "RigCell.h"
#include "RigMainGrid.h"

#include "RimEclipseView.h"
#include "RimProject.h"



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

