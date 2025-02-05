/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotAxisAnnotation.h"

#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include "RimEclipseCase.h"
#include "RimPlot.h"
#include "RimTools.h"
#include "RimViewWindow.h"

#include "RiaColorTools.h"

#include "cafPdmFieldCvfColor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimPlotAxisAnnotation, "RimPlotAxisAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisAnnotation::RimPlotAxisAnnotation()
{
    m_annotationType = AnnotationType::LINE;
    CAF_PDM_InitObject( "Plot Axis Annotation", ":/LeftAxis16x16.png" );

    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value" );

    CAF_PDM_InitFieldNoDefault( &m_rangeStart, "RangeStart", "Range Start" );
    CAF_PDM_InitFieldNoDefault( &m_rangeEnd, "RangeEnd", "Range End" );

    caf::AppEnum<Qt::PenStyle> defaultStyle = Qt::PenStyle::SolidLine;
    CAF_PDM_InitField( &m_penStyle, "PenStyle", defaultStyle, "Pen Style" );

    CAF_PDM_InitFieldNoDefault( &m_textAlignment, "TextAlignment", "Text Alignment" );
    m_textAlignment = RiaDefines::TextAlignment::RIGHT;

    CAF_PDM_InitFieldNoDefault( &m_color, "Color", "Color" );
    m_color = cvf::Color3f( cvf::Color3f::BLACK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::setName( const QString& name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::setValue( double value )
{
    m_value = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisAnnotation::AnnotationType RimPlotAxisAnnotation::annotationType() const
{
    return m_annotationType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotAxisAnnotation::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotAxisAnnotation::value() const
{
    return m_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotAxisAnnotation::rangeStart() const
{
    return m_rangeStart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotAxisAnnotation::rangeEnd() const
{
    return m_rangeEnd();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RimPlotAxisAnnotation::color() const
{
    return QColor( 0, 0, 100 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::setAlignment( RiaDefines::TextAlignment alignment )
{
    m_textAlignment = alignment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::setPenStyle( Qt::PenStyle penStyle )
{
    m_penStyle = penStyle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::PenStyle RimPlotAxisAnnotation::penStyle() const
{
    return m_penStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::setColor( const cvf::Color3f& color )
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::TextAlignment RimPlotAxisAnnotation::textAlignment() const
{
    return m_textAlignment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotAxisAnnotation::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotAxisAnnotation::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot* parentPlot = firstAncestorOrThisOfType<RimPlot>();
    if ( parentPlot )
    {
        parentPlot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_value );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisAnnotation::setAnnotationType( AnnotationType annoType )
{
    m_annotationType = annoType;
}
