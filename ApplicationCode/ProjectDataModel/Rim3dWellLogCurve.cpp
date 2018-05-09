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

#include "RigCurveDataTools.h"
#include "Riv3dWellLogCurveGeometryGenerator.h"

#include "Rim3dWellLogCurveCollection.h"
#include "RimProject.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfVector3.h"

#include <algorithm>

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
        addItem(Rim3dWellLogCurve::VERTICAL_ABOVE, "VERTICAL_ABOVE", "Above");
        addItem(Rim3dWellLogCurve::VERTICAL_CENTER, "VERTICAL_CENTER", "Centered - Vertical");
        addItem(Rim3dWellLogCurve::VERTICAL_BELOW, "VERTICAL_BELOW", "Below");
        addItem(Rim3dWellLogCurve::HORIZONTAL_LEFT, "HORIZONTAL_LEFT", "Left");
        addItem(Rim3dWellLogCurve::HORIZONTAL_CENTER, "HORIZONTAL_CENTER", "Centered - Horizontal");
        addItem(Rim3dWellLogCurve::HORIZONTAL_RIGHT, "HORIZONTAL_RIGHT", "Right");
        setDefault(Rim3dWellLogCurve::VERTICAL_ABOVE);
    }

    template<>
    void AppEnum< Rim3dWellLogCurve::DrawStyle >::setUp()
    {
        addItem(Rim3dWellLogCurve::LINE, "LINE", "Line");
        addItem(Rim3dWellLogCurve::FILLED, "FILLED", "Filled");
        setDefault(Rim3dWellLogCurve::LINE);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::Rim3dWellLogCurve()
    : m_minCurveDataValue(-std::numeric_limits<float>::infinity())
    , m_maxCurveDataValue(std::numeric_limits<float>::infinity())
{
    CAF_PDM_InitObject("3d Well Log Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show3dWellLogCurve", true, "Show 3d Well Log Curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_minCurveUIValue, "MinCurveValue", -std::numeric_limits<float>::infinity(), "Minimum Curve Value", "", "Clip curve values below this.", "");
    CAF_PDM_InitField(&m_maxCurveUIValue, "MaxCurveValue", std::numeric_limits<float>::infinity(), "Maximum Curve Value", "", "Clip curve values above this.", "");

    CAF_PDM_InitField(&m_drawPlane, "DrawPlane", DrawPlaneEnum(VERTICAL_ABOVE), "Draw Plane", "", "", "");
    CAF_PDM_InitField(&m_drawStyle, "DrawStyle", DrawStyleEnum(LINE), "Draw Style", "", "", "");
    CAF_PDM_InitField(&m_color, "CurveColor", cvf::Color3f(0.0f, 0.0f, 0.0f), "Curve Color", "", "", "");
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
Rim3dWellLogCurve::DrawStyle Rim3dWellLogCurve::drawStyle() const
{
    return m_drawStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Rim3dWellLogCurve::drawPlaneAngle() const
{
    switch (drawPlane())
    {
        case HORIZONTAL_LEFT:
        case HORIZONTAL_CENTER:
            return cvf::PI_D / 2.0;
        case HORIZONTAL_RIGHT:
            return -cvf::PI_D / 2.0;
        case VERTICAL_ABOVE:
        case VERTICAL_CENTER:
            return 0.0;
        case VERTICAL_BELOW:
            return cvf::PI_D;
        default:
            return 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f Rim3dWellLogCurve::color() const
{
    return m_color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurve::isShowingCurve() const
{
    return m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::setColor(const cvf::Color3f& color)
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Rim3dWellLogCurve::minCurveUIValue() const
{
    return m_minCurveUIValue();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float Rim3dWellLogCurve::maxCurveUIValue() const
{
    return m_maxCurveUIValue();
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
void Rim3dWellLogCurve::configurationUiOrdering(caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* configurationGroup = uiOrdering.addNewGroup("Curve Appearance");
    configurationGroup->add(&m_drawPlane);
//  Disable filled draw style in the GUI because of triangle stitching issue #2860.
//  configurationGroup->add(&m_drawStyle);
    configurationGroup->add(&m_color);
    configurationGroup->add(&m_minCurveUIValue);
    configurationGroup->add(&m_maxCurveUIValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (m_minCurveDataValue == -std::numeric_limits<float>::infinity() &&
        m_maxCurveDataValue == std::numeric_limits<float>::infinity())
    {
    	this->resetMinMaxValues();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::initAfterRead()
{
    this->createCurveAutoName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::resetMinMaxValuesAndUpdateUI()
{
    this->resetMinMaxValues();
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogCurve::findClosestPointOnCurve(const cvf::Vec3d& globalIntersection,
                                                cvf::Vec3d*       closestPoint,
                                                double*           measuredDepthAtPoint,
                                                double*           valueAtPoint) const
{
    if (m_geometryGenerator.notNull())
    {
        return m_geometryGenerator->findClosestPointOnCurve(globalIntersection, closestPoint, measuredDepthAtPoint, valueAtPoint);
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::setGeometryGenerator(Riv3dWellLogCurveGeometryGenerator* generator)
{
    m_geometryGenerator = generator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<Riv3dWellLogCurveGeometryGenerator> Rim3dWellLogCurve::geometryGenerator()
{
    return m_geometryGenerator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::resetMinMaxValues()
{
    std::vector<double> values;
    std::vector<double> measuredDepths;
    this->curveValuesAndMds(&values, &measuredDepths);
    double foundMinValue = std::numeric_limits<float>::infinity();
    double foundMaxValue = -std::numeric_limits<float>::infinity();
    for (double value : values)
    {
        if (RigCurveDataTools::isValidValue(value, false))
        {
            foundMinValue = std::min(foundMinValue, value);
            foundMaxValue = std::max(foundMaxValue, value);
        }
    }

    m_minCurveDataValue = foundMinValue;
    m_maxCurveDataValue = foundMaxValue;

    m_minCurveUIValue = m_minCurveDataValue;
    m_maxCurveUIValue = m_maxCurveDataValue;

    m_minCurveUIValue.uiCapability()->setUiName(QString("Minimum Curve Value (%1)").arg(m_minCurveDataValue));
    m_maxCurveUIValue.uiCapability()->setUiName(QString("Maximum Curve Value (%1)").arg(m_maxCurveDataValue));
}
