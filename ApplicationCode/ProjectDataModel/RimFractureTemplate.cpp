/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RimFractureTemplate.h"

#include "RigTesselatorTools.h"

#include "RimFracture.h"
#include "RimProject.h"

#include "cafPdmObject.h"

#include "cvfVector3.h"



namespace caf
{
    template<>
   
    void caf::AppEnum< RimFractureTemplate::FracOrientationEnum>::setUp()
    {
        addItem(RimFractureTemplate::AZIMUTH, "Az", "Azimuth");
        addItem(RimFractureTemplate::ALONG_WELL_PATH, "AlongWellPath", "Along Well Path");
        addItem(RimFractureTemplate::TRANSVERSE_WELL_PATH, "TransverseWellPath", "Transverse (normal) to Well Path");

        setDefault(RimFractureTemplate::TRANSVERSE_WELL_PATH);
    }
}


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimFractureTemplate, "RimFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::RimFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Fracture Template"), "Name", "", "", "");

    CAF_PDM_InitField(&orientation, "Orientation",      caf::AppEnum<FracOrientationEnum>(TRANSVERSE_WELL_PATH), "Fracture orientation", "", "", "");
    CAF_PDM_InitField(&azimuthAngle, "AzimuthAngle",    0.0f, "Azimuth Angle", "", "", ""); //Is this correct description?
    CAF_PDM_InitField(&skinFactor, "SkinFactor", 1.0f, "Skin Factor", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::~RimFractureTemplate()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFractureTemplate::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
// 
//     if (changedField == &halfLength || changedField == &height || changedField == &azimuthAngle || changedField == &perforationLength || changedField == &orientation)
//     {
//         //Changes to one of these parameters should change all fractures with this fracture template attached. 
//         RimProject* proj;
//         this->firstAncestorOrThisOfType(proj);
//         if (proj)
//         {
//             //Regenerate geometry
//             std::vector<RimFracture*> fractures;
//             proj->descendantsIncludingThisOfType(fractures);
// 
//             for (RimFracture* fracture : fractures)
//             {
//                 if (fracture->attachedFractureDefinition() == this)
//                 {
//                     if (changedField == &halfLength || changedField == &height)
//                     {
//                         fracture->setRecomputeGeometryFlag();
//                     }
// 
//                     if (changedField == &azimuthAngle && (abs(oldValue.toDouble() - fracture->azimuth()) < 1e-5))
//                     {
//                         fracture->azimuth = azimuthAngle;
//                         fracture->setRecomputeGeometryFlag();
//                     }
// 
//                     if (changedField == &orientation)
//                     {
//                         fracture->setAzimuth();
//                         if (orientation() == FracOrientationEnum::AZIMUTH)
//                         {
//                             fracture->azimuth = azimuthAngle;
//                         }
// 
//                         fracture->setRecomputeGeometryFlag();
//                     }
// 
//                     if (changedField == &perforationLength && (abs(oldValue.toDouble() - fracture->perforationLength()) < 1e-5))
//                     {
//                         fracture->perforationLength = perforationLength;
//                     }
// 
//                 }
//             }
// 
//             proj->createDisplayModelAndRedrawAllViews();
//         }
//     }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);
}
