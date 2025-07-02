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

#include "RiaPlotDefines.h"
#include "RimPlotAxisAnnotation.h"
#include "RiuGuiTheme.h"
#include "RiuQwtPlotCurveDefines.h"

#include "cafCategoryMapper.h"
#include "cvfMath.h"

#include "qwt_plot.h"
#include "qwt_plot_zoneitem.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"

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
                                                RiaDefines::Orientation                       depthOrientation,
                                                const std::vector<std::pair<double, double>>& regionRanges,
                                                RiaDefines::RegionDisplay                     regionDisplay,
                                                const caf::ColorTable&                        colorTable,
                                                int                                           shadingAlphaByte,
                                                bool                                          showNames /*= true */,
                                                RiaDefines::TrackSpan                         trackSpan /*= FULL_WIDTH*/,
                                                const std::vector<Qt::BrushStyle>&            brushStyles /* = {}*/,
                                                int                                           fontSize )
{
    if ( names.size() != regionRanges.size() ) return;
    m_plot = plot;

    double delta = 0.5;

    std::vector<int> categoryIndices( names.size() );
    std::iota( categoryIndices.begin(), categoryIndices.end(), 0 );

    caf::CategoryMapper catMapper;
    catMapper.setCategories( categoryIndices );
    catMapper.setInterpolateColors( colorTable.color3ubArray() );

    RiaDefines::Orientation annotationOrientation = RiaDefines::Orientation::HORIZONTAL;
    if ( depthOrientation == RiaDefines::Orientation::HORIZONTAL ) annotationOrientation = RiaDefines::Orientation::VERTICAL;

    for ( size_t i = 0; i < names.size(); i++ )
    {
        auto* line( new QwtPlotMarker() );

        QString name;
        if ( showNames && !names[i].isEmpty() )
        {
            name = names[i];
            if ( ( regionDisplay & RiaDefines::COLOR_SHADING ) == 0 && names[i].toLower().indexOf( "top" ) == -1 )
            {
                name += " Top";
            }
        }
        if ( regionDisplay & RiaDefines::COLOR_SHADING )
        {
            cvf::Color3ub cvfColor = catMapper.mapToColor( static_cast<double>( i ) );
            QColor        shadingColor( cvfColor.r(), cvfColor.g(), cvfColor.b(), shadingAlphaByte );

            auto* shading = new QwtPlotZoneItem();

            if ( depthOrientation == RiaDefines::Orientation::HORIZONTAL )
                shading->setOrientation( Qt::Vertical );
            else
                shading->setOrientation( Qt::Horizontal );

            shading->setInterval( regionRanges[i].first, regionRanges[i].second );
            shading->setPen( shadingColor, 0.0, Qt::NoPen );
            QBrush brush( shadingColor );
            if ( i < brushStyles.size() )
            {
                brush.setStyle( brushStyles[i] );
            }
            shading->setBrush( brush );
            shading->attach( m_plot );
            shading->setZ( -100.0 );
            shading->setXAxis( QwtAxis::XTop );
            m_plotItems.push_back( shading );
        }

        QColor lineColor( 0, 0, 0, 0 );
        QColor textColor( 0, 0, 0, 255 );
        if ( regionDisplay & RiaDefines::DARK_LINES || regionDisplay & RiaDefines::COLORED_LINES || regionDisplay & RiaDefines::LIGHT_LINES )
        {
            cvf::Color3ub cvfColor = catMapper.mapToColor( static_cast<double>( i ) );
            QColor        cycledColor( cvfColor.r(), cvfColor.g(), cvfColor.b() );

            if ( regionDisplay & RiaDefines::DARK_LINES )
            {
                lineColor = QColor( 50, 50, 100 );
            }
            else if ( regionDisplay & RiaDefines::LIGHT_LINES )
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

        RiuPlotAnnotationTool::setLineProperties( line,
                                                  name,
                                                  annotationOrientation,
                                                  regionRanges[i].first,
                                                  Qt::DashLine,
                                                  lineColor,
                                                  textColor,
                                                  horizontalAlignment,
                                                  fontSize );
        line->attach( m_plot );
        m_plotItems.push_back( line );

        if ( ( i != names.size() - 1 ) && cvf::Math::abs( regionRanges[i].second - regionRanges[i + 1].first ) > delta )
        {
            auto* bottomLine( new QwtPlotMarker() );

            RiuPlotAnnotationTool::setLineProperties( bottomLine,
                                                      QString(),
                                                      annotationOrientation,
                                                      regionRanges[i].second,
                                                      Qt::DashLine,
                                                      lineColor,
                                                      textColor,
                                                      Qt::AlignRight,
                                                      fontSize );
            bottomLine->attach( m_plot );
            m_plotItems.push_back( bottomLine );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachWellPicks( QwtPlot* plot, const std::vector<QString>& names, const std::vector<double>& yPositions )
{
    detachAllAnnotations();

    if ( names.size() != yPositions.size() ) return;
    m_plot = plot;

    for ( size_t i = 0; i < names.size(); i++ )
    {
        auto* line( new QwtPlotMarker() );
        RiuPlotAnnotationTool::setLineProperties( line, names[i], RiaDefines::Orientation::HORIZONTAL, yPositions[i] );
        line->attach( m_plot );
        m_plotItems.push_back( line );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachAnnotationLine( QwtPlot*                plot,
                                                  const QColor&           color,
                                                  const QString&          annotationText,
                                                  Qt::PenStyle            penStyle,
                                                  const double            position,
                                                  RiaDefines::Orientation orientation,
                                                  Qt::Alignment           horizontalAlignment,
                                                  int                     lineWidth,
                                                  RiaDefines::Orientation labelOrientation )
{
    m_plot = plot;

    auto* line( new QwtPlotMarker() );

    auto textColor = color;
    if ( orientation == RiaDefines::Orientation::VERTICAL )
    {
        textColor = RiuGuiTheme::getColorByVariableName( "textColor" );
    }

    RiuPlotAnnotationTool::setLineProperties( line,
                                              annotationText,
                                              orientation,
                                              position,
                                              penStyle,
                                              color,
                                              textColor,
                                              horizontalAlignment,
                                              0,
                                              lineWidth,
                                              labelOrientation );
    m_plotItems.push_back( line );
    line->attach( m_plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachAnnotationRange( QwtPlot*                plot,
                                                   const QColor&           color,
                                                   const QString&          annotationText,
                                                   const double            rangeStart,
                                                   const double            rangeEnd,
                                                   RiaDefines::Orientation orientation )
{
    m_plot = plot;
    if ( orientation == RiaDefines::Orientation::HORIZONTAL )
    {
        RiuPlotAnnotationTool::horizontalRange( annotationText,
                                                std::make_pair( rangeStart, rangeEnd ),
                                                color,
                                                RiuGuiTheme::getColorByVariableName( "textColor" ) );
    }
    else if ( orientation == RiaDefines::Orientation::VERTICAL )
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
void RiuPlotAnnotationTool::detachAllAnnotations()
{
    if ( m_plot )
    {
        for ( auto& plotItem : m_plotItems )
        {
            plotItem->detach();
            delete plotItem;
        }
    }
    m_plotItems.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::Alignment RiuPlotAnnotationTool::textAlignment( RiaDefines::TextAlignment alignment )
{
    switch ( alignment )
    {
        case RiaDefines::TextAlignment::LEFT:
            return Qt::AlignLeft;
        case RiaDefines::TextAlignment::CENTER:
            return Qt::AlignHCenter;
        case RiaDefines::TextAlignment::RIGHT:
            return Qt::AlignRight;
    }
    return Qt::AlignRight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::Alignment RiuPlotAnnotationTool::trackTextAlignment( RiaDefines::TrackSpan trackSpan )
{
    switch ( trackSpan )
    {
        case RiaDefines::TrackSpan::FULL_WIDTH:
            return Qt::AlignRight;
        case RiaDefines::TrackSpan::LEFT_COLUMN:
            return Qt::AlignLeft;
        case RiaDefines::TrackSpan::CENTRE_COLUMN:
            return Qt::AlignCenter;
        case RiaDefines::TrackSpan::RIGHT_COLUMN:
            return Qt::AlignRight;
    }
    return Qt::AlignRight;
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

    auto* shading = new QwtPlotZoneItem();
    shading->setOrientation( Qt::Horizontal );
    shading->setInterval( yRange.first, yRange.second );
    shading->setPen( shadingColor, 0.0, Qt::NoPen );
    QBrush brush( shadingColor );
    shading->setBrush( brush );
    shading->attach( m_plot );
    shading->setZ( -100.0 );
    shading->setXAxis( QwtAxis::XBottom );
    m_plotItems.push_back( shading );

    auto* line( new QwtPlotMarker() );
    RiuPlotAnnotationTool::setLineProperties( line, name, RiaDefines::Orientation::HORIZONTAL, yRange.first, Qt::DashLine, color, color, horizontalAlignment );
    line->attach( m_plot );
    m_plotItems.push_back( line );

    auto* bottomLine( new QwtPlotMarker() );
    RiuPlotAnnotationTool::setLineProperties( bottomLine, QString(), RiaDefines::Orientation::HORIZONTAL, yRange.second, Qt::DashLine, color, color );

    bottomLine->attach( m_plot );
    m_plotItems.push_back( bottomLine );
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

    auto* shading = new QwtPlotZoneItem();
    shading->setOrientation( Qt::Vertical );
    shading->setInterval( xRange.first, xRange.second );
    shading->setPen( shadingColor, 0.0, Qt::NoPen );
    QBrush brush( shadingColor );
    shading->setBrush( brush );
    shading->attach( m_plot );
    shading->setZ( -100.0 );
    shading->setXAxis( QwtAxis::XBottom );
    m_plotItems.push_back( shading );

    QStringList labels = name.split( " - " );

    // Define a lambda to calculate pixel distance between two timestamps
    auto getPixelDistanceBetweenTimestamps = []( QwtPlot* plot, time_t time1, time_t time2 ) -> double
    {
        const QwtScaleMap& scaleMap = plot->canvasMap( QwtAxis::XBottom );

        double t1 = static_cast<double>( time1 );
        double t2 = static_cast<double>( time2 );

        double pixel1 = scaleMap.transform( t1 );
        double pixel2 = scaleMap.transform( t2 );

        return std::abs( pixel2 - pixel1 );
    };

    // Show labels inside the range by default
    Qt::Alignment leftLineAlignment  = Qt::AlignRight;
    Qt::Alignment rightLineAlignment = Qt::AlignLeft;

    const auto distance          = getPixelDistanceBetweenTimestamps( m_plot, xRange.first, xRange.second );
    const auto distanceThreshold = 200.0;
    if ( distance < distanceThreshold )
    {
        // Show labels outside if the range is narrow
        leftLineAlignment  = Qt::AlignLeft;
        rightLineAlignment = Qt::AlignRight;
    }

    auto* line( new QwtPlotMarker() );
    RiuPlotAnnotationTool::setLineProperties( line,
                                              labels[0],
                                              RiaDefines::Orientation::VERTICAL,
                                              xRange.first,
                                              Qt::SolidLine,
                                              color,
                                              textColor,
                                              leftLineAlignment | horizontalAlignment );
    line->attach( m_plot );
    m_plotItems.push_back( line );

    auto* rightLine( new QwtPlotMarker() );
    RiuPlotAnnotationTool::setLineProperties( rightLine,
                                              labels.size() == 2 ? labels[1] : QString(),
                                              RiaDefines::Orientation::VERTICAL,
                                              xRange.second,
                                              Qt::SolidLine,
                                              color,
                                              textColor,
                                              rightLineAlignment | horizontalAlignment );
    rightLine->attach( m_plot );
    m_plotItems.push_back( rightLine );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::setLineProperties( QwtPlotMarker*          line,
                                               const QString&          name,
                                               RiaDefines::Orientation orientation,
                                               double                  linePosition,
                                               Qt::PenStyle            lineStyle,
                                               const QColor&           color /*= QColor( 0, 0, 100 )*/,
                                               const QColor&           textColor /*= QColor( 0, 0, 100 )*/,
                                               Qt::Alignment           horizontalAlignment /*= Qt::AlignRight*/,
                                               int                     fontSize /*= 0 */,
                                               int                     lineWidth,
                                               RiaDefines::Orientation labelOrientation )
{
    QPen curvePen;
    curvePen.setStyle( lineStyle );
    curvePen.setColor( color );
    curvePen.setWidth( lineWidth );

    line->setLinePen( curvePen );
    QwtText label( name );
    label.setColor( textColor );
    if ( fontSize > 0 ) label.setFont( QFont( label.font().key(), fontSize ) );
    line->setLabel( label );
    line->setLabelAlignment( horizontalAlignment | Qt::AlignBottom );

    line->setLabelOrientation( labelOrientation == RiaDefines::Orientation::HORIZONTAL ? Qt::Horizontal : Qt::Vertical );

    if ( orientation == RiaDefines::Orientation::HORIZONTAL )
    {
        line->setLineStyle( QwtPlotMarker::HLine );
        line->setYValue( linePosition );
    }
    else
    {
        line->setLineStyle( QwtPlotMarker::VLine );
        line->setXValue( linePosition );
    }

    line->setZ( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ANNOTATION ) );
}
