/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RiuQwtPlotTools.h"

#include "RiaApplication.h"
#include "RiaColorTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiuPlotAxis.h"

#include "Summary/RiaSummaryAddressAnalyzer.h"

#include "RimEnsembleCurveSet.h"
#include "RimPlotCurve.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"

#include "RiuGuiTheme.h"
#include "RiuPlotCurveSymbolImageCreator.h"
#include "RiuQwtPlotLegend.h"

#include "qwt_axis.h"
#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_graphic.h"
#include "qwt_painter.h"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_item.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_shapeitem.h"
#include "qwt_scale_widget.h"
#include "qwt_text.h"

#include <QCryptographicHash>
#include <QImage>
#include <QPainterPath>
#include <QSet>

#include <map>
#include <set>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::setCommonPlotBehaviour( QwtPlot* plot )
{
    // Plot background and frame look

    QPalette newPalette( plot->palette() );
    newPalette.setColor( QPalette::Window, Qt::white );
    plot->setPalette( newPalette );

    plot->setAutoFillBackground( true );
    plot->setCanvasBackground( Qt::white );
    plot->plotLayout()->setCanvasMargin( 0, -1 );

    QFrame* canvasFrame = dynamic_cast<QFrame*>( plot->canvas() );
    canvasFrame->setFrameShape( QFrame::Box );

    // Grid
    QwtPlotGrid* grid = new QwtPlotGrid;
    grid->attach( plot );
    QPen gridPen( Qt::SolidLine );
    grid->setPen( gridPen );
    RiuGuiTheme::styleQwtItem( grid );

    // Axis number font
    int axisFontSize =
        caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), caf::FontTools::RelativeSize::Medium );
    QFont axisFont = plot->axisFont( QwtAxis::XBottom );
    axisFont.setPointSize( axisFontSize );

    plot->setAxisFont( QwtAxis::XBottom, axisFont );
    plot->setAxisFont( QwtAxis::XTop, axisFont );
    plot->setAxisFont( QwtAxis::YLeft, axisFont );
    plot->setAxisFont( QwtAxis::YRight, axisFont );

    // Axis title font
    std::vector<QwtAxis::Position> axes = { QwtAxis::XBottom, QwtAxis::XTop, QwtAxis::YLeft, QwtAxis::YRight };

    for ( QwtAxis::Position axis : axes )
    {
        QwtText axisTitle     = plot->axisTitle( axis );
        QFont   axisTitleFont = axisTitle.font();
        axisTitleFont.setPointSize( axisFontSize );
        axisTitleFont.setBold( false );
        axisTitle.setFont( axisTitleFont );
        axisTitle.setRenderFlags( Qt::AlignRight );

        plot->setAxisTitle( axis, axisTitle );
    }

    // Set a focus policy to allow it taking key press events.
    // This is not strictly necessary since this widget inherit QwtPlot
    // which already has a focus policy.
    // However, for completeness we still do it here.
    plot->setFocusPolicy( Qt::WheelFocus );

    // Enable mousetracking and event filter
    plot->canvas()->setMouseTracking( true );
    plot->plotLayout()->setAlignCanvasToScales( true );

    plot->setContentsMargins( 1, 1, 1, 1 );

    // Store the pointer address as an object name. This way each plot can be identified uniquely for CSS-stylesheets
    QString objectName = QString( "%1" ).arg( reinterpret_cast<uint64_t>( plot ) );
    plot->setObjectName( objectName );

    QString canvasName = QString( "%1" ).arg( reinterpret_cast<uint64_t>( plot->canvas() ) );
    plot->canvas()->setObjectName( canvasName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::setDefaultAxes( QwtPlot* plot )
{
    plot->setAxesCount( QwtAxis::XBottom, 1 );
    plot->setAxesCount( QwtAxis::YLeft, 1 );

    plot->axisWidget( QwtAxis::XBottom )->setMargin( 0 );
    plot->axisWidget( QwtAxis::YLeft )->setMargin( 0 );
    plot->axisWidget( QwtAxis::XTop )->setMargin( 0 );
    plot->axisWidget( QwtAxis::YRight )->setMargin( 0 );

    plot->setAxisMaxMinor( QwtAxis::XBottom, 2 );
    plot->setAxisMaxMinor( QwtAxis::YLeft, 3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::enableDateBasedBottomXAxis( QwtPlot*                         plot,
                                                  const QString&                   dateFormat,
                                                  const QString&                   timeFormat,
                                                  RiaDefines::DateFormatComponents dateComponents,
                                                  RiaDefines::TimeFormatComponents timeComponents )
{
    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw( Qt::UTC );

    std::set<QwtDate::IntervalType> intervals =
        { QwtDate::Year, QwtDate::Month, QwtDate::Week, QwtDate::Day, QwtDate::Hour, QwtDate::Minute, QwtDate::Second, QwtDate::Millisecond };

    for ( QwtDate::IntervalType interval : intervals )
    {
        scaleDraw->setDateFormat( interval, dateTimeFormatForInterval( interval, dateFormat, timeFormat, dateComponents, timeComponents ) );
    }

    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine( Qt::UTC );
    plot->setAxisScaleEngine( QwtAxis::XBottom, scaleEngine );
    plot->setAxisScaleDraw( QwtAxis::XBottom, scaleDraw );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuQwtPlotTools::dateTimeFormatForInterval( QwtDate::IntervalType            interval,
                                                    const QString&                   dateFormat,
                                                    const QString&                   timeFormat,
                                                    RiaDefines::DateFormatComponents dateComponents,
                                                    RiaDefines::TimeFormatComponents timeComponents )
{
    if ( dateComponents != RiaDefines::DateFormatComponents::DATE_FORMAT_UNSPECIFIED &&
         timeComponents != RiaDefines::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED )
    {
        return RiaQDateTimeTools::timeFormatString( timeFormat, timeComponents ) + "\n" +
               RiaQDateTimeTools::dateFormatString( dateFormat, dateComponents );
    }
    else
    {
        switch ( interval )
        {
            case QwtDate::Millisecond:
                return RiaQDateTimeTools::timeFormatString( timeFormat,
                                                            RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND_MILLISECOND );
            case QwtDate::Second:
                return RiaQDateTimeTools::timeFormatString( timeFormat, RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );
            case QwtDate::Minute:
            {
                QString fullFormat =
                    RiaQDateTimeTools::timeFormatString( timeFormat, RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );
                fullFormat += "\n";
                fullFormat += RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
                return fullFormat;
            }
            case QwtDate::Hour:
            {
                QString fullFormat = RiaQDateTimeTools::timeFormatString( timeFormat, RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR );
                if ( !fullFormat.endsWith( "AP" ) )
                {
                    fullFormat += ":00";
                }
                fullFormat += "\n";
                fullFormat += RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
                return fullFormat;
            }
            case QwtDate::Day:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
            case QwtDate::Week:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH );
            case QwtDate::Month:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH );
            case QwtDate::Year:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR );
            default:
                return RiaQDateTimeTools::dateFormatString( dateFormat, RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotShapeItem*
    RiuQwtPlotTools::createBoxShape( const QString& label, double startX, double endX, double startY, double endY, QColor color, Qt::BrushStyle brushStyle )
{
    return createBoxShapeT<QwtPlotShapeItem>( label, startX, endX, startY, endY, color, brushStyle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtAxis::Position RiuQwtPlotTools::toQwtPlotAxisEnum( RiaDefines::PlotAxis riaPlotAxis )
{
    if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
        return QwtAxis::YLeft;
    else if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        return QwtAxis::YRight;
    else if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        return QwtAxis::XBottom;
    else if ( riaPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_TOP )
        return QwtAxis::XTop;

    return QwtAxis::YLeft;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RiuQwtPlotTools::fromQwtPlotAxis( QwtAxis::Position axis )
{
    if ( axis == QwtAxis::YLeft )
        return RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    else if ( axis == QwtAxis::YRight )
        return RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    else if ( axis == QwtAxis::XBottom )
        return RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
    else if ( axis == QwtAxis::XTop )
        return RiaDefines::PlotAxis::PLOT_AXIS_TOP;

    return RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::updateLegendData( RiuQwtPlotLegend* legend, const std::vector<RimPlotCurve*>& curves )
{
    QList<QwtLegendData> legendDataList = createLegendData( curves );

    legend->updateLegend( QVariant(), legendDataList );
}

//--------------------------------------------------------------------------------------------------
/// Collect legend data, one entry per legend item, from the plots. Items with identical title and icon are merged.
//--------------------------------------------------------------------------------------------------
QList<QList<QwtLegendData>> RiuQwtPlotTools::uniqueLegendData( const std::vector<QwtPlot*>& plots )
{
    QList<QList<QwtLegendData>> uniqueData;
    QSet<QString>               keys;

    for ( QwtPlot* plot : plots )
    {
        if ( !plot ) continue;

        for ( QwtPlotItem* item : plot->itemList() )
        {
            if ( !item->testItemAttribute( QwtPlotItem::Legend ) ) continue;

            QList<QwtLegendData> legendData = item->legendData();
            if ( legendData.isEmpty() ) continue;

            QString title = legendData.first().value( QwtLegendData::TitleRole ).value<QwtText>().text();

            // Include the icon in the key, so entries with the same title but different color or style are all kept
            QString key  = title;
            auto    icon = legendData.first().value( QwtLegendData::IconRole ).value<QwtGraphic>();
            if ( !icon.isNull() )
            {
                QImage     image = icon.toImage( QSize( 32, 16 ).expandedTo( icon.defaultSize().toSize() ) );
                QByteArray bytes( reinterpret_cast<const char*>( image.constBits() ), image.sizeInBytes() );
                key += "|" + QString::fromLatin1( QCryptographicHash::hash( bytes, QCryptographicHash::Md5 ).toHex() );
            }
            if ( keys.contains( key ) ) continue;

            keys.insert( key );
            uniqueData.append( legendData );
        }
    }

    return uniqueData;
}

//--------------------------------------------------------------------------------------------------
/// Draw a legend icon consisting of an optional horizontal line and/or an optional symbol.
//--------------------------------------------------------------------------------------------------
static QwtGraphic createCurveLegendIcon( const QSize&                        size,
                                         bool                                drawLine,
                                         const QColor&                       lineColor,
                                         RiuPlotCurveSymbol::PointSymbolEnum symbol,
                                         const QColor&                       symbolColor )
{
    QwtGraphic graphic;
    graphic.setDefaultSize( size );
    graphic.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &graphic );
    painter.setRenderHint( QPainter::Antialiasing );

    // QwtGraphic computes its own bounding rect from the actually painted content (see
    // QwtGraphic::drawImage()/drawPath()), and the legend later rescales that bounding rect to fill its
    // fixed icon slot. Without this, a symbol painted smaller than the full icon ends up with a smaller
    // bounding rect that gets rescaled back up to fill the icon slot, defeating the size reduction below.
    // Draw an invisible full-size rect first to pin the bounding rect to the full icon size.
    QPainterPath boundsPath;
    boundsPath.addRect( 0.0, 0.0, size.width(), size.height() );
    painter.setPen( Qt::NoPen );
    painter.setBrush( Qt::NoBrush );
    painter.drawPath( boundsPath );

    if ( drawLine )
    {
        // A bit thicker than a plain hairline, so colored curve lines are as visually prominent as curve symbols.
        const double lineWidth = 3.0;

        QPen pn;
        pn.setCapStyle( Qt::FlatCap );
        pn.setColor( lineColor );
        pn.setWidthF( lineWidth );

        painter.setPen( pn );

        const double y = 0.5 * size.height();
        QwtPainter::drawLine( &painter, 0.0, y, size.width(), y );
    }

    if ( symbol != RiuPlotCurveSymbol::SYMBOL_NONE )
    {
        // Draw the symbol clearly smaller than the full icon, so it does not dominate the icon visually.
        const double symbolScale = 0.45;
        QSize        symbolSize( static_cast<int>( size.width() * symbolScale ), static_cast<int>( size.height() * symbolScale ) );

        auto image = RiuPlotCurveSymbolImageCreator::createSymbolImage( symbol, symbolSize, symbolColor );

        QPoint p( ( size.width() - symbolSize.width() ) / 2, ( size.height() - symbolSize.height() ) / 2 );
        painter.drawImage( p, image );
    }

    return graphic;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QwtLegendData> RiuQwtPlotTools::createLegendData( const std::vector<RimPlotCurve*>& curves )
{
    QList<QwtLegendData> legendDataList;

    for ( auto c : curves )
    {
        QwtLegendData test;
        test.setValue( QwtLegendData::Role::TitleRole, c->curveName() );

        c->updateUiIconFromPlotSymbol();
        auto icon = c->uiIcon();
        auto size = icon->availableSizes().first();
        // see QwtPlotCurve::legendIcon

        QColor     curveColor = RiaColorTools::toQColor( c->color() );
        QwtGraphic graphic    = createCurveLegendIcon( size, /*drawLine=*/true, curveColor, c->symbol(), curveColor );

        QVariant v = QVariant::fromValue( graphic );
        test.setValue( QwtLegendData::Role::IconRole, v );

        legendDataList.push_back( test );
    }

    return legendDataList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static QwtLegendData createLegendEntry( const QString& title, const QwtGraphic& icon )
{
    QwtLegendData entry;
    entry.setValue( QwtLegendData::Role::TitleRole, title );
    entry.setValue( QwtLegendData::Role::IconRole, QVariant::fromValue( icon ) );
    return entry;
}

namespace
{
enum class SummaryLegendDimension
{
    CASE,
    VECTOR,
    OBJECT // Well, group, region, network, block, aquifer, ... (see RifEclipseSummaryAddress::itemUiText())
};

QString dimensionValue( RimSummaryCurve* curve, SummaryLegendDimension dimension )
{
    switch ( dimension )
    {
        case SummaryLegendDimension::CASE:
            return curve->summaryCaseY()->displayCaseName();
        case SummaryLegendDimension::VECTOR:
            return QString::fromStdString( curve->summaryAddressY().vectorName() );
        case SummaryLegendDimension::OBJECT:
            return QString::fromStdString( curve->summaryAddressY().itemUiText() );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Returns a legend label for each curve property (color or symbol), if the property is given by the
/// dimension (each dimension value has one property). Dimension values sharing a property are combined
/// into one label. Returns an empty map if the property is not given by the dimension, or does not vary.
//--------------------------------------------------------------------------------------------------
template <typename T, typename PropertyFunc>
std::map<T, QString> labelsForProperties( const std::vector<RimSummaryCurve*>& curves, SummaryLegendDimension dimension, PropertyFunc property )
{
    std::map<QString, T> propertyForValue;
    for ( auto curve : curves )
    {
        auto it = propertyForValue.emplace( dimensionValue( curve, dimension ), property( curve ) ).first;
        if ( it->second != property( curve ) ) return {};
    }

    std::map<T, QStringList> valuesForProperty;
    for ( const auto& [value, prop] : propertyForValue )
        valuesForProperty[prop].push_back( value );
    if ( valuesForProperty.size() < 2 ) return {};

    std::map<T, QString> labels;
    for ( const auto& [prop, values] : valuesForProperty )
        labels[prop] = values.join( ", " );

    return labels;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Create a color legend and a symbol legend when the curve color is given by one summary dimension (case,
/// vector or object, e.g. well, group or region) and the curve symbol by another, e.g. one color per vector
/// and one symbol per case. This gives (color count + symbol count) entries instead of one entry per
/// color/symbol combination.
///
/// Returns false if the curves are not plain summary curves, or if no such pair of dimensions is found.
//--------------------------------------------------------------------------------------------------
bool RiuQwtPlotTools::createColorAndSymbolLegendData( const std::vector<RimPlotCurve*>& curves,
                                                      QList<QList<QwtLegendData>>&      colorLegendData,
                                                      QList<QList<QwtLegendData>>&      symbolLegendData )
{
    std::vector<RimSummaryCurve*>         summaryCurves;
    std::vector<RifEclipseSummaryAddress> addresses;
    std::set<QString>                     caseNames;
    std::set<std::string>                 objectNames;

    for ( auto curve : curves )
    {
        auto summaryCurve = dynamic_cast<RimSummaryCurve*>( curve );
        if ( !summaryCurve || !summaryCurve->summaryCaseY() ) return false;
        if ( summaryCurve->firstAncestorOrThisOfType<RimEnsembleCurveSet>() ) return false;

        summaryCurves.push_back( summaryCurve );
        addresses.push_back( summaryCurve->summaryAddressY() );
        caseNames.insert( summaryCurve->summaryCaseY()->displayCaseName() );
        objectNames.insert( summaryCurve->summaryAddressY().itemUiText() );
    }

    RiaSummaryAddressAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    // Candidate dimensions are the ones with more than one value, in order of preference for the color legend
    std::vector<SummaryLegendDimension> dimensions;
    if ( caseNames.size() > 1 ) dimensions.push_back( SummaryLegendDimension::CASE );
    if ( analyzer.quantities().size() > 1 ) dimensions.push_back( SummaryLegendDimension::VECTOR );

    // Field and other addresses without an object have no object text to use as a legend label
    if ( objectNames.size() > 1 && !objectNames.contains( "" ) ) dimensions.push_back( SummaryLegendDimension::OBJECT );

    auto curveColor  = []( RimSummaryCurve* curve ) { return curve->color(); };
    auto curveSymbol = []( RimSummaryCurve* curve ) { return curve->symbol(); };

    for ( auto colorDimension : dimensions )
    {
        auto colors = labelsForProperties<cvf::Color3f>( summaryCurves, colorDimension, curveColor );
        if ( colors.empty() ) continue;

        for ( auto symbolDimension : dimensions )
        {
            if ( symbolDimension == colorDimension ) continue;

            auto symbols = labelsForProperties<RiuPlotCurveSymbol::PointSymbolEnum>( summaryCurves, symbolDimension, curveSymbol );
            if ( symbols.empty() ) continue;

            // Same square icon size as the regular curve legend icons (see RimPlotCurve::updateUiIconFromPlotSymbol())
            const QSize iconSize( 24, 24 );

            for ( const auto& [color, label] : colors )
            {
                const QColor lineColor = RiaColorTools::toQColor( color );
                colorLegendData.push_back(
                    { createLegendEntry( label,
                                         createCurveLegendIcon( iconSize, true, lineColor, RiuPlotCurveSymbol::SYMBOL_NONE, lineColor ) ) } );
            }

            // Neutral symbol color, as color is given by the color legend
            const QColor symbolColor( 60, 60, 60 );
            for ( const auto& [symbol, label] : symbols )
            {
                symbolLegendData.push_back(
                    { createLegendEntry( label, createCurveLegendIcon( iconSize, false, symbolColor, symbol, symbolColor ) ) } );
            }

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotTools::enableGridLines( QwtPlot* plot, QwtAxis::Position axis, bool enableMajor, bool enableMinor )
{
    // NB: Make a copy, not reference to avoid iterator invalidation
    QwtPlotItemList plotItems = plot->itemList( QwtPlotItem::Rtti_PlotGrid );
    for ( QwtPlotItem* plotItem : plotItems )
    {
        auto* grid = static_cast<QwtPlotGrid*>( plotItem );
        if ( axis == QwtAxis::XTop || axis == QwtAxis::XBottom )
        {
            grid->setXAxis( axis );
            grid->enableX( enableMajor );
            grid->enableXMin( enableMinor );
        }
        else
        {
            grid->setYAxis( axis );
            grid->enableY( enableMajor );
            grid->enableYMin( enableMinor );
        }
        grid->setMajorPen( Qt::lightGray, 1.0, Qt::SolidLine );
        grid->setMinorPen( Qt::lightGray, 1.0, Qt::DashLine );
    }
}
