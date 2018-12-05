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

#include "RimAnnotationTextAppearance.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"


CAF_PDM_SOURCE_INIT(RimAnnotationTextAppearance, "RimAnnotationTextAppearance");


namespace caf
{
template<>
void RimAnnotationTextAppearance::FontSize::setUp()
{
    addItem(RiaFontCache::FONT_SIZE_8, "FONT_SIZE_8", "8");
    addItem(RiaFontCache::FONT_SIZE_10, "FONT_SIZE_10", "10");
    addItem(RiaFontCache::FONT_SIZE_12, "FONT_SIZE_12", "12");
    addItem(RiaFontCache::FONT_SIZE_14, "FONT_SIZE_14", "14");
    addItem(RiaFontCache::FONT_SIZE_16, "FONT_SIZE_16", "16");
    addItem(RiaFontCache::FONT_SIZE_24, "FONT_SIZE_24", "24");
    addItem(RiaFontCache::FONT_SIZE_32, "FONT_SIZE_32", "32");

    setDefault(RiaFontCache::FONT_SIZE_8);
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationTextAppearance::RimAnnotationTextAppearance()
{
    CAF_PDM_InitObject("TextAnnotation", ":/WellCollection.png", "", "");

    auto prefs = RiaApplication::instance()->preferences();
    auto defaultBackgroundColor = prefs->defaultViewerBackgroundColor();

    CAF_PDM_InitField(&m_fontSize,          "FontSize", FontSize(), "Font Size", "", "", "");
    CAF_PDM_InitField(&m_fontColor,         "FontColor", cvf::Color3f(cvf::Color3f::BLACK),  "Font Color", "", "", "");
    CAF_PDM_InitField(&m_backgroundColor,   "BackgroundColor", defaultBackgroundColor , "Background Color", "", "", "");
    CAF_PDM_InitField(&m_anchorLineColor,   "AnchorLineColor", cvf::Color3f(cvf::Color3f::BLACK), "Anchor Line Color", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::setFontSize(FontSize size)
{
    m_fontSize = size;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::setFontColor(const cvf::Color3f& newColor)
{
    m_fontColor = newColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::setBackgroundColor(const cvf::Color3f& newColor)
{
    m_backgroundColor = newColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::setAnchorLineColor(const cvf::Color3f& newColor)
{
    m_anchorLineColor = newColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationTextAppearance::FontSize RimAnnotationTextAppearance::fontSize() const
{
    return m_fontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimAnnotationTextAppearance::fontColor() const
{
    return m_fontColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimAnnotationTextAppearance::backgroundColor() const
{
    return m_backgroundColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimAnnotationTextAppearance::anchorLineColor() const
{
    return m_anchorLineColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_fontSize);
    uiOrdering.add(&m_fontColor);
    uiOrdering.add(&m_backgroundColor);
    uiOrdering.add(&m_anchorLineColor);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue)
{
    RimAnnotationCollectionBase* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);

    if (annColl)
    {
        annColl->scheduleRedrawOfRelevantViews();
    }
}
