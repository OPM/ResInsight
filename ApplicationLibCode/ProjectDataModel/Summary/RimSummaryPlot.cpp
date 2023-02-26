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
#include "RiaColorTools.h"
#include "RiaDefines.h"
#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"
#include "RiaPreferencesSummary.h"
#include "RiaRegressionTestRunner.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RifReaderEclipseSummary.h"

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
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurvesData.h"
#include "RimSummaryPlotAxisFormatter.h"
#include "RimSummaryPlotControls.h"
#include "RimSummaryPlotFilterTextCurveSetEditor.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuPlotAxis.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotItem.h"
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

#include "qwt_date.h"
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
#include <cmath>
#include <limits>
#include <set>

CAF_PDM_SOURCE_INIT( RimSummaryPlot, "SummaryPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot( bool isCrossPlot )
    : RimPlot()
    , m_isCrossPlot( isCrossPlot )
    , curvesChanged( this )
    , axisChanged( this )
    , plotZoomedByUser( this )
    , titleChanged( this )
    , m_isValid( true )
    , axisChangedReloadRequired( this )
{
    CAF_PDM_InitScriptableObject( "Summary Plot", ":/SummaryPlotLight16x16.png", "", "A Summary Plot" );

    CAF_PDM_InitScriptableField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title" );
    CAF_PDM_InitScriptableField( &m_description, "PlotDescription", QString( "Summary Plot" ), "Name" );
    CAF_PDM_InitScriptableField( &m_normalizeCurveYValues, "normalizeCurveYValues", false, "Normalize all curves" );
#ifdef USE_QTCHARTS
    bool useQtChart = RiaPreferences::current()->useQtChartsAsDefaultPlotType();
    CAF_PDM_InitScriptableField( &m_useQtChartsPlot, "useQtChartsPlot", useQtChart, "Use Qt Charts" );
    m_useQtChartsPlot.uiCapability()->setUiHidden( true );
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

    CAF_PDM_InitFieldNoDefault( &m_axisPropertiesArray, "AxisProperties", "Axes", ":/Axes16x16.png" );

    if ( m_isCrossPlot )
    {
        addNewAxisProperties( RiuPlotAxis::defaultBottom(), "Bottom" );
    }
    else
    {
        auto* timeAxisProperties = new RimSummaryTimeAxisProperties;
        timeAxisProperties->settingsChanged.connect( this, &RimSummaryPlot::timeAxisSettingsChanged );
        timeAxisProperties->requestLoadDataAndUpdate.connect( this, &RimSummaryPlot::timeAxisSettingsChangedReloadRequired );

        m_axisPropertiesArray.push_back( timeAxisProperties );
    }

    auto leftAxis = addNewAxisProperties( RiuPlotAxis::defaultLeft(), "Left" );
    leftAxis->setAlwaysRequired( true );

    auto rightAxis = addNewAxisProperties( RiuPlotAxis::defaultRight(), "Right" );
    rightAxis->setAlwaysRequired( true );

    CAF_PDM_InitFieldNoDefault( &m_textCurveSetEditor, "SummaryPlotFilterTextCurveSetEditor", "Text Filter Curve Creator" );
    m_textCurveSetEditor.uiCapability()->setUiTreeHidden( true );
    m_textCurveSetEditor = new RimSummaryPlotFilterTextCurveSetEditor;

    m_nameHelperAllCurves = std::make_unique<RimSummaryPlotNameHelper>();

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );
    m_sourceStepping = new RimSummaryPlotSourceStepping;
    if ( m_isCrossPlot )
    {
        m_sourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::UNION_X_Y_AXIS );
    }
    else
    {
        m_sourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS );
    }

    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeHidden( true );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_fallbackPlotName, "AlternateName", "AlternateName" );
    m_fallbackPlotName.uiCapability()->setUiReadOnly( true );
    m_fallbackPlotName.uiCapability()->setUiHidden( true );
    m_fallbackPlotName.xmlCapability()->disableIO();

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
    m_isValid = false;

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
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

    if ( m_summaryPlot ) m_summaryPlot->clearAnnotationObjects();

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
        updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    }
    else
    {
        updateTimeAxis( timeAxisProperties() );
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
bool RimSummaryPlot::isCurveHighlightSupported() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties* RimSummaryPlot::timeAxisProperties()
{
    // Find the first time axis (which is correct since there is only one).
    for ( const auto& ap : m_axisPropertiesArray )
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
QString RimSummaryPlot::asciiDataForSummaryPlotExport( RiaDefines::DateTimePeriod resamplingPeriod, bool showTimeAsLongString ) const
{
    std::vector<RimSummaryCurve*> curves;
    this->descendantsIncludingThisOfType( curves );

    auto gridCurves  = m_gridTimeHistoryCurves.children();
    auto asciiCurves = m_asciiDataCurves.children();

    QString text = RimSummaryCurvesData::createTextForExport( curves, asciiCurves, gridCurves, resamplingPeriod, showTimeAsLongString );

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
void RimSummaryPlot::onAxisSelected( RiuPlotAxis axis, bool toggle )
{
    RiuPlotMainWindowTools::showPlotMainWindow();

    caf::PdmObject* itemToSelect = axisPropertiesForPlotAxis( axis );

    RiuPlotMainWindowTools::selectOrToggleObject( itemToSelect, toggle );
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
    if ( m_isCrossPlot ) return { RimSummaryDataSourceStepping::Axis::X_AXIS, RimSummaryDataSourceStepping::Axis::Y_AXIS };

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
        allCurveDefs.insert( RiaSummaryCurveDefinition( curve->summaryCaseY(), curve->summaryAddressY(), curve->isEnsembleCurve() ) );
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

    std::copy_if( visibleCurves.begin(), visibleCurves.end(), std::back_inserter( visibleStackedCurves ), []( RimSummaryCurve* curve ) {
        return curve->isStacked();
    } );

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

    if ( m_description().isEmpty() )
    {
        RimMultiPlot* plotWindow = nullptr;
        firstAncestorOrThisOfType( plotWindow );

        size_t index = 0;
        if ( plotWindow ) index = plotWindow->plotIndex( this );

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
void RimSummaryPlot::copyAxisPropertiesFromOther( const RimSummaryPlot& sourceSummaryPlot )
{
    for ( auto ap : sourceSummaryPlot.plotAxes() )
    {
        QString data = ap->writeObjectToXmlString();

        auto axisProperty = axisPropertiesForPlotAxis( ap->plotAxisType() );
        if ( axisProperty )
        {
            axisProperty->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::copyAxisPropertiesFromOther( RiaDefines::PlotAxis plotAxisType, const RimSummaryPlot& sourceSummaryPlot )
{
    for ( auto ap : sourceSummaryPlot.plotAxes() )
    {
        if ( ap->plotAxisType().axis() != plotAxisType ) continue;

        QString data = ap->writeObjectToXmlString();

        axisPropertiesForPlotAxis( ap->plotAxisType() )->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::copyMatchingAxisPropertiesFromOther( const RimSummaryPlot& summaryPlot )
{
    for ( auto apToCopy : summaryPlot.plotAxes() )
    {
        for ( auto ap : plotAxes() )
        {
            if ( ap->objectName().compare( apToCopy->objectName() ) == 0 )
            {
                QString data = apToCopy->writeObjectToXmlString();
                ap->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
            }
        }
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
    size_t resamplableSummaryCurveCount         = std::count_if( summaryCurves.begin(), summaryCurves.end(), []( RimSummaryCurve* curve ) {
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
    applyDefaultCurveAppearances( summaryCurves() );
    applyDefaultCurveAppearances( ensembleCurveSetCollection()->curveSets() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::applyDefaultCurveAppearances( std::vector<RimSummaryCurve*> curvesToUpdate )
{
    std::set<RiaSummaryCurveDefinition> allCurveDefs = this->summaryAndEnsembleCurveDefinitions();
    RimSummaryCurveAppearanceCalculator curveLookCalc( allCurveDefs );

    for ( auto& curve : curvesToUpdate )
    {
        curve->resetAppearance();
        curveLookCalc.setupCurveLook( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::applyDefaultCurveAppearances( std::vector<RimEnsembleCurveSet*> ensembleCurvesToUpdate )
{
    auto allCurveSets = ensembleCurveSetCollection()->curveSets();

    for ( auto curveSet : ensembleCurvesToUpdate )
    {
        size_t colorIndex = 0;

        auto it = std::find( allCurveSets.begin(), allCurveSets.end(), curveSet );
        if ( it != allCurveSets.end() )
        {
            colorIndex = std::distance( allCurveSets.begin(), it );
        }

        if ( curveSet->colorMode() != RimEnsembleCurveSet::ColorMode::SINGLE_COLOR ) continue;

        cvf::Color3f curveColor = RimSummaryCurveAppearanceCalculator::computeTintedCurveColorForAddress( curveSet->summaryAddress(),
                                                                                                          static_cast<int>( colorIndex ) );

        auto adr = curveSet->summaryAddress();
        if ( adr.isHistoryVector() ) curveColor = RiaPreferencesSummary::current()->historyCurveContrastColor();

        curveSet->setColor( curveColor );
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
/// Update regular axis with numerical axis values - i.e. not time axis.
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateNumericalAxis( RiaDefines::PlotAxis plotAxis )
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        RiuPlotAxis riuPlotAxis = axisProperties->plotAxisType();
        if ( riuPlotAxis.axis() == plotAxis )
        {
            auto* axisProps = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
            if ( axisProps )
            {
                if ( axisProperties->isActive() && hasVisibleCurvesForAxis( riuPlotAxis ) )
                {
                    plotWidget()->enableAxis( riuPlotAxis, true );
                }
                else
                {
                    plotWidget()->enableAxis( riuPlotAxis, false );
                }

                if ( !hasVisibleCurvesForAxis( riuPlotAxis ) )
                {
                    axisProps->setNameForUnusedAxis();
                }
                else
                {
                    std::set<QString> timeHistoryQuantities;

                    for ( auto c : visibleTimeHistoryCurvesForAxis( riuPlotAxis ) )
                    {
                        timeHistoryQuantities.insert( c->quantityName() );
                    }

                    std::vector<RiaSummaryCurveDefinition> curveDefs;
                    for ( auto summaryCurve : summaryCurves() )
                    {
                        if ( summaryCurve->axisY() != riuPlotAxis ) continue;

                        curveDefs.push_back( summaryCurve->curveDefinitionY() );
                    }

                    for ( auto curveSet : ensembleCurveSetCollection()->curveSets() )
                    {
                        if ( curveSet->axisY() != riuPlotAxis ) continue;

                        RiaSummaryCurveDefinition def( curveSet->summaryCaseCollection(), curveSet->summaryAddress() );
                        curveDefs.push_back( def );
                    }

                    RimSummaryPlotAxisFormatter calc( axisProps, {}, curveDefs, visibleAsciiDataCurvesForAxis( riuPlotAxis ), timeHistoryQuantities );

                    calc.applyAxisPropertiesToPlot( plotWidget() );
                }
            }

            plotWidget()->enableAxisNumberLabels( riuPlotAxis, axisProps->showNumbers() );

            RimSummaryTimeAxisProperties::LegendTickmarkCount tickmarkCountEnum = axisProps->majorTickmarkCount();
            int maxTickmarkCount = RimPlotAxisPropertiesInterface::tickmarkCountFromEnum( tickmarkCountEnum );
            plotWidget()->setAutoTickIntervalCounts( riuPlotAxis, maxTickmarkCount, maxTickmarkCount );
        }
    }
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

        if ( timeAxisProperties->tickmarkType() == RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_COUNT )
        {
            RimSummaryTimeAxisProperties::LegendTickmarkCount tickmarkCountEnum = timeAxisProperties->majorTickmarkCount();
            int maxTickmarkCount = RimPlotAxisPropertiesInterface::tickmarkCountFromEnum( tickmarkCountEnum );

            plotWidget()->setAxisMaxMajor( RiuPlotAxis::defaultBottom(), maxTickmarkCount );
        }
        else if ( timeAxisProperties->tickmarkType() == RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_CUSTOM )
        {
            createAndSetCustomTimeAxisTickmarks( timeAxisProperties );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomForAxis( RimPlotAxisPropertiesInterface* axisProperties )
{
    RimSummaryTimeAxisProperties* timeAxisProps = dynamic_cast<RimSummaryTimeAxisProperties*>( axisProperties );
    if ( timeAxisProps )
    {
        updateZoomForTimeAxis( timeAxisProps );
        return;
    }

    RimPlotAxisProperties* axisProps = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
    if ( axisProps )
    {
        updateZoomForNumericalAxis( axisProps );
        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomForNumericalAxis( RimPlotAxisProperties* axisProperties )
{
    if ( !axisProperties ) return;

    const auto plotAxis = axisProperties->plotAxisType();
    if ( axisProperties->isAutoZoom() )
    {
        if ( axisProperties->isLogarithmicScaleEnabled() )
        {
            plotWidget()->setAxisScaleType( plotAxis, RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC );
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

            if ( axisProperties->isAxisInverted() )
            {
                std::swap( min, max );
            }

            plotWidget()->setAxisScale( axisProperties->plotAxisType(), min, max );
        }
        else if ( ( plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT ) &&
                  isOnlyWaterCutCurvesVisible( plotAxis ) )
        {
            plotWidget()->setAxisScale( axisProperties->plotAxisType(), 0.0, 1.0 );
        }
        else
        {
            plotWidget()->setAxisAutoScale( axisProperties->plotAxisType(), true );
        }
    }
    else
    {
        double min = axisProperties->visibleRangeMin();
        double max = axisProperties->visibleRangeMax();
        if ( axisProperties->isAxisInverted() ) std::swap( min, max );
        plotWidget()->setAxisScale( axisProperties->plotAxisType(), min, max );
    }

    plotWidget()->setAxisInverted( axisProperties->plotAxisType(), axisProperties->isAxisInverted() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomForTimeAxis( RimSummaryTimeAxisProperties* timeAxisProperties )
{
    if ( !timeAxisProperties || !plotWidget() ) return;

    if ( timeAxisProperties->isAutoZoom() )
    {
        plotWidget()->setAxisAutoScale( timeAxisProperties->plotAxisType(), true );
    }
    else
    {
        double min = timeAxisProperties->visibleRangeMin();
        double max = timeAxisProperties->visibleRangeMax();
        plotWidget()->setAxisScale( timeAxisProperties->plotAxisType(), min, max );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::createAndSetCustomTimeAxisTickmarks( RimSummaryTimeAxisProperties* timeAxisProperties )
{
    if ( !timeAxisProperties || !plotWidget() ) return;

    const auto [minValue, maxValue] = plotWidget()->axisRange( RiuPlotAxis::defaultBottom() );
    const auto tickmarkList = timeAxisProperties->createTickmarkList( QwtDate::toDateTime( minValue ), QwtDate::toDateTime( maxValue ) );

    plotWidget()->setMajorTicksList( RiuPlotAxis::defaultBottom(), tickmarkList, minValue, maxValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::overrideTimeAxisSettingsIfTooManyCustomTickmarks( RimSummaryTimeAxisProperties* timeAxisProperties, bool showMessageBox )
{
    if ( !timeAxisProperties || timeAxisProperties->tickmarkType() != RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_CUSTOM )
    {
        return;
    }

    const uint MAX_NUM_TICKS = 100;

    // Prevent too large number of ticks by overriding time axis interval and step to
    // prevent large number of tickmarks by accident.
    const auto [minValue, maxValue] = plotWidget()->axisRange( RiuPlotAxis::defaultBottom() );
    const double ticksInterval      = timeAxisProperties->getTickmarkIntervalDouble();
    const uint   numTicks           = static_cast<uint>( std::ceil( ( maxValue - minValue ) / ticksInterval ) );
    if ( numTicks > MAX_NUM_TICKS )
    {
        if ( showMessageBox )
        {
            QString errorTitle   = "Too many custom tickmarks";
            QString errorMessage = QString( "The current configuration generates more than %1 number of custom tickmarks. "
                                            "To prevent slow response the configuration will be reset to default: \n\n"
                                            "Interval = Years and Interval Step = 1." )
                                       .arg( MAX_NUM_TICKS );
            RiaLogging::errorInMessageBox( RiuPlotMainWindow::instance(), errorTitle, errorMessage );
        }

        timeAxisProperties->setTickmarkInterval( RimSummaryTimeAxisProperties::TickmarkInterval::YEARS );
        timeAxisProperties->setTickmarkIntervalStep( 1 );
        timeAxisProperties->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isOnlyWaterCutCurvesVisible( RiuPlotAxis plotAxis )
{
    auto curves = visibleSummaryCurvesForAxis( plotAxis );
    if ( curves.empty() ) return false;

    size_t waterCutCurveCount = 0;
    for ( auto c : curves )
    {
        auto quantityName = c->summaryAddressY().vectorName();

        if ( RiaStdStringTools::endsWith( quantityName, "WCT" ) ) waterCutCurveCount++;
    }

    return ( waterCutCurveCount == curves.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::scheduleReplotIfVisible()
{
    if ( showWindow() && plotWidget() ) plotWidget()->scheduleReplot();
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
    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
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

    axisChanged.send( this );
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
        plotWidget()->ensureAxisIsCreated( curve->axisY() );

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
                            if ( curveSet->colorMode() == RimEnsembleCurveSet::ColorMode::BY_ENSEMBLE_PARAM && plotWidget() &&
                                 curveSet->legendFrame() )
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

    curvesChanged.send();
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
    return m_gridTimeHistoryCurves.children();
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
    if ( m_description().isEmpty() )
    {
        return &m_fallbackPlotName;
    }
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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

        titleChanged.send();
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
            curveSet->deletePlotCurves();
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
        scheduleReplotIfVisible();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::updateStackedCurveDataForRelevantAxes()
{
    bool anyStackedCurvesPresent = false;
    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
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
    if ( !m_isValid ) return;

    bool isPlotEditor = ( uiConfigName == RicSummaryPlotEditorUi::CONFIGURATION_NAME );

    if ( !isPlotEditor ) uiTreeOrdering.add( &m_axisPropertiesArray );

    for ( auto& curve : m_summaryCurveCollection->curves() )
    {
        uiTreeOrdering.add( curve );
    }

    if ( !m_isCrossPlot )
    {
        for ( auto& curveSet : m_ensembleCurveSetCollection->curveSets() )
        {
            uiTreeOrdering.add( curveSet );
        }
    }

    if ( !isPlotEditor )
    {
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

    RimMultiPlot* plotWindow = nullptr;
    firstAncestorOrThisOfType( plotWindow );
    if ( plotWindow == nullptr ) updateMdiWindowVisibility();

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
    if ( !plotWidget() ) return;

    for ( const auto& axisProperty : m_axisPropertiesArray )
    {
        updateZoomForAxis( axisProperty );
    }

    plotWidget()->updateAxes();
    updateZoomFromParentPlot();
    plotWidget()->updateZoomDependentCurveProperties();

    // Must create and set new custom tickmarks for time axis after zoom update
    auto* timeAxisProps = timeAxisProperties();
    if ( timeAxisProps && timeAxisProps->tickmarkType() == RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_CUSTOM )
    {
        // Protection against too many custom tickmarks
        const bool showErrorMessageBox = false;
        overrideTimeAxisSettingsIfTooManyCustomTickmarks( timeAxisProps, showErrorMessageBox );

        // Create and set tickmarks based on settings
        createAndSetCustomTimeAxisTickmarks( timeAxisProps );
    }

    scheduleReplotIfVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomFromParentPlot()
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        if ( !axisProperties ) continue;

        auto [axisMin, axisMax] = plotWidget()->axisRange( axisProperties->plotAxisType() );
        if ( axisProperties->isAxisInverted() ) std::swap( axisMin, axisMax );

        if ( auto propertyAxis = dynamic_cast<RimPlotAxisProperties*>( axisProperties ) )
        {
            propertyAxis->setAutoValueVisibleRangeMax( axisMax );
            propertyAxis->setAutoValueVisibleRangeMin( axisMin );
        }

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
    scheduleReplotIfVisible();
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
    axisChanged.send( this );
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::timeAxisSettingsChanged( const caf::SignalEmitter* emitter )
{
    axisChanged.send( this );

    auto* timeAxisProps = timeAxisProperties();
    if ( !timeAxisProps )
    {
        return;
    }

    if ( !timeAxisProps->isAutoZoom() && plotWidget() )
    {
        // If not auto zoom - the new date and time ranges must be set and axes updated
        plotWidget()->setAxisScale( RiuPlotAxis::defaultBottom(), timeAxisProps->visibleRangeMin(), timeAxisProps->visibleRangeMax() );
        plotWidget()->updateAxes();
    }

    // Protection against too many custom tickmarks
    const bool showErrorMessageBox = true;
    overrideTimeAxisSettingsIfTooManyCustomTickmarks( timeAxisProps, showErrorMessageBox );

    updateTimeAxis( timeAxisProps );
    scheduleReplotIfVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::timeAxisSettingsChangedReloadRequired( const caf::SignalEmitter* emitter )
{
    axisChangedReloadRequired.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    axisChanged.send( this );
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
    axisProperties->setNameAndAxis( name, name, plotAxis.axis(), plotAxis.index() );
    m_axisPropertiesArray.push_back( axisProperties );
    connectAxisSignals( axisProperties );

    return axisProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RimSummaryPlot::visibleCurvesForLegend()
{
    std::vector<RimPlotCurve*> curves;

    for ( auto c : summaryCurves() )
    {
        if ( !c->isCurveVisible() ) continue;
        if ( !c->showInLegend() ) continue;
        curves.push_back( c );
    }

    for ( auto curveSet : curveSets() )
    {
        if ( !curveSet->isCurvesVisible() ) continue;
        if ( curveSet->colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR )
        {
            auto curveSetCurves = curveSet->curves();

            if ( !curveSetCurves.empty() ) curves.push_back( curveSetCurves.front() );
        }
    }

    return curves;
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
        axisProperties->setNameAndAxis( axisProperties->objectName(),
                                        axisProperties->axisTitleText(),
                                        fixedUpPlotAxis.axis(),
                                        fixedUpPlotAxis.index() );

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
void RimSummaryPlot::deleteAllGridTimeHistoryCurves()
{
    m_gridTimeHistoryCurves.deleteChildren();
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
std::pair<int, std::vector<RimSummaryCurve*>> RimSummaryPlot::handleSummaryCaseDrop( RimSummaryCase* summaryCase )
{
    int                           newCurves = 0;
    std::vector<RimSummaryCurve*> curves;

    std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>> dataVectorMap;

    for ( auto& curve : summaryCurves() )
    {
        const auto addr = curve->summaryAddressY();
        dataVectorMap[addr].insert( curve->summaryCaseY() );
    }

    for ( const auto& [addr, cases] : dataVectorMap )
    {
        if ( cases.count( summaryCase ) > 0 ) continue;

        curves.push_back( addNewCurveY( addr, summaryCase ) );
        newCurves++;
    }

    return { newCurves, curves };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, std::vector<RimEnsembleCurveSet*>> RimSummaryPlot::handleEnsembleDrop( RimSummaryCaseCollection* ensemble )
{
    int                               newCurves = 0;
    std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

    std::map<RifEclipseSummaryAddress, std::set<RimSummaryCaseCollection*>> dataVectorMap;

    for ( auto& curve : curveSets() )
    {
        const auto addr = curve->summaryAddress();
        dataVectorMap[addr].insert( curve->summaryCaseCollection() );
    }

    for ( const auto& [addr, ensembles] : dataVectorMap )
    {
        if ( ensembles.count( ensemble ) > 0 ) continue;

        auto curveSet = addNewEnsembleCurveY( addr, ensemble );
        curveSetsToUpdate.push_back( curveSet );
        newCurves++;
    }

    return { newCurves, curveSetsToUpdate };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, std::vector<RimSummaryCurve*>> RimSummaryPlot::handleAddressCollectionDrop( RimSummaryAddressCollection* addressCollection )
{
    int                           newCurves = 0;
    std::vector<RimSummaryCurve*> curves;

    auto droppedName = addressCollection->name().toStdString();

    auto summaryCase  = RiaSummaryTools::summaryCaseById( addressCollection->caseId() );
    auto ensembleCase = RiaSummaryTools::ensembleById( addressCollection->ensembleId() );

    std::vector<RiaSummaryCurveDefinition>                     sourceCurveDefs;
    std::map<RiaSummaryCurveDefinition, std::set<std::string>> newCurveDefsWithObjectNames;

    if ( summaryCase && !ensembleCase )
    {
        for ( auto& curve : summaryCurves() )
        {
            sourceCurveDefs.push_back( curve->curveDefinitionY() );
        }
    }

    if ( ensembleCase )
    {
        auto curveSets = m_ensembleCurveSetCollection->curveSets();
        for ( auto curveSet : curveSets )
        {
            sourceCurveDefs.push_back( RiaSummaryCurveDefinition( ensembleCase, curveSet->summaryAddress() ) );
        }
    }

    for ( auto& curveDef : sourceCurveDefs )
    {
        auto        newCurveDef = curveDef;
        auto        curveAdr    = newCurveDef.summaryAddress();
        std::string objectIdentifierString;
        if ( ( curveAdr.category() == RifEclipseSummaryAddress::SUMMARY_WELL ) &&
             ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL ) )
        {
            objectIdentifierString = curveAdr.wellName();
            curveAdr.setWellName( droppedName );
            newCurveDef.setSummaryAddress( curveAdr );
        }
        else if ( ( curveAdr.category() == RifEclipseSummaryAddress::SUMMARY_GROUP ) &&
                  ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::GROUP ) )
        {
            objectIdentifierString = curveAdr.groupName();
            curveAdr.setGroupName( droppedName );
            newCurveDef.setSummaryAddress( curveAdr );
        }
        else if ( ( curveAdr.category() == RifEclipseSummaryAddress::SUMMARY_REGION ) &&
                  ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION ) )
        {
            objectIdentifierString = std::to_string( curveAdr.regionNumber() );

            int droppedRegion = std::stoi( droppedName );

            curveAdr.setRegion( droppedRegion );
            newCurveDef.setSummaryAddress( curveAdr );
        }

        if ( !objectIdentifierString.empty() )
        {
            newCurveDefsWithObjectNames[newCurveDef].insert( objectIdentifierString );
            const auto& addr = curveDef.summaryAddress();
            if ( !addr.isHistoryVector() && RiaPreferencesSummary::current()->appendHistoryVectors() )
            {
                auto historyAddr = addr;
                historyAddr.setVectorName( addr.vectorName() + RifReaderEclipseSummary::historyIdentifier() );

                auto historyCurveDef = newCurveDef;
                historyCurveDef.setSummaryAddress( historyAddr );
                newCurveDefsWithObjectNames[historyCurveDef].insert( objectIdentifierString );
            }
        }
    }

    for ( auto& [curveDef, objectNames] : newCurveDefsWithObjectNames )
    {
        // Skip adding new curves if the object name is already present for the curve definition
        if ( objectNames.count( droppedName ) > 0 ) continue;

        if ( curveDef.ensemble() )
        {
            auto addresses = curveDef.ensemble()->ensembleSummaryAddresses();
            if ( addresses.find( curveDef.summaryAddress() ) != addresses.end() )
            {
                addNewEnsembleCurveY( curveDef.summaryAddress(), curveDef.ensemble() );
                newCurves++;
            }
        }
        else if ( curveDef.summaryCase() )
        {
            if ( curveDef.summaryCase()->summaryReader() && curveDef.summaryCase()->summaryReader()->hasAddress( curveDef.summaryAddress() ) )
            {
                curves.push_back( addNewCurveY( curveDef.summaryAddress(), curveDef.summaryCase() ) );
                newCurves++;
            }
        }
    }

    return { newCurves, curves };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, std::vector<RimSummaryCurve*>> RimSummaryPlot::handleSummaryAddressDrop( RimSummaryAddress* summaryAddr )
{
    int                           newCurves = 0;
    std::vector<RimSummaryCurve*> curves;

    std::vector<RifEclipseSummaryAddress> newCurveAddresses;
    newCurveAddresses.push_back( summaryAddr->address() );
    if ( !summaryAddr->address().isHistoryVector() && RiaPreferencesSummary::current()->appendHistoryVectors() )
    {
        auto historyAddr = summaryAddr->address();
        historyAddr.setVectorName( summaryAddr->address().vectorName() + RifReaderEclipseSummary::historyIdentifier() );
        newCurveAddresses.push_back( historyAddr );
    }

    if ( summaryAddr->isEnsemble() )
    {
        std::map<RifEclipseSummaryAddress, std::set<RimSummaryCaseCollection*>> dataVectorMap;

        for ( auto& curve : curveSets() )
        {
            const auto addr = curve->summaryAddress();
            dataVectorMap[addr].insert( curve->summaryCaseCollection() );
        }

        auto ensemble = RiaSummaryTools::ensembleById( summaryAddr->ensembleId() );
        if ( ensemble )
        {
            for ( const auto& droppedAddress : newCurveAddresses )
            {
                auto addresses = ensemble->ensembleSummaryAddresses();
                if ( addresses.find( droppedAddress ) == addresses.end() ) continue;

                bool skipAddress = false;
                if ( dataVectorMap.count( droppedAddress ) > 0 )
                {
                    skipAddress = ( dataVectorMap[droppedAddress].count( ensemble ) > 0 );
                }

                if ( !skipAddress )
                {
                    addNewEnsembleCurveY( droppedAddress, ensemble );
                    newCurves++;
                }
            }
        }
    }
    else
    {
        std::map<RifEclipseSummaryAddress, std::set<RimSummaryCase*>> dataVectorMap;

        for ( auto& curve : summaryCurves() )
        {
            const auto addr = curve->summaryAddressY();
            dataVectorMap[addr].insert( curve->summaryCaseY() );
        }

        auto summaryCase = RiaSummaryTools::summaryCaseById( summaryAddr->caseId() );
        if ( summaryCase )
        {
            for ( const auto& droppedAddress : newCurveAddresses )
            {
                if ( !summaryCase->summaryReader() || !summaryCase->summaryReader()->hasAddress( droppedAddress ) ) continue;

                bool skipAddress = false;

                if ( dataVectorMap.count( droppedAddress ) > 0 )
                {
                    skipAddress = ( dataVectorMap[droppedAddress].count( summaryCase ) > 0 );
                }

                if ( !skipAddress )
                {
                    curves.push_back( addNewCurveY( droppedAddress, summaryCase ) );
                    newCurves++;
                }
            }
        }
    }
    return { newCurves, curves };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects )
{
    int                               accumulatedCurveCount = 0;
    std::vector<RimSummaryCurve*>     curvesToUpdate;
    std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

    for ( auto obj : objects )
    {
        auto summaryCase = dynamic_cast<RimSummaryCase*>( obj );
        if ( summaryCase )
        {
            auto [curveCount, curvesCreated] = handleSummaryCaseDrop( summaryCase );
            accumulatedCurveCount += curveCount;
            curvesToUpdate.insert( curvesToUpdate.end(), curvesCreated.begin(), curvesCreated.end() );
            continue;
        }
        auto ensemble = dynamic_cast<RimSummaryCaseCollection*>( obj );
        if ( ensemble )
        {
            auto [curveCount, curvesCreated] = handleEnsembleDrop( ensemble );
            accumulatedCurveCount += curveCount;
            curveSetsToUpdate.insert( curveSetsToUpdate.end(), curvesCreated.begin(), curvesCreated.end() );
            continue;
        }

        auto summaryAddr = dynamic_cast<RimSummaryAddress*>( obj );
        if ( summaryAddr )
        {
            auto [curveCount, curvesCreated] = handleSummaryAddressDrop( summaryAddr );
            accumulatedCurveCount += curveCount;
            curvesToUpdate.insert( curvesToUpdate.end(), curvesCreated.begin(), curvesCreated.end() );
            continue;
        }

        auto addressCollection = dynamic_cast<RimSummaryAddressCollection*>( obj );
        if ( addressCollection )
        {
            if ( addressCollection->isFolder() )
            {
                for ( auto coll : addressCollection->subFolders() )
                {
                    auto [curveCount, curvesCreated] = handleAddressCollectionDrop( coll );
                    accumulatedCurveCount += curveCount;
                    curvesToUpdate.insert( curvesToUpdate.end(), curvesCreated.begin(), curvesCreated.end() );
                }
                continue;
            }
            else
            {
                auto [curveCount, curvesCreated] = handleAddressCollectionDrop( addressCollection );
                accumulatedCurveCount += curveCount;
                curvesToUpdate.insert( curvesToUpdate.end(), curvesCreated.begin(), curvesCreated.end() );
                continue;
            }
        }
    }

    if ( accumulatedCurveCount > 0 )
    {
        applyDefaultCurveAppearances( curvesToUpdate );
        applyDefaultCurveAppearances( curveSetsToUpdate );

        loadDataAndUpdate();

        curvesChanged.send();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryPlot::addNewCurveY( const RifEclipseSummaryAddress& address, RimSummaryCase* summaryCase )
{
    auto* newCurve = new RimSummaryCurve();
    newCurve->setSummaryCaseY( summaryCase );
    newCurve->setSummaryAddressYAndApplyInterpolation( address );
    addCurveNoUpdate( newCurve );

    return newCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimSummaryPlot::addNewEnsembleCurveY( const RifEclipseSummaryAddress& address, RimSummaryCaseCollection* ensemble )
{
    auto* curveSet = new RimEnsembleCurveSet();

    curveSet->setSummaryCaseCollection( ensemble );
    curveSet->setSummaryAddressAndStatisticsFlag( address );

    cvf::Color3f curveColor =
        RimSummaryCurveAppearanceCalculator::computeTintedCurveColorForAddress( curveSet->summaryAddress(),
                                                                                static_cast<int>(
                                                                                    ensembleCurveSetCollection()->curveSetCount() ) );

    auto adr = curveSet->summaryAddress();
    if ( adr.isHistoryVector() ) curveColor = RiaPreferencesSummary::current()->historyCurveContrastColor();

    curveSet->setColor( curveColor );

    ensembleCurveSetCollection()->addCurveSet( curveSet );

    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onPlotZoomed()
{
    // Disable auto scale in plot engine
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );

    // Disable auto value for min/max fields
    for ( auto p : plotYAxes() )
    {
        p->enableAutoValueMinMax( false );
    }

    plotZoomedByUser.send();

    updateZoomFromParentPlot();

    timeAxisSettingsChanged( nullptr );

    axisChanged.send( this );
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
        mainOptions->add( &m_colSpan );
    }
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle );

    mainOptions->add( &m_normalizeCurveYValues );

    if ( isMdiWindow() )
    {
        RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );
    }

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

        // Disable all use of QtCharts for now. If a plot was created using QtCharts during the period this flag was
        // active, the use of QtCharts was stored in the project file or template file. Set flag to false to force use
        // of Qwt
        useQtCharts = false;

        if ( useQtCharts )
        {
            m_summaryPlot = std::make_unique<RiuSummaryQtChartsPlot>( this, mainWindowParent );
        }
        else
        {
            m_summaryPlot = std::make_unique<RiuSummaryQwtPlot>( this, mainWindowParent );
        }
#else
        m_summaryPlot = std::make_unique<RiuSummaryQwtPlot>( this, mainWindowParent );
#endif

        QObject::connect( plotWidget(), SIGNAL( curveOrderNeedsUpdate() ), this, SLOT( onUpdateCurveOrder() ) );

        for ( const auto& axisProperties : m_axisPropertiesArray )
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
            m_summaryCurveCollection->setParentPlotNoReplot( plotWidget() );
        }

        if ( m_ensembleCurveSetCollection )
        {
            m_ensembleCurveSetCollection->setParentPlotNoReplot( plotWidget() );
        }

        this->connect( plotWidget(), SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        updatePlotTitle();
    }

    plotWidget()->setParent( mainWindowParent );

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
                    plotAxisProperties->setNameAndAxis( axisProperties->objectName(), axisProperties->axisTitleText(), axis.axis(), 0 );
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

    for ( const auto& axisProperties : m_axisPropertiesArray )
    {
        auto plotAxisProperties = dynamic_cast<RimPlotAxisProperties*>( axisProperties.p() );
        if ( plotAxisProperties )
        {
            connectAxisSignals( plotAxisProperties );
        }
        auto* timeAxis = dynamic_cast<RimSummaryTimeAxisProperties*>( axisProperties.p() );
        if ( timeAxis )
        {
            timeAxis->settingsChanged.connect( this, &RimSummaryPlot::timeAxisSettingsChanged );
            timeAxis->requestLoadDataAndUpdate.connect( this, &RimSummaryPlot::timeAxisSettingsChangedReloadRequired );
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
            if ( curve->summaryAddressY().isCalculated() )
            {
                RiaSummaryTools::getSummaryCasesAndAddressesForCalculation( curve->summaryAddressY().id(), sumCases, addresses );
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
            if ( c->isCurveVisible() )
            {
                c->updateCurveNameNoLegendUpdate();
            }
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
    curvesChanged.send();

    updateStackedCurveData();
    scheduleReplotIfVisible();

    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex )
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
    for ( const auto& ap : m_axisPropertiesArray )
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
    for ( const auto& ap : m_axisPropertiesArray )
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
    for ( const auto& ap : m_axisPropertiesArray )
    {
        axisProps.push_back( ap );
    }

    return axisProps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisProperties*> RimSummaryPlot::plotYAxes() const
{
    std::vector<RimPlotAxisProperties*> axisProps;
    for ( const auto& ap : m_axisPropertiesArray )
    {
        auto plotAxisProp = dynamic_cast<RimPlotAxisProperties*>( ap.p() );
        if ( plotAxisProp ) axisProps.push_back( plotAxisProp );
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
        USE_MATCHING_UNIT,
        USE_MATCHING_VECTOR
    };

    auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;

    auto destinationUnit = RiaStdStringTools::toUpper( destinationCurve->unitNameY() );
    if ( destinationUnit.empty() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

    auto anyCurveWithUnitText = [this, destinationCurve] {
        for ( auto c : summaryCurves() )
        {
            if ( c == destinationCurve ) continue;

            if ( !c->unitNameY().empty() ) return true;
        }

        return false;
    };

    if ( !anyCurveWithUnitText() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

    if ( strategy == AxisAssignmentStrategy::USE_MATCHING_VECTOR )
    {
        // Special handling if curve unit is matching. Try to match on summary vector name to avoid creation of new axis

        for ( auto c : summaryCurves() )
        {
            if ( c == destinationCurve ) continue;

            if ( c->summaryAddressY().vectorName() == destinationCurve->summaryAddressY().vectorName() )
            {
                destinationCurve->setLeftOrRightAxisY( c->axisY() );
                return;
            }
        }
    }
    else if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
    {
        bool isLeftUsed  = false;
        bool isRightUsed = false;

        for ( auto c : summaryCurves() )
        {
            if ( c == destinationCurve ) continue;

            if ( c->axisY() == RiuPlotAxis::defaultLeft() ) isLeftUsed = true;
            if ( c->axisY() == RiuPlotAxis::defaultRight() ) isRightUsed = true;

            auto currentUnit = RiaStdStringTools::toUpper( c->unitNameY() );

            if ( currentUnit == destinationUnit )
            {
                for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
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

    RiaDefines::PlotAxis plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;

    if ( strategy == AxisAssignmentStrategy::ALTERNATING )
    {
        size_t axisCountLeft  = 0;
        size_t axisCountRight = 0;
        for ( const auto& ap : m_axisPropertiesArray )
        {
            if ( ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
                axisCountLeft++;
            else if ( ap->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
                axisCountRight++;
        }

        if ( axisCountLeft > axisCountRight ) plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    }
    else if ( strategy == AxisAssignmentStrategy::ALL_TO_LEFT )
    {
        plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    }
    else if ( strategy == AxisAssignmentStrategy::ALL_TO_RIGHT )
    {
        plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    }

    RiuPlotAxis newPlotAxis = RiuPlotAxis::defaultLeft();
    if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
    {
        QString axisObjectName = "New Axis";
        if ( !destinationCurve->summaryAddressY().uiText().empty() )
            axisObjectName = QString::fromStdString( destinationCurve->summaryAddressY().uiText() );

        newPlotAxis = plotWidget()->createNextPlotAxis( plotAxisType );
        addNewAxisProperties( newPlotAxis, axisObjectName );
    }

    destinationCurve->setLeftOrRightAxisY( newPlotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    if ( childArray == &m_axisPropertiesArray )
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
            for ( const auto& axisProperties : m_axisPropertiesArray )
            {
                usedPlotAxis.insert( axisProperties->plotAxisType() );
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
RimSummaryPlotSourceStepping* RimSummaryPlot::sourceStepper()
{
    return m_sourceStepping();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onUpdateCurveOrder()
{
    m_summaryCurveCollection->updateCurveOrder();
}
