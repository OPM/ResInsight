/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimStatisticsPlot.h"

#include "RiaColorTools.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RimPlot.h"
#include "RimProject.h"
#include "RimTools.h"

#include "RiuPlotMainWindow.h"

#include "RigHistogramData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"

#include "cafPdmUiSliderEditor.h"
#include "cvfAssert.h"

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <cmath>

namespace caf
{
template <>
void caf::AppEnum<RimStatisticsPlot::HistogramFrequencyType>::setUp()
{
    addItem( RimStatisticsPlot::HistogramFrequencyType::ABSOLUTE_FREQUENCY, "ABSOLUTE_FREQUENCY", "Absolute Frequency" );
    addItem( RimStatisticsPlot::HistogramFrequencyType::RELATIVE_FREQUENCY, "RELATIVE_FREQUENCY", "Relative Frequency" );
    addItem( RimStatisticsPlot::HistogramFrequencyType::RELATIVE_FREQUENCY_PERCENT, "RELATIVE_FREQUENCY_PERCENT", "Relative Frequency [%]" );
    setDefault( RimStatisticsPlot::HistogramFrequencyType::RELATIVE_FREQUENCY_PERCENT );
}
template <>
void caf::AppEnum<RimStatisticsPlot::GraphType>::setUp()

{
    addItem( RimStatisticsPlot::GraphType::BAR_GRAPH, "BAR_GRAPH", "Bar Graph" );
    addItem( RimStatisticsPlot::GraphType::LINE_GRAPH, "LINE_GRAPH", "Line Graph" );
    setDefault( RimStatisticsPlot::GraphType::BAR_GRAPH );
}

} // namespace caf

