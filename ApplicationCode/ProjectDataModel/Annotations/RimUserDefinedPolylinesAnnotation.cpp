/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RimUserDefinedPolylinesAnnotation.h"

#include "RimAnnotationCollection.h"
#include "RimAnnotationLineAppearance.h"

#include "RigPolyLinesData.h"

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> xydToXyzVector(const std::vector<cvf::Vec3d>& xyds)
{
    std::vector<cvf::Vec3d> xyzs;
    for (const auto& xyd : xyds)
    {
        auto xyz = xyd;
        xyz.z() = -xyd.z();
        xyzs.push_back(xyz);
    }
    return xyzs;
}


CAF_PDM_SOURCE_INIT(RimUserDefinedPolylinesAnnotation, "UserDefinedPolylinesAnnotation");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedPolylinesAnnotation::RimUserDefinedPolylinesAnnotation()
{
    CAF_PDM_InitObject("PolyLines Annotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_pointsXyd, "PointsXyd", {}, "", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedPolylinesAnnotation::~RimUserDefinedPolylinesAnnotation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimUserDefinedPolylinesAnnotation::polyLinesData()
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData; 
    std::vector<std::vector<cvf::Vec3d> > lines; 
    lines.push_back(xydToXyzVector(m_pointsXyd()));
    pld->setPolyLines(lines);

    return pld;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUserDefinedPolylinesAnnotation::isEmpty()
{
    return m_pointsXyd().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_pointsXyd);

    auto appearanceGroup = uiOrdering.addNewGroup("Line Appearance");
    appearance()->uiOrdering(uiConfigName, *appearanceGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                         const QVariant&            oldValue,
                                                         const QVariant&            newValue)
{
    RimAnnotationCollection* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);

    annColl->scheduleRedrawOfRelevantViews();
}


