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

#include "Rim3dWellLogCurveCollection.h"
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
{
    CAF_PDM_InitObject("3d Well Log Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show3dWellLogCurve", true, "Show 3d Well Log Curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_drawPlane, "DrawPlane", DrawPlaneEnum(VERTICAL_ABOVE), "Draw Plane", "", "", "");
    CAF_PDM_InitField(&m_color, "CurveColor", cvf::Color3f(0.0f, 0.0f, 0.0f), "Curve Color", "", "", "");
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
void Rim3dWellLogCurve::configurationUiOrdering(caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* configurationGroup = uiOrdering.addNewGroup("Curve Configuration");
    configurationGroup->add(&m_drawPlane);
    configurationGroup->add(&m_color);
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
