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

#include "RimStimPlanFractureTemplate.h"

#include "RigTesselatorTools.h"

#include "RimFracture.h"
#include "RimProject.h"

#include "cafPdmObject.h"

#include "cvfVector3.h"



CAF_PDM_SOURCE_INIT(RimStimPlanFractureTemplate, "RimStimPlanFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::~RimStimPlanFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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
// 
// 



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* polygonIndices)
{
//     RigEllipsisTesselator tesselator(20);
// 
//     float a = halfLength;
//     float b = height / 2.0f;
// 
//     tesselator.tesselateEllipsis(a, b, polygonIndices, nodeCoords);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fracturePolygon()
{
     std::vector<cvf::Vec3f> polygon;
// 
//     std::vector<cvf::Vec3f> nodeCoords;
//     std::vector<cvf::uint>  polygonIndices;
// 
//     fractureGeometry(&nodeCoords, &polygonIndices);
// 
//     for (size_t i = 1; i < nodeCoords.size(); i++)
//     {
//         polygon.push_back(nodeCoords[i]);
//     }
// 
     return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{

}
