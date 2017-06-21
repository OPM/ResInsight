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
#include "RigSingleWellResultsData.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"

#include "cafPdmUiDoubleSliderEditor.h"



CAF_PDM_SOURCE_INIT(RimSimWellFracture, "SimWellFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::RimSimWellFracture(void)
{
    CAF_PDM_InitObject("SimWellFracture", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(&m_location, "MeasuredDepth", 0.0f, "Pseudo Length Location", "", "", "");
    m_location.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_displayIJK, "Cell_IJK", "Cell IJK", "", "", "");
    m_displayIJK.registerGetMethod(this, &RimSimWellFracture::createOneBasedIJKText);
    m_displayIJK.uiCapability()->setUiReadOnly(true);

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
void RimSimWellFracture::setClosestWellCoord(cvf::Vec3d& position, size_t branchIndex)
{
    updateBranchGeometry();

    double location = m_branchCenterLines[branchIndex].locationAlongWellCoords(position);

    m_branchIndex = static_cast<int>(branchIndex);

    m_location = location;
    updateFracturePositionFromLocation();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateAzimuthFromFractureTemplate()
{
    RimFractureTemplate::FracOrientationEnum orientation;
    if (fractureTemplate()) orientation = fractureTemplate()->orientationType();
    else orientation = RimFractureTemplate::AZIMUTH;

    if (orientation == RimFractureTemplate::ALONG_WELL_PATH || orientation== RimFractureTemplate::TRANSVERSE_WELL_PATH)
    {
        double simWellAzimuth = wellAzimuthAtFracturePosition();

        if (orientation == RimFractureTemplate::TRANSVERSE_WELL_PATH )
        {
            azimuth = simWellAzimuth;
        }
        if (orientation == RimFractureTemplate::ALONG_WELL_PATH)
        {
            if (simWellAzimuth + 90 < 360) azimuth = simWellAzimuth + 90;
            else azimuth = simWellAzimuth - 90;
        }
    }
    else //Azimuth value read from template 
    {
        if (fractureTemplate()) azimuth = fractureTemplate()->azimuthAngle;
        else azimuth = 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSimWellFracture::wellAzimuthAtFracturePosition()
{
    updateBranchGeometry();
    double simWellAzimuth = m_branchCenterLines[m_branchIndex].simWellAzimuthAngle(fracturePosition());
    return simWellAzimuth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSimWellFracture::wellDipAtFracturePosition()
{
    updateBranchGeometry();
    double simWellDip = m_branchCenterLines[m_branchIndex].simWellDipAngle(fracturePosition());
    return simWellDip;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimFracture::fieldChangedByUi(changedField, oldValue, newValue);

    if (   changedField == &m_location
        || changedField == &m_branchIndex
        )
    {
        updateFracturePositionFromLocation();

        RimFractureTemplate::FracOrientationEnum orientation;
        if (fractureTemplate()) orientation = fractureTemplate()->orientationType();
        else orientation = RimFractureTemplate::AZIMUTH;

        if (orientation != RimFractureTemplate::AZIMUTH)
        {
            updateAzimuthFromFractureTemplate();
        }

        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj) proj->reloadCompletionTypeResultsInAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::recomputeWellCenterlineCoordinates()
{
    m_branchCenterLines.clear();

    updateBranchGeometry();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateFracturePositionFromLocation()
{
    updateBranchGeometry();

    if (m_branchCenterLines.size() > 0)
    {
        cvf::Vec3d interpolated = m_branchCenterLines[m_branchIndex()].interpolatedPointAlongWellPath(m_location());

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
    RimFracture::defineUiOrdering(uiConfigName, uiOrdering);

    uiOrdering.add(nameField());
    uiOrdering.add(&showPolygonFractureOutline);

    caf::PdmUiGroup* locationGroup = uiOrdering.addNewGroup("Location / Orientation");
    locationGroup->add(&m_location);
    locationGroup->add(&m_branchIndex);
    locationGroup->add(&azimuth);
    locationGroup->add(&dip);
    locationGroup->add(&tilt);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Properties");
    propertyGroup->add(&m_fractureUnit);
    propertyGroup->add(&m_fractureTemplate);
    propertyGroup->add(&stimPlanTimeIndexToPlot);
    propertyGroup->add(&perforationLength);
    propertyGroup->add(&perforationEfficiency);
    propertyGroup->add(&wellDiameter);

    caf::PdmUiGroup* fractureCenterGroup = uiOrdering.addNewGroup("Fracture Center Info");
    fractureCenterGroup->add(&m_uiAnchorPosition);
    fractureCenterGroup->add(&m_displayIJK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    RimFracture::defineEditorAttribute(field, uiConfigName, attribute);

    if (field == &m_location)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

        if (myAttr)
        {
            updateBranchGeometry();

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
RigMainGrid* RimSimWellFracture::ownerCaseMainGrid() const
{
    RimEclipseView* ownerEclView;
    this->firstAncestorOrThisOfType(ownerEclView);

    if (ownerEclView) 
        return ownerEclView->mainGrid();
    else 
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateBranchGeometry()
{
    if (m_branchCenterLines.size() == 0)
    {
        setBranchGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::setBranchGeometry()
{
    m_branchCenterLines.clear();

    RimEclipseWell* rimWell = nullptr;
    this->firstAncestorOrThisOfType(rimWell);
    CVF_ASSERT(rimWell);

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    rimWell->calculateWellPipeStaticCenterLine(pipeBranchesCLCoords, 
                                               pipeBranchesCellIds);

    for (const auto& branch : pipeBranchesCLCoords)
    {
        RigSimulationWellCoordsAndMD wellPathWithMD(branch);

        m_branchCenterLines.push_back(wellPathWithMD);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSimWellFracture::createOneBasedIJKText() const
{
    RigMainGrid* mainGrid = ownerCaseMainGrid();
    size_t i,j,k;
    size_t anchorCellIdx = findAnchorEclipseCell(mainGrid);
    if (anchorCellIdx == cvf::UNDEFINED_SIZE_T) return "";

    bool ok = mainGrid->ijkFromCellIndex(anchorCellIdx, &i, &j, &k);
    if (!ok) return "";

    return QString("[%1, %2, %3]").arg(i + 1).arg(j + 1).arg(k + 1);
}
