/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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

#include "RimWellPathCompletion.h"

#include "RimProject.h"

#include "cafPdmUiListEditor.h"

CAF_PDM_SOURCE_INIT(RimWellPathCompletion, "WellPathCompletion");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCompletion::RimWellPathCompletion()
{
    CAF_PDM_InitObject("WellPathCompletion", ":/Well.png", "", "");
    CAF_PDM_InitFieldNoDefault(&m_coordinates, "Coordinates", "Coordinates", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_measuredDepths, "MeasuredDepth", "MeasuredDepth", "", "", "");
    m_measuredDepths.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_name.uiCapability()->setUiHidden(true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCompletion::~RimWellPathCompletion()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletion::setCoordinates(std::vector< cvf::Vec3d > coordinates)
{
    m_coordinates = coordinates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletion::setMeasuredDepths(std::vector< double > measuredDepths)
{
    m_measuredDepths = measuredDepths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletion::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj) proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletion::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* geometryGroup =  uiOrdering.addNewGroup("Geometry");
    geometryGroup->add(&m_coordinates);
    geometryGroup->add(&m_measuredDepths);
}
