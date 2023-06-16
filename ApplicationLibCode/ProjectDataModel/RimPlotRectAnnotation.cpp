/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimPlotRectAnnotation.h"

#include "RimPlot.h"
#include "RimTools.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimPlotRectAnnotation, "RimPlotRectAnnotation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotRectAnnotation::RimPlotRectAnnotation()
{
    CAF_PDM_InitObject( "Plot Rect Annotation", ":/LeftAxis16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_minX, "MinX", "Min X" );
    CAF_PDM_InitFieldNoDefault( &m_maxX, "MaxX", "Max X" );
    CAF_PDM_InitFieldNoDefault( &m_minY, "MinY", "Min Y" );
    CAF_PDM_InitFieldNoDefault( &m_maxY, "MaxY", "Max Y" );

    CAF_PDM_InitField( &m_color, "Color", cvf::Color3f( cvf::Color3f::LIGHT_GRAY ), "Color" );
    CAF_PDM_InitField( &m_transparency, "Transparency", 0.1, "Transparency" );
    m_transparency.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_text, "Text", "Text" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotRectAnnotation::setRangeX( double minX, double maxX )
{
    m_minX = minX;
    m_maxX = maxX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotRectAnnotation::setRangeY( double minY, double maxY )
{
    m_minY = minY;
    m_maxY = maxY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimPlotRectAnnotation::rangeX() const
{
    return { m_minX, m_maxX };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimPlotRectAnnotation::rangeY() const
{
    return { m_minY, m_maxY };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotRectAnnotation::setColor( const cvf::Color3f& color )
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPlotRectAnnotation::color() const
{
    return m_color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotRectAnnotation::setTransparency( double transparency )
{
    m_transparency = transparency;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotRectAnnotation::transparency() const
{
    return m_transparency;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotRectAnnotation::setText( const QString& text )
{
    m_text = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotRectAnnotation::text() const
{
    return m_text;
}
