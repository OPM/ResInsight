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

#include "RimProject.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_ABSTRACT_SOURCE_INIT(Rim3dWellLogCurve, "Rim3dWellLogCurve");

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
    CAF_PDM_InitObject("3d Well Log Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show3dWellLogCurve", true, "Show 3d Well Log Curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_drawPlane, "DrawPlane", "Draw Plane", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_drawStyle, "DrawStyle", "Draw Style", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_coloringStyle, "ColoringStyle", "Coloring Style", "", "", "");

    CAF_PDM_InitField(&m_name, "Name", QString("3D Well Log Curve"), "3d Well Log Curve", "", "", "");
    m_name.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::~Rim3dWellLogCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::updateCurveIn3dView()
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::DrawPlane Rim3dWellLogCurve::drawPlane() const
{
    return m_drawPlane();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurve::toggleState() const
{
    return m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dWellLogCurve::objectToggleField()
{
    return &m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    if (changedField == &m_showCurve)
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->createDisplayModelAndRedrawAllViews();
    }
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
void Rim3dWellLogCurve::appearanceUiOrdering(caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_drawPlane);
    uiOrdering.add(&m_drawStyle);
    uiOrdering.add(&m_coloringStyle);
}

