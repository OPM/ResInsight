/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryPlot.h"

#include "RiaColorTables.h"
#include "RiaDefines.h"
#include "RiaFieldHandleTools.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"
#include "RiaRegressionTestRunner.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RicfCommandObject.h"

#include "SummaryPlotCommands/RicSummaryPlotEditorUi.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "PlotTemplates/RimPlotTemplateFolderItem.h"
#include "RimAsciiDataCurve.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimMultiPlot.h"
#include "RimPlotAxisLogRangeCalculator.h"
#include "RimPlotAxisProperties.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurvesData.h"
#include "RimSummaryPlotAxisFormatter.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryPlotControls.h"
#include "RimSummaryPlotFilterTextCurveSetEditor.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuPlotAxis.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuTreeViewEventFilter.h"

#ifdef USE_QTCHARTS
#include "RiuSummaryQtChartsPlot.h"
#endif

#include "cvfColor3.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManager.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_textlabel.h"
#include "qwt_text.h"

#include <QDateTime>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QString>

#include <algorithm>
#include <limits>
#include <set>

CAF_PDM_SOURCE_INIT( RimSummaryPlot, "SummaryPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot( bool isCrossPlot )
    : RimPlot()
    , m_isCrossPlot( isCrossPlot )
{
    CAF_PDM_InitScriptableObject( "Summary Plot", ":/SummaryPlotLight16x16.png", "", "A Summary Plot" );

    CAF_PDM_InitScriptableField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title" );
    CAF_PDM_InitScriptableField( &m_description, "PlotDescription", QString( "Summary Plot" ), "Name" );
    CAF_PDM_InitScriptableField( &m_normalizeCurveYValues, "normalizeCurveYValues", false, "Normalize all curves" );
#ifdef USE_QTCHARTS
    bool useQtChart = RiaPreferences::current()->useQtChartsAsDefaultPlotType();
    CAF_PDM_InitScriptableField( &m_useQtChartsPlot, "useQtChartsPlot", useQtChart, "Use Qt Charts" );
#endif
    CAF_PDM_InitFieldNoDefault( &m_summaryCurveCollection, "SummaryCurveCollection", "" );
    m_summaryCurveCollection.uiCapability()->setUiTreeHidden( true );
    m_summaryCurveCollection = new RimSummaryCurveCollection;
    m_summaryCurveCollection->curvesChanged.connect( this, &RimSummaryPlot::onCurveCollectionChanged );

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSetCollection, "EnsembleCurveSetCollection", "" );
    m_ensembleCurveSetCollection.uiCapability()->setUiTreeHidden( true );
    m_ensembleCurveSetCollection = new RimEnsembleCurveSetCollection();

    CAF_PDM_InitFieldNoDefault( &m_gridTimeHistoryCurves, "GridTimeHistoryCurves", "" );
    m_gridTimeHistoryCurves.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_asciiDataCurves, "AsciiDataCurves", "" );
    m_asciiDataCurves.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_axisProperties, "AxisProperties", "Axes", ":/Axes16x16.png" );

    auto leftAxis = addNewAxisProperties( RiuPlotAxis::defaultLeft(), "Left" );
    leftAxis->setAlwaysRequired( true );

    auto rightAxis = addNewAxisProperties( RiuPlotAxis::defaultRight(), "Right" );
    rightAxis->setAlwaysRequired( true );

    if ( m_isCrossPlot )
    {
        addNewAxisProperties( RiuPlotAxis::defaultBottom(), "Bottom" );
    }
    else
    {
        auto* timeAxisProperties = new RimSummaryTimeAxisProperties;
        m_axisProperties.push_back( timeAxisProperties );
    }

    CAF_PDM_InitFieldNoDefault( &m_textCurveSetEditor, "SummaryPlotFilterTextCurveSetEditor", "Text Filter Curve Creator" );
    m_textCurveSetEditor.uiCapability()->setUiTreeHidden( true );
    m_textCurveSetEditor = new RimSummaryPlotFilterTextCurveSetEditor;

    m_nameHelperAllCurves = std::make_unique<RimSummaryPlotNameHelper>();

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );
    m_sourceStepping = new RimSummaryPlotSourceStepping;
    m_sourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS );
    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeHidden( true );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();

    setPlotInfoLabel( "Filters Active" );

    // Obsolete axis fields
    CAF_PDM_InitFieldNoDefault( &m_leftYAxisProperties_OBSOLETE, "LeftYAxisProperties", "Left Y Axis" );
    m_leftYAxisProperties_OBSOLETE.uiCapability()->setUiTreeHidden( true );
    m_leftYAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_leftYAxisProperties_OBSOLETE = new RimPlotAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_rightYAxisProperties_OBSOLETE, "RightYAxisProperties", "Right Y Axis" );
    m_rightYAxisProperties_OBSOLETE.uiCapability()->setUiTreeHidden( true );
    m_rightYAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_rightYAxisProperties_OBSOLETE = new RimPlotAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_bottomAxisProperties_OBSOLETE, "BottomAxisProperties", "Bottom X Axis" );
    m_bottomAxisProperties_OBSOLETE.uiCapability()->setUiTreeHidden( true );
    m_bottomAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_bottomAxisProperties_OBSOLETE = new RimPlotAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_timeAxisProperties_OBSOLETE, "TimeAxisProperties", "Time Axis" );
    m_timeAxisProperties_OBSOLETE.uiCapability()->setUiTreeHidden( true );
    m_timeAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_timeAxisProperties_OBSOLETE = new RimSummaryTimeAxisProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::~RimSummaryPlot()
{
    removeMdiWindowFromMdiArea();

    deletePlotCurvesAndPlotWidget();

    delete m_summaryCurveCollection;
    delete m_ensembleCurveSetCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxes()
{
    updateAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    updateAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

    if ( timeAxisProperties() && plotWidget() )
    {
        m_summaryPlot->updateAnnotationObjects( timeAxisProperties() );
    }

    RimPlotAxisPropertiesInterface* leftYAxisProperties = axisPropertiesForPlotAxis( RiuPlotAxis::defaultLeft() );
    if ( leftYAxisProperties && plotWidget() )
    {
        m_summaryPlot->updateAnnotationObjects( leftYAxisProperties );
    }

    if ( m_isCrossPlot )
    {
        updateAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    }
    else
    {
        updateTimeAxis( timeAxisProperties() );
    }

    if ( plotWidget() )
    {
        plotWidget()->updateAxes();
        plotWidget()->scheduleReplot();
    }

    updateZoomInParentPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isLogarithmicScaleEnabled( RiuPlotAxis plotAxis ) const
{
    auto axisProperties = axisPropertiesForPlotAxis( plotAxis );
    if ( !axisProperties ) return false;

    return axisProperties->isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties* RimSummaryPlot::timeAxisProperties()
{
    // Find the first time axis (which is correct since there is only one).
    for ( const auto& ap : m_axisProperties )
    {
        auto* timeAxis = dynamic_cast<RimSummaryTimeAxisProperties*>( ap.p() );
        if ( timeAxis ) return timeAxis;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
time_t RimSummaryPlot::firstTimeStepOfFirstCurve()
{
    RimSummaryCurve* firstCurve = nullptr;

    if ( m_summaryCurveCollection )
    {
        std::vector<RimSummaryCurve*> curves = m_summaryCurveCollection->curves();
        size_t                        i      = 0;
        while ( firstCurve == nullptr && i < curves.size() )
        {
            firstCurve = curves[i];
            ++i;
        }
    }

    if ( firstCurve && !firstCurve->timeStepsY().empty() )
    {
        return firstCurve->timeStepsY()[0];
    }
    return time_t( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryPlot::viewWidget()
{
    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimSummaryPlot::plotWidget()
{
    if ( !m_summaryPlot ) return nullptr;

    return m_summaryPlot->plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::asciiDataForPlotExport() const
{
    return asciiDataForSummaryPlotExport( RiaDefines::DateTimePeriod::YEAR, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::asciiDataForSummaryPlotExport( RiaDefines::DateTimePeriod resamplingPeriod,
                                                       bool                       showTimeAsLongString ) const
{
    std::vector<RimSummaryCurve*> curves;
    this->descendantsIncludingThisOfType( curves );

    auto gridCurves  = m_gridTimeHistoryCurves.childObjects();
    auto asciiCurves = m_asciiDataCurves.childObjects();

    QString text =
        RimSummaryCurvesData::createTextForExport( curves, asciiCurves, gridCurves, resamplingPeriod, showTimeAsLongString );

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimSummaryPlot::findPdmObjectFromPlotCurve( const RiuPlotCurve* plotCurve ) const
{
    for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    {
        if ( curve->isSameCurve( plotCurve ) )
        {
            return curve;
        }
    }

    for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    {
        if ( curve->isSameCurve( plotCurve ) )
        {
            return curve;
        }
    }

    if ( m_summaryCurveCollection )
    {
        RimSummaryCurve* foundCurve = m_summaryCurveCollection->findRimCurveFromPlotCurve( plotCurve );

        if ( foundCurve )
        {
            m_summaryCurveCollection->setCurrentSummaryCurve( foundCurve );

            return foundCurve;
        }
    }

    if ( m_ensembleCurveSetCollection )
    {
        RimSummaryCurve* foundCurve = m_ensembleCurveSetCollection->findRimCurveFromPlotCurve( plotCurve );

        if ( foundCurve )
        {
            return foundCurve;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onAxisSelected( int axis, bool toggle )
{
    RiuPlotMainWindowTools::showPlotMainWindow();

    caf::PdmObject* itemToSelect = nullptr;
    if ( axis == QwtPlot::yLeft )
    {
        itemToSelect = m_leftYAxisProperties_OBSOLETE;
    }
    else if ( axis == QwtPlot::yRight )
    {
        itemToSelect = m_rightYAxisProperties_OBSOLETE;
    }
    else if ( axis == QwtPlot::xBottom )
    {
        if ( m_isCrossPlot )
        {
            itemToSelect = m_bottomAxisProperties_OBSOLETE;
        }
        else
        {
            itemToSelect = m_timeAxisProperties_OBSOLETE;
        }
    }

    if ( toggle )
    {
        RiuPlotMainWindowTools::toggleItemInSelection( itemToSelect );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( itemToSelect );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::moveCurvesToPlot( RimSummaryPlot* plot, const std::vector<RimSummaryCurve*> curves, int insertAtPosition )
{
    CAF_ASSERT( plot );

    std::set<RimSummaryPlot*> srcPlots;

    for ( auto curve : curves )
    {
        RimSummaryPlot* srcPlot = nullptr;

        curve->firstAncestorOrThisOfTypeAsserted( srcPlot );

        srcPlot->removeCurve( curve );
        srcPlots.insert( srcPlot );
    }

    for ( auto srcPlot : srcPlots )
    {
        srcPlot->updateConnectedEditors();
        srcPlot->loadDataAndUpdate();
    }
    for ( size_t cIdx = 0; cIdx < curves.size(); ++cIdx )
    {
        if ( insertAtPosition >= 0 )
        {
            size_t position = (size_t)insertAtPosition + cIdx;
            plot->insertCurve( curves[cIdx], position );
        }
        else
        {
            plot->addCurveNoUpdate( curves[cIdx] );
        }
    }

    plot->updateConnectedEditors();
    plot->updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::curvesForStepping( RimSummaryDataSourceStepping::Axis axis ) const
{
    auto curveForStepping = summaryCurveCollection()->curveForSourceStepping();
    if ( curveForStepping )
    {
        return { curveForStepping };
    }

    return summaryCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*> RimSummaryPlot::curveSets() const
{
    return ensembleCurveSetCollection()->curveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::allCurves( RimSummaryDataSourceStepping::Axis axis ) const
{
    return summaryCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryDataSourceStepping::Axis> RimSummaryPlot::availableAxes() const
{
    if ( m_isCrossPlot )
        return { RimSummaryDataSourceStepping::Axis::X_AXIS, RimSummaryDataSourceStepping::Axis::Y_AXIS };

    return { RimSummaryDataSourceStepping::Axis::X_AXIS };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::summaryAndEnsembleCurves() const
{
    std::vector<RimSummaryCurve*> curves = summaryCurves();

    for ( const auto& curveSet : ensembleCurveSetCollection()->curveSets() )
    {
        for ( const auto& curve : curveSet->curves() )
        {
            curves.push_back( curve );
        }
    }
    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaSummaryCurveDefinition> RimSummaryPlot::summaryAndEnsembleCurveDefinitions() const
{
    std::set<RiaSummaryCurveDefinition> allCurveDefs;

    for ( const auto& curve : this->summaryAndEnsembleCurves() )
    {
        allCurveDefs.insert(
            RiaSummaryCurveDefinition( curve->summaryCaseY(), curve->summaryAddressY(), curve->isEnsembleCurve() ) );
    }
    return allCurveDefs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::summaryCurves() const
{
    return m_summaryCurveCollection->curves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteAllSummaryCurves()
{
    m_summaryCurveCollection->deleteAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection* RimSummaryPlot::summaryCurveCollection() const
{
    return m_summaryCurveCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::visibleStackedSummaryCurvesForAxis( RiuPlotAxis plotAxis )
{
    auto visibleCurves = visibleSummaryCurvesForAxis( plotAxis );

    std::vector<RimSummaryCurve*> visibleStackedCurves;

    std::copy_if( visibleCurves.begin(),
                  visibleCurves.end(),
                  std::back_inserter( visibleStackedCurves ),
                  []( RimSummaryCurve* curve ) { return curve->isStacked(); } );

    return visibleStackedCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updatePlotTitle()
{
    m_nameHelperAllCurves->clear();
    updateNameHelperWithCurveData( m_nameHelperAllCurves.get() );
    if ( m_useAutoPlotTitle )
    {
        m_description = m_nameHelperAllCurves->plotTitle();
    }

    updateCurveNames();
    updateMdiWindowTitle();

    if ( plotWidget() )
    {
        QString plotTitle = description();
        plotWidget()->setPlotTitle( plotTitle );
        plotWidget()->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
        plotWidget()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimSummaryNameHelper* RimSummaryPlot::activePlotTitleHelperAllCurves() const
{
    if ( m_useAutoPlotTitle() )
    {
        return m_nameHelperAllCurves.get();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimSummaryNameHelper* RimSummaryPlot::plotTitleHelper() const
{
    return m_nameHelperAllCurves.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::generatedPlotTitleFromAllCurves() const
{
    RimSummaryPlotNameHelper nameHelper;
    updateNameHelperWithCurveData( &nameHelper );
    return nameHelper.plotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::copyAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot )
{
    for ( auto ap : sourceSummaryPlot.plotAxes() )
    {
        QString data = ap->writeObjectToXmlString();

        axisPropertiesForPlotAxis( ap->plotAxisType() )
            ->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAll()
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
void RimSummaryPlot::updateLegend()
{
    if ( plotWidget() )
    {
        plotWidget()->setInternalLegendVisible( m_showPlotLegends && !isSubPlot() );

        for ( auto c : summaryCurves() )
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
void RimSummaryPlot::setPlotInfoLabel( const QString& label )
{
    auto qwtText = QwtText( label );
    qwtText.setRenderFlags( Qt::AlignBottom | Qt::AlignRight );

    QFont font;
    font.setBold( true );
    qwtText.setFont( font );

    m_plotInfoLabel = std::make_unique<QwtPlotTextLabel>();
    m_plotInfoLabel->setText( qwtText );
    m_plotInfoLabel->setMargin( 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::showPlotInfoLabel( bool show )
{
    auto* qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidget() );
    if ( !qwtPlotWidget ) return;

    if ( show )
        m_plotInfoLabel->attach( qwtPlotWidget->qwtPlot() );
    else
        m_plotInfoLabel->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updatePlotInfoLabel()
{
    bool anyCurveSetFiltered = false;
    for ( auto group : m_ensembleCurveSetCollection->curveSets() )
    {
        if ( group->isFiltered() )
        {
            anyCurveSetFiltered = true;
            break;
        }
    }
    showPlotInfoLabel( anyCurveSetFiltered );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::containsResamplableCurves() const
{
    std::vector<RimSummaryCurve*> summaryCurves = summaryAndEnsembleCurves();
    size_t                        resamplableSummaryCurveCount =
        std::count_if( summaryCurves.begin(), summaryCurves.end(), []( RimSummaryCurve* curve ) {
            return curve->summaryCaseY() ? !curve->summaryCaseY()->isObservedData() : false;
        } );

    return !m_gridTimeHistoryCurves.empty() || resamplableSummaryCurveCount > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryPlot::singleColorCurveCount() const
{
    auto   allCurveSets = ensembleCurveSetCollection()->curveSets();
    size_t colorIndex   = std::count_if( allCurveSets.begin(), allCurveSets.end(), []( RimEnsembleCurveSet* curveSet ) {
        return curveSet->colorMode() == RimEnsembleCurveSet::ColorMode::SINGLE_COLOR;
    } );

    colorIndex += curveCount();

    return colorIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::applyDefaultCurveAppearances()
{
    std::set<RiaSummaryCurveDefinition> allCurveDefs = this->summaryAndEnsembleCurveDefinitions();

    RimSummaryCurveAppearanceCalculator curveLookCalc( allCurveDefs );

    // Summary curves
    for ( auto& curve : this->summaryCurves() )
    {
        curve->resetAppearance();
        curveLookCalc.setupCurveLook( curve );
    }

    // Ensemble curve sets
    int colorIndex = 0;
    for ( auto& curveSet : this->ensembleCurveSetCollection()->curveSets() )
    {
        if ( curveSet->colorMode() != RimEnsembleCurveSet::ColorMode::SINGLE_COLOR ) continue;
        curveSet->setColor( RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( colorIndex++ ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setNormalizationEnabled( bool enable )
{
    m_normalizeCurveYValues = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isNormalizationEnabled()
{
    return m_normalizeCurveYValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxis( RiaDefines::PlotAxis plotAxis )
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* yAxisProperties : m_axisProperties )
    {
        RiuPlotAxis riuPlotAxis = yAxisProperties->plotAxisType();
        if ( riuPlotAxis.axis() == plotAxis )
        {
            auto* axisProperties = dynamic_cast<RimPlotAxisProperties*>( yAxisProperties );
            if ( yAxisProperties->isActive() && hasVisibleCurvesForAxis( riuPlotAxis ) && axisProperties )
            {
                plotWidget()->enableAxis( riuPlotAxis, true );

                std::set<QString> timeHistoryQuantities;

                for ( auto c : visibleTimeHistoryCurvesForAxis( riuPlotAxis ) )
                {
                    timeHistoryQuantities.insert( c->quantityName() );
                }

                RimSummaryPlotAxisFormatter calc( axisProperties,
                                                  visibleSummaryCurvesForAxis( riuPlotAxis ),
                                                  {},
                                                  visibleAsciiDataCurvesForAxis( riuPlotAxis ),
                                                  timeHistoryQuantities );
                calc.applyAxisPropertiesToPlot( plotWidget() );
            }
            else
            {
                plotWidget()->enableAxis( riuPlotAxis, false );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomForAxis( RiuPlotAxis plotAxis )
{
    RimPlotAxisPropertiesInterface* yAxisProps = axisPropertiesForPlotAxis( plotAxis );
    if ( !yAxisProps ) return;

    if ( yAxisProps->isAutoZoom() )
    {
        if ( yAxisProps->isLogarithmicScaleEnabled() )
        {
            plotWidget()->setAxisScaleType( yAxisProps->plotAxisType(), RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC );

            std::vector<const RimPlotCurve*> plotCurves;

            for ( RimSummaryCurve* c : visibleSummaryCurvesForAxis( plotAxis ) )
            {
                plotCurves.push_back( c );
            }

            for ( RimGridTimeHistoryCurve* c : visibleTimeHistoryCurvesForAxis( plotAxis ) )
            {
                plotCurves.push_back( c );
            }

            for ( RimAsciiDataCurve* c : visibleAsciiDataCurvesForAxis( plotAxis ) )
            {
                plotCurves.push_back( c );
            }

            double                        min, max;
            RimPlotAxisLogRangeCalculator calc( plotAxis.axis(), plotCurves );
            calc.computeAxisRange( &min, &max );

            if ( yAxisProps->isAxisInverted() )
            {
                std::swap( min, max );
            }

            plotWidget()->setAxisScale( yAxisProps->plotAxisType(), min, max );
        }
        else if ( ( plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
                    plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT ) &&
                  isOnlyWaterCutCurvesVisible( plotAxis ) )
        {
            plotWidget()->setAxisScale( yAxisProps->plotAxisType(), 0.0, 1.0 );
        }
        else
        {
            plotWidget()->setAxisAutoScale( yAxisProps->plotAxisType(), true );
        }
    }
    else
    {
        plotWidget()->setAxisScale( yAxisProps->plotAxisType(),
                                    yAxisProps->visibleRangeMin(),
                                    yAxisProps->visibleRangeMax() );
    }

    plotWidget()->setAxisInverted( yAxisProps->plotAxisType(), yAxisProps->isAxisInverted() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isOnlyWaterCutCurvesVisible( RiuPlotAxis plotAxis )
{
    size_t waterCutCurveCount = 0;
    auto   curves             = visibleSummaryCurvesForAxis( plotAxis );
    for ( auto c : curves )
    {
        auto quantityName = c->summaryAddressY().quantityName();

        if ( RiaStdStringTools::endsWith( quantityName, "WCT" ) ) waterCutCurveCount++;
    }

    return ( waterCutCurveCount == curves.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::visibleSummaryCurvesForAxis( RiuPlotAxis plotAxis ) const
{
    std::vector<RimSummaryCurve*> curves;

    if ( plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
    {
        if ( m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible() )
        {
            for ( RimSummaryCurve* curve : m_summaryCurveCollection->curves() )
            {
                if ( curve->isCurveVisible() )
                {
                    curves.push_back( curve );
                }
            }
        }
    }
    else
    {
        if ( m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible() )
        {
            for ( RimSummaryCurve* curve : m_summaryCurveCollection->curves() )
            {
                if ( curve->isCurveVisible() && curve->axisY() == plotAxis )
                {
                    curves.push_back( curve );
                }
            }
        }

        if ( m_ensembleCurveSetCollection && m_ensembleCurveSetCollection->isCurveSetsVisible() )
        {
            for ( RimEnsembleCurveSet* curveSet : m_ensembleCurveSetCollection->curveSets() )
            {
                for ( RimSummaryCurve* curve : curveSet->curves() )
                {
                    if ( curve->isCurveVisible() && curve->axisY() == plotAxis )
                    {
                        curves.push_back( curve );
                    }
                }
            }
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::hasVisibleCurvesForAxis( RiuPlotAxis plotAxis ) const
{
    if ( !visibleSummaryCurvesForAxis( plotAxis ).empty() )
    {
        return true;
    }

    if ( !visibleTimeHistoryCurvesForAxis( plotAxis ).empty() )
    {
        return true;
    }

    if ( !visibleAsciiDataCurvesForAxis( plotAxis ).empty() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface* RimSummaryPlot::axisPropertiesForPlotAxis( RiuPlotAxis plotAxis ) const
{
    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisProperties )
    {
        if ( axisProperties->plotAxisType() == plotAxis ) return axisProperties;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridTimeHistoryCurve*> RimSummaryPlot::visibleTimeHistoryCurvesForAxis( RiuPlotAxis plotAxis ) const
{
    std::vector<RimGridTimeHistoryCurve*> curves;

    for ( const auto& c : m_gridTimeHistoryCurves )
    {
        if ( c->isCurveVisible() )
        {
            if ( c->yAxis() == plotAxis || plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
            {
                curves.push_back( c );
            }
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimAsciiDataCurve*> RimSummaryPlot::visibleAsciiDataCurvesForAxis( RiuPlotAxis plotAxis ) const
{
    std::vector<RimAsciiDataCurve*> curves;

    for ( const auto& c : m_asciiDataCurves )
    {
        if ( c->isCurveVisible() )
        {
            if ( c->yAxis() == plotAxis || plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
            {
                curves.push_back( c );
            }
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateTimeAxis( RimSummaryTimeAxisProperties* timeAxisProperties )
{
    if ( !plotWidget() ) return;

    if ( !timeAxisProperties->isActive() )
    {
        plotWidget()->enableAxis( RiuPlotAxis::defaultBottom(), false );

        return;
    }

    if ( timeAxisProperties->timeMode() == RimSummaryTimeAxisProperties::DATE )
    {
        RiaDefines::DateFormatComponents dateComponents = timeAxisProperties->dateComponents();
        RiaDefines::TimeFormatComponents timeComponents = timeAxisProperties->timeComponents();

        const QString& dateFormat = timeAxisProperties->dateFormat();
        const QString& timeFormat = timeAxisProperties->timeFormat();

        m_summaryPlot->useDateBasedTimeAxis( dateFormat, timeFormat, dateComponents, timeComponents );
    }
    else
    {
        m_summaryPlot->useTimeBasedTimeAxis();
    }

    plotWidget()->enableAxis( RiuPlotAxis::defaultBottom(), true );

    {
        Qt::AlignmentFlag alignment = Qt::AlignCenter;
        if ( timeAxisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            alignment = Qt::AlignRight;
        }

        plotWidget()->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(),
                                                timeAxisProperties->titleFontSize(),
                                                timeAxisProperties->valuesFontSize(),
                                                true,
                                                alignment );
        plotWidget()->setAxisTitleText( RiuPlotAxis::defaultBottom(), timeAxisProperties->title() );
        plotWidget()->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), timeAxisProperties->showTitle );

        {
            RimSummaryTimeAxisProperties::LegendTickmarkCount tickmarkCountEnum = timeAxisProperties->majorTickmarkCount();

            int maxTickmarkCount = 8;

            switch ( tickmarkCountEnum )
            {
                case RimSummaryTimeAxisProperties::LegendTickmarkCount::TICKMARK_VERY_FEW:
                    maxTickmarkCount = 2;
                    break;
                case RimSummaryTimeAxisProperties::LegendTickmarkCount::TICKMARK_FEW:
                    maxTickmarkCount = 4;
                    break;
                case RimSummaryTimeAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT:
                    maxTickmarkCount = 8; // Taken from QwtPlot::initAxesData()
                    break;
                case RimSummaryTimeAxisProperties::LegendTickmarkCount::TICKMARK_MANY:
                    maxTickmarkCount = 10;
                    break;
                default:
                    break;
            }

            plotWidget()->setAxisMaxMajor( RiuPlotAxis::defaultBottom(), maxTickmarkCount );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateCaseNameHasChanged()
{
    if ( m_summaryCurveCollection )
    {
        m_summaryCurveCollection->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addTimeAnnotation( time_t time )
{
    RimSummaryTimeAxisProperties* axisProps = timeAxisProperties();
    {
        auto* annotation = new RimTimeAxisAnnotation;
        annotation->setTime( time );

        axisProps->appendAnnotation( annotation );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addTimeRangeAnnotation( time_t startTime, time_t endTime )
{
    RimSummaryTimeAxisProperties* axisProps = timeAxisProperties();
    {
        auto* annotation = new RimTimeAxisAnnotation;
        annotation->setTimeRange( startTime, endTime );

        axisProps->appendAnnotation( annotation );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::removeAllTimeAnnotations()
{
    RimSummaryTimeAxisProperties* axisProps = timeAxisProperties();
    axisProps->removeAllAnnotations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updateZoomInParentPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveAndUpdate( RimSummaryCurve* curve, bool autoAssignPlotAxis )
{
    if ( curve )
    {
        m_summaryCurveCollection->addCurve( curve );
        connectCurveToPlot( curve, true, autoAssignPlotAxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveNoUpdate( RimSummaryCurve* curve, bool autoAssignPlotAxis )
{
    if ( curve )
    {
        m_summaryCurveCollection->addCurve( curve );
        connectCurveToPlot( curve, false, autoAssignPlotAxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::insertCurve( RimSummaryCurve* curve, size_t insertAtPosition )
{
    if ( curve )
    {
        m_summaryCurveCollection->insertCurve( curve, insertAtPosition );
        connectCurveToPlot( curve, false, true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::connectCurveToPlot( RimSummaryCurve* curve, bool update, bool autoAssignPlotAxis )
{
    if ( autoAssignPlotAxis ) assignPlotAxis( curve );

    connectCurveSignals( curve );
    if ( plotWidget() )
    {
        if ( update )
        {
            curve->setParentPlotAndReplot( plotWidget() );
            this->updateAxes();
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
void RimSummaryPlot::removeCurve( RimSummaryCurve* curve )
{
    m_summaryCurveCollection->removeCurve( curve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteCurve( RimSummaryCurve* curve )
{
    deleteCurves( { curve } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteCurves( const std::vector<RimSummaryCurve*>& curves )
{
    for ( const auto curve : curves )
    {
        if ( m_summaryCurveCollection )
        {
            for ( auto& c : m_summaryCurveCollection->curves() )
            {
                if ( c == curve )
                {
                    disconnectCurveSignals( curve );
                    m_summaryCurveCollection->deleteCurve( curve );
                    continue;
                }
            }
        }
        if ( m_ensembleCurveSetCollection )
        {
            for ( auto& curveSet : m_ensembleCurveSetCollection->curveSets() )
            {
                for ( auto& c : curveSet->curves() )
                {
                    if ( c == curve )
                    {
                        curveSet->deleteCurve( curve );
                        if ( curveSet->curves().empty() )
                        {
                            if ( curveSet->colorMode() == RimEnsembleCurveSet::ColorMode::BY_ENSEMBLE_PARAM &&
                                 plotWidget() && curveSet->legendFrame() )
                            {
                                plotWidget()->removeOverlayFrame( curveSet->legendFrame() );
                            }
                            m_ensembleCurveSetCollection->deleteCurveSet( curveSet );
                        }
                        continue;
                    }
                }
            }
        }
    }

    RiuPlotMainWindowTools::refreshToolbars();

    updateCaseNameHasChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteCurvesAssosiatedWithCase( RimSummaryCase* summaryCase )
{
    if ( m_summaryCurveCollection )
    {
        m_summaryCurveCollection->deleteCurvesAssosiatedWithCase( summaryCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSetCollection* RimSummaryPlot::ensembleCurveSetCollection() const
{
    return m_ensembleCurveSetCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addGridTimeHistoryCurve( RimGridTimeHistoryCurve* curve )
{
    CVF_ASSERT( curve );

    m_gridTimeHistoryCurves.push_back( curve );
    if ( plotWidget() )
    {
        curve->setParentPlotAndReplot( plotWidget() );
        this->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addGridTimeHistoryCurveNoUpdate( RimGridTimeHistoryCurve* curve )
{
    CVF_ASSERT( curve );

    m_gridTimeHistoryCurves.push_back( curve );
    if ( plotWidget() )
    {
        curve->setParentPlotNoReplot( plotWidget() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridTimeHistoryCurve*> RimSummaryPlot::gridTimeHistoryCurves() const
{
    return m_gridTimeHistoryCurves.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addAsciiDataCruve( RimAsciiDataCurve* curve )
{
    CVF_ASSERT( curve );

    m_asciiDataCurves.push_back( curve );
    if ( plotWidget() )
    {
        curve->setParentPlotAndReplot( plotWidget() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryPlot::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showPlotTitle || changedField == &m_description || changedField == &m_useAutoPlotTitle )
    {
        updatePlotTitle();
        updateConnectedEditors();

        if ( !m_useAutoPlotTitle )
        {
            // When auto name of plot is turned off, update the auto name for all curves

            for ( auto c : summaryCurves() )
            {
                c->updateCurveNameNoLegendUpdate();
            }
        }
    }

    if ( changedField == &m_showPlotLegends ) updateLegend();

#ifdef USE_QTCHARTS
    if ( changedField == &m_useQtChartsPlot )
    {
        // Hide window
        setShowWindow( false );

        // Detach and destroy plot curves
        for ( auto c : summaryCurves() )
        {
            c->detach( true );
        }

        for ( auto& curveSet : this->ensembleCurveSetCollection()->curveSets() )
        {
            curveSet->detachPlotCurves( true );
        }

        // Destroy viewer
        removeMdiWindowFromMdiArea();
        deletePlotCurvesAndPlotWidget();
    }
#endif

    if ( changedField == &m_normalizeCurveYValues )
    {
        this->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateStackedCurveData()
{
    auto anyStackedCurvesPresent = updateStackedCurveDataForRelevantAxes();

    if ( plotWidget() && anyStackedCurvesPresent )
    {
        reattachAllCurves();
        plotWidget()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::updateStackedCurveDataForRelevantAxes()
{
    bool anyStackedCurvesPresent = false;
    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisProperties )
    {
        if ( axisProperties->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
             axisProperties->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        {
            anyStackedCurvesPresent |= updateStackedCurveDataForAxis( axisProperties->plotAxisType() );
        }
    }

    return anyStackedCurvesPresent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::updateStackedCurveDataForAxis( RiuPlotAxis plotAxis )
{
    auto stackedCurves = visibleStackedSummaryCurvesForAxis( plotAxis );
    if ( stackedCurves.empty() ) return false;

    std::map<RiaDefines::PhaseType, size_t> curvePhaseCount;
    for ( RimSummaryCurve* curve : stackedCurves )
    {
        // Apply a area filled style if it isn't already set
        if ( curve->fillStyle() == Qt::NoBrush )
        {
            curve->setFillStyle( Qt::SolidPattern );
        }

        curve->loadDataAndUpdate( false );

        curvePhaseCount[curve->phaseType()]++;
    }

    {
        // Z-position of curve, to draw them in correct order
        double zPos = -10000.0;

        std::vector<time_t> allTimeSteps;
        for ( RimSummaryCurve* curve : stackedCurves )
        {
            allTimeSteps.insert( allTimeSteps.end(), curve->timeStepsY().begin(), curve->timeStepsY().end() );
        }
        std::sort( allTimeSteps.begin(), allTimeSteps.end() );
        allTimeSteps.erase( std::unique( allTimeSteps.begin(), allTimeSteps.end() ), allTimeSteps.end() );

        std::vector<double> allStackedValues( allTimeSteps.size(), 0.0 );

        size_t stackIndex = 0u;
        for ( RimSummaryCurve* curve : stackedCurves )
        {
            for ( size_t i = 0; i < allTimeSteps.size(); ++i )
            {
                double value = curve->yValueAtTimeT( allTimeSteps[i] );
                if ( value != std::numeric_limits<double>::infinity() )
                {
                    allStackedValues[i] += value;
                }
            }

            curve->setOverrideCurveDataY( allTimeSteps, allStackedValues );
            curve->setZOrder( zPos );
            if ( curve->isStackedWithPhaseColors() )
            {
                curve->assignStackColor( stackIndex, curvePhaseCount[curve->phaseType()] );
            }
            zPos -= 1.0;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimSummaryPlot::snapshotWindowContent()
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
void RimSummaryPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( uiConfigName == RicSummaryPlotEditorUi::CONFIGURATION_NAME )
    {
        uiTreeOrdering.add( &m_summaryCurveCollection );
        if ( !m_isCrossPlot )
        {
            uiTreeOrdering.add( &m_ensembleCurveSetCollection );
        }
    }
    else
    {
        uiTreeOrdering.add( &m_axisProperties );

        uiTreeOrdering.add( &m_summaryCurveCollection );
        if ( !m_isCrossPlot )
        {
            uiTreeOrdering.add( &m_ensembleCurveSetCollection );
        }
        uiTreeOrdering.add( &m_gridTimeHistoryCurves );
        uiTreeOrdering.add( &m_asciiDataCurves );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onLoadDataAndUpdate()
{
    updatePlotTitle();
    updateMdiWindowVisibility();

    if ( m_summaryCurveCollection )
    {
        m_summaryCurveCollection->loadDataAndUpdate( false );
    }

    m_ensembleCurveSetCollection->loadDataAndUpdate( false );

    for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    {
        curve->loadDataAndUpdate( false );
    }

    for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    {
        curve->loadDataAndUpdate( false );
    }

    if ( plotWidget() )
    {
        plotWidget()->setInternalLegendVisible( m_showPlotLegends && !isSubPlot() );
        plotWidget()->setLegendFontSize( legendFontSize() );
        plotWidget()->updateLegend();
    }
    this->updateAxes();

    m_textCurveSetEditor->updateTextFilter();

    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomInParentPlot()
{
    if ( plotWidget() )
    {
        for ( const auto& axisProperty : m_axisProperties )
        {
            updateZoomForAxis( axisProperty->plotAxisType() );
        }

        plotWidget()->updateAxes();
        updateZoomFromParentPlot();
        plotWidget()->updateZoomDependentCurveProperties();
        plotWidget()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomFromParentPlot()
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisProperties )
    {
        auto [axisMin, axisMax] = plotWidget()->axisRange( axisProperties->plotAxisType() );
        axisProperties->setVisibleRangeMax( axisMax );
        axisProperties->setVisibleRangeMin( axisMin );
        axisProperties->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deletePlotCurvesAndPlotWidget()
{
    if ( isDeletable() )
    {
        detachAllPlotItems();

        if ( plotWidget() )
        {
            plotWidget()->setParent( nullptr );
        }

        deleteAllPlotCurves();

        if ( m_summaryPlot )
        {
            m_summaryPlot.reset();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::connectCurveSignals( RimSummaryCurve* curve )
{
    curve->dataChanged.connect( this, &RimSummaryPlot::curveDataChanged );
    curve->visibilityChanged.connect( this, &RimSummaryPlot::curveVisibilityChanged );
    curve->appearanceChanged.connect( this, &RimSummaryPlot::curveAppearanceChanged );
    curve->stackingChanged.connect( this, &RimSummaryPlot::curveStackingChanged );
    curve->stackingColorsChanged.connect( this, &RimSummaryPlot::curveStackingColorsChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::disconnectCurveSignals( RimSummaryCurve* curve )
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
void RimSummaryPlot::curveDataChanged( const caf::SignalEmitter* emitter )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::curveVisibilityChanged( const caf::SignalEmitter* emitter, bool visible )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::curveAppearanceChanged( const caf::SignalEmitter* emitter )
{
    if ( plotWidget() )
    {
        plotWidget()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::curveStackingChanged( const caf::SignalEmitter* emitter, bool stacked )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::curveStackingColorsChanged( const caf::SignalEmitter* emitter, bool stackWithPhaseColors )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::connectAxisSignals( RimPlotAxisProperties* axis )
{
    axis->settingsChanged.connect( this, &RimSummaryPlot::axisSettingsChanged );
    axis->logarithmicChanged.connect( this, &RimSummaryPlot::axisLogarithmicChanged );
    axis->axisPositionChanged.connect( this, &RimSummaryPlot::axisPositionChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::axisSettingsChanged( const caf::SignalEmitter* emitter )
{
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimSummaryPlot::addNewAxisProperties( RiaDefines::PlotAxis plotAxis, const QString& name )
{
    RiuPlotAxis newPlotAxis = plotWidget()->createNextPlotAxis( plotAxis );
    return addNewAxisProperties( newPlotAxis, name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimSummaryPlot::addNewAxisProperties( RiuPlotAxis plotAxis, const QString& name )
{
    auto* axisProperties = new RimPlotAxisProperties;
    axisProperties->setNameAndAxis( name, plotAxis.axis(), plotAxis.index() );
    m_axisProperties.push_back( axisProperties );
    connectAxisSignals( axisProperties );

    return axisProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::axisPositionChanged( const caf::SignalEmitter* emitter,
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
        axisProperties->setNameAndAxis( axisProperties->name(), fixedUpPlotAxis.axis(), fixedUpPlotAxis.index() );

        // Move all attached curves
        for ( auto curve : summaryCurves() )
        {
            if ( curve->axisY() == oldPlotAxis ) curve->setLeftOrRightAxisY( fixedUpPlotAxis );
        }

        for ( auto curveSet : ensembleCurveSetCollection()->curveSets() )
        {
            if ( curveSet->axisY() == oldPlotAxis ) curveSet->setLeftOrRightAxisY( fixedUpPlotAxis );
        }

        // Remove the now unused axis (but keep the default axis)
        if ( oldPlotAxis != RiuPlotAxis::defaultLeft() && oldPlotAxis != RiuPlotAxis::defaultRight() )
        {
            auto oldAxisProperties = axisPropertiesForPlotAxis( oldPlotAxis );
            if ( oldAxisProperties ) m_axisProperties.removeChildObject( oldAxisProperties );
        }

        std::set<RiuPlotAxis> usedPlotAxis;
        for ( const auto& axisProperties : m_axisProperties )
        {
            usedPlotAxis.insert( axisProperties->plotAxisType() );
        }

        plotWidget()->pruneAxes( usedPlotAxis );
    }

    // This is probably to much, but difficult to find the required updates
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteAllGridTimeHistoryCurves()
{
    m_gridTimeHistoryCurves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::enableAutoPlotTitle( bool enable )
{
    m_useAutoPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::autoPlotTitle() const
{
    return m_useAutoPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryPlot::handleSummaryCaseDrop( RimSummaryCase* summaryCase )
{
    int newCurves = 0;

    std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>> dataVectorMap;

    for ( auto& curve : summaryCurves() )
    {
        const auto curveAddress = curve->summaryAddressY();
        dataVectorMap[curveAddress].insert( curve->summaryCaseY() );
    }

    for ( const auto& [addr, cases] : dataVectorMap )
    {
        if ( cases.count( summaryCase ) > 0 ) continue;

        addNewCurveY( addr, summaryCase );
        newCurves++;
    }

    return newCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryPlot::handleAddressCollectionDrop( RimSummaryAddressCollection* addressCollection )
{
    int  newCurves   = 0;
    auto droppedName = addressCollection->name().toStdString();

    if ( addressCollection->isEnsemble() ) return 0;

    auto summaryCase = RiaSummaryTools::summaryCaseById( addressCollection->caseId() );
    if ( summaryCase )
    {
        if ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL )
        {
            std::map<std::string, std::set<std::string>> dataVectorMap;

            for ( auto& curve : summaryCurves() )
            {
                const auto curveAddress = curve->summaryAddressY();
                if ( curveAddress.category() == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL )
                {
                    dataVectorMap[curveAddress.quantityName()].insert( curveAddress.wellName() );
                }
            }

            for ( auto& [vectorName, wellNames] : dataVectorMap )
            {
                if ( wellNames.count( droppedName ) > 0 ) continue;

                addNewCurveY( RifEclipseSummaryAddress::wellAddress( vectorName, droppedName ), summaryCase );
                newCurves++;
            }
        }
        else if ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL_GROUP )
        {
            std::map<std::string, std::set<std::string>> dataVectorMap;

            for ( auto& curve : summaryCurves() )
            {
                const auto curveAddress = curve->summaryAddressY();
                if ( curveAddress.category() == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_GROUP )
                {
                    dataVectorMap[curveAddress.quantityName()].insert( curveAddress.wellGroupName() );
                }
            }

            for ( auto& [vectorName, wellGroupNames] : dataVectorMap )
            {
                if ( wellGroupNames.count( droppedName ) > 0 ) continue;

                addNewCurveY( RifEclipseSummaryAddress::wellGroupAddress( vectorName, droppedName ), summaryCase );
                newCurves++;
            }
        }
        else if ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION )
        {
            std::map<std::string, std::set<int>> dataVectorMap;

            for ( auto& curve : summaryCurves() )
            {
                const auto curveAddress = curve->summaryAddressY();
                if ( curveAddress.category() == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION )
                {
                    dataVectorMap[curveAddress.quantityName()].insert( curveAddress.regionNumber() );
                }
            }

            int droppedRegion = std::stoi( droppedName );

            for ( auto& [vectorName, regionNumbers] : dataVectorMap )
            {
                if ( regionNumbers.count( droppedRegion ) > 0 ) continue;

                addNewCurveY( RifEclipseSummaryAddress::regionAddress( vectorName, droppedRegion ), summaryCase );
                newCurves++;
            }
        }
    }

    return newCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryPlot::handleSummaryAddressDrop( RimSummaryAddress* summaryAddr )
{
    int newCurves = 0;

    if ( summaryAddr->isEnsemble() )
    {
        auto ensemble = RiaSummaryTools::ensembleById( summaryAddr->ensembleId() );
        if ( ensemble )
        {
            addNewEnsembleCurveY( summaryAddr->address(), ensemble );
            newCurves++;
        }
    }
    else
    {
        auto summaryCase = RiaSummaryTools::summaryCaseById( summaryAddr->caseId() );
        if ( summaryCase )
        {
            addNewCurveY( summaryAddr->address(), summaryCase );
            newCurves++;
        }
    }
    return newCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects )
{
    int newCurves = 0;

    for ( auto obj : objects )
    {
        auto summaryCase = dynamic_cast<RimSummaryCase*>( obj );
        if ( summaryCase )
        {
            newCurves += handleSummaryCaseDrop( summaryCase );
            continue;
        }

        auto summaryAddr = dynamic_cast<RimSummaryAddress*>( obj );
        if ( summaryAddr )
        {
            newCurves += handleSummaryAddressDrop( summaryAddr );
            continue;
        }

        auto addressCollection = dynamic_cast<RimSummaryAddressCollection*>( obj );
        if ( addressCollection )
        {
            newCurves += handleAddressCollectionDrop( addressCollection );
            continue;
        }
    }

    if ( newCurves > 0 )
    {
        applyDefaultCurveAppearances();
        loadDataAndUpdate();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addNewCurveY( const RifEclipseSummaryAddress& address, RimSummaryCase* summaryCase )
{
    auto* newCurve = new RimSummaryCurve();
    newCurve->setSummaryCaseY( summaryCase );
    newCurve->setSummaryAddressYAndApplyInterpolation( address );
    addCurveNoUpdate( newCurve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addNewEnsembleCurveY( const RifEclipseSummaryAddress& address, RimSummaryCaseCollection* ensemble )
{
    auto* curveSet = new RimEnsembleCurveSet();

    curveSet->setSummaryCaseCollection( ensemble );
    curveSet->setSummaryAddress( address );
    ensembleCurveSetCollection()->addCurveSet( curveSet );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onPlotZoomed()
{
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );
    updateZoomFromParentPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        auto group = uiOrdering.addNewGroup( "Data Source" );
        m_sourceStepping()->uiOrdering( uiConfigName, *group );
    }

    caf::PdmUiGroup* mainOptions = uiOrdering.addNewGroup( "General Plot Options" );
    mainOptions->setCollapsedByDefault( true );
#ifdef USE_QTCHARTS
    mainOptions->add( &m_useQtChartsPlot );
#endif
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
        mainOptions->add( &m_rowSpan );
        mainOptions->add( &m_colSpan );
    }
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle );

    if ( isMdiWindow() )
    {
        uiOrderingForPlotLayout( uiConfigName, *mainOptions );
    }

    mainOptions->add( &m_normalizeCurveYValues );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimSummaryPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    if ( !plotWidget() )
    {
#ifdef USE_QTCHARTS
        bool useQtCharts = m_useQtChartsPlot;

        auto regTestRunner = RiaRegressionTestRunner::instance();
        if ( regTestRunner->isRunningRegressionTests() )
        {
            if ( regTestRunner->overridePlotEngine() == RiaRegressionTest::PlotEngine::USE_QWT )
                useQtCharts = false;
            else if ( regTestRunner->overridePlotEngine() == RiaRegressionTest::PlotEngine::USER_QTCHARTS )
                useQtCharts = true;
        }

        if ( useQtCharts )
        {
            m_summaryPlot = std::make_unique<RiuSummaryQtChartsPlot>( this );
        }
        else
        {
            m_summaryPlot = std::make_unique<RiuSummaryQwtPlot>( this, mainWindowParent );
        }
#else
        m_summaryPlot = std::make_unique<RiuSummaryQwtPlot>( this, mainWindowParent );
#endif

        for ( const auto& axisProperties : m_axisProperties )
        {
            plotWidget()->ensureAxisIsCreated( axisProperties->plotAxisType() );
        }

        for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
        {
            curve->setParentPlotNoReplot( plotWidget() );
        }

        for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
        {
            curve->setParentPlotNoReplot( plotWidget() );
        }

        if ( m_summaryCurveCollection )
        {
            m_summaryCurveCollection->setParentPlotAndReplot( plotWidget() );
        }

        if ( m_ensembleCurveSetCollection )
        {
            m_ensembleCurveSetCollection->setParentPlotAndReplot( plotWidget() );
        }

        this->connect( plotWidget(), SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        updatePlotTitle();
    }

    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteViewWidget()
{
    deletePlotCurvesAndPlotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::initAfterRead()
{
    RimViewWindow::initAfterRead();

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2021.10.2" ) )
    {
        auto copyAxis = [this]( RiuPlotAxis axis, auto sourceObject ) {
            auto axisProperties = axisPropertiesForPlotAxis( axis );
            if ( axisProperties )
            {
                QString data = sourceObject->writeObjectToXmlString();

                // This operation will overwrite the plot axis side, default is left
                axisProperties->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );

                auto plotAxisProperties = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
                if ( plotAxisProperties )
                {
                    // Reset the plot axis for the axis property
                    plotAxisProperties->setNameAndAxis( axisProperties->name(), axis.axis(), 0 );
                }
            }
        };

        copyAxis( RiuPlotAxis::defaultLeft(), m_leftYAxisProperties_OBSOLETE.v() );
        copyAxis( RiuPlotAxis::defaultRight(), m_rightYAxisProperties_OBSOLETE.v() );

        if ( m_isCrossPlot )
            copyAxis( RiuPlotAxis::defaultBottom(), m_bottomAxisProperties_OBSOLETE.v() );
        else
            copyAxis( RiuPlotAxis::defaultBottom(), m_timeAxisProperties_OBSOLETE.v() );
    }

    for ( const auto& axisProperties : m_axisProperties )
    {
        auto plotAxisProperties = dynamic_cast<RimPlotAxisProperties*>( axisProperties.p() );
        if ( plotAxisProperties )
        {
            connectAxisSignals( plotAxisProperties );
        }
    }

    for ( auto curve : summaryCurves() )
    {
        connectCurveSignals( curve );
    }

    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateNameHelperWithCurveData( RimSummaryPlotNameHelper* nameHelper ) const
{
    if ( !nameHelper ) return;

    nameHelper->clear();
    std::vector<RifEclipseSummaryAddress>  addresses;
    std::vector<RimSummaryCase*>           sumCases;
    std::vector<RimSummaryCaseCollection*> ensembleCases;

    if ( m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible() )
    {
        for ( RimSummaryCurve* curve : m_summaryCurveCollection->curves() )
        {
            if ( curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
            {
                RiaSummaryTools::getSummaryCasesAndAddressesForCalculation( curve->summaryAddressY().id(),
                                                                            sumCases,
                                                                            addresses );
            }
            else
            {
                addresses.push_back( curve->summaryAddressY() );
                sumCases.push_back( curve->summaryCaseY() );

                if ( curve->summaryCaseX() )
                {
                    sumCases.push_back( curve->summaryCaseX() );

                    if ( curve->summaryAddressX().category() != RifEclipseSummaryAddress::SUMMARY_INVALID )
                    {
                        addresses.push_back( curve->summaryAddressX() );
                    }
                }
            }
        }
    }

    for ( auto curveSet : m_ensembleCurveSetCollection->curveSets() )
    {
        addresses.push_back( curveSet->summaryAddress() );
        ensembleCases.push_back( curveSet->summaryCaseCollection() );
    }

    nameHelper->clear();
    nameHelper->appendAddresses( addresses );
    nameHelper->setSummaryCases( sumCases );
    nameHelper->setEnsembleCases( ensembleCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::doUpdateLayout()
{
    updateFonts();

    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::detachAllPlotItems()
{
    if ( m_summaryCurveCollection )
    {
        m_summaryCurveCollection->detachPlotCurves();
    }

    if ( m_ensembleCurveSetCollection )
    {
        m_ensembleCurveSetCollection->detachPlotCurves();
    }

    for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    {
        curve->detach();
    }

    for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    {
        curve->detach();
    }

    m_plotInfoLabel->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteAllPlotCurves()
{
    for ( auto* c : summaryCurves() )
    {
        c->deletePlotCurve();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateCurveNames()
{
    if ( m_summaryCurveCollection->isCurvesVisible() )
    {
        for ( auto c : summaryCurves() )
        {
            if ( c->isCurveVisible() ) c->updateCurveNameNoLegendUpdate();
        }
    }

    for ( auto curveSet : m_ensembleCurveSetCollection->curveSets() )
    {
        curveSet->updateEnsembleLegendItem();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::detachAllCurves()
{
    detachAllPlotItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::reattachAllCurves()
{
    if ( m_summaryCurveCollection )
    {
        m_summaryCurveCollection->reattachPlotCurves();
    }

    m_ensembleCurveSetCollection->reattachPlotCurves();

    for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    {
        curve->reattach();
    }

    for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    {
        curve->reattach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::handleGlobalKeyEvent( QKeyEvent* keyEvent )
{
    return RimSummaryPlotControls::handleKeyEvents( sourceSteppingObjectForKeyEventHandling(), keyEvent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onCurveCollectionChanged( const SignalEmitter* emitter )
{
    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping* RimSummaryPlot::sourceSteppingObjectForKeyEventHandling() const
{
    return m_sourceStepping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlot::fieldsToShowInToolbar()
{
    std::vector<caf::PdmFieldHandle*> toolBarFields;

    {
        auto fields = m_textCurveSetEditor->fieldsToShowInToolbar();
        toolBarFields.insert( std::end( toolBarFields ), std::begin( fields ), std::end( fields ) );
    }

    bool anyFieldsAvailableForSummary = false;

    auto sourceObject = sourceSteppingObjectForKeyEventHandling();
    if ( sourceObject )
    {
        auto fields = sourceObject->fieldsToShowInToolbar();
        toolBarFields.insert( std::end( toolBarFields ), std::begin( fields ), std::end( fields ) );

        anyFieldsAvailableForSummary = !fields.empty();
    }

    if ( !anyFieldsAvailableForSummary )
    {
        // Show ensemble stepping if no fields are available from summary stepping
        auto fields = ensembleCurveSetCollection()->fieldsToShowInToolbar();
        toolBarFields.insert( std::end( toolBarFields ), std::begin( fields ), std::end( fields ) );
    }

    return toolBarFields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setAutoScaleXEnabled( bool enabled )
{
    for ( const auto& ap : m_axisProperties )
    {
        if ( ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_TOP ||
             ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        {
            ap->setAutoZoom( enabled );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setAutoScaleYEnabled( bool enabled )
{
    for ( const auto& ap : m_axisProperties )
    {
        if ( ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
             ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        {
            ap->setAutoZoom( enabled );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryPlot::curveCount() const
{
    return m_summaryCurveCollection->curves().size() + m_gridTimeHistoryCurves.size() + m_asciiDataCurves.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isDeletable() const
{
    RimMultiPlot* plotWindow = nullptr;
    firstAncestorOrThisOfType( plotWindow );
    return plotWindow == nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisPropertiesInterface*> RimSummaryPlot::plotAxes() const
{
    std::vector<RimPlotAxisPropertiesInterface*> axisProps;
    for ( const auto& ap : m_axisProperties )
    {
        axisProps.push_back( ap );
    }

    return axisProps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::assignPlotAxis( RimSummaryCurve* destinationCurve )
{
    enum class AxisAssignmentStrategy
    {
        ALL_TO_LEFT,
        ALL_TO_RIGHT,
        ALTERNATING,
        USE_MATCHING_UNIT
    };

    RiaDefines::PlotAxis plotAxis = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;

    auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;
    if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
    {
        auto destinationUnit = destinationCurve->unitNameY();

        bool isLeftUsed  = false;
        bool isRightUsed = false;

        for ( auto c : summaryCurves() )
        {
            if ( c == destinationCurve ) continue;

            if ( c->axisY() == RiuPlotAxis::defaultLeft() ) isLeftUsed = true;
            if ( c->axisY() == RiuPlotAxis::defaultRight() ) isRightUsed = true;

            auto currentUnit = c->unitNameY();

            if ( currentUnit == destinationUnit )
            {
                for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisProperties )
                {
                    if ( axisProperties->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
                         axisProperties->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
                    {
                        destinationCurve->setLeftOrRightAxisY( c->axisY() );

                        return;
                    }
                }
            }
        }

        if ( !isLeftUsed )
        {
            destinationCurve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
            return;
        }

        if ( !isRightUsed )
        {
            destinationCurve->setLeftOrRightAxisY( RiuPlotAxis::defaultRight() );
            return;
        }

        strategy = AxisAssignmentStrategy::ALTERNATING;
    }

    if ( strategy == AxisAssignmentStrategy::ALTERNATING )
    {
        size_t axisCountLeft  = 0;
        size_t axisCountRight = 0;
        for ( const auto& ap : m_axisProperties )
        {
            if ( ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
                axisCountLeft++;
            else if ( ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
                axisCountRight++;
        }

        if ( axisCountLeft > axisCountRight ) plotAxis = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    }
    else if ( strategy == AxisAssignmentStrategy::ALL_TO_LEFT )
    {
        plotAxis = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    }
    else if ( strategy == AxisAssignmentStrategy::ALL_TO_RIGHT )
    {
        plotAxis = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    }

    RiuPlotAxis newPlotAxis = RiuPlotAxis::defaultLeft();
    if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
    {
        QString axisObjectName = "New Axis";
        if ( !destinationCurve->summaryAddressY().uiText().empty() )
            axisObjectName = QString::fromStdString( destinationCurve->summaryAddressY().uiText() );

        newPlotAxis = plotWidget()->createNextPlotAxis( plotAxis );
        addNewAxisProperties( newPlotAxis, axisObjectName );
    }

    destinationCurve->setLeftOrRightAxisY( newPlotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                     std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    if ( childArray == &m_axisProperties )
    {
        for ( caf::PdmObjectHandle* reffingObj : referringObjects )
        {
            auto* curve    = dynamic_cast<RimSummaryCurve*>( reffingObj );
            auto* curveSet = dynamic_cast<RimEnsembleCurveSet*>( reffingObj );
            if ( curve )
            {
                curve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
            }
            else if ( curveSet )
            {
                curveSet->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
            }
        }

        if ( plotWidget() )
        {
            std::set<RiuPlotAxis> usedPlotAxis;
            for ( const auto& axisProperties : m_axisProperties )
            {
                usedPlotAxis.insert( axisProperties->plotAxisType() );
            }

            plotWidget()->pruneAxes( usedPlotAxis );
            updateAxes();
            plotWidget()->scheduleReplot();
        }
    }
}
