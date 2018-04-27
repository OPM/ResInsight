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
#include "Riv3dWellLogCurveGeomertyGenerator.h"

#include "Rim3dWellLogCurveCollection.h"
#include "RimProject.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfVector3.h"

#include <algorithm>
#include <cmath> // Needed for HUGE_VAL on Linux

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
        addItem(Rim3dWellLogCurve::VERTICAL_BELOW, "VERTICAL_BELOW", "Below");
        addItem(Rim3dWellLogCurve::HORIZONTAL_LEFT, "HORIZONTAL_LEFT", "Left");
        addItem(Rim3dWellLogCurve::HORIZONTAL_RIGHT, "HORIZONTAL_RIGHT", "Right");
        setDefault(Rim3dWellLogCurve::VERTICAL_ABOVE);
    } 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::Rim3dWellLogCurve()
    : m_minCurveDataValue(-HUGE_VAL)
    , m_maxCurveDataValue(HUGE_VAL)
{
    CAF_PDM_InitObject("3d Well Log Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show3dWellLogCurve", true, "Show 3d Well Log Curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_minCurveValue, "MinCurveValue", -HUGE_VAL, "Minimum Curve Value", "", "Clip curve values below this.", "");
    CAF_PDM_InitField(&m_maxCurveValue, "MaxCurveValue", HUGE_VAL, "Maximum Curve Value", "", "Clip curve values above this.", "");
    m_minCurveValue.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    m_maxCurveValue.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_drawPlane, "DrawPlane", DrawPlaneEnum(VERTICAL_ABOVE), "Draw Plane", "", "", "");
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
double Rim3dWellLogCurve::minCurveValue() const
{
    return m_minCurveValue();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Rim3dWellLogCurve::maxCurveValue() const
{
    return m_maxCurveValue();
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
    caf::PdmUiGroup* configurationGroup = uiOrdering.addNewGroup("Curve Configuration");
    configurationGroup->add(&m_drawPlane);
    configurationGroup->add(&m_color);
    configurationGroup->add(&m_minCurveValue);
    configurationGroup->add(&m_maxCurveValue);
}

QList<caf::PdmOptionItemInfo> Rim3dWellLogCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &m_drawPlane)
    {
        Rim3dWellLogCurveCollection* collection;
        this->firstAncestorOrThisOfTypeAsserted(collection);
        if (collection->planePositionVertical() == Rim3dWellLogCurveCollection::ALONG_WELLPATH)
        {
            options.push_back(caf::PdmOptionItemInfo(DrawPlaneEnum::uiText(Rim3dWellLogCurve::VERTICAL_ABOVE), Rim3dWellLogCurve::VERTICAL_ABOVE));
            options.push_back(caf::PdmOptionItemInfo(DrawPlaneEnum::uiText(Rim3dWellLogCurve::VERTICAL_BELOW), Rim3dWellLogCurve::VERTICAL_BELOW));
        }
        else
        {
            options.push_back(caf::PdmOptionItemInfo(QString("Vertical"), Rim3dWellLogCurve::VERTICAL_ABOVE));
        }
        if (collection->planePositionHorizontal() == Rim3dWellLogCurveCollection::ALONG_WELLPATH)
        {
            options.push_back(caf::PdmOptionItemInfo(DrawPlaneEnum::uiText(Rim3dWellLogCurve::HORIZONTAL_LEFT), Rim3dWellLogCurve::HORIZONTAL_LEFT));
            options.push_back(caf::PdmOptionItemInfo(DrawPlaneEnum::uiText(Rim3dWellLogCurve::HORIZONTAL_RIGHT), Rim3dWellLogCurve::HORIZONTAL_RIGHT));
        }
        else
        {
            options.push_back(caf::PdmOptionItemInfo(QString("Horizontal"), Rim3dWellLogCurve::HORIZONTAL_LEFT));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiDoubleSliderEditorAttribute* sliderAttribute = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
    if (sliderAttribute)
    {
        if (m_minCurveDataValue == -HUGE_VAL &&
            m_maxCurveDataValue == HUGE_VAL)
        {
            this->resetMinMaxValues();
        }
        if (field == &m_minCurveValue)
        {
            sliderAttribute->m_minimum = m_minCurveDataValue;
            sliderAttribute->m_maximum = m_maxCurveDataValue;
        }
        else if (field == &m_maxCurveValue)
        {
            sliderAttribute->m_minimum = m_minCurveDataValue;
            sliderAttribute->m_maximum = m_maxCurveDataValue;
        }
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
    double foundMinValue = HUGE_VAL;
    double foundMaxValue = -HUGE_VAL;
    for (double value : values)
    {
        if (RigCurveDataTools::isValidValue(value, false))
        {
            foundMinValue = std::min(foundMinValue, value);
            foundMaxValue = std::max(foundMaxValue, value);
        }
    }

    // Update data boundaries
    m_minCurveDataValue = foundMinValue;
    m_maxCurveDataValue = foundMaxValue;

    // Update selected GUI boundaries
    m_minCurveValue = m_minCurveDataValue;
    m_maxCurveValue = m_maxCurveDataValue;
}
