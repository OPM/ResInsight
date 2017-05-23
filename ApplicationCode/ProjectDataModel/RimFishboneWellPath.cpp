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

#include "RimFishboneWellPath.h"

#include "RimProject.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT(RimFishboneWellPath, "WellPathCompletion");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishboneWellPath::RimFishboneWellPath()
{
    CAF_PDM_InitObject("WellPathCompletion", ":/Well.png", "", "");
    CAF_PDM_InitFieldNoDefault(&m_coordinates, "Coordinates", "Coordinates", "", "", "");
    m_coordinates.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_measuredDepths, "MeasuredDepth", "MeasuredDepth", "", "", "");
    m_measuredDepths.uiCapability()->setUiHidden(true);
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_displayCoordinates, "DisplayCoordinates", "Coordinates", "", "", "");
    m_displayCoordinates.registerGetMethod(this, &RimFishboneWellPath::displayCoordinates);
    m_displayCoordinates.uiCapability()->setUiReadOnly(true);
    m_displayCoordinates.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_displayCoordinates.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LabelPosType::TOP);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishboneWellPath::~RimFishboneWellPath()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPath::setCoordinates(std::vector< cvf::Vec3d > coordinates)
{
    m_coordinates = coordinates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPath::setMeasuredDepths(std::vector< double > measuredDepths)
{
    m_measuredDepths = measuredDepths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPath::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj) proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPath::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_displayCoordinates);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFishboneWellPath::displayCoordinates() const
{
    CVF_ASSERT(m_coordinates().size() == m_measuredDepths().size());

    QStringList displayValues;

    displayValues.push_back(QString("X\tY\tZ\tMD"));
    for (size_t i = 0; i < m_coordinates().size(); i++)
    {
        const cvf::Vec3d& coords = m_coordinates()[i];
        const double& measuredDepth = m_measuredDepths()[i];
        displayValues.push_back(QString("%1\t%2\t%3\t%4").arg(coords.x(), 0, 'f', 2).arg(coords.y(), 0, 'f', 2).arg(coords.z(), 0, 'f', 2).arg(measuredDepth, 0, 'f', 2));
    }

    return displayValues.join("\n");
}
