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

#include "cafPdmObject.h"
#include "RimProject.h"
#include "RimFracture.h"
#include "RigTesselatorTools.h"
#include "cvfVector3.h"



namespace caf
{
    template<>
   
    void caf::AppEnum< RimEllipseFractureTemplate::FracOrientationEnum>::setUp()
    {
        addItem(RimEllipseFractureTemplate::AZIMUTH, "Az", "Azimuth");
        addItem(RimEllipseFractureTemplate::ALONG_WELL_PATH, "AlongWellPath", "Along Well Path");
        addItem(RimEllipseFractureTemplate::TRANSVERSE_WELL_PATH, "TransverseWellPath", "Transverse (normal) to Well Path");

        setDefault(RimEllipseFractureTemplate::TRANSVERSE_WELL_PATH);
    }
}


CAF_PDM_SOURCE_INIT(RimEllipseFractureTemplate, "FractureDefinition");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::RimEllipseFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Fracture Template"), "Name", "", "", "");

    CAF_PDM_InitField(&halfLength,  "HalfLength",       650.0f,  "Halflength X_f", "", "", "");
    CAF_PDM_InitField(&height,      "Height",           75.0f,   "Height", "", "", "");
    CAF_PDM_InitField(&width,       "Width",            1.0f,    "Width", "", "", "");
    CAF_PDM_InitField(&orientation, "Orientation",      caf::AppEnum<FracOrientationEnum>(TRANSVERSE_WELL_PATH), "Fracture orientation", "", "", "");
    CAF_PDM_InitField(&azimuthAngle, "AzimuthAngle",    0.0f, "Angle (if Azimuth Orientation)", "", "", ""); //Is this correct description?
    CAF_PDM_InitField(&perforationLength, "PerforationLength", 0.0f, "Lenght of well perforation", "", "", ""); //Is this correct description?


    CAF_PDM_InitField(&skinFactor,  "SkinFactor",       1.0f,    "Skin Factor", "", "", "");
    CAF_PDM_InitField(&permeability,"Permeability",     22000.f, "Permeability", "", "", "");
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
caf::PdmFieldHandle* RimEllipseFractureTemplate::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

    if (changedField == &halfLength || changedField == &height || changedField == &azimuthAngle || changedField == &perforationLength)
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
double RimEllipseFractureTemplate::effectiveKh()
{
    //TODO: Handle different units!
    return width * permeability;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* polygonIndices)
{
    RigEllipsisTesselator tesselator(20);

    float a = halfLength;
    float b = height / 2.0f;

    tesselator.tesselateEllipsis(a, b, polygonIndices, nodeCoords);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimEllipseFractureTemplate::fracturePolygon()
{
    std::vector<cvf::Vec3f> polygon;

    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  polygonIndices;

    fractureGeometry(&nodeCoords, &polygonIndices);

    for (size_t i = 1; i < nodeCoords.size(); i++)
    {
        polygon.push_back(nodeCoords[i]);
    }

    return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fracture geometry definition");
    geometryGroup->add(&halfLength);
    geometryGroup->add(&height);
    geometryGroup->add(&orientation);
    geometryGroup->add(&azimuthAngle);
    geometryGroup->add(&perforationLength);

    caf::PdmUiGroup* group = uiOrdering.addNewGroup("Fracture properties");
    group->add(&permeability);
    group->add(&width);
    group->add(&skinFactor);
}
