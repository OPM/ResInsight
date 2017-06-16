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

#include "RimWellPathFracture.h"

#include "RigWellPath.h"

#include "RimEllipseFractureTemplate.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmUiDoubleSliderEditor.h"



CAF_PDM_SOURCE_INIT(RimWellPathFracture, "WellPathFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::RimWellPathFracture(void)
{
    CAF_PDM_InitObject("Fracture", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(         &m_measuredDepth,          "MeasuredDepth",        0.0f, "Measured Depth Location", "", "", "");
    m_measuredDepth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::~RimWellPathFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathFracture::measuredDepth() const
{
    return m_measuredDepth();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::setMeasuredDepth(double mdValue)
{
    m_measuredDepth = mdValue;

    updatePositionFromMeasuredDepth();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimFracture::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_measuredDepth)
    {
        updatePositionFromMeasuredDepth();
        updateAzimuthFromFractureTemplate();

        RimProject* proj = nullptr;
        this->firstAncestorOrThisOfType(proj);
        if (proj) proj->reloadCompletionTypeResultsInAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::updateAzimuthFromFractureTemplate()
{
    RimFractureTemplate::FracOrientationEnum orientation;
    if (fractureTemplate()) orientation = fractureTemplate()->orientationType();
    else orientation = RimFractureTemplate::AZIMUTH;

    if (orientation == RimFractureTemplate::ALONG_WELL_PATH || orientation == RimFractureTemplate::TRANSVERSE_WELL_PATH)
    {

        double wellPathAzimuth = wellAzimuthAtFracturePosition();

        if (orientation == RimFractureTemplate::TRANSVERSE_WELL_PATH)
        {
            azimuth = wellPathAzimuth;
        }
        if (orientation == RimFractureTemplate::ALONG_WELL_PATH)
        {
            if (wellPathAzimuth + 90 < 360) azimuth = wellPathAzimuth + 90;
            else azimuth = wellPathAzimuth - 90;
        }
    }
    //TODO: Reset value if choosing azimuth in frac template!
    //     else //Azimuth value read from template 
//     {
//         azimuth = attachedFractureTemplate()->azimuthAngle;
//     }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathFracture::wellAzimuthAtFracturePosition()
{
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(this);
    if (!objHandle) return cvf::UNDEFINED_DOUBLE;

    RimWellPath* wellPath = nullptr;
    objHandle->firstAncestorOrThisOfType(wellPath);
    if (!wellPath) return cvf::UNDEFINED_DOUBLE;

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    double wellPathAzimuth = wellPathGeometry->wellPathAzimuthAngle(fracturePosition());
    return wellPathAzimuth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::updatePositionFromMeasuredDepth()
{
    cvf::Vec3d positionAlongWellpath = cvf::Vec3d::ZERO;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(this);
    if (!objHandle) return;

    RimWellPath* wellPath = nullptr;
    objHandle->firstAncestorOrThisOfType(wellPath);
    if (!wellPath) return;

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    positionAlongWellpath = wellPathGeometry->interpolatedPointAlongWellPath(m_measuredDepth());

    this->setAnchorPosition(positionAlongWellpath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimFracture::defineUiOrdering(uiConfigName, uiOrdering);

    uiOrdering.add(nameField());
    uiOrdering.add(&showPolygonFractureOutline);

    caf::PdmUiGroup* locationGroup = uiOrdering.addNewGroup("Location / Orientation");
    locationGroup->add(&m_measuredDepth);
    locationGroup->add(&azimuth);
    locationGroup->add(&dip);
    locationGroup->add(&tilt);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Properties");
    propertyGroup->add(&m_fractureTemplate);
    propertyGroup->add(&stimPlanTimeIndexToPlot);
    propertyGroup->add(&perforationLength);
    propertyGroup->add(&perforationEfficiency);
    propertyGroup->add(&wellDiameter);

    caf::PdmUiGroup* fractureCenterGroup = uiOrdering.addNewGroup("Fracture Center Info");
    fractureCenterGroup->add(&m_uiAnchorPosition);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    RimFracture::defineEditorAttribute(field, uiConfigName, attribute);

    if (field == &m_measuredDepth)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

        if (myAttr)
        {
            RimWellPath* rimWellPath = nullptr;
            this->firstAncestorOrThisOfType(rimWellPath);
            CVF_ASSERT(rimWellPath);

            RigWellPath* wellPathGeo = rimWellPath->wellPathGeometry();
            CVF_ASSERT(wellPathGeo);
            {
                if (wellPathGeo->m_measuredDepths.size() > 2)
                {
                    myAttr->m_minimum = wellPathGeo->m_measuredDepths.front();
                    myAttr->m_maximum = wellPathGeo->m_measuredDepths.back();
                }
            }
        }
    }
}

