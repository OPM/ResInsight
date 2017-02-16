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

#include "RimEllipseFractureTemplate.h"

#include "RigTesselatorTools.h"

#include "RimDefines.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"

#include "cafPdmObject.h"

#include "cvfVector3.h"



CAF_PDM_SOURCE_INIT(RimEllipseFractureTemplate, "RimEllipseFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::RimEllipseFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&halfLength,  "HalfLength",       650.0f,  "Halflength X_f", "", "", "");
    CAF_PDM_InitField(&height,      "Height",           75.0f,   "Height", "", "", "");
    CAF_PDM_InitField(&width,       "Width",            1.0f,    "Width", "", "", "");
    CAF_PDM_InitField(&perforationLength, "PerforationLength", 0.0f, "Lenght of well perforation", "", "", ""); //Is this correct description?

    CAF_PDM_InitField(&permeability,"Permeability",     22000.f, "Permeability [mD]", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::~RimEllipseFractureTemplate()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

    if (changedField == &halfLength || changedField == &height || changedField == &azimuthAngle || changedField == &perforationLength || changedField == &orientation)
    {
        //Changes to one of these parameters should change all fractures with this fracture template attached. 
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            //Regenerate geometry
            std::vector<RimFracture*> fractures;
            proj->descendantsIncludingThisOfType(fractures);

            for (RimFracture* fracture : fractures)
            {
                if (fracture->attachedFractureDefinition() == this)
                {
                    if (changedField == &halfLength || changedField == &height)
                    {
                        fracture->setRecomputeGeometryFlag();
                    }

                    if (changedField == &azimuthAngle && (abs(oldValue.toDouble() - fracture->azimuth()) < 1e-5))
                    {
                        fracture->azimuth = azimuthAngle;
                        fracture->setRecomputeGeometryFlag();
                    }

                    if (changedField == &orientation)
                    {
                        fracture->setAzimuth();
                        if (orientation() == FracOrientationEnum::AZIMUTH)
                        {
                            fracture->azimuth = azimuthAngle;
                        }

                        fracture->setRecomputeGeometryFlag();
                    }

                    if (changedField == &perforationLength && (abs(oldValue.toDouble() - fracture->perforationLength()) < 1e-5))
                    {
                        fracture->perforationLength = perforationLength;
                    }

                }
            }

            proj->createDisplayModelAndRedrawAllViews();
        }
    }





}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices)
{
    RigEllipsisTesselator tesselator(20);

    float a = halfLength;
    float b = height / 2.0f;

    tesselator.tesselateEllipsis(a, b, triangleIndices, nodeCoords);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimEllipseFractureTemplate::fracturePolygon()
{
    std::vector<cvf::Vec3f> polygon;

    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  triangleIndices;

    fractureGeometry(&nodeCoords, &triangleIndices);

    for (size_t i = 1; i < nodeCoords.size(); i++)
    {
        polygon.push_back(nodeCoords[i]);
    }

    return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::changeUnits()
{
    if (fractureTemplateUnit == RimDefines::UNITS_METRIC)
    {
        halfLength = RimDefines::meterToFeet(halfLength);
        height = RimDefines::meterToFeet(height);
        width = RimDefines::meterToInch(width);
        perforationLength = RimDefines::meterToFeet(perforationLength);
        fractureTemplateUnit = RimDefines::UNITS_FIELD;
        //TODO: Darcy unit?
    }
    else if (fractureTemplateUnit == RimDefines::UNITS_FIELD)
    {
        halfLength = RimDefines::feetToMeter(halfLength);
        height = RimDefines::feetToMeter(height);
        width = RimDefines::inchToMeter(width);
        perforationLength = RimDefines::feetToMeter(perforationLength);
        fractureTemplateUnit = RimDefines::UNITS_METRIC;
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimFractureTemplate::defineUiOrdering(uiConfigName, uiOrdering);
    
    if (fractureTemplateUnit == RimDefines::UNITS_METRIC)
    {
        halfLength.uiCapability()->setUiName("Halflenght Xf [m]");
        height.uiCapability()->setUiName("Height [m]");
        width.uiCapability()->setUiName("Width [m]");
        perforationLength.uiCapability()->setUiName("Length of well perforation [m]");
    }
    else if (fractureTemplateUnit == RimDefines::UNITS_FIELD)
    {
        halfLength.uiCapability()->setUiName("Halflenght Xf [Ft]");
        height.uiCapability()->setUiName("Height [Ft]");
        width.uiCapability()->setUiName("Width [inches]");
        perforationLength.uiCapability()->setUiName("Length of well perforation [Ft]");
    }

    
    uiOrdering.add(&name);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fracture geometry");
    geometryGroup->add(&halfLength);
    geometryGroup->add(&height);
    geometryGroup->add(&orientation);
    geometryGroup->add(&azimuthAngle);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Fracture properties");
    propertyGroup->add(&fractureConductivity);
    propertyGroup->add(&permeability);
    propertyGroup->add(&width);
    propertyGroup->add(&skinFactor);
    propertyGroup->add(&perforationLength);
}

