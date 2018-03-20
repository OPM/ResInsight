/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Rim3dWellLogCurveCollection.h"

#include "Rim3dWellLogCurve.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT(Rim3dWellLogCurveCollection, "Rim3dWellLogCurveCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurveCollection::Rim3dWellLogCurveCollection()
{
    CAF_PDM_InitObject("3D Track", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showPlot, "Show3dWellLogCurves", true, "Show 3d Well Log Curves", "", "", "");
    m_showPlot.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_showGrid, "Show3dWellLogGrid", true, "Show Grid", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_3dWellLogCurves, "ArrayOf3dWellLogCurves", "", "", "", "");
    m_3dWellLogCurves.uiCapability()->setUiTreeHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurveCollection::~Rim3dWellLogCurveCollection()
{
    m_3dWellLogCurves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurveCollection::has3dWellLogCurves() const
{
    return !m_3dWellLogCurves.empty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurveCollection::add3dWellLogCurve(Rim3dWellLogCurve* curve)
{
    if (curve)
    {
        m_3dWellLogCurves.push_back(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurveCollection::showGrid() const
{
    return m_showGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurveCollection::showPlot() const
{
    return m_showPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dWellLogCurve*> Rim3dWellLogCurveCollection::vectorOf3dWellLogCurves() const
{
    std::vector<Rim3dWellLogCurve*> curves;
    for (auto& wellLogCurve : m_3dWellLogCurves)
    {
        curves.push_back(wellLogCurve);
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurveCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dWellLogCurveCollection::objectToggleField()
{
    return &m_showPlot;
}
