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
#include "RimEclipseWell.h"
#include "RimProject.h"

#include "cafPdmUiDoubleSliderEditor.h"



CAF_PDM_SOURCE_INIT(RimSimWellFracture, "SimWellFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::RimSimWellFracture(void)
{
    CAF_PDM_InitObject("SimWellFracture", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(&measuredDepth, "MeasuredDepth", 0.0f, "Measured Depth Location", "", "", "");
    measuredDepth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_branchIndex, "Branch", 0, "Branch", "", "", "");
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
void RimSimWellFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimFracture::fieldChangedByUi(changedField, oldValue, newValue);

    if (   changedField == &measuredDepth
        || changedField == &m_branchIndex
        )
    {
        updateFractureAnchorPosition();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateFractureAnchorPosition()
{
    if (m_branchCenterLines.size() == 0)
    {
        updateBranchGeometry();
    }

    if (m_branchCenterLines.size() > 0)
    {
        cvf::Vec3d interpolated = m_branchCenterLines[m_branchIndex()].interpolatedPointAlongWellPath(measuredDepth());

        this->setAnchorPosition(interpolated);

        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj) proj->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

    uiOrdering.add(&measuredDepth);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Properties");
    geometryGroup->add(&azimuth);
    geometryGroup->add(&m_fractureTemplate);

    caf::PdmUiGroup* fractureCenterGroup = uiOrdering.addNewGroup("Fracture Center Info");
    fractureCenterGroup->add(&m_uiAnchorPosition);
    fractureCenterGroup->add(&m_displayIJK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &measuredDepth)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

        if (myAttr)
        {
            if (m_branchCenterLines.size() == 0)
            {
                updateBranchGeometry();
            }

            if (m_branchCenterLines.size() > 0)
            {
                const RigSimulationWellCoordsAndMD& pointAndMd = m_branchCenterLines[m_branchIndex];

                myAttr->m_minimum = pointAndMd.measuredDepths().front();
                myAttr->m_maximum = pointAndMd.measuredDepths().back();
                myAttr->m_sliderTickCount = pointAndMd.measuredDepths().back();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSimWellFracture::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options = RimFracture::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (fieldNeedingOptions == &m_branchIndex)
    {
        if (m_branchCenterLines.size() == 0)
        {
            updateBranchGeometry();
        }

        if (m_branchCenterLines.size() > 0)
        {
            size_t branchCount = m_branchCenterLines.size();

            for (size_t bIdx = 0; bIdx < branchCount; ++bIdx)
            {
                // Use 1-based index in UI
                options.push_back(caf::PdmOptionItemInfo(QString::number(bIdx + 1), QVariant::fromValue(bIdx)));
            }
        }
    }

    return options;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateBranchGeometry()
{
    m_branchCenterLines.clear();

    RimEclipseWell* rimWell = nullptr;
    this->firstAncestorOrThisOfType(rimWell);
    CVF_ASSERT(rimWell);

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    rimWell->calculateWellPipeStaticCenterLine(pipeBranchesCLCoords, pipeBranchesCellIds);

    for (const auto& branch : pipeBranchesCLCoords)
    {
        RigSimulationWellCoordsAndMD wellPathWithMD(branch);

        m_branchCenterLines.push_back(wellPathWithMD);
    }
}

