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

#include "RiuGuiTheme.h"

#include "cafCategoryMapper.h"
#include "cvfMath.h"

#include "qwt_plot.h"
#include "qwt_plot_zoneitem.h"

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
void RiuPlotAnnotationTool::attachNamedRegions( QwtPlot*                                      plot,
                                                const std::vector<QString>&                   names,
                                                const std::pair<double, double>               xRange,
                                                const std::vector<std::pair<double, double>>& yPositions,
                                                RegionDisplay                                 regionDisplay,
                                                const caf::ColorTable&                        colorTable,
                                                int                                           shadingAlphaByte,
                                                bool                                          showNames /*= true */,
                                                TrackSpan                          trackSpan /*= FULL_WIDTH*/,
                                                const std::vector<Qt::BrushStyle>& brushStyles /* = {}*/,
                                                int                                fontSize )
{
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
        QwtPlotMarker* line( new QwtPlotMarker() );

        QString name;
        if ( showNames && !names[i].isEmpty() )
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

            QwtPlotZoneItem* shading = new QwtPlotZoneItem();
            shading->setOrientation( Qt::Horizontal );
            shading->setInterval( yPositions[i].first, yPositions[i].second );
            shading->setPen( shadingColor, 0.0, Qt::NoPen );
            QBrush brush( shadingColor );
            if ( i < brushStyles.size() )
            {
                brush.setStyle( brushStyles[i] );
            }
            shading->setBrush( brush );
            shading->attach( m_plot );
            shading->setZ( -100.0 );
            shading->setXAxis( QwtPlot::xTop );
            m_horizontalMarkers.push_back( std::move( shading ) );
        }

        QColor lineColor( 0, 0, 0, 0 );
        QColor textColor( 0, 0, 0, 255 );
        if ( regionDisplay & DARK_LINES || regionDisplay & COLORED_LINES || regionDisplay & LIGHT_LINES )
        {
            cvf::Color3ub cvfColor = catMapper.mapToColor( static_cast<double>( i ) );
            QColor        cycledColor( cvfColor.r(), cvfColor.g(), cvfColor.b() );

            if ( regionDisplay & DARK_LINES )
            {
                lineColor = QColor( 50, 50, 100 );
            }
            else if ( regionDisplay & LIGHT_LINES )
            {
                lineColor = QColor( 200, 200, 200 );
            }
            else
            {
                lineColor = cycledColor;
            }
            textColor = lineColor;
        }
        Qt::Alignment horizontalAlignment = trackTextAlignment( trackSpan );
        RiuPlotAnnotationTool::horizontalDashedLine( line,
                                                     name,
                                                     yPositions[i].first,
                                                     lineColor,
                                                     textColor,
                                                     horizontalAlignment,
                                                     fontSize );
        line->attach( m_plot );
        m_horizontalMarkers.push_back( std::move( line ) );

        if ( ( i != names.size() - 1 ) && cvf::Math::abs( yPositions[i].second - yPositions[i + 1].first ) > delta )
        {
            QwtPlotMarker* bottomLine( new QwtPlotMarker() );
            RiuPlotAnnotationTool::horizontalDashedLine( bottomLine,
                                                         QString(),
                                                         yPositions[i].second,
                                                         lineColor,
                                                         textColor,
                                                         Qt::AlignRight,
                                                         fontSize );

            bottomLine->attach( m_plot );
            m_horizontalMarkers.push_back( std::move( bottomLine ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachWellPicks( QwtPlot*                    plot,
                                             const std::vector<QString>& names,
                                             const std::vector<double>&  yPositions )
{
    detachAllAnnotations( Orientation::HORIZONTAL );

    if ( names.size() != yPositions.size() ) return;
    m_plot = plot;

    for ( size_t i = 0; i < names.size(); i++ )
    {
        QwtPlotMarker* line( new QwtPlotMarker() );
        RiuPlotAnnotationTool::horizontalDashedLine( line, names[i], yPositions[i] );
        line->attach( m_plot );
        m_horizontalMarkers.push_back( std::move( line ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachAnnotationLine( QwtPlot*       plot,
                                                  const QColor&  color,
                                                  const QString& annotationText,
                                                  const double   position,
                                                  Orientation    orientation )
{
    m_plot = plot;

    QwtPlotMarker* line( new QwtPlotMarker() );
    if ( orientation == Orientation::HORIZONTAL )
    {
        RiuPlotAnnotationTool::horizontalDashedLine( line, annotationText, position, color, color );
        m_horizontalMarkers.push_back( std::move( line ) );
    }
    else if ( orientation == Orientation::VERTICAL )
    {
        RiuPlotAnnotationTool::verticalLine( line,
                                             annotationText,
                                             position,
                                             color,
                                             RiuGuiTheme::getColorByVariableName( "textColor" ) );
        m_verticalMarkers.push_back( std::move( line ) );
    }
    line->attach( m_plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachAnnotationRange( QwtPlot*       plot,
                                                   const QColor&  color,
                                                   const QString& annotationText,
                                                   const double   rangeStart,
                                                   const double   rangeEnd,
                                                   Orientation    orientation )
{
    m_plot = plot;
    if ( orientation == Orientation::HORIZONTAL )
    {
        RiuPlotAnnotationTool::horizontalRange( annotationText,
                                                std::make_pair( rangeStart, rangeEnd ),
                                                color,
                                                RiuGuiTheme::getColorByVariableName( "textColor" ) );
    }
    else if ( orientation == Orientation::VERTICAL )
    {
        RiuPlotAnnotationTool::verticalRange( annotationText,
                                              std::make_pair( rangeStart, rangeEnd ),
                                              color,
                                              RiuGuiTheme::getColorByVariableName( "textColor" ),
                                              Qt::AlignHCenter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::detachAllAnnotations( Orientation orientation )
{
    if ( m_plot )
    {
        if ( orientation == Orientation::HORIZONTAL )
        {
            for ( size_t i = 0; i < m_horizontalMarkers.size(); i++ )
            {
                m_horizontalMarkers[i]->detach();
                delete m_horizontalMarkers[i];
            }
        }
        else if ( orientation == Orientation::VERTICAL )
        {
            for ( size_t i = 0; i < m_verticalMarkers.size(); i++ )
            {
                m_verticalMarkers[i]->detach();
                delete m_verticalMarkers[i];
            }
        }
    }
    if ( orientation == Orientation::HORIZONTAL )
    {
        m_horizontalMarkers.clear();
    }
    else if ( orientation == Orientation::VERTICAL )
    {
        m_verticalMarkers.clear();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::detachAllAnnotations()
{
    detachAllAnnotations( Orientation::HORIZONTAL );
    detachAllAnnotations( Orientation::VERTICAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::Alignment RiuPlotAnnotationTool::trackTextAlignment( TrackSpan trackSpan )
{
    switch ( trackSpan )
    {
        case TrackSpan::FULL_WIDTH:
            return Qt::AlignRight;
        case TrackSpan::LEFT_COLUMN:
            return Qt::AlignLeft;
        case TrackSpan::CENTRE_COLUMN:
            return Qt::AlignCenter;
        case TrackSpan::RIGHT_COLUMN:
            return Qt::AlignRight;
    }
    return Qt::AlignRight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::horizontalDashedLine( QwtPlotMarker* line,
                                                  const QString& name,
                                                  double         yValue,
                                                  const QColor&  color /*= QColor(0, 0, 100) */,
                                                  const QColor&  textColor /*= QColor(0, 0, 100) */,
                                                  Qt::Alignment  horizontalAlignment /*= Qt::AlignRight */,
                                                  int            fontSize )
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
    if ( fontSize > 0 ) label.setFont( QFont( label.font().key(), fontSize ) );
    line->setLabel( label );
    line->setLabelAlignment( horizontalAlignment | Qt::AlignBottom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::horizontalRange( const QString&                  name,
                                             const std::pair<double, double> yRange,
                                             const QColor&                   color /*= QColor( 0, 0, 100 )*/,
                                             const QColor&                   textColor /*= QColor( 0, 0, 100 )*/,
                                             Qt::Alignment                   horizontalAlignment /*= Qt::AlignRight */ )
{
    QColor shadingColor = color;
    shadingColor.setAlpha( 10 );

    QwtPlotZoneItem* shading = new QwtPlotZoneItem();
    shading->setOrientation( Qt::Horizontal );
    shading->setInterval( yRange.first, yRange.second );
    shading->setPen( shadingColor, 0.0, Qt::NoPen );
    QBrush brush( shadingColor );
    shading->setBrush( brush );
    shading->attach( m_plot );
    shading->setZ( -100.0 );
    shading->setXAxis( QwtPlot::xBottom );
    m_horizontalMarkers.push_back( std::move( shading ) );

    QwtPlotMarker* line( new QwtPlotMarker() );
    RiuPlotAnnotationTool::horizontalDashedLine( line, name, yRange.first, color, color, horizontalAlignment );
    line->attach( m_plot );
    m_horizontalMarkers.push_back( std::move( line ) );

    QwtPlotMarker* bottomLine( new QwtPlotMarker() );
    RiuPlotAnnotationTool::horizontalDashedLine( bottomLine, QString(), yRange.second, color, color );

    bottomLine->attach( m_plot );
    m_horizontalMarkers.push_back( std::move( bottomLine ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::verticalRange( const QString&                  name,
                                           const std::pair<double, double> xRange,
                                           const QColor&                   color /*= QColor( 0, 0, 100 )*/,
                                           const QColor&                   textColor /*= QColor( 0, 0, 100 )*/,
                                           Qt::Alignment                   horizontalAlignment /*= Qt::AlignRight */ )
{
    QColor shadingColor = color;
    shadingColor.setAlpha( 50 );

    QwtPlotZoneItem* shading = new QwtPlotZoneItem();
    shading->setOrientation( Qt::Vertical );
    shading->setInterval( xRange.first, xRange.second );
    shading->setPen( shadingColor, 0.0, Qt::NoPen );
    QBrush brush( shadingColor );
    shading->setBrush( brush );
    shading->attach( m_plot );
    shading->setZ( -100.0 );
    shading->setXAxis( QwtPlot::xBottom );
    m_verticalMarkers.push_back( std::move( shading ) );

    QStringList labels = name.split( " - " );

    QwtPlotMarker* line( new QwtPlotMarker() );
    RiuPlotAnnotationTool::verticalLine( line,
                                         labels[0],
                                         xRange.first,
                                         color,
                                         textColor,
                                         Qt::SolidLine,
                                         Qt::AlignRight | horizontalAlignment );
    line->attach( m_plot );
    m_verticalMarkers.push_back( std::move( line ) );

    QwtPlotMarker* rightLine( new QwtPlotMarker() );
    RiuPlotAnnotationTool::verticalLine( rightLine,
                                         labels.size() == 2 ? labels[1] : QString(),
                                         xRange.second,
                                         color,
                                         textColor,
                                         Qt::SolidLine,
                                         Qt::AlignLeft | horizontalAlignment );
    rightLine->attach( m_plot );
    m_verticalMarkers.push_back( std::move( rightLine ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::verticalLine( QwtPlotMarker* line,
                                          const QString& name,
                                          double         xValue,
                                          const QColor&  color /*= QColor(0, 0, 100) */,
                                          const QColor&  textColor /*= QColor(0, 0, 100) */,
                                          Qt::PenStyle   lineStyle /*= Qt::DashLine */,
                                          Qt::Alignment  horizontalAlignment /*= Qt::AlignRight | Qt::AlignBottom */ )
{
    QPen curvePen;
    curvePen.setStyle( lineStyle );
    curvePen.setColor( color );
    curvePen.setWidth( 1 );

    line->setAxes( QwtPlot::xBottom, QwtPlot::yLeft );
    line->setLineStyle( QwtPlotMarker::VLine );
    line->setLinePen( curvePen );
    line->setXValue( xValue );
    QwtText label( name );
    label.setColor( textColor );
    line->setLabel( label );
    line->setLabelAlignment( horizontalAlignment );
    line->setLabelOrientation( Qt::Orientation::Vertical );
}
