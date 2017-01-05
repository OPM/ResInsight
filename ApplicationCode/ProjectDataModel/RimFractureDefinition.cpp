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

#include "RimFractureDefinition.h"

#include "cafPdmObject.h"



namespace caf
{
    template<>
   
    void caf::AppEnum< RimFractureDefinition::FracOrientationEnum>::setUp()
    {
        addItem(RimFractureDefinition::AZIMUTH, "Az", "Azimuth");
        addItem(RimFractureDefinition::ALONG_WELL_PATH, "AlongWellPath", "Along Well Path");
        addItem(RimFractureDefinition::TRANSVERSE_WELL_PATH, "TransverseWellPath", "Transverse (normal) to Well Path");

        setDefault(RimFractureDefinition::TRANSVERSE_WELL_PATH);
    }
}


CAF_PDM_SOURCE_INIT(RimFractureDefinition, "FractureDefinition");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureDefinition::RimFractureDefinition(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Fracture Template"), "Name", "", "", "");

    CAF_PDM_InitField(&halfLength,  "HalfLength",       650.0f,  "Halflength X_f", "", "", "");
    CAF_PDM_InitField(&height,      "Height",           75.0f,   "Height", "", "", "");
    CAF_PDM_InitField(&width,       "Width",            1.0f,    "Width", "", "", "");
    CAF_PDM_InitField(&orientation, "Orientation",      caf::AppEnum<FracOrientationEnum>(TRANSVERSE_WELL_PATH), "Fracture orientation", "", "", "");

    CAF_PDM_InitField(&skinFactor,  "SkinFactor",       1.0f,    "Skin Factor", "", "", "");
    CAF_PDM_InitField(&permeability,"Permeability",     22000.f, "Permeability", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureDefinition::~RimFractureDefinition()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFractureDefinition::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureDefinition::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fracture geometry definition");
    geometryGroup->add(&halfLength);
    geometryGroup->add(&height);
    geometryGroup->add(&width);
    geometryGroup->add(&orientation);


    caf::PdmUiGroup* group = uiOrdering.addNewGroup("Fracture properties");
    group->add(&permeability);
    group->add(&skinFactor);
}
