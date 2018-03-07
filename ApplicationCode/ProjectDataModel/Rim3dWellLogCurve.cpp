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

#include "Rim3dWellLogCurve.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"


//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(Rim3dWellLogCurve, "Rim3dWellLogCurve");

namespace caf
{
    template<>
    void AppEnum< Rim3dWellLogCurve::DrawPlane >::setUp()
    {
        addItem(Rim3dWellLogCurve::HORIZONTAL_LEFT, "HORIZONTAL_LEFT", "Horizontal - Left");
        addItem(Rim3dWellLogCurve::HORIZONTAL_RIGHT, "HORIZONTAL_RIGHT", "Horizontal - Right");
        addItem(Rim3dWellLogCurve::VERTICAL_ABOVE, "VERTICAL_ABOVE", "Vertical - Above");
        addItem(Rim3dWellLogCurve::VERTICAL_BELOW, "VERTICAL_BELOW", "Vertical - Below");
        addItem(Rim3dWellLogCurve::CAMERA_ALIGNED_SIDE1, "CAMERA_ALIGNED_SIDE_1", "Camera Aligned - Side 1");
        addItem(Rim3dWellLogCurve::CAMERA_ALIGNED_SIDE2, "CAMERA_ALIGNED_SIDE_2", "Camera Aligned - Side 2");
        setDefault(Rim3dWellLogCurve::HORIZONTAL_LEFT);
    }

    template<>
    void AppEnum< Rim3dWellLogCurve::DrawStyle >::setUp()
    {
        addItem(Rim3dWellLogCurve::LINE, "LINE", "Line");
        addItem(Rim3dWellLogCurve::FILLED, "FILLED", "Filled");
        setDefault(Rim3dWellLogCurve::LINE);
    }

    template<>
    void AppEnum< Rim3dWellLogCurve::ColoringStyle >::setUp()
    {
        addItem(Rim3dWellLogCurve::SINGLE_COLOR, "SINGLE_COLOR", "Single Color");
        addItem(Rim3dWellLogCurve::CURVE_VALUE, "CURVE_VALUE", "Curve Value");
        addItem(Rim3dWellLogCurve::OTHER_RESULT, "OTHER_RESULT", "Other Result");
        setDefault(Rim3dWellLogCurve::SINGLE_COLOR);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::Rim3dWellLogCurve()
{
    CAF_PDM_InitObject("3d Well Log Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_drawPlane, "DrawPlane", "Draw Plane", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_drawStyle, "DrawStyle", "Draw Style", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_coloringStyle, "ColoringStyle", "Coloring Style", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_eclipseResultDefinition = new RimEclipseResultDefinition;
    m_eclipseResultDefinition->findField("MResultType")->uiCapability()->setUiName("Result Type");

    CAF_PDM_InitFieldNoDefault(&m_geomResultDefinition, "CurveGeomechResult", "", "", "", "");
    m_geomResultDefinition.uiCapability()->setUiHidden(true);
    m_geomResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_geomResultDefinition = new RimGeoMechResultDefinition;

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", 0, "Time Step", "", "", "");

    CAF_PDM_InitField(&m_name, "Name", QString("3d Well Log Curve"), "3d Well Log Curve", "", "", "");
    m_name.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::~Rim3dWellLogCurve()
{
    delete m_geomResultDefinition;
    delete m_eclipseResultDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dWellLogCurve::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");

    curveDataGroup->add(&m_case);

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    if (eclipseCase)
    {
        m_eclipseResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);

    }
    else if (geomCase)
    {
        m_geomResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);

    }

    if ((eclipseCase && m_eclipseResultDefinition->hasDynamicResult())
        || geomCase)
    {
        curveDataGroup->add(&m_timeStep);
    }

    caf::PdmUiGroup* formatGroup = uiOrdering.addNewGroup("Appearance");
    formatGroup->add(&m_drawPlane);
    formatGroup->add(&m_drawStyle);
    formatGroup->add(&m_coloringStyle);

    uiOrdering.skipRemainingFields();
}
