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

#include "RiaColorTables.h"
#include "Rim3dWellLogCurve.h"
#include "RimWellPath.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT(Rim3dWellLogCurveCollection, "Rim3dWellLogCurveCollection");

namespace caf
{
    template<>
    void AppEnum< Rim3dWellLogCurveCollection::PlanePosition >::setUp()
    {
        addItem(Rim3dWellLogCurveCollection::ALONG_WELLPATH, "ALONG_WELLPATH", "On One Side of Well Path");
        addItem(Rim3dWellLogCurveCollection::ON_WELLPATH, "ON_WELLPATH", "On Well Path");
        setDefault(Rim3dWellLogCurveCollection::ALONG_WELLPATH);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurveCollection::Rim3dWellLogCurveCollection()
{
    CAF_PDM_InitObject("3D Track", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showPlot, "Show3dWellLogCurves", true, "Show 3d Well Log Curves", "", "", "");
    m_showPlot.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_planePosition, "PlanePosition", "Plane Position", "", "", "");

    CAF_PDM_InitField(&m_showGrid, "Show3dWellLogGrid", true, "Show Grid", "", "", "");
    CAF_PDM_InitField(&m_showBackground, "Show3dWellLogBackground", false, "Show Background", "", "", "");
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
        size_t index = m_3dWellLogCurves.size();
        curve->setColor(RiaColorTables::wellLogPlotPaletteColors().cycledColor3f(index));
        m_3dWellLogCurves.push_back(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurveCollection::remove3dWellLogCurve(Rim3dWellLogCurve* curve)
{
    m_3dWellLogCurves.removeChildObject(curve);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurveCollection::isShowingPlot() const
{
    return m_showPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurveCollection::isShowingGrid() const
{
    return m_showGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurveCollection::isShowingBackground() const
{
    return m_showBackground;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurveCollection::PlanePosition Rim3dWellLogCurveCollection::planePosition() const
{
    return m_planePosition();
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
void Rim3dWellLogCurveCollection::redrawAffectedViewsAndEditors()
{
    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfType(proj);
    if (proj)
    {
        proj->createDisplayModelAndRedrawAllViews();
    }
    RimWellPath* path = nullptr;
    this->firstAncestorOrThisOfType(path);
    if (path)
    {
        path->updateConnectedEditors();
    }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurveCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* settingsGroup = uiOrdering.addNewGroup("Draw Plane Settings");

    settingsGroup->add(&m_planePosition);
    
    caf::PdmUiGroup* appearanceSettingsGroup = uiOrdering.addNewGroup("Draw Plane Appearance");
    appearanceSettingsGroup->add(&m_showGrid);
    appearanceSettingsGroup->add(&m_showBackground);

}
