/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Statoil ASA
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

#include "RimHistogramPlot.h"

#include "RiaLogging.h"
#include "RiaPlotDefines.h"
#include "Summary/RiaSummaryTools.h"

#include "RicHistogramPlotTools.h"

#include "Histogram/RimEnsembleParameterHistogramDataSource.h"
#include "Histogram/RimEnsembleSummaryVectorHistogramDataSource.h"
#include "RimHistogramCurve.h"
#include "RimHistogramCurveCollection.h"
#include "RimMultiPlot.h"
#include "RimPlotAxisLogRangeCalculator.h"
#include "Summary/Ensemble/RimSummaryEnsembleParameter.h"
#include "Summary/Ensemble/RimSummaryFileSetEnsemble.h"
#include "Summary/RimSummaryAddress.h"
#include "Tools/RimPlotAxisTools.h"

#include "RiuPlotAxis.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotWidget.h"

#include "cafAssert.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManager.h"

#include "qwt_text.h"

#include <QDateTime>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QString>

#include <algorithm>
#include <cmath>
#include <set>

namespace caf
{
template <>
void caf::AppEnum<RimHistogramPlot::FrequencyType>::setUp()
{
    addItem( RimHistogramPlot::FrequencyType::ABSOLUTE_FREQUENCY, "ABSOLUTE_FREQUENCY", "Absolute Frequency" );
    addItem( RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY, "RELATIVE_FREQUENCY", "Relative Frequency" );
    addItem( RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY_PERCENT, "RELATIVE_FREQUENCY_PERCENT", "Relative Frequency [%]" );
    setDefault( RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY_PERCENT );
}
template <>
void caf::AppEnum<RimHistogramPlot::GraphType>::setUp()

{
    addItem( RimHistogramPlot::GraphType::BAR_GRAPH, "BAR_GRAPH", "Bar Graph" );
    addItem( RimHistogramPlot::GraphType::LINE_GRAPH, "LINE_GRAPH", "Line Graph" );
    setDefault( RimHistogramPlot::GraphType::BAR_GRAPH );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimHistogramPlot, "HistogramPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot::RimHistogramPlot()
    : curvesChanged( this )
    , axisChanged( this )
    , titleChanged( this )
    , m_isValid( true )
    , axisChangedReloadRequired( this )
    , autoTitleChanged( this )
    , m_legendPosition( RiuPlotWidget::Legend::BOTTOM )
{
    CAF_PDM_InitScriptableObject( "Histogram Plot", ":/HistogramPlotLight16x16.png", "", "A Histogram Plot" );

    CAF_PDM_InitScriptableField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title" );
    CAF_PDM_InitScriptableField( &m_description, "PlotDescription", QString( "Histogram Plot" ), "Name" );
    CAF_PDM_InitScriptableField( &m_normalizeCurveYValues, "normalizeCurveYValues", false, "Normalize all curves" );

    CAF_PDM_InitFieldNoDefault( &m_histogramCurveCollection, "HistogramCurveCollection", "" );
    m_histogramCurveCollection = new RimHistogramCurveCollection();
    m_histogramCurveCollection->curvesChanged.connect( this, &RimHistogramPlot::onCurveCollectionChanged );

    CAF_PDM_InitFieldNoDefault( &m_axisPropertiesArray, "AxisProperties", "Axes", ":/Axes16x16.png" );
    m_axisPropertiesArray.uiCapability()->setUiTreeHidden( false );

    auto leftAxis = addNewAxisProperties( RiuPlotAxis::defaultLeft(), "Left" );
    leftAxis->setAlwaysRequired( true );

    auto bottomAxis = addNewAxisProperties( RiuPlotAxis::defaultBottom(), "Bottom" );
    bottomAxis->setAlwaysRequired( true );

    CAF_PDM_InitFieldNoDefault( &m_histogramFrequencyType, "HistogramFrequencyType", "Frequency" );
    CAF_PDM_InitFieldNoDefault( &m_graphType, "GraphType", "Graph Type" );

    CAF_PDM_InitFieldNoDefault( &m_fallbackPlotName, "AlternateName", "AlternateName" );
    m_fallbackPlotName.uiCapability()->setUiReadOnly( true );
    m_fallbackPlotName.uiCapability()->setUiHidden( true );
    m_fallbackPlotName.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot::~RimHistogramPlot()
{
    m_isValid = false;

    removeMdiWindowFromMdiArea();

    deletePlotCurvesAndPlotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::initAfterRead()
{
    for ( auto curve : histogramCurves() )
    {
        connectCurveSignals( curve );
    }

    for ( const auto& axisProperties : m_axisPropertiesArray )
    {
        auto plotAxisProperties = dynamic_cast<RimPlotAxisProperties*>( axisProperties.p() );
        if ( plotAxisProperties )
        {
            connectAxisSignals( plotAxisProperties );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateAxes()
{
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_TOP );

    updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isLogarithmicScaleEnabled( RiuPlotAxis plotAxis ) const
{
    auto axisProperties = axisPropertiesForPlotAxis( plotAxis );
    if ( !axisProperties ) return false;

    return axisProperties->isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isCurveHighlightSupported() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimHistogramPlot::viewWidget()
{
    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimHistogramPlot::plotWidget()
{
    if ( !m_histogramPlot ) return nullptr;

    return m_histogramPlot.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramPlot::asciiDataForPlotExport() const
{
    return asciiDataForHistogramPlotExport( RiaDefines::DateTimePeriod::YEAR, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramPlot::asciiDataForHistogramPlotExport( RiaDefines::DateTimePeriod resamplingPeriod, bool showTimeAsLongString ) const
{
    // TODO: add implementation
    QString text;
    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onAxisSelected( RiuPlotAxis axis, bool toggle )
{
    RiuPlotMainWindowTools::showPlotMainWindow();

    caf::PdmObject* itemToSelect = axisPropertiesForPlotAxis( axis );

    RiuPlotMainWindowTools::selectOrToggleObject( itemToSelect, toggle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramCurve*> RimHistogramPlot::histogramCurves() const
{
    return m_histogramCurveCollection->curves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updatePlotTitle()
{
    if ( m_description().isEmpty() )
    {
        auto multiPlot = firstAncestorOrThisOfType<RimMultiPlot>();

        size_t index = 0;
        if ( multiPlot ) index = multiPlot->plotIndex( this );

        QString title      = QString( "Sub Plot %1" ).arg( index + 1 );
        m_fallbackPlotName = title;
    }

    updateCurveNames();
    updateMdiWindowTitle();

    if ( plotWidget() )
    {
        QString plotTitle = description();
        plotWidget()->setPlotTitle( plotTitle );
        plotWidget()->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
        scheduleReplotIfVisible();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateAll()
{
    if ( plotWidget() )
    {
        updatePlotTitle();
        plotWidget()->updateLegend();
        updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateLegend()
{
    if ( plotWidget() )
    {
        if ( m_showPlotLegends && !isSubPlot() )
        {
            plotWidget()->insertLegend( m_legendPosition );
        }
        else
        {
            plotWidget()->clearLegend();
        }

        for ( auto c : histogramCurves() )
        {
            c->updateLegendEntryVisibilityNoPlotUpdate();
        }
    }

    reattachAllCurves();
    if ( plotWidget() )
    {
        plotWidget()->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setLegendPosition( RiuPlotWidget::Legend position )
{
    m_legendPosition = position;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setNormalizationEnabled( bool enable )
{
    m_normalizeCurveYValues = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isNormalizationEnabled()
{
    return m_normalizeCurveYValues();
}

//--------------------------------------------------------------------------------------------------
///
/// Update regular axis with numerical axis values - i.e. not time axis.
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateNumericalAxis( RiaDefines::PlotAxis plotAxis )
{
    if ( !plotWidget() ) return;

    for ( const RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        RiuPlotAxis riuPlotAxis = axisProperties->plotAxis();
        if ( riuPlotAxis.axis() == plotAxis )
        {
            auto* axisProps = dynamic_cast<const RimPlotAxisProperties*>( axisProperties );
            if ( !axisProps ) continue;

            bool hasVisibleCurveForAxis = !visibleHistogramCurvesForAxis( riuPlotAxis ).empty();
            bool shouldEnable           = axisProperties->isActive() && hasVisibleCurveForAxis;
            plotWidget()->enableAxis( riuPlotAxis, shouldEnable );

            plotWidget()->enableAxisNumberLabels( riuPlotAxis, axisProps->showNumbers() );

            RimPlotAxisPropertiesInterface::LegendTickmarkCount tickmarkCountEnum = axisProps->majorTickmarkCount();
            int maxTickmarkCount = RimPlotAxisPropertiesInterface::tickmarkCountFromEnum( tickmarkCountEnum );
            plotWidget()->setAutoTickIntervalCounts( riuPlotAxis, maxTickmarkCount, maxTickmarkCount );

            Qt::AlignmentFlag titleAlignment = Qt::AlignCenter;
            if ( axisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
            {
                titleAlignment = Qt::AlignRight;
            }

            bool titleBold = false;
            plotWidget()->setAxisFontsAndAlignment( riuPlotAxis,
                                                    axisProperties->titleFontSize(),
                                                    axisProperties->valuesFontSize(),
                                                    titleBold,
                                                    titleAlignment );
            plotWidget()->setAxisTitleEnabled( riuPlotAxis, true );

            RimPlotAxisTools::applyAxisScaleDraw( plotWidget(), riuPlotAxis, axisProps );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateZoomForAxis( RimPlotAxisPropertiesInterface* axisProperties )
{
    if ( auto axisProps = dynamic_cast<RimPlotAxisProperties*>( axisProperties ) )
    {
        updateZoomForNumericalAxis( axisProps );
        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateZoomForNumericalAxis( RimPlotAxisProperties* axisProperties )
{
    if ( !axisProperties ) return;

    const auto plotAxis = axisProperties->plotAxis();
    if ( axisProperties->isAutoZoom() )
    {
        if ( axisProperties->isLogarithmicScaleEnabled() )
        {
            plotWidget()->setAxisScaleType( plotAxis, RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC );
            std::vector<const RimPlotCurve*> plotCurves;

            for ( RimHistogramCurve* c : visibleHistogramCurvesForAxis( plotAxis ) )
            {
                plotCurves.push_back( c );
            }

            double                        min, max;
            RimPlotAxisLogRangeCalculator calc( plotAxis.axis(), plotCurves );
            calc.computeAxisRange( &min, &max );

            if ( axisProperties->isAxisInverted() )
            {
                std::swap( min, max );
            }

            plotWidget()->setAxisScale( axisProperties->plotAxis(), min, max );
        }
        else
        {
            plotWidget()->setAxisAutoScale( axisProperties->plotAxis(), true );
        }
    }
    else
    {
        double min = axisProperties->visibleRangeMin();
        double max = axisProperties->visibleRangeMax();
        if ( axisProperties->isAxisInverted() ) std::swap( min, max );
        plotWidget()->setAxisScale( axisProperties->plotAxis(), min, max );
    }

    plotWidget()->setAxisInverted( axisProperties->plotAxis(), axisProperties->isAxisInverted() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::scheduleReplotIfVisible()
{
    if ( showWindow() && plotWidget() ) plotWidget()->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramCurve*> RimHistogramPlot::visibleHistogramCurvesForAxis( RiuPlotAxis plotAxis ) const
{
    std::vector<RimHistogramCurve*> curves;

    if ( m_histogramCurveCollection && m_histogramCurveCollection->isCurvesVisible() )
    {
        for ( RimHistogramCurve* curve : m_histogramCurveCollection->curves() )
        {
            if ( curve->isChecked() && ( curve->axisY() == plotAxis || curve->axisX() == plotAxis ) )
            {
                curves.push_back( curve );
            }
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface* RimHistogramPlot::axisPropertiesForPlotAxis( RiuPlotAxis plotAxis ) const
{
    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        if ( axisProperties->plotAxis() == plotAxis ) return axisProperties;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updatePlotWidgetFromAxisRanges();

    axisChanged.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::addCurveNoUpdate( RimHistogramCurve* curve, bool autoAssignPlotAxis )
{
    if ( curve )
    {
        m_histogramCurveCollection->addCurve( curve );
        connectCurveToPlot( curve, false, autoAssignPlotAxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::connectCurveToPlot( RimHistogramCurve* curve, bool update, bool autoAssignPlotAxis )
{
    if ( autoAssignPlotAxis ) assignPlotAxis( curve );

    connectCurveSignals( curve );
    if ( plotWidget() )
    {
        plotWidget()->ensureAxisIsCreated( curve->axisY() );

        if ( update )
        {
            curve->setParentPlotAndReplot( plotWidget() );
            updateAxes();
        }
        else
        {
            curve->setParentPlotNoReplot( plotWidget() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimHistogramPlot::userDescriptionField()
{
    if ( m_description().isEmpty() )
    {
        return &m_fallbackPlotName;
    }
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::zoomAllForMultiPlot()
{
    if ( auto multiPlot = firstAncestorOrThisOfType<RimMultiPlot>() )
    {
        multiPlot->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_description )
    {
        m_useAutoPlotTitle = false;
    }

    if ( changedField == &m_useAutoPlotTitle || changedField == &m_description )
    {
        autoTitleChanged.send( m_useAutoPlotTitle() );
    }

    if ( changedField == &m_showPlotTitle || changedField == &m_description || changedField == &m_useAutoPlotTitle )
    {
        updatePlotTitle();
        updateConnectedEditors();

        if ( !m_useAutoPlotTitle )
        {
            // When auto name of plot is turned off, update the auto name for all curves
            for ( auto c : histogramCurves() )
            {
                c->updateCurveNameNoLegendUpdate();
            }
        }

        titleChanged.send();
    }

    if ( changedField == &m_showPlotLegends ) updateLegend();

    if ( changedField == &m_graphType )
    {
        for ( auto c : histogramCurves() )
        {
            c->setAppearanceFromGraphType( m_graphType() );
        }
    }

    if ( changedField == &m_normalizeCurveYValues || changedField == &m_histogramFrequencyType || changedField == &m_graphType )
    {
        loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateStackedCurveData()
{
    auto anyStackedCurvesPresent = true; // updateStackedCurveDataForRelevantAxes();

    if ( plotWidget() && anyStackedCurvesPresent )
    {
        reattachAllCurves();
        scheduleReplotIfVisible();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimHistogramPlot::snapshotWindowContent()
{
    QImage image;

    if ( plotWidget() )
    {
        QPixmap pix = plotWidget()->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( !m_isValid ) return;

    uiTreeOrdering.add( &m_axisPropertiesArray );

    for ( auto& curve : m_histogramCurveCollection->curves() )
    {
        uiTreeOrdering.add( curve );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onLoadDataAndUpdate()
{
    updatePlotTitle();

    auto plotWindow = firstAncestorOrThisOfType<RimMultiPlot>();
    if ( plotWindow == nullptr ) updateMdiWindowVisibility();

    if ( m_histogramCurveCollection )
    {
        m_histogramCurveCollection->loadDataAndUpdate( false );
    }

    if ( plotWidget() )
    {
        if ( m_showPlotLegends && !isSubPlot() )
        {
            plotWidget()->insertLegend( m_legendPosition );
        }
        else
        {
            plotWidget()->clearLegend();
        }
        plotWidget()->setLegendFontSize( legendFontSize() );
        plotWidget()->updateLegend();
    }
    updateAxes();

    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updatePlotWidgetFromAxisRanges()
{
    if ( !plotWidget() ) return;

    for ( const auto& axisProperty : m_axisPropertiesArray )
    {
        updateZoomForAxis( axisProperty );
    }

    plotWidget()->updateAxes();
    updateAxisRangesFromPlotWidget();
    plotWidget()->updateZoomDependentCurveProperties();

    scheduleReplotIfVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateAxisRangesFromPlotWidget()
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        if ( !axisProperties ) continue;

        auto [axisMin, axisMax] = plotWidget()->axisRange( axisProperties->plotAxis() );
        if ( axisProperties->isAxisInverted() ) std::swap( axisMin, axisMax );

        if ( auto propertyAxis = dynamic_cast<RimPlotAxisProperties*>( axisProperties ) )
        {
            propertyAxis->setAutoValueVisibleRangeMax( axisMax );
            propertyAxis->setAutoValueVisibleRangeMin( axisMin );
        }

        axisProperties->setVisibleRangeMax( axisMax );
        axisProperties->setVisibleRangeMin( axisMin );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::deletePlotCurvesAndPlotWidget()
{
    if ( isDeletable() )
    {
        detachAllPlotItems();
        deleteAllPlotCurves();

        if ( m_histogramPlot )
        {
            // The RiuPlotWidget is owned by Qt, and will be deleted when its parent is destructed.
            m_histogramPlot.clear();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::connectCurveSignals( RimStackablePlotCurve* curve )
{
    curve->dataChanged.connect( this, &RimHistogramPlot::curveDataChanged );
    curve->visibilityChanged.connect( this, &RimHistogramPlot::curveVisibilityChanged );
    curve->appearanceChanged.connect( this, &RimHistogramPlot::curveAppearanceChanged );
    curve->stackingChanged.connect( this, &RimHistogramPlot::curveStackingChanged );
    curve->stackingColorsChanged.connect( this, &RimHistogramPlot::curveStackingColorsChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::disconnectCurveSignals( RimStackablePlotCurve* curve )
{
    curve->dataChanged.disconnect( this );
    curve->visibilityChanged.disconnect( this );
    curve->appearanceChanged.disconnect( this );
    curve->stackingChanged.disconnect( this );
    curve->stackingColorsChanged.disconnect( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveDataChanged( const caf::SignalEmitter* emitter )
{
    loadDataAndUpdate();

    curvesChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveVisibilityChanged( const caf::SignalEmitter* emitter, bool visible )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveAppearanceChanged( const caf::SignalEmitter* emitter )
{
    scheduleReplotIfVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveStackingChanged( const caf::SignalEmitter* emitter, bool stacked )
{
    loadDataAndUpdate();

    // Change of stacking can result in very large y-axis changes, so zoom all
    zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveStackingColorsChanged( const caf::SignalEmitter* emitter, bool stackWithPhaseColors )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::connectAxisSignals( RimPlotAxisProperties* axis )
{
    axis->settingsChanged.connect( this, &RimHistogramPlot::axisSettingsChanged );
    axis->logarithmicChanged.connect( this, &RimHistogramPlot::axisLogarithmicChanged );
    axis->axisPositionChanged.connect( this, &RimHistogramPlot::axisPositionChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::axisSettingsChanged( const caf::SignalEmitter* emitter )
{
    axisChanged.send( this );
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    axisChanged.send( this );
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimHistogramPlot::addNewAxisProperties( RiaDefines::PlotAxis plotAxis, const QString& name )
{
    RiuPlotAxis newPlotAxis = plotWidget()->createNextPlotAxis( plotAxis );
    return addNewAxisProperties( newPlotAxis, name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimHistogramPlot::addNewAxisProperties( RiuPlotAxis plotAxis, const QString& name )
{
    auto* axisProperties = new RimPlotAxisProperties;
    axisProperties->configureForHistogramUse();
    axisProperties->enableAutoValueForAllFields( true );
    axisProperties->setNameAndAxis( name, name, plotAxis.axis(), plotAxis.index() );
    m_axisPropertiesArray.push_back( axisProperties );
    connectAxisSignals( axisProperties );

    return axisProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RimHistogramPlot::visibleCurvesForLegend()
{
    std::vector<RimPlotCurve*> curves;

    for ( auto c : histogramCurves() )
    {
        if ( !c->isChecked() ) continue;
        if ( !c->showInLegend() ) continue;
        curves.push_back( c );
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::axisPositionChanged( const caf::SignalEmitter* emitter,
                                            RimPlotAxisProperties*    axisProperties,
                                            RiuPlotAxis               oldPlotAxis,
                                            RiuPlotAxis               newPlotAxis )
{
    if ( !axisProperties ) return;

    if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
    {
        // Make sure the new axis on the correct side exists.
        RiuPlotAxis fixedUpPlotAxis = plotWidget()->createNextPlotAxis( newPlotAxis.axis() );
        // The index can change so need to update.
        axisProperties->setNameAndAxis( axisProperties->objectName(),
                                        axisProperties->axisTitleText(),
                                        fixedUpPlotAxis.axis(),
                                        fixedUpPlotAxis.index() );

        // // Move all attached curves
        for ( auto curve : histogramCurves() )
        {
            if ( curve->axisY() == oldPlotAxis ) curve->setLeftOrRightAxisY( fixedUpPlotAxis );
        }

        // Remove the now unused axis (but keep the default axis)
        if ( oldPlotAxis != RiuPlotAxis::defaultLeft() && oldPlotAxis != RiuPlotAxis::defaultRight() )
        {
            auto oldAxisProperties = axisPropertiesForPlotAxis( oldPlotAxis );
            if ( oldAxisProperties ) m_axisPropertiesArray.removeChild( oldAxisProperties );
            plotWidget()->moveAxis( oldPlotAxis, newPlotAxis );
        }

        updateAxes();
    }

    // This is probably to much, but difficult to find the required updates
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::enableAutoPlotTitle( bool enable )
{
    m_useAutoPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::autoPlotTitle() const
{
    return m_useAutoPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onPlotZoomed()
{
    // Disable auto scale in plot engine
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );

    // Disable auto value for min/max fields
    for ( auto p : plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
    {
        p->enableAutoValueMinMax( false );
    }

    updateAxisRangesFromPlotWidget();

    axisChanged.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* mainOptions = uiOrdering.addNewGroup( "General Plot Options" );

    if ( isMdiWindow() )
    {
        mainOptions->add( &m_showPlotTitle );
        if ( m_showPlotTitle )
        {
            mainOptions->add( &m_useAutoPlotTitle );
            mainOptions->add( &m_description );
        }
    }
    else
    {
        mainOptions->add( &m_useAutoPlotTitle );
        mainOptions->add( &m_description );
        mainOptions->add( &m_colSpan );
    }

    mainOptions->add( &m_normalizeCurveYValues );

    uiOrdering.add( &m_histogramFrequencyType );
    uiOrdering.add( &m_graphType );

    if ( isMdiWindow() )
    {
        RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimHistogramPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    CAF_ASSERT( mainWindowParent );
    if ( !plotWidget() )
    {
        m_histogramPlot = new RiuQwtPlotWidget( this, mainWindowParent );

        QObject::connect( plotWidget(), SIGNAL( curveOrderNeedsUpdate() ), this, SLOT( onUpdateCurveOrder() ) );

        for ( const auto& axisProperties : m_axisPropertiesArray )
        {
            plotWidget()->ensureAxisIsCreated( axisProperties->plotAxis() );
        }

        if ( m_histogramCurveCollection )
        {
            m_histogramCurveCollection->setParentPlotNoReplot( plotWidget() );
        }

        connect( plotWidget(), SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        updatePlotTitle();
    }

    plotWidget()->setParent( mainWindowParent );

    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::deleteViewWidget()
{
    deletePlotCurvesAndPlotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::doUpdateLayout()
{
    updateFonts();

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::detachAllPlotItems()
{
    if ( m_histogramCurveCollection )
    {
        m_histogramCurveCollection->detachPlotCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::deleteAllPlotCurves()
{
    for ( auto* c : histogramCurves() )
    {
        c->deletePlotCurve();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateCurveNames()
{
    if ( m_histogramCurveCollection->isCurvesVisible() )
    {
        for ( auto c : histogramCurves() )
        {
            if ( c->isChecked() )
            {
                c->updateCurveNameNoLegendUpdate();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::detachAllCurves()
{
    detachAllPlotItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::reattachAllCurves()
{
    if ( m_histogramCurveCollection )
    {
        m_histogramCurveCollection->reattachPlotCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onCurveCollectionChanged( const SignalEmitter* emitter )
{
    curvesChanged.send();

    updateStackedCurveData();
    scheduleReplotIfVisible();

    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex )
{
    auto wrapper = dynamic_cast<RiuQwtPlotItem*>( plotItem.get() );
    if ( !wrapper ) return;

    auto qwtPlotItem = wrapper->qwtPlotItem();
    if ( !qwtPlotItem ) return;

    auto riuPlotCurve = dynamic_cast<RiuQwtPlotCurve*>( qwtPlotItem );
    if ( !riuPlotCurve ) return;

    auto rimPlotCurve = riuPlotCurve->ownerRimCurve();

    RiuPlotMainWindowTools::selectOrToggleObject( rimPlotCurve, toggle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setAutoScaleXEnabled( bool enabled )
{
    for ( const auto& ap : m_axisPropertiesArray )
    {
        if ( ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_TOP || ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        {
            ap->setAutoZoom( enabled );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setAutoScaleYEnabled( bool enabled )
{
    for ( const auto& ap : m_axisPropertiesArray )
    {
        if ( ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        {
            ap->setAutoZoom( enabled );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimHistogramPlot::curveCount() const
{
    return m_histogramCurveCollection->curves().size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isDeletable() const
{
    auto plotWindow = firstAncestorOrThisOfType<RimMultiPlot>();
    return plotWindow == nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisPropertiesInterface*> RimHistogramPlot::allPlotAxes() const
{
    return m_axisPropertiesArray.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisProperties*> RimHistogramPlot::plotAxes( RimPlotAxisProperties::Orientation orientation ) const
{
    std::vector<RimPlotAxisProperties*> axisProps;
    for ( const auto& ap : m_axisPropertiesArray )
    {
        auto plotAxisProp = dynamic_cast<RimPlotAxisProperties*>( ap.p() );
        if ( !plotAxisProp ) continue;

        if ( ( orientation == RimPlotAxisProperties::Orientation::ANY ) ||
             ( orientation == RimPlotAxisProperties::Orientation::VERTICAL && plotAxisProp->plotAxis().isVertical() ) )
        {
            axisProps.push_back( plotAxisProp );
        }
        else if ( ( orientation == RimPlotAxisProperties::Orientation::ANY ) ||
                  ( orientation == RimPlotAxisProperties::Orientation::HORIZONTAL && plotAxisProp->plotAxis().isHorizontal() ) )
        {
            axisProps.push_back( plotAxisProp );
        }
    }

    return axisProps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::assignPlotAxis( RimHistogramCurve* destinationCurve )
{
    destinationCurve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
    destinationCurve->setTopOrBottomAxisX( RiuPlotAxis::defaultBottom() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    if ( childArray == &m_axisPropertiesArray )
    {
        for ( caf::PdmObjectHandle* reffingObj : referringObjects )
        {
            auto* curve = dynamic_cast<RimHistogramCurve*>( reffingObj );
            if ( curve )
            {
                curve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
            }
        }

        if ( plotWidget() )
        {
            std::set<RiuPlotAxis> usedPlotAxis;
            for ( const auto& axisProperties : m_axisPropertiesArray )
            {
                usedPlotAxis.insert( axisProperties->plotAxis() );
            }

            plotWidget()->pruneAxes( usedPlotAxis );
            updateAxes();
            scheduleReplotIfVisible();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onUpdateCurveOrder()
{
    m_histogramCurveCollection->updateCurveOrder();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot::FrequencyType RimHistogramPlot::frequencyType() const
{
    return m_histogramFrequencyType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot::GraphType RimHistogramPlot::graphType() const
{
    return m_graphType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects )
{
    for ( auto obj : objects )
    {
        if ( auto fileSet = dynamic_cast<RimSummaryFileSetEnsemble*>( obj ) )
        {
            RicHistogramPlotTools::appendEnsembleToHistogram( this, fileSet );
        }
        else if ( auto parameter = dynamic_cast<RimSummaryEnsembleParameter*>( obj ) )
        {
            auto ensemble = RiaSummaryTools::ensembleById( parameter->ensembleId() );
            if ( ensemble == nullptr ) continue;

            auto newDataSource = new RimEnsembleParameterHistogramDataSource();
            newDataSource->setEnsemble( ensemble );
            newDataSource->setEnsembleParameter( parameter->name() );

            RicHistogramPlotTools::appendEnsembleParameterHistogramCurve( this, newDataSource );
        }
        else if ( auto address = dynamic_cast<RimSummaryAddress*>( obj ) )
        {
            if ( !address->isEnsemble() ) continue;

            auto ensemble = RiaSummaryTools::ensembleById( address->ensembleId() );
            if ( ensemble == nullptr ) continue;

            auto newDataSource = new RimEnsembleSummaryVectorHistogramDataSource();
            newDataSource->setDefaults(); // get default timestep
            auto fileAddress = address->address();
            newDataSource->setSummaryAddress( fileAddress );
            newDataSource->setEnsemble( ensemble );

            RicHistogramPlotTools::createHistogramCurve( this, newDataSource );
        }
    }

    scheduleReplotIfVisible();
}