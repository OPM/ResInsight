/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimWellLogRegionAnnotationSettings.h"

#include "RimColorLegend.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"
#include "RimWellLogTrack.h"

#include "cafPdmUiSliderEditor.h"

CAF_PDM_SOURCE_INIT( RimWellLogRegionAnnotationSettings, "RimWellLogRegionAnnotationSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRegionAnnotationSettings::RimWellLogRegionAnnotationSettings()
{
    CAF_PDM_InitObject( "Region Annotation Settings" );

    CAF_PDM_InitFieldNoDefault( &m_regionAnnotationType, "AnnotationType", "Region Annotations" );
    CAF_PDM_InitFieldNoDefault( &m_regionAnnotationDisplay, "RegionDisplay", "Region Display" );

    CAF_PDM_InitFieldNoDefault( &m_colorShadingLegend, "ColorShadingLegend", "Colors" );
    m_colorShadingLegend = RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::NORMAL );

    CAF_PDM_InitField( &m_colorShadingTransparency, "ColorShadingTransparency", 50, "Color Transparency" );
    m_colorShadingTransparency.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showRegionLabels, "ShowFormationLabels", true, "Show Labels" );

    caf::FontTools::RelativeSizeEnum regionLabelFontSizeDefault = caf::FontTools::RelativeSize::Small;
    CAF_PDM_InitField( &m_regionLabelFontSize, "RegionLabelFontSize", regionLabelFontSizeDefault, "Font Size" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRegionAnnotationSettings::~RimWellLogRegionAnnotationSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::RegionAnnotationType RimWellLogRegionAnnotationSettings::annotationType() const
{
    return m_regionAnnotationType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::setAnnotationType( RiaDefines::RegionAnnotationType type )
{
    m_regionAnnotationType = type;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::RegionDisplay RimWellLogRegionAnnotationSettings::annotationDisplay() const
{
    return m_regionAnnotationDisplay();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::setAnnotationDisplay( RiaDefines::RegionDisplay display )
{
    m_regionAnnotationDisplay = display;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimWellLogRegionAnnotationSettings::colorShadingLegend() const
{
    return m_colorShadingLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::setColorShadingLegend( RimColorLegend* legend )
{
    m_colorShadingLegend = legend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogRegionAnnotationSettings::colorShadingTransparency() const
{
    return m_colorShadingTransparency();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::setColorShadingTransparency( int transparency )
{
    m_colorShadingTransparency = transparency;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogRegionAnnotationSettings::showRegionLabels() const
{
    return m_showRegionLabels();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::setShowRegionLabels( bool show )
{
    m_showRegionLabels = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FontTools::RelativeSize RimWellLogRegionAnnotationSettings::regionLabelFontSize() const
{
    return m_regionLabelFontSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::setRegionLabelFontSize( caf::FontTools::RelativeSize fontSize )
{
    m_regionLabelFontSize = fontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogRegionAnnotationSettings::showFormations() const
{
    return m_regionAnnotationType() == RiaDefines::RegionAnnotationType::FORMATION_ANNOTATIONS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::uiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_regionAnnotationType );
    uiOrdering.add( &m_regionAnnotationDisplay );
    uiOrdering.add( &m_showRegionLabels );

    if ( m_regionAnnotationType() == RiaDefines::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS )
    {
        uiOrdering.add( &m_regionLabelFontSize );
    }

    if ( m_regionAnnotationDisplay() & RiaDefines::COLOR_SHADING || m_regionAnnotationDisplay() & RiaDefines::COLORED_LINES )
    {
        uiOrdering.add( &m_colorShadingLegend );
        if ( m_regionAnnotationDisplay() & RiaDefines::COLOR_SHADING )
        {
            uiOrdering.add( &m_colorShadingTransparency );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                           const QVariant&            oldValue,
                                                           const QVariant&            newValue )
{
    auto track = firstAncestorOrThisOfType<RimWellLogTrack>();
    if ( track )
    {
        track->loadDataAndUpdate();
        track->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRegionAnnotationSettings::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                                QString                    uiConfigName,
                                                                caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_colorShadingTransparency )
    {
        auto sliderAttrib = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( sliderAttrib )
        {
            sliderAttrib->m_minimum = 0;
            sliderAttrib->m_maximum = 100;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogRegionAnnotationSettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_colorShadingLegend )
    {
        RimTools::colorLegendOptionItems( &options );
    }

    return options;
}
