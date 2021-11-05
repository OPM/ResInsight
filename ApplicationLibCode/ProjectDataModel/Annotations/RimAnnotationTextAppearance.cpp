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

#include "RiaPreferences.h"

#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"

CAF_PDM_SOURCE_INIT( RimAnnotationTextAppearance, "RimAnnotationTextAppearance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationTextAppearance::RimAnnotationTextAppearance()
{
    CAF_PDM_InitObject( "TextAnnotation", ":/WellCollection.png", "", "" );

    auto prefs                  = RiaPreferences::current();
    auto defaultBackgroundColor = prefs->defaultViewerBackgroundColor();

    CAF_PDM_InitFieldNoDefault( &m_fontSize, "FontSize", "Font Size" );
    m_fontSize = prefs->defaultAnnotationFontSize();

    CAF_PDM_InitField( &m_fontColor, "FontColor", cvf::Color3f( cvf::Color3f::BLACK ), "Font Color" );
    CAF_PDM_InitField( &m_backgroundColor, "BackgroundColor", defaultBackgroundColor, "Background Color" );
    CAF_PDM_InitField( &m_anchorLineColor,
                       "AnchorLineColor",
                       cvf::Color3f( cvf::Color3f::BLACK ),
                       "Anchor Line Color",
                       "",
                       "",
                       "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::setFontSize( FontSize size )
{
    m_fontSize = size;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::setBackgroundColor( const cvf::Color3f& newColor )
{
    m_backgroundColor = newColor;
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
void RimAnnotationTextAppearance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_fontSize );
    uiOrdering.add( &m_fontColor );
    uiOrdering.add( &m_backgroundColor );
    uiOrdering.add( &m_anchorLineColor );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationTextAppearance::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue )
{
    RimAnnotationCollectionBase* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( annColl );

    if ( annColl )
    {
        annColl->scheduleRedrawOfRelevantViews();
    }
}
