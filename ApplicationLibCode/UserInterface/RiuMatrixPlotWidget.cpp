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

#include "RiuMatrixPlotWidget.h"

#include "RiaColorTools.h"
#include "RiaPreferences.h"

#include "RimRegularLegendConfig.h"
#include "RimViewWindow.h"

#include "RiuAbstractLegendFrame.h"
#include "RiuCategoryLegendFrame.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuScalarMapperLegendFrame.h"

#include "cvfColor3.h"

#include "qwt_plot_marker.h"
#include "qwt_scale_draw.h"
#include "qwt_text.h"

#include <QHBoxLayout>

class MatrixShapeItem : public QwtPlotShapeItem
{
public:
    MatrixShapeItem( const QString& title = QString() )
        : QwtPlotShapeItem( title )
    {
    }

public:
    double value;
    size_t rowIndex;
    size_t columnIndex;
};

class TextScaleDraw : public QwtScaleDraw
{
public:
    TextScaleDraw( const std::map<size_t, QString>& tickLabels )
        : m_tickLabels( tickLabels )
    {
    }

    QwtText label( double value ) const override
    {
        size_t intValue = static_cast<size_t>( value + 0.25 );
        auto   it       = m_tickLabels.find( intValue );
        return it != m_tickLabels.end() ? it->second : "";
    }

private:
    std::map<size_t, QString> m_tickLabels;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMatrixPlotWidget::RiuMatrixPlotWidget( RimViewWindow* ownerViewWindow, RimRegularLegendConfig* legendConfig, QWidget* parent )
    : matrixCellSelected( this )
    , m_ownerViewWindow( ownerViewWindow )
    , m_legendConfig( legendConfig )
{
    // Configure main layout
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins( 15, 15, 15, 15 );
    setLayout( mainLayout );

    // White background
    QPalette palette = this->palette();
    palette.setColor( QPalette::Window, Qt::white );
    setAutoFillBackground( true );
    setPalette( palette );

    // Add plot to main layout
    m_plotWidget = new RiuQwtPlotWidget( nullptr, parent );
    m_plotWidget->qwtPlot()->insertLegend( nullptr );
    mainLayout->addWidget( m_plotWidget );

    // Add legend to main layout
    if ( m_legendConfig )
    {
        m_legendFrame = m_legendConfig->makeLegendFrame();
        mainLayout->addWidget( m_legendFrame );
    }

    // Configure plot widget to be a matrix plot?
    m_plotWidget->enableGridLines( RiuPlotAxis::defaultTop(), false, false );
    m_plotWidget->enableGridLines( RiuPlotAxis::defaultBottom(), false, false );
    m_plotWidget->enableGridLines( RiuPlotAxis::defaultRight(), false, false );
    m_plotWidget->enableGridLines( RiuPlotAxis::defaultLeft(), false, false );

    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMatrixPlotWidget::~RiuMatrixPlotWidget()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlot* RiuMatrixPlotWidget::qwtPlot() const
{
    return m_plotWidget->qwtPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::clearPlotData()
{
    m_columnHeaders = {};
    m_rowHeaders    = {};
    m_rowValues     = {};
    m_plotWidget->qwtPlot()->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setColumnHeaders( const std::vector<QString>& columnHeaders )
{
    if ( m_columnHeaders.empty() )
    {
        m_columnHeaders = columnHeaders;
    }
    else if ( columnHeaders.size() == m_columnHeaders.size() )
    {
        m_columnHeaders = columnHeaders;
    }
    CAF_ASSERT( "Column headers must be assigned for an empty matrix or re-assigned with an equal number of columns!" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setRowValues( const QString& rowLabel, const std::vector<double>& values )
{
    CAF_ASSERT( !m_columnHeaders.empty() && "Matrix column headers are not configured - headers are empty!" );
    CAF_ASSERT( values.size() == m_columnHeaders.size() && "Number of row values must be equal number of configured matrix columns" );

    // Insert in front to get rows from bottom to top in plot
    m_rowHeaders.insert( m_rowHeaders.begin(), rowLabel );
    m_rowValues.insert( m_rowValues.begin(), values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::createPlot()
{
    updateAxes();
    createMatrixCells();
    scheduleReplot();

    auto scalarMapperFrame = dynamic_cast<RiuScalarMapperLegendFrame*>( m_legendFrame.data() );
    auto categoryFrame     = dynamic_cast<RiuCategoryLegendFrame*>( m_legendFrame.data() );
    if ( scalarMapperFrame )
    {
        scalarMapperFrame->setScalarMapper( m_legendConfig->scalarMapper() );
        scalarMapperFrame->updateTickValues();
        scalarMapperFrame->update();
    }
    if ( categoryFrame )
    {
        categoryFrame->update();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::scheduleReplot()
{
    m_plotWidget->scheduleReplot();
}

RimViewWindow* RiuMatrixPlotWidget::ownerViewWindow() const
{
    return m_ownerViewWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::contextMenuEvent( QContextMenuEvent* )
{
    // Added empty override to preventing menu for Mdi Area
    // I.e.: RiuContextMenuLauncher for RiuPlotMainWindow (mdi area)
    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setPlotTitleEnabled( bool enabled )
{
    m_plotWidget->setPlotTitleEnabled( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setLegendFontSize( int fontSize )
{
    m_plotWidget->setLegendFontSize( fontSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setPlotTitle( const QString& title )
{
    m_plotWidget->setPlotTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setShowValueLabel( bool showValueLabel )
{
    m_showValueLabel = showValueLabel;

    // Due to few data points - clear plot and create matrix cells with new label flag
    m_plotWidget->qwtPlot()->detachItems();
    createMatrixCells();
    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setRowTitle( const QString& title )
{
    m_rowTitle = title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setInvalidValueColor( const cvf::Color3ub& color )
{
    m_invalidValueColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setUseInvalidValueColor( bool useInvalidValueColor )
{
    m_useInvalidValueColor = useInvalidValueColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setInvalidValueRange( double min, double max )
{
    CAF_ASSERT( min <= max && "Min must be less or equal to max!" );

    m_invalidValueRange = std::make_pair( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setColumnTitle( const QString& title )
{
    m_columnTitle = title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setPlotTitleFontSize( int fontSize )
{
    m_plotWidget->setPlotTitleFontSize( fontSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setAxisTitleFontSize( int fontSize )
{
    m_axisTitleFontSize = fontSize;
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), m_axisTitleFontSize, m_axisLabelFontSize );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(), m_axisTitleFontSize, m_axisLabelFontSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setAxisLabelFontSize( int fontSize )
{
    m_axisLabelFontSize = fontSize;
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), m_axisTitleFontSize, m_axisLabelFontSize );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(), m_axisTitleFontSize, m_axisLabelFontSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setValueFontSize( int fontSize )
{
    m_valueFontSize = fontSize;

    // Due to few data points - clear plot and create matrix cells with new font size
    m_plotWidget->qwtPlot()->detachItems();
    createMatrixCells();
    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::setMaxColumnLabelCount( int maxLabelCount )
{
    m_maxColumnLabelCount = maxLabelCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::updateAxes()
{
    if ( !m_plotWidget ) return;

    // Labels on y-axis
    const int maxLabelCount = 1000;
    m_plotWidget->qwtPlot()->setAxisScaleDraw( QwtAxis::YLeft, new TextScaleDraw( createIndexLabelMap( m_rowHeaders, maxLabelCount ) ) );
    m_plotWidget->qwtPlot()->setAxisScaleEngine( QwtAxis::YLeft, new RiuQwtLinearScaleEngine );
    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), m_rowTitle );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), m_axisTitleFontSize, m_axisLabelFontSize, false, Qt::AlignCenter );
    m_plotWidget->setAxisLabelsAndTicksEnabled( RiuPlotAxis::defaultLeft(), true, false );
    m_plotWidget->setAxisRange( RiuPlotAxis::defaultLeft(), 0.0, static_cast<double>( m_rowHeaders.size() ) + 1 );
    m_plotWidget->setMajorAndMinorTickIntervalsAndRange( RiuPlotAxis::defaultLeft(),
                                                         1.0,
                                                         0.0,
                                                         0.5,
                                                         static_cast<double>( m_rowHeaders.size() ) - 0.5,
                                                         0.0,
                                                         static_cast<double>( m_rowHeaders.size() ) );

    // Labels on column axis
    auto scaleDraw = new TextScaleDraw( createIndexLabelMap( m_columnHeaders, m_maxColumnLabelCount ) );
    scaleDraw->setLabelRotation( 30.0 );
    m_plotWidget->qwtPlot()->setAxisScaleDraw( QwtAxis::XBottom, scaleDraw );
    m_plotWidget->qwtPlot()->setAxisScaleEngine( QwtAxis::XBottom, new RiuQwtLinearScaleEngine );
    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), m_columnTitle );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(),
                                            m_axisTitleFontSize,
                                            m_axisLabelFontSize,
                                            false,
                                            Qt::AlignCenter | Qt::AlignTop );
    m_plotWidget->setAxisLabelsAndTicksEnabled( RiuPlotAxis::defaultBottom(), true, false );
    m_plotWidget->setAxisRange( RiuPlotAxis::defaultBottom(), 0.0, static_cast<double>( m_columnHeaders.size() ) + 1 );
    m_plotWidget->setMajorAndMinorTickIntervalsAndRange( RiuPlotAxis::defaultBottom(),
                                                         1.0,
                                                         0.0,
                                                         0.5,
                                                         static_cast<double>( m_columnHeaders.size() ) - 0.5,
                                                         0.0,
                                                         static_cast<double>( m_columnHeaders.size() ) );

    m_plotWidget->qwtPlot()->setAxisLabelAlignment( QwtAxis::XBottom, Qt::AlignRight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::createMatrixCells()
{
    CAF_ASSERT( m_legendConfig.notNull() && m_legendConfig->scalarMapper() && "Scalar mapper must be set for legend config!" );

    for ( size_t rowIdx = 0u; rowIdx < m_rowValues.size(); ++rowIdx )
    {
        for ( size_t colIdx = 0u; colIdx < m_rowValues[rowIdx].size(); ++colIdx )
        {
            const double value = m_rowValues[rowIdx][colIdx];
            const auto   label = QString( "%1" ).arg( value, 0, 'f', 2 );

            cvf::Color3ub color = m_legendConfig->scalarMapper()->mapToColor( value );

            if ( m_useInvalidValueColor && m_invalidValueRange.first <= value && value <= m_invalidValueRange.second )
            {
                color = m_invalidValueColor;
            }

            QColor qColor( color.r(), color.g(), color.b() );
            auto   rectangle = RiuQwtPlotTools::createBoxShapeT<MatrixShapeItem>( label,
                                                                                static_cast<double>( colIdx ),
                                                                                static_cast<double>( colIdx ) + 1.0,
                                                                                static_cast<double>( rowIdx ),
                                                                                static_cast<double>( rowIdx ) + 1,
                                                                                qColor );

            rectangle->value       = value;
            rectangle->rowIndex    = rowIdx;
            rectangle->columnIndex = colIdx;
            rectangle->attach( m_plotWidget->qwtPlot() );

            if ( m_showValueLabel )
            {
                QwtText      textLabel( label );
                cvf::Color3f contrastColor = RiaColorTools::contrastColor( cvf::Color3f( color ) );
                textLabel.setColor( RiaColorTools::toQColor( contrastColor ) );
                QFont font = textLabel.font();
                font.setPointSize( m_valueFontSize );
                textLabel.setFont( font );
                QwtPlotMarker* marker = new QwtPlotMarker();
                marker->setLabel( textLabel );
                marker->setXValue( colIdx + 0.5 );
                marker->setYValue( rowIdx + 0.5 );
                marker->attach( m_plotWidget->qwtPlot() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, QString> RiuMatrixPlotWidget::createIndexLabelMap( const std::vector<QString>& labels, int maxLabelCount )
{
    if ( labels.empty() ) return {};

    int increment = 1;
    if ( (int)labels.size() > maxLabelCount )
    {
        increment = (int)labels.size() / ( maxLabelCount - 1 );
        increment = std::max( 1, increment );
    }

    std::map<size_t, QString> indexLabelMap;
    for ( size_t i = 0; i < labels.size(); i += increment )
    {
        indexLabelMap.emplace( i, labels[i] );
    }

    indexLabelMap.emplace( labels.size() - 1, labels.back() );

    return indexLabelMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMatrixPlotWidget::onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex )
{
    RiuQwtPlotItem* qwtPlotItem = dynamic_cast<RiuQwtPlotItem*>( plotItem.get() );
    if ( !qwtPlotItem ) return;

    MatrixShapeItem* matrixItem = dynamic_cast<MatrixShapeItem*>( qwtPlotItem->qwtPlotItem() );
    if ( matrixItem )
    {
        matrixCellSelected.send( std::make_pair( static_cast<int>( matrixItem->rowIndex ), static_cast<int>( matrixItem->columnIndex ) ) );
    }
}
