/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimAnnotationLineAppearance.h"
#include "RimAnnotationCollection.h"

namespace caf
{
template<>
void RimAnnotationLineAppearance::LineStyle::setUp()
{
    addItem(RimAnnotationLineAppearance::STYLE_SOLID, "STYLE_SOLID", "Solid");
    addItem(RimAnnotationLineAppearance::STYLE_DASH, "STYLE_DASH", "Dashes");

    setDefault(RimAnnotationLineAppearance::STYLE_SOLID);
}
}


CAF_PDM_SOURCE_INIT(RimAnnotationLineAppearance, "RimAnnotationLineAppearance");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance::RimAnnotationLineAppearance()
{
    CAF_PDM_InitObject("AnnotationLineAppearance", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_color,     "Color",     cvf::Color3f(cvf::Color3f::BLACK),  "Line Color", "", "", "");
    CAF_PDM_InitField(&m_thickness, "Thickness", 2,                                  "Line Thickness", "", "", "");

    // Stippling not yet supported. Needs new stuff in VizFwk
    CAF_PDM_InitField(&m_style,     "Style",     LineStyle(),                        "Style", "", "", "");
    m_style.uiCapability()->setUiHidden(true);
    m_style.xmlCapability()->disableIO();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::setColor(const cvf::Color3f& newColor)
{
    m_color = newColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimAnnotationLineAppearance::color() const
{
    return m_color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationLineAppearance::isDashed() const
{
    return m_style() == STYLE_DASH;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAnnotationLineAppearance::thickness() const
{
    return m_thickness();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_color);
    uiOrdering.add(&m_style);
    uiOrdering.add(&m_thickness);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue)
{
    RimAnnotationCollection* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);
    annColl->scheduleRedrawOfRelevantViews(); 
}

CAF_PDM_SOURCE_INIT(RimPolylineAppearance, "RimPolylineAppearance");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineAppearance::RimPolylineAppearance()
{
    CAF_PDM_InitObject("PolylineAppearance", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_sphereColor, "SphereColor", cvf::Color3f(cvf::Color3f::BLACK), "Sphere Color", "", "", "");
    CAF_PDM_InitField(&m_sphereRadius, "SphereRadius", 15, "Sphere Radius", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::setSphereColor(const cvf::Color3f& color)
{
    m_sphereColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPolylineAppearance::sphereColor() const
{
    return m_sphereColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::setSphereRadius(int radius)
{
    m_sphereRadius = radius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPolylineAppearance::sphereRadius() const
{
    return m_sphereRadius();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimAnnotationLineAppearance::defineUiOrdering(uiConfigName, uiOrdering);

    uiOrdering.add(&m_sphereColor);
    uiOrdering.add(&m_sphereRadius);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue)
{
    RimAnnotationLineAppearance::fieldChangedByUi(changedField, oldValue, newValue);
}
