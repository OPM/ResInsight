/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiuPlotAnnotationTool.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"

#include "cafCategoryMapper.h"
#include "cvfMath.h"

#include "qwt_plot.h"
#include "qwt_plot_shapeitem.h"

#include <QString>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAnnotationTool::~RiuPlotAnnotationTool()
{
    detachAllAnnotations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachNamedRegions( QwtPlot*                                     plot,
                                                const std::vector<QString>&                  names,
                                                const std::pair<double, double>              xRange,
                                                const std::vector<std::pair<double, double>> yPositions,
                                                RegionDisplay                                regionDisplay,
                                                const caf::ColorTable&                       colorTable,
                                                int                                          shadingAlphaByte,
                                                bool                                         showNames /*= true */,
                                                bool                                         detachExisting /*= true*/,
                                                double                                       xStart /*= 0.0*/,
                                                double                                       xEnd /*= 1.0*/ )
{
    if ( detachExisting )
    {
        detachAllAnnotations();
    }
    if ( names.size() != yPositions.size() ) return;
    m_plot = plot;

    double delta = 0.5;

    std::vector<int> categoryIndices( names.size() );
    std::iota( categoryIndices.begin(), categoryIndices.end(), 0 );

    caf::CategoryMapper catMapper;
    catMapper.setCategories( categoryIndices );
    catMapper.setInterpolateColors( colorTable.color3ubArray() );

    for ( size_t i = 0; i < names.size(); i++ )
    {
        if ( names[i].isEmpty() ) continue;

        QwtPlotMarker* line( new QwtPlotMarker() );

        QString name;
        if ( showNames )
        {
            name = names[i];
            if ( ( regionDisplay & COLOR_SHADING ) == 0 && names[i].toLower().indexOf( "top" ) == -1 )
            {
                name += " Top";
            }
        }
        if ( regionDisplay & COLOR_SHADING )
        {
            cvf::Color3ub cvfColor = catMapper.mapToColor( static_cast<double>( i ) );
            QColor        shadingColor( cvfColor.r(), cvfColor.g(), cvfColor.b(), shadingAlphaByte );

            QwtPlotShapeItem* shading = new QwtPlotShapeItem( name );

            QwtInterval axisInterval = m_plot->axisInterval( QwtPlot::xBottom );

            QRectF shadingRect( axisInterval.minValue() + axisInterval.width() * xStart,
                                yPositions[i].first,
                                xEnd * axisInterval.width(),
                                yPositions[i].second - yPositions[i].first );

            shading->setRect( shadingRect );
            shading->setPen( shadingColor, 0.0, Qt::NoPen );
            shading->setBrush( QBrush( shadingColor ) );
            shading->attach( m_plot );
            shading->setZ( -100.0 );
            shading->setXAxis( QwtPlot::xBottom );
            m_markers.push_back( std::move( shading ) );
        }

        QColor lineColor( 0, 0, 0, 0 );
        QColor textColor( 0, 0, 0, 255 );
        if ( regionDisplay & DARK_LINES || regionDisplay & COLORED_LINES )
        {
            cvf::Color3ub cvfColor = catMapper.mapToColor( static_cast<double>( i ) );
            QColor        cycledColor( cvfColor.r(), cvfColor.g(), cvfColor.b() );

            lineColor = regionDisplay & DARK_LINES ? QColor( 0, 0, 100 ) : cycledColor;
            textColor = lineColor;
        }
        RiuPlotAnnotationTool::horizontalDashedLineWithColor( line, lineColor, textColor, name, yPositions[i].first );
        line->attach( m_plot );
        m_markers.push_back( std::move( line ) );

        if ( ( i != names.size() - 1 ) && cvf::Math::abs( yPositions[i].second - yPositions[i + 1].first ) > delta )
        {
            QwtPlotMarker* bottomLine( new QwtPlotMarker() );
            RiuPlotAnnotationTool::horizontalDashedLineWithColor( bottomLine,
                                                                  lineColor,
                                                                  textColor,
                                                                  QString(),
                                                                  yPositions[i].second );

            bottomLine->attach( m_plot );
            m_markers.push_back( std::move( bottomLine ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachWellPicks( QwtPlot*                    plot,
                                             const std::vector<QString>& names,
                                             const std::vector<double>   yPositions )
{
    detachAllAnnotations();

    if ( names.size() != yPositions.size() ) return;
    m_plot = plot;

    for ( size_t i = 0; i < names.size(); i++ )
    {
        QwtPlotMarker* line( new QwtPlotMarker() );
        RiuPlotAnnotationTool::horizontalDashedLine( line, names[i], yPositions[i] );
        line->attach( m_plot );
        m_markers.push_back( std::move( line ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachAnnotationLine( QwtPlot*       plot,
                                                  const QColor&  color,
                                                  const QString& annotationText,
                                                  const double   yPosition )
{
    m_plot = plot;

    QwtPlotMarker* line( new QwtPlotMarker() );
    RiuPlotAnnotationTool::horizontalDashedLineWithColor( line, color, color, annotationText, yPosition );
    line->attach( m_plot );
    m_markers.push_back( std::move( line ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::detachAllAnnotations()
{
    if ( m_plot )
    {
        for ( size_t i = 0; i < m_markers.size(); i++ )
        {
            m_markers[i]->detach();
            delete m_markers[i];
        }
    }
    m_markers.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::horizontalDashedLine( QwtPlotMarker* line, const QString& name, double yValue )
{
    horizontalDashedLineWithColor( line, QColor( 0, 0, 100 ), QColor( 0, 0, 100 ), name, yValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::horizontalDashedLineWithColor(
    QwtPlotMarker* line, const QColor& color, const QColor& textColor, const QString& name, double yValue )
{
    QPen curvePen;
    curvePen.setStyle( Qt::DashLine );
    curvePen.setColor( color );
    curvePen.setWidth( 1 );

    line->setLineStyle( QwtPlotMarker::HLine );
    line->setLinePen( curvePen );
    line->setYValue( yValue );
    QwtText label( name );
    label.setColor( textColor );
    line->setLabel( label );
    line->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
}