CAF_PDM_ABSTRACT_SOURCE_INIT( RimStatisticsPlot, "StatisticsPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsPlot::RimStatisticsPlot()
{
    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name" );
    m_plotWindowTitle.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_numHistogramBins, "NumHistogramBins", 50, "Number of Bins" );
    m_numHistogramBins.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_histogramBarColor, "HistogramBarColor", cvf::Color3f( cvf::Color3f::SKY_BLUE ), "Color" );

    CAF_PDM_InitField( &m_histogramGapWidth, "HistogramGapWidth", 0.0, "Gap Width [%]" );
    m_histogramGapWidth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_histogramFrequencyType, "HistogramFrequencyType", "Frequency" );

    CAF_PDM_InitField( &m_precision, "Precision", 4, "Significant Digits", "", "The number of significant digits displayed in the legend numbers", "" );
    CAF_PDM_InitField( &m_tickNumberFormat,
                       "TickNumberFormat",
                       caf::AppEnum<RiaNumberFormat::NumberFormatType>( RiaNumberFormat::NumberFormatType::AUTO ),
                       "Number format" );

    CAF_PDM_InitFieldNoDefault( &m_graphType, "GraphType", "Graph Type" );

    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    m_axisValueFontSize = caf::FontTools::RelativeSize::Small;
    m_axisTitleFontSize = caf::FontTools::RelativeSize::Medium;

    m_plotLegendsHorizontal.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsPlot::~RimStatisticsPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimStatisticsPlot::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimStatisticsPlot::createPlotWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    return createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsPlot::description() const
{
    return m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStatisticsPlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStatisticsPlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimStatisticsPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        QPixmap pix = m_viewer->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimStatisticsPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_viewer = new RiuQtChartView( this, mainWindowParent );
    m_viewer->setRenderHint( QPainter::Antialiasing );
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_axisTitleFontSize || changedField == &m_axisValueFontSize )
    {
        updateLayout();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::onPlotAdditionOrRemoval()
{
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewer )
    {
        QPainter painter( paintDevice );
        m_viewer->render( &painter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::doUpdateLayout()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::cleanupBeforeClose()
{
    if ( m_viewer )
    {
        m_viewer->setParent( nullptr );
        delete m_viewer;
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiLineEditorAttribute* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
    if ( field == &m_numHistogramBins && lineEditorAttr != nullptr )
    {
        // Limit histogram bins to positive value
        QIntValidator* validator  = new QIntValidator( 2, 10000, nullptr );
        lineEditorAttr->validator = validator;
    }

    caf::PdmUiDoubleSliderEditorAttribute* sliderAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
    if ( field == &m_histogramGapWidth && sliderAttr != nullptr )
    {
        sliderAttr->m_minimum = 0.0;
        sliderAttr->m_maximum = 100.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStatisticsPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions );

    if ( !options.empty() ) return options;

    if ( fieldNeedingOptions == &m_axisTitleFontSize || fieldNeedingOptions == &m_axisValueFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::uiOrderingForHistogram( QString uiConfigName, caf::PdmUiOrdering& uiOrdering, bool showHistogramBins )
{
    caf::PdmUiGroup* histogramGroup = uiOrdering.addNewGroup( "Histogram" );
    if ( showHistogramBins ) histogramGroup->add( &m_numHistogramBins );
    histogramGroup->add( &m_histogramBarColor );
    histogramGroup->add( &m_graphType );
    if ( m_graphType == GraphType::BAR_GRAPH ) histogramGroup->add( &m_histogramGapWidth );
    histogramGroup->add( &m_histogramFrequencyType );
    histogramGroup->add( &m_precision );
    histogramGroup->add( &m_tickNumberFormat );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::uiOrderingForLegendsAndFonts( QString uiConfigName, caf::PdmUiOrdering& uiOrdering, bool showLegendPosition )
{
    RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering, showLegendPosition );

    auto* fontGroup = uiOrdering.findGroup( "Fonts" );
    fontGroup->add( &m_axisTitleFontSize );
    fontGroup->add( &m_axisValueFontSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    performAutoNameUpdate();
    updatePlots();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::updatePlots()
{
    if ( !hasStatisticsData() ) return;

    RigHistogramData histogramData = createStatisticsData();

    if ( !histogramData.isHistogramVectorValid() ) return;

    QBarSet* set0     = new QBarSet( m_plotWindowTitle );
    double   minValue = std::numeric_limits<double>::max();
    double   maxValue = -std::numeric_limits<double>::max();

    QColor color = RiaColorTools::toQColor( m_histogramBarColor );

    // Make border same color as bar when user wants max bar width
    if ( m_histogramGapWidth() == 0.0 )
    {
        set0->setBorderColor( color );
    }

    double sumElements = 0.0;
    for ( double value : histogramData.histogram )
        sumElements += value;

    QLineSeries* lineSeries = new QLineSeries();
    lineSeries->setName( m_plotWindowTitle );

    QPen pen( color );
    pen.setWidth( 2 );
    lineSeries->setPen( pen );

    double binSize   = ( histogramData.max - histogramData.min ) / histogramData.histogram.size();
    double binCenter = histogramData.min;
    for ( double value : histogramData.histogram )
    {
        if ( m_histogramFrequencyType() == HistogramFrequencyType::RELATIVE_FREQUENCY ) value /= sumElements;
        if ( m_histogramFrequencyType() == HistogramFrequencyType::RELATIVE_FREQUENCY_PERCENT ) value = value / sumElements * 100.0;
        *set0 << value;
        *lineSeries << QPointF( binCenter, value );
        binCenter += binSize;

        minValue = std::min( minValue, value );
        maxValue = std::max( maxValue, value );
    }
    set0->setColor( color );
    lineSeries->setColor( color );

    QBarSeries* series = new QBarSeries();
    series->setBarWidth( ( 100.0 - m_histogramGapWidth() ) / 100.0 );
    series->append( set0 );

    QChart* chart = new QChart();
    if ( m_graphType == GraphType::BAR_GRAPH ) chart->addSeries( series );
    if ( m_graphType == GraphType::LINE_GRAPH ) chart->addSeries( lineSeries );
    chart->setTitle( uiName() );

    // Axis
    double xAxisSize      = histogramData.max - histogramData.min;
    double xAxisExtension = xAxisSize * 0.02;

    QValueAxis* axisX = new QValueAxis();
    axisX->setRange( histogramData.min - xAxisExtension, histogramData.max + xAxisExtension );
    axisX->setLabelFormat( RiaNumberFormat::sprintfFormat( m_tickNumberFormat(), m_precision ) );
    axisX->setTitleText( createXAxisTitle() );
    chart->addAxis( axisX, Qt::AlignBottom );

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange( minValue, maxValue );
    axisY->setLabelFormat( RiaNumberFormat::sprintfFormat( m_tickNumberFormat(), m_precision ) );
    axisY->setTitleText( createYAxisTitle() );
    chart->addAxis( axisY, Qt::AlignLeft );

    // Scaling to match font sizes in Qwt
    const double fontScalingToMatchQwt = 1.5;

    // Create vertical lines for statistics data
    std::vector<std::pair<QString, double>> statisticsData = { { "P90", histogramData.p90 },
                                                               { "Mean", histogramData.mean },
                                                               { "P10", histogramData.p10 } };

    for ( const auto& [name, value] : statisticsData )
    {
        if ( std::isinf( value ) ) continue;

        QLineSeries* series = new QLineSeries();
        chart->addSeries( series );
        series->append( value, minValue );
        series->append( value, maxValue );
        series->attachAxis( axisX );
        series->attachAxis( axisY );
        series->setName( QString( "%1 (%2)" ).arg( name ).arg( value ) );

        // Dummy point for label at top of vertical statistics value line
        QLineSeries* labelSeries = new QLineSeries();
        chart->addSeries( labelSeries );
        labelSeries->append( value, maxValue );
        labelSeries->attachAxis( axisX );
        labelSeries->attachAxis( axisY );
        labelSeries->setPointLabelsVisible( true );
        labelSeries->setPointLabelsClipping( false );
        labelSeries->setPointLabelsFormat( QString( "%1 - @xPoint" ).arg( name ) );

        // Set font of label equal axis value font
        QFont labelFont = QFont();
        labelFont.setPixelSize( fontScalingToMatchQwt * axisValueFontSize() );
        labelSeries->setPointLabelsFont( labelFont );

        // Remove legend for dummy point
        QList<QLegendMarker*> labelMarker = chart->legend()->markers( labelSeries );
        if ( !labelMarker.empty() ) labelMarker.back()->setVisible( false );
    }

    // Set axis value font
    QFont axisYValueFont = axisY->labelsFont();
    axisYValueFont.setPixelSize( fontScalingToMatchQwt * axisValueFontSize() );
    axisY->setLabelsFont( axisYValueFont );
    QFont axisXValueFont = axisX->labelsFont();
    axisXValueFont.setPixelSize( fontScalingToMatchQwt * axisValueFontSize() );
    axisX->setLabelsFont( axisXValueFont );

    // Set axis title font
    QFont axisYTitleFont = axisY->titleFont();
    axisYTitleFont.setPixelSize( fontScalingToMatchQwt * axisTitleFontSize() );
    axisY->setTitleFont( axisYTitleFont );
    QFont axisXTitleFont = axisX->titleFont();
    axisXTitleFont.setPixelSize( fontScalingToMatchQwt * axisTitleFontSize() );
    axisX->setTitleFont( axisXTitleFont );

    // Set plot title font
    QFont titleFont = chart->titleFont();
    titleFont.setPixelSize( fontScalingToMatchQwt * titleFontSize() );
    chart->setTitleFont( titleFont );

    // Set legend font
    QLegend* legend = chart->legend();
    if ( legend )
    {
        QFont legendFont = legend->font();
        legendFont.setPixelSize( fontScalingToMatchQwt * legendFontSize() );
        legend->setFont( legendFont );
        legend->setVisible( legendsVisible() );
    }

    m_viewer->setChart( chart );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimStatisticsPlot::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsPlot::performAutoNameUpdate()
{
    QString name      = createAutoName();
    m_plotWindowTitle = name;
    setUiName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsPlot::createYAxisTitle() const
{
    return caf::AppEnum<RimStatisticsPlot::HistogramFrequencyType>::uiText( m_histogramFrequencyType() );
}
