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
#include "RiaCurveMerger.h"
#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"
#include "RiaPreferencesSummary.h"
#include "RiaRegressionTestRunner.h"
#include "RiaStdStringTools.h"
#include "Summary/RiaSummaryCurveDefinition.h"
#include "Summary/RiaSummaryDefines.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RifEclipseSummaryAddressDefines.h"

#include "SummaryPlotCommands/RicSummaryPlotEditorUi.h"

#include "Annotations/RimTimeAxisAnnotationUpdater.h"
#include "RimAsciiDataCurve.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimMultiPlot.h"
#include "RimPlotAxisLogRangeCalculator.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurvesData.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlotAxisFormatter.h"
#include "RimSummaryPlotControls.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryTimeAxisProperties.h"
#include "Tools/RimPlotAxisTools.h"

#include "RiuPlotAxis.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotWidget.h"
#include "RiuSummaryQwtPlot.h"

#include "cvfColor3.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
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

struct RimSummaryPlot::CurveInfo
{
    int                               curveCount = 0;
    std::vector<RimSummaryCurve*>     curves;
    std::vector<RimEnsembleCurveSet*> curveSets;

    void appendCurveInfo( const CurveInfo& other )
    {
        curveCount += other.curveCount;
        curves.insert( curves.end(), other.curves.begin(), other.curves.end() );
        curveSets.insert( curveSets.end(), other.curveSets.begin(), other.curveSets.end() );
    };
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot()
    : curvesChanged( this )
    , axisChanged( this )
    , titleChanged( this )
    , m_isValid( true )
    , axisChangedReloadRequired( this )
    , autoTitleChanged( this )
    , m_legendPosition( RiuPlotWidget::Legend::BOTTOM )
{
    CAF_PDM_InitScriptableObject( "Summary Plot", ":/SummaryPlotLight16x16.png", "", "A Summary Plot" );

    CAF_PDM_InitScriptableField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title" );
    CAF_PDM_InitScriptableField( &m_description, "PlotDescription", QString( "Summary Plot" ), "Name" );
    CAF_PDM_InitScriptableField( &m_normalizeCurveYValues, "normalizeCurveYValues", false, "Normalize all curves" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCurveCollection, "SummaryCurveCollection", "" );
    m_summaryCurveCollection = new RimSummaryCurveCollection;
    m_summaryCurveCollection->curvesChanged.connect( this, &RimSummaryPlot::onCurveCollectionChanged );

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSetCollection, "EnsembleCurveSetCollection", "" );
    m_ensembleCurveSetCollection = new RimEnsembleCurveSetCollection();

    CAF_PDM_InitFieldNoDefault( &m_gridTimeHistoryCurves, "GridTimeHistoryCurves", "" );

    CAF_PDM_InitFieldNoDefault( &m_asciiDataCurves, "AsciiDataCurves", "" );

    CAF_PDM_InitFieldNoDefault( &m_axisPropertiesArray, "AxisProperties", "Axes", ":/Axes16x16.png" );
    m_axisPropertiesArray.uiCapability()->setUiTreeHidden( false );

    auto leftAxis = addNewAxisProperties( RiuPlotAxis::defaultLeft(), "Left" );
    leftAxis->setAlwaysRequired( true );

    auto rightAxis = addNewAxisProperties( RiuPlotAxis::defaultRight(), "Right" );
    rightAxis->setAlwaysRequired( true );

    m_nameHelperAllCurves = std::make_unique<RimSummaryPlotNameHelper>();

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );
    m_sourceStepping = new RimSummaryPlotSourceStepping;

    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_fallbackPlotName, "AlternateName", "AlternateName" );
    m_fallbackPlotName.uiCapability()->setUiReadOnly( true );
    m_fallbackPlotName.uiCapability()->setUiHidden( true );
    m_fallbackPlotName.xmlCapability()->disableIO();

    setPlotInfoLabel( "Filters Active" );

    // Obsolete axis fields
    CAF_PDM_InitFieldNoDefault( &m_leftYAxisProperties_OBSOLETE, "LeftYAxisProperties", "Left Y Axis" );
    m_leftYAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_leftYAxisProperties_OBSOLETE = new RimPlotAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_rightYAxisProperties_OBSOLETE, "RightYAxisProperties", "Right Y Axis" );
    m_rightYAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_rightYAxisProperties_OBSOLETE = new RimPlotAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_bottomAxisProperties_OBSOLETE, "BottomAxisProperties", "Bottom X Axis" );
    m_bottomAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_bottomAxisProperties_OBSOLETE = new RimPlotAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_timeAxisProperties_OBSOLETE, "TimeAxisProperties", "Time Axis" );
    m_timeAxisProperties_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_timeAxisProperties_OBSOLETE = new RimSummaryTimeAxisProperties;

    ensureRequiredAxisObjectsForCurves();
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

    updateAnnotationsInPlotWidget();

    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_TOP );
    updateTimeAxis( timeAxisProperties() );

    updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAnnotationsInPlotWidget()
{
    if ( m_summaryPlot ) m_summaryPlot->clearAnnotationObjects();

    if ( !plotWidget() ) return;

    if ( timeAxisProperties() )
    {
        m_summaryPlot->updateAnnotationObjects( timeAxisProperties() );
    }

    if ( auto leftYAxisProperties = axisPropertiesForPlotAxis( RiuPlotAxis::defaultLeft() ) )
    {
        m_summaryPlot->updateAnnotationObjects( leftYAxisProperties );
    }
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
    std::vector<RimSummaryCurve*> allCurves = descendantsIncludingThisOfType<RimSummaryCurve>();

    std::vector<RimSummaryCurve*> crossPlotCurves;
    std::vector<RimSummaryCurve*> curves;
    std::vector<RimSummaryCurve*> observedCurves;

    for ( auto c : allCurves )
    {
        if ( c->axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
        {
            crossPlotCurves.push_back( c );
        }
        else if ( c->summaryCaseY() && c->summaryCaseY()->isObservedData() && !c->isRegressionCurve() )
        {
            observedCurves.push_back( c );
        }
        else
        {
            curves.push_back( c );
        }
    }

    auto gridCurves  = m_gridTimeHistoryCurves.childrenByType();
    auto asciiCurves = m_asciiDataCurves.childrenByType();

    QString text;
    text += RimSummaryCurvesData::createTextForExport( curves, asciiCurves, gridCurves, resamplingPeriod, showTimeAsLongString );

    if ( !observedCurves.empty() )
    {
        text += "\n\n------------ Observed Curves --------------";
        text += RimSummaryCurvesData::createTextForExport( observedCurves, {}, {}, RiaDefines::DateTimePeriod::NONE, showTimeAsLongString );
    }

    text += RimSummaryCurvesData::createTextForCrossPlotCurves( crossPlotCurves );

    return text;
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
std::vector<RimSummaryCurve*> RimSummaryPlot::curvesForStepping() const
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
std::vector<RimSummaryCurve*> RimSummaryPlot::allCurves() const
{
    return summaryCurves();
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

    for ( const auto& curve : summaryAndEnsembleCurves() )
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
    for ( auto ap : sourceSummaryPlot.allPlotAxes() )
    {
        QString data = ap->writeObjectToXmlString();

        auto axisProperty = axisPropertiesForPlotAxis( ap->plotAxis() );
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
    for ( auto ap : sourceSummaryPlot.allPlotAxes() )
    {
        if ( ap->plotAxis().axis() != plotAxisType ) continue;

        QString data = ap->writeObjectToXmlString();

        auto axisProperty = axisPropertiesForPlotAxis( ap->plotAxis() );
        if ( axisProperty )
        {
            axisProperty->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::copyMatchingAxisPropertiesFromOther( const RimSummaryPlot& summaryPlot )
{
    for ( auto apToCopy : summaryPlot.allPlotAxes() )
    {
        for ( auto ap : allPlotAxes() )
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
        if ( m_showPlotLegends && !isSubPlot() )
        {
            plotWidget()->insertLegend( m_legendPosition );
        }
        else
        {
            plotWidget()->clearLegend();
        }

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
void RimSummaryPlot::setLegendPosition( RiuPlotWidget::Legend position )
{
    m_legendPosition = position;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setPlotInfoLabel( const QString& label )
{
    auto qwtText = QwtText( label );
    qwtText.setRenderFlags( Qt::AlignTop | Qt::AlignRight );

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
size_t RimSummaryPlot::singleColorCurveCount() const
{
    auto allCurveSets = ensembleCurveSetCollection()->curveSets();

    size_t colorIndex =
        std::count_if( allCurveSets.begin(),
                       allCurveSets.end(),
                       []( RimEnsembleCurveSet* curveSet )
                       { return RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ); } );

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
    std::set<RiaSummaryCurveDefinition> allCurveDefs = summaryAndEnsembleCurveDefinitions();
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
    std::vector<QColor> usedColors;
    for ( auto c : ensembleCurveSetCollection()->curveSets() )
    {
        // ensembleCurvesToUpdate can be present in the ensembleCurveSetCollection()->curveSets() vector, exclude this from used
        // colors
        if ( std::find( ensembleCurvesToUpdate.begin(), ensembleCurvesToUpdate.end(), c ) == ensembleCurvesToUpdate.end() )
        {
            usedColors.push_back( c->mainEnsembleColor() );
        }
    }

    for ( auto curveSet : ensembleCurvesToUpdate )
    {
        cvf::Color3f curveColor = cvf::Color3f::ORANGE;

        const auto adr = curveSet->summaryAddressY();
        if ( adr.isHistoryVector() )
        {
            curveColor = RiaPreferencesSummary::current()->historyCurveContrastColor();
        }
        else
        {
            if ( RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ) )
            {
                std::vector<QColor> candidateColors;
                if ( RiaPreferencesSummary::current()->colorCurvesByPhase() )
                {
                    // Put the the phase color as first candidate, will then be used if there is only one ensemble in the plot
                    candidateColors.push_back( RiaColorTools::toQColor( RimSummaryCurveAppearanceCalculator::assignColorByPhase( adr ) ) );
                }

                auto summaryColors = RiaColorTables::summaryCurveDefaultPaletteColors();
                for ( int i = 0; i < static_cast<int>( summaryColors.size() ); i++ )
                {
                    candidateColors.push_back( summaryColors.cycledQColor( i ) );
                }

                for ( const auto& candidateCol : candidateColors )
                {
                    if ( std::find( usedColors.begin(), usedColors.end(), candidateCol ) == usedColors.end() )
                    {
                        curveColor = RiaColorTools::fromQColorTo3f( candidateCol );
                        usedColors.push_back( candidateCol );
                        break;
                    }
                }
            }
        }

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
        RiuPlotAxis riuPlotAxis = axisProperties->plotAxis();
        if ( riuPlotAxis.axis() == plotAxis )
        {
            auto* axisProps = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
            if ( !axisProps ) continue;

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
                    if ( summaryCurve->axisY() == riuPlotAxis )
                    {
                        curveDefs.emplace_back( summaryCurve->summaryCaseY(), summaryCurve->summaryAddressY(), summaryCurve->isEnsembleCurve() );
                    }
                    if ( summaryCurve->axisX() == riuPlotAxis )
                    {
                        RiaSummaryCurveDefinition def;
                        def.setSummaryCaseX( summaryCurve->summaryCaseX() );
                        def.setSummaryAddressX( summaryCurve->summaryAddressX() );

                        curveDefs.push_back( def );
                    }
                }

                for ( auto curveSet : ensembleCurveSetCollection()->curveSets() )
                {
                    if ( curveSet->axisY() == riuPlotAxis )
                    {
                        RiaSummaryCurveDefinition def( curveSet->summaryEnsemble(), curveSet->summaryAddressY() );
                        curveDefs.push_back( def );
                    }
                    if ( curveSet->axisX() == riuPlotAxis )
                    {
                        RiaSummaryCurveDefinition def;
                        def.setEnsemble( curveSet->summaryEnsemble() );
                        def.setSummaryAddressX( curveSet->curveAddress().summaryAddressX() );

                        curveDefs.push_back( def );
                    }
                }

                RimSummaryPlotAxisFormatter calc( axisProps, {}, curveDefs, visibleAsciiDataCurvesForAxis( riuPlotAxis ), timeHistoryQuantities );

                calc.applyAxisPropertiesToPlot( plotWidget() );
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
    if ( !plotWidget() || !timeAxisProperties ) return;

    bool anyTimeHistoryCurvePresent = false;
    {
        if ( !visibleTimeHistoryCurvesForAxis( RimSummaryPlot::plotAxisForTime() ).empty() ) anyTimeHistoryCurvePresent = true;
        if ( !visibleAsciiDataCurvesForAxis( RimSummaryPlot::plotAxisForTime() ).empty() ) anyTimeHistoryCurvePresent = true;
        if ( !visibleSummaryCurvesForAxis( RimSummaryPlot::plotAxisForTime() ).empty() ) anyTimeHistoryCurvePresent = true;
    }

    if ( !anyTimeHistoryCurvePresent || !timeAxisProperties->isActive() )
    {
        plotWidget()->enableAxis( RimSummaryPlot::plotAxisForTime(), false );

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

    plotWidget()->enableAxis( RimSummaryPlot::plotAxisForTime(), true );
    plotWidget()->enableAxisNumberLabels( RimSummaryPlot::plotAxisForTime(), timeAxisProperties->showLabels() );

    {
        Qt::AlignmentFlag alignment = Qt::AlignCenter;
        if ( timeAxisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            alignment = Qt::AlignRight;
        }

        plotWidget()->setAxisFontsAndAlignment( RimSummaryPlot::plotAxisForTime(),
                                                timeAxisProperties->titleFontSize(),
                                                timeAxisProperties->valuesFontSize(),
                                                true,
                                                alignment );
        plotWidget()->setAxisTitleText( RimSummaryPlot::plotAxisForTime(), timeAxisProperties->title() );
        plotWidget()->setAxisTitleEnabled( RimSummaryPlot::plotAxisForTime(), timeAxisProperties->showTitle() );

        if ( timeAxisProperties->tickmarkType() == RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_COUNT )
        {
            RimSummaryTimeAxisProperties::LegendTickmarkCount tickmarkCountEnum = timeAxisProperties->majorTickmarkCount();
            int maxTickmarkCount = RimPlotAxisPropertiesInterface::tickmarkCountFromEnum( tickmarkCountEnum );

            plotWidget()->setAxisMaxMajor( RimSummaryPlot::plotAxisForTime(), maxTickmarkCount );
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
    if ( auto timeAxisProps = dynamic_cast<RimSummaryTimeAxisProperties*>( axisProperties ) )
    {
        updateZoomForTimeAxis( timeAxisProps );
        return;
    }

    if ( auto axisProps = dynamic_cast<RimPlotAxisProperties*>( axisProperties ) )
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

    const auto plotAxis = axisProperties->plotAxis();
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

            plotWidget()->setAxisScale( axisProperties->plotAxis(), min, max );
        }
        else if ( ( plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT ) &&
                  isOnlyWaterCutCurvesVisible( plotAxis ) )
        {
            plotWidget()->setAxisScale( axisProperties->plotAxis(), 0.0, 1.0 );
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
void RimSummaryPlot::updateZoomForTimeAxis( RimSummaryTimeAxisProperties* timeAxisProperties )
{
    if ( !timeAxisProperties || !plotWidget() ) return;

    if ( timeAxisProperties->isAutoZoom() )
    {
        plotWidget()->setAxisAutoScale( timeAxisProperties->plotAxis(), true );
    }
    else
    {
        double min = timeAxisProperties->visibleRangeMin();
        double max = timeAxisProperties->visibleRangeMax();
        plotWidget()->setAxisScale( timeAxisProperties->plotAxis(), min, max );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::createAndSetCustomTimeAxisTickmarks( RimSummaryTimeAxisProperties* timeAxisProperties )
{
    if ( !timeAxisProperties || !plotWidget() ) return;

    const auto [minValue, maxValue] = plotWidget()->axisRange( RimSummaryPlot::plotAxisForTime() );
    const auto tickmarkList = timeAxisProperties->createTickmarkList( QwtDate::toDateTime( minValue ), QwtDate::toDateTime( maxValue ) );

    plotWidget()->setMajorTicksList( RimSummaryPlot::plotAxisForTime(), tickmarkList, minValue, maxValue );
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
    const auto [minValue, maxValue] = plotWidget()->axisRange( RimSummaryPlot::plotAxisForTime() );
    const double ticksInterval      = timeAxisProperties->getTickmarkIntervalDouble();
    const auto   numTicks           = static_cast<uint>( std::ceil( ( maxValue - minValue ) / ticksInterval ) );
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

    for ( const auto& c : curves )
    {
        auto quantityName = c->summaryAddressY().vectorName();
        if ( !RiaStdStringTools::endsWith( quantityName, "WCT" ) ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::enableCurvePointTracking( bool enable )
{
    if ( m_summaryPlot ) m_summaryPlot->enableCurvePointTracking( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::any RimSummaryPlot::valueForKey( std::string key ) const
{
    if ( key == "TimeStampZoomOperation" )
    {
        return m_lastZoomTime;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimSummaryPlot::plotAxisForTime()
{
    return RiuPlotAxis::defaultBottom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::ensureRequiredAxisObjectsForCurves()
{
    // Always make sure time axis properties are present
    if ( !timeAxisProperties() )
    {
        auto* axisProperties = new RimSummaryTimeAxisProperties;
        axisProperties->settingsChanged.connect( this, &RimSummaryPlot::timeAxisSettingsChanged );
        axisProperties->requestLoadDataAndUpdate.connect( this, &RimSummaryPlot::timeAxisSettingsChangedReloadRequired );

        m_axisPropertiesArray.push_back( axisProperties );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::findOrAssignPlotAxisX( RimSummaryCurve* curve )
{
    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        {
            auto propertyAxis = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
            if ( propertyAxis )
            {
                curve->setTopOrBottomAxisX( propertyAxis->plotAxis() );

                return;
            }
        }
    }

    if ( curve->summaryCaseX() != nullptr )
    {
        if ( !plotWidget() )
        {
            // Assign a default bottom axis if no plot widget is present. This can happens during project load and transformation to new
            // cross plot structure in RimMainPlotCollection::initAfterRead()

            QString axisObjectName = "New Axis";
            if ( !curve->summaryAddressX().uiText().empty() ) axisObjectName = QString::fromStdString( curve->summaryAddressX().uiText() );

            RiuPlotAxis newPlotAxis = RiuPlotAxis::defaultBottomForSummaryVectors();
            addNewAxisProperties( newPlotAxis, axisObjectName );

            curve->setTopOrBottomAxisX( newPlotAxis );

            return;
        }

        if ( plotWidget()->isMultiAxisSupported() )
        {
            QString axisObjectName = "New Axis";
            if ( !curve->summaryAddressX().uiText().empty() ) axisObjectName = QString::fromStdString( curve->summaryAddressX().uiText() );

            RiuPlotAxis newPlotAxis = plotWidget()->createNextPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
            addNewAxisProperties( newPlotAxis, axisObjectName );
            if ( plotWidget() )
            {
                plotWidget()->ensureAxisIsCreated( newPlotAxis );
            }

            updateAxes();
            curve->setTopOrBottomAxisX( newPlotAxis );
        }
    }
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

    if ( m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible() )
    {
        for ( RimSummaryCurve* curve : m_summaryCurveCollection->curves() )
        {
            if ( curve->isChecked() && ( curve->axisY() == plotAxis || curve->axisX() == plotAxis ) )
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
                if ( curve->isChecked() && ( curve->axisY() == plotAxis || curve->axisX() == plotAxis ) )
                {
                    curves.push_back( curve );
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
        if ( axisProperties->plotAxis() == plotAxis ) return axisProperties;
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
        if ( c->isChecked() )
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
        if ( c->isChecked() )
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
void RimSummaryPlot::updateAndRedrawTimeAnnotations()
{
    RimTimeAxisAnnotationUpdater::updateTimeAnnotationObjects( this );

    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updatePlotWidgetFromAxisRanges();

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
void RimSummaryPlot::removeCurve( RimSummaryCurve* curve )
{
    m_summaryCurveCollection->removeCurve( curve );
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
                        curveSet->deleteRealizationCurve( curve );
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
    connectCurveSignals( curve );

    if ( plotWidget() )
    {
        curve->setParentPlotAndReplot( plotWidget() );
        updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridTimeHistoryCurve*> RimSummaryPlot::gridTimeHistoryCurves() const
{
    return m_gridTimeHistoryCurves.childrenByType();
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
void RimSummaryPlot::zoomAllForMultiPlot()
{
    if ( auto multiPlot = firstAncestorOrThisOfType<RimMultiPlot>() )
    {
        multiPlot->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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

            for ( auto c : summaryCurves() )
            {
                c->updateCurveNameNoLegendUpdate();
            }
        }

        titleChanged.send();
    }

    if ( changedField == &m_showPlotLegends ) updateLegend();

    if ( changedField == &m_normalizeCurveYValues )
    {
        loadDataAndUpdate();
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
        if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
             axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        {
            anyStackedCurvesPresent |= updateStackedCurveDataForAxis( axisProperties->plotAxis() );
        }
    }

    return anyStackedCurvesPresent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::updateStackedCurveDataForAxis( RiuPlotAxis plotAxis )
{
    // See RimWellLogTrack::updateStackedCurveData() for vertical plots

    struct CurveStackingItem
    {
        RimStackablePlotCurve* curve;
        RiaDefines::PhaseType  phase;
        std::vector<double>    xValues;
        std::vector<double>    yValues;
    };

    std::vector<CurveStackingItem> stackingItems;

    for ( auto summaryCurve : visibleSummaryCurvesForAxis( plotAxis ) )
    {
        if ( summaryCurve->isStacked() )
        {
            auto xValues = RiuPlotCurve::fromTime_t( summaryCurve->timeStepsY() );
            auto yValues = summaryCurve->valuesY();

            if ( !xValues.empty() && xValues.size() == yValues.size() )
            {
                stackingItems.push_back( { summaryCurve, summaryCurve->phaseType(), xValues, yValues } );
            }
        }
    }

    // Add stack data for time history curves
    for ( auto gridCurve : gridTimeHistoryCurves() )
    {
        if ( gridCurve->yAxis() == plotAxis && gridCurve->isStacked() && gridCurve->isChecked() )
        {
            auto xValues = RiuPlotCurve::fromTime_t( gridCurve->timeStepValues() );
            auto yValues = gridCurve->yValues();

            if ( !xValues.empty() && xValues.size() == yValues.size() )
            {
                stackingItems.push_back( { gridCurve, gridCurve->phaseType(), xValues, yValues } );
            }
        }
    }

    if ( stackingItems.empty() ) return true;

    RiaCurveMerger<double>                  curveMerger( RiaCurveDefines::InterpolationMethod::LINEAR );
    std::map<RiaDefines::PhaseType, size_t> curvePhaseCount;

    for ( auto stackingItem : stackingItems )
    {
        curveMerger.addCurveData( stackingItem.xValues, stackingItem.yValues );
        curvePhaseCount[stackingItem.phase]++;
    }
    curveMerger.computeInterpolatedValues( true );

    std::vector<double> allXValues = curveMerger.allXValues();
    std::vector<double> allStackedValues( allXValues.size(), 0.0 );

    // Z-position of curve, to draw them in correct order
    double zPos = -10000.0;

    for ( size_t index = 0; index < stackingItems.size(); index++ )
    {
        auto yValues = curveMerger.interpolatedYValuesForAllXValues( index );
        for ( size_t i = 0; i < allXValues.size(); ++i )
        {
            double value = yValues[i];
            if ( value != std::numeric_limits<double>::infinity() )
            {
                allStackedValues[i] += value;
            }
        }

        auto curve = stackingItems[index].curve;
        curve->setSamplesFromXYValues( allXValues, allStackedValues, false );
        curve->setZOrder( zPos );
        if ( curve->isStackedWithPhaseColors() )
        {
            curve->assignStackColor( index, curvePhaseCount[curve->phaseType()] );
        }
        zPos -= 1.0;
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

    for ( auto& curveSet : m_ensembleCurveSetCollection->curveSets() )
    {
        uiTreeOrdering.add( curveSet );
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

    auto plotWindow = firstAncestorOrThisOfType<RimMultiPlot>();
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

    // Load data for regression curves, as they depend on data loaded by curves updated previously in this function
    if ( m_summaryCurveCollection )
    {
        auto curves = m_summaryCurveCollection->curves();
        for ( auto c : curves )
        {
            if ( c->isRegressionCurve() )
            {
                c->loadDataAndUpdate( false );
            }
        }
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
void RimSummaryPlot::updatePlotWidgetFromAxisRanges()
{
    if ( !plotWidget() ) return;

    for ( const auto& axisProperty : m_axisPropertiesArray )
    {
        updateZoomForAxis( axisProperty );
    }

    plotWidget()->updateAxes();
    updateAxisRangesFromPlotWidget();
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
void RimSummaryPlot::updateAxisRangesFromPlotWidget()
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        if ( !axisProperties ) continue;

        if ( !plotWidget()->axisEnabled( axisProperties->plotAxis() ) ) continue;

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
void RimSummaryPlot::deletePlotCurvesAndPlotWidget()
{
    if ( isDeletable() )
    {
        detachAllPlotItems();
        deleteAllPlotCurves();

        if ( m_summaryPlot )
        {
            // The RiuPlotWidget is owned by Qt, and will be deleted when its parent is destructed.
            m_summaryPlot.clear();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::connectCurveSignals( RimStackablePlotCurve* curve )
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
void RimSummaryPlot::disconnectCurveSignals( RimStackablePlotCurve* curve )
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

    curvesChanged.send();
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

    // Change of stacking can result in very large y-axis changes, so zoom all
    zoomAll();
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
        plotWidget()->setAxisScale( RimSummaryPlot::plotAxisForTime(), timeAxisProps->visibleRangeMin(), timeAxisProps->visibleRangeMax() );
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
    axisProperties->enableAutoValueForAllFields( true );
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
        if ( !c->isChecked() ) continue;
        if ( !c->showInLegend() ) continue;
        curves.push_back( c );
    }

    for ( auto curveSet : curveSets() )
    {
        if ( !curveSet->isCurvesVisible() ) continue;
        if ( RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ) )
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
void RimSummaryPlot::deleteUnlockedGridTimeHistoryCurves()
{
    auto allGridCurves = m_gridTimeHistoryCurves.childrenByType();
    for ( auto c : allGridCurves )
    {
        if ( c->isLocked() ) continue;

        disconnectCurveSignals( c );

        m_gridTimeHistoryCurves.removeChild( c );
        delete c;
    }
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
RimSummaryPlot::CurveInfo RimSummaryPlot::handleSummaryCaseDrop( RimSummaryCase* summaryCase )
{
    std::map<std::pair<RifEclipseSummaryAddress, RifEclipseSummaryAddress>, std::set<RimSummaryCase*>> dataVectorMap;

    for ( auto& curve : summaryCurves() )
    {
        const auto addr  = curve->summaryAddressY();
        const auto addrX = curve->summaryAddressX();

        // NB! This concept is used to make it possible to avoid adding curves for a case that is already present
        // To be complete, the summaryCaseX() should also be checked, but this is not done for now
        dataVectorMap[std::make_pair( addr, addrX )].insert( curve->summaryCaseY() );
    }

    std::vector<RimSummaryCurve*> curves;

    for ( const auto& [addressPair, cases] : dataVectorMap )
    {
        if ( cases.count( summaryCase ) > 0 ) continue;

        const auto& [addrY, addrX] = addressPair;

        curves.push_back( addNewCurve( addrY, summaryCase, addrX, summaryCase ) );
    }

    return { .curveCount = static_cast<int>( curves.size() ), .curves = curves, .curveSets = {} };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::CurveInfo RimSummaryPlot::handleEnsembleDrop( RimSummaryEnsemble* ensemble )
{
    int                               newCurves = 0;
    std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

    std::map<RiaSummaryCurveAddress, std::set<RimSummaryEnsemble*>> dataVectorMap;

    for ( auto& curve : curveSets() )
    {
        const auto addr = curve->curveAddress();
        dataVectorMap[addr].insert( curve->summaryEnsemble() );
    }

    for ( const auto& [addr, ensembles] : dataVectorMap )
    {
        if ( ensembles.count( ensemble ) > 0 ) continue;

        auto curveSet = RiaSummaryPlotTools::addNewEnsembleCurve( this, addr, ensemble );
        curveSetsToUpdate.push_back( curveSet );
        newCurves++;
    }

    return { newCurves, {}, curveSetsToUpdate };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::CurveInfo RimSummaryPlot::handleAddressCollectionDrop( RimSummaryAddressCollection* addressCollection )
{
    int                               newCurves = 0;
    std::vector<RimSummaryCurve*>     curves;
    std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

    auto droppedName = addressCollection->name().toStdString();

    auto summaryCase  = RiaSummaryTools::summaryCaseById( addressCollection->caseId() );
    auto ensembleCase = RiaSummaryTools::ensembleById( addressCollection->ensembleId() );

    std::vector<RiaSummaryCurveDefinition>                     sourceCurveDefs;
    std::map<RiaSummaryCurveDefinition, std::set<std::string>> newCurveDefsWithObjectNames;

    if ( summaryCase && !ensembleCase )
    {
        for ( auto& curve : summaryCurves() )
        {
            sourceCurveDefs.push_back( curve->curveDefinition() );
        }
    }

    if ( ensembleCase )
    {
        auto curveSets = m_ensembleCurveSetCollection->curveSets();
        for ( auto curveSet : curveSets )
        {
            sourceCurveDefs.emplace_back( ensembleCase, curveSet->curveAddress() );
        }
    }

    for ( auto& curveDef : sourceCurveDefs )
    {
        auto       newCurveDef = curveDef;
        const auto curveAdr    = newCurveDef.summaryAddressY();

        std::string objectIdentifierString;
        if ( ( curveAdr.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL ) &&
             ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL ) )
        {
            objectIdentifierString = curveAdr.wellName();
        }
        else if ( ( curveAdr.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP ) &&
                  ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::GROUP ) )
        {
            objectIdentifierString = curveAdr.groupName();
        }
        else if ( ( curveAdr.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK ) &&
                  ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::NETWORK ) )
        {
            objectIdentifierString = curveAdr.networkName();
        }
        else if ( ( curveAdr.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION ) &&
                  ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION ) )
        {
            objectIdentifierString = std::to_string( curveAdr.regionNumber() );
        }
        else if ( ( curveAdr.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_SEGMENT ) &&
                  ( addressCollection->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL_SEGMENT ) )
        {
            objectIdentifierString = std::to_string( curveAdr.wellSegmentNumber() );
        }

        if ( !objectIdentifierString.empty() )
        {
            newCurveDef.setIdentifierText( curveAdr.category(), droppedName );

            newCurveDefsWithObjectNames[newCurveDef].insert( objectIdentifierString );
            const auto& addr = curveDef.summaryAddressY();
            if ( !addr.isHistoryVector() && RiaPreferencesSummary::current()->appendHistoryVectors() )
            {
                auto historyAddr = addr;
                historyAddr.setVectorName( addr.vectorName() + RifEclipseSummaryAddressDefines::historyIdentifier() );

                auto historyCurveDef = newCurveDef;
                historyCurveDef.setSummaryAddressY( historyAddr );
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
            if ( addresses.find( curveDef.summaryAddressY() ) != addresses.end() )
            {
                auto curveSet = RiaSummaryPlotTools::addNewEnsembleCurve( this, curveDef.summaryCurveAddress(), curveDef.ensemble() );
                curveSetsToUpdate.push_back( curveSet );
                newCurves++;
            }
        }
        else if ( curveDef.summaryCaseY() )
        {
            if ( curveDef.summaryCaseY()->summaryReader() && curveDef.summaryCaseY()->summaryReader()->hasAddress( curveDef.summaryAddressY() ) )
            {
                auto curve =
                    addNewCurve( curveDef.summaryAddressY(), curveDef.summaryCaseY(), curveDef.summaryAddressX(), curveDef.summaryCaseX() );
                curves.push_back( curve );
                if ( curveDef.summaryCaseX() )
                {
                    curve->setAxisTypeX( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR );
                    curve->setSummaryCaseX( curveDef.summaryCaseX() );
                    curve->setSummaryAddressX( curveDef.summaryAddressX() );
                    findOrAssignPlotAxisX( curve );
                }
                newCurves++;
            }
        }
    }

    return { newCurves, curves, curveSetsToUpdate };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::CurveInfo RimSummaryPlot::handleSummaryAddressDrop( RimSummaryAddress* summaryAddr )
{
    int                               newCurves = 0;
    std::vector<RimSummaryCurve*>     curves;
    std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

    std::vector<RifEclipseSummaryAddress> newCurveAddresses;
    newCurveAddresses.push_back( summaryAddr->address() );
    if ( !summaryAddr->address().isHistoryVector() && RiaPreferencesSummary::current()->appendHistoryVectors() )
    {
        auto historyAddr = summaryAddr->address();
        historyAddr.setVectorName( summaryAddr->address().vectorName() + RifEclipseSummaryAddressDefines::historyIdentifier() );
        newCurveAddresses.push_back( historyAddr );
    }

    if ( summaryAddr->isEnsemble() )
    {
        std::map<RifEclipseSummaryAddress, std::set<RimSummaryEnsemble*>> dataVectorMap;

        for ( auto& curve : curveSets() )
        {
            const auto addr = curve->summaryAddressY();
            dataVectorMap[addr].insert( curve->summaryEnsemble() );
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
                    auto curveSet = RiaSummaryPlotTools::addNewEnsembleCurve( this,
                                                                              RiaSummaryCurveAddress( RifEclipseSummaryAddress::timeAddress(),
                                                                                                      droppedAddress ),
                                                                              ensemble );

                    curveSetsToUpdate.push_back( curveSet );
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
                    curves.push_back( addNewCurve( droppedAddress, summaryCase, RifEclipseSummaryAddress::timeAddress(), nullptr ) );
                    newCurves++;
                }
            }
        }
    }
    return { newCurves, curves, curveSetsToUpdate };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects )
{
    CurveInfo curveInfo;
    for ( auto obj : objects )
    {
        if ( auto summaryCase = dynamic_cast<RimSummaryCase*>( obj ) )
        {
            curveInfo.appendCurveInfo( handleSummaryCaseDrop( summaryCase ) );
        }
        else if ( auto ensemble = dynamic_cast<RimSummaryEnsemble*>( obj ) )
        {
            curveInfo.appendCurveInfo( handleEnsembleDrop( ensemble ) );
        }
        else if ( auto summaryAddr = dynamic_cast<RimSummaryAddress*>( obj ) )
        {
            curveInfo.appendCurveInfo( handleSummaryAddressDrop( summaryAddr ) );
        }

        else if ( auto addressCollection = dynamic_cast<RimSummaryAddressCollection*>( obj ) )
        {
            if ( addressCollection->isFolder() )
            {
                for ( auto coll : addressCollection->subFolders() )
                {
                    auto localInfo = handleAddressCollectionDrop( coll );
                    curveInfo.appendCurveInfo( localInfo );
                }
            }
            else
            {
                curveInfo.appendCurveInfo( handleAddressCollectionDrop( addressCollection ) );
            }
        }
    }

    if ( curveInfo.curveCount > 0 )
    {
        applyDefaultCurveAppearances( curveInfo.curves );
        applyDefaultCurveAppearances( curveInfo.curveSets );

        loadDataAndUpdate();
        zoomAll();

        curvesChanged.send();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryPlot::addNewCurve( const RifEclipseSummaryAddress& address,
                                              RimSummaryCase*                 summaryCase,
                                              const RifEclipseSummaryAddress& addressX,
                                              RimSummaryCase*                 summaryCaseX )
{
    auto newCurve = RiaSummaryPlotTools::createCurve( summaryCase, address );

    // This address is RifEclipseSummaryAddress::time() if the curve is a time plot. Otherwise it is the address of the summary vector
    // used for the x-axis
    if ( addressX.category() != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_TIME )
    {
        newCurve->setAxisTypeX( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR );
        newCurve->setSummaryAddressX( addressX );
        newCurve->setSummaryCaseX( summaryCaseX );
    }

    addCurveNoUpdate( newCurve );

    return newCurve;
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
    for ( auto p : plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
    {
        p->enableAutoValueMinMax( false );
    }

    updateAxisRangesFromPlotWidget();

    timeAxisSettingsChanged( nullptr );

    axisChanged.send( this );

    m_lastZoomTime = std::chrono::steady_clock::now();
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
    CAF_ASSERT( mainWindowParent );
    if ( !plotWidget() )
    {
        m_summaryPlot = new RiuSummaryQwtPlot( this, mainWindowParent );

        QObject::connect( plotWidget(), SIGNAL( curveOrderNeedsUpdate() ), this, SLOT( onUpdateCurveOrder() ) );

        for ( const auto& axisProperties : m_axisPropertiesArray )
        {
            plotWidget()->ensureAxisIsCreated( axisProperties->plotAxis() );
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

        connect( plotWidget(), SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

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
        auto copyAxis = [this]( RiuPlotAxis axis, auto sourceObject )
        {
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
        copyAxis( RiuPlotAxis::defaultBottomForSummaryVectors(), m_bottomAxisProperties_OBSOLETE.v() );
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

    for ( auto curve : gridTimeHistoryCurves() )
    {
        connectCurveSignals( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateNameHelperWithCurveData( RimSummaryPlotNameHelper* nameHelper ) const
{
    if ( !nameHelper ) return;

    nameHelper->clear();
    std::vector<RiaSummaryCurveAddress> addresses;
    std::vector<RimSummaryCase*>        sumCases;
    std::vector<RimSummaryEnsemble*>    ensembleCases;

    if ( m_summaryCurveCollection && m_summaryCurveCollection->isCurvesVisible() )
    {
        for ( RimSummaryCurve* curve : m_summaryCurveCollection->curves() )
        {
            addresses.push_back( curve->curveAddress() );
            sumCases.push_back( curve->summaryCaseY() );

            if ( curve->summaryCaseX() )
            {
                sumCases.push_back( curve->summaryCaseX() );
            }
        }
    }

    for ( auto curveSet : m_ensembleCurveSetCollection->curveSets() )
    {
        addresses.push_back( curveSet->curveAddress() );
        ensembleCases.push_back( curveSet->summaryEnsemble() );
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

    loadDataAndUpdate();
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
            if ( c->isChecked() )
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

    updateAndRedrawTimeAnnotations();
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
    auto axisTypes = m_summaryCurveCollection->horizontalAxisTypes();

    if ( axisTypes.contains( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR ) )
    {
        return summaryCurveCollection()->sourceSteppingObject();
    }

    return m_sourceStepping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setAutoScaleXEnabled( bool enabled )
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
void RimSummaryPlot::setAutoScaleYEnabled( bool enabled )
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
size_t RimSummaryPlot::curveCount() const
{
    return m_summaryCurveCollection->curves().size() + m_gridTimeHistoryCurves.size() + m_asciiDataCurves.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isDeletable() const
{
    auto plotWindow = firstAncestorOrThisOfType<RimMultiPlot>();
    return plotWindow == nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisPropertiesInterface*> RimSummaryPlot::allPlotAxes() const
{
    return m_axisPropertiesArray.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisProperties*> RimSummaryPlot::plotAxes( RimPlotAxisProperties::Orientation orientation ) const
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
void RimSummaryPlot::assignPlotAxis( RimSummaryCurve* destinationCurve )
{
    assignXPlotAxis( destinationCurve );
    assignYPlotAxis( destinationCurve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto countAxes = []( const std::vector<RimPlotAxisPropertiesInterface*>& axes, RiaDefines::PlotAxis axis )
{ return std::count_if( axes.begin(), axes.end(), [axis]( const auto& ap ) { return ap->plotAxis().axis() == axis; } ); };

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::assignYPlotAxis( RimSummaryCurve* curve )
{
    enum class AxisAssignmentStrategy
    {
        ALTERNATING,
        USE_MATCHING_UNIT,
        USE_MATCHING_VECTOR
    };

    auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;

    auto destinationUnit = RiaStdStringTools::toUpper( curve->unitNameY() );
    if ( destinationUnit.empty() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

    auto anyCurveWithUnitText = [this, curve]
    {
        for ( auto c : summaryCurves() )
        {
            if ( c == curve ) continue;

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
            if ( c == curve ) continue;

            auto incomingAxisText = RimPlotAxisTools::axisTextForAddress( curve->summaryAddressY() );
            auto currentAxisText  = RimPlotAxisTools::axisTextForAddress( c->summaryAddressY() );
            if ( incomingAxisText == currentAxisText )
            {
                curve->setLeftOrRightAxisY( c->axisY() );
                return;
            }
        }
    }
    else if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
    {
        for ( auto c : summaryCurves() )
        {
            if ( c == curve ) continue;

            auto currentUnit = RiaStdStringTools::toUpper( c->unitNameY() );
            if ( currentUnit == destinationUnit )
            {
                for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
                {
                    if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
                         axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
                    {
                        curve->setLeftOrRightAxisY( c->axisY() );

                        return;
                    }
                }
            }
        }

        strategy = AxisAssignmentStrategy::ALTERNATING;
    }

    auto isDefaultLeftAndRightUsed = [this]( RimSummaryCurve* currentCurve ) -> std::pair<bool, bool>
    {
        bool defaultLeftUsed  = false;
        bool defaultRightUsed = false;

        for ( auto c : summaryCurves() )
        {
            if ( c == currentCurve ) continue;

            if ( c->axisY() == RiuPlotAxis::defaultLeft() ) defaultLeftUsed = true;
            if ( c->axisY() == RiuPlotAxis::defaultRight() ) defaultRightUsed = true;
        }

        return std::make_pair( defaultLeftUsed, defaultRightUsed );
    };

    auto [defaultLeftUsed, defaultRightUsed] = isDefaultLeftAndRightUsed( curve );
    if ( !defaultLeftUsed )
    {
        curve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
        return;
    }

    if ( !defaultRightUsed )
    {
        curve->setLeftOrRightAxisY( RiuPlotAxis::defaultRight() );
        return;
    }

    RiaDefines::PlotAxis plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    if ( strategy == AxisAssignmentStrategy::ALTERNATING )
    {
        size_t axisCountLeft  = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
        size_t axisCountRight = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

        if ( axisCountLeft > axisCountRight ) plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    }

    if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
    {
        auto newPlotAxis = plotWidget()->createNextPlotAxis( plotAxisType );
        addNewAxisProperties( newPlotAxis, "New Axis" );

        curve->setLeftOrRightAxisY( newPlotAxis );
        return;
    }

    // If we get here, we have no more axes to assign to, use left axis as fallback
    curve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::assignXPlotAxis( RimSummaryCurve* curve )
{
    RiuPlotAxis newPlotAxis = RimSummaryPlot::plotAxisForTime();

    if ( curve->axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
    {
        enum class AxisAssignmentStrategy
        {
            ALL_TOP,
            ALL_BOTTOM,
            ALTERNATING,
            USE_MATCHING_UNIT,
            USE_MATCHING_VECTOR
        };

        auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;

        auto destinationUnit = RiaStdStringTools::toUpper( curve->unitNameX() );
        if ( destinationUnit.empty() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

        auto anyCurveWithUnitText = [this, curve]
        {
            for ( auto c : summaryCurves() )
            {
                if ( c == curve ) continue;

                if ( !c->unitNameX().empty() ) return true;
            }

            return false;
        };

        if ( !anyCurveWithUnitText() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

        if ( strategy == AxisAssignmentStrategy::USE_MATCHING_VECTOR )
        {
            // Special handling if curve unit is matching. Try to match on summary vector name to avoid creation of new axis

            for ( auto c : summaryCurves() )
            {
                if ( c == curve ) continue;

                auto incomingAxisText = RimPlotAxisTools::axisTextForAddress( curve->summaryAddressY() );
                auto currentAxisText  = RimPlotAxisTools::axisTextForAddress( c->summaryAddressY() );
                if ( incomingAxisText == currentAxisText )
                {
                    curve->setTopOrBottomAxisX( c->axisX() );
                    return;
                }
            }
        }
        else if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
        {
            bool isTopUsed    = false;
            bool isBottomUsed = false;

            for ( auto c : summaryCurves() )
            {
                if ( c == curve ) continue;

                if ( c->axisX() == RiuPlotAxis::defaultTop() ) isTopUsed = true;
                if ( c->axisX() == RiuPlotAxis::defaultBottomForSummaryVectors() ) isBottomUsed = true;

                auto currentUnit = RiaStdStringTools::toUpper( c->unitNameX() );

                if ( currentUnit == destinationUnit )
                {
                    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
                    {
                        if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_TOP ||
                             axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
                        {
                            curve->setTopOrBottomAxisX( c->axisX() );

                            return;
                        }
                    }
                }
            }

            if ( !isTopUsed )
            {
                curve->setTopOrBottomAxisX( RiuPlotAxis::defaultTop() );
                return;
            }

            if ( !isBottomUsed )
            {
                curve->setTopOrBottomAxisX( RiuPlotAxis::defaultBottomForSummaryVectors() );
                return;
            }

            strategy = AxisAssignmentStrategy::ALTERNATING;
        }

        RiaDefines::PlotAxis plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_TOP;

        if ( strategy == AxisAssignmentStrategy::ALTERNATING )
        {
            size_t axisCountTop = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_TOP );
            size_t axisCountBot = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );

            if ( axisCountTop > axisCountBot ) plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
        }
        else if ( strategy == AxisAssignmentStrategy::ALL_TOP )
        {
            plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_TOP;
        }
        else if ( strategy == AxisAssignmentStrategy::ALL_BOTTOM )
        {
            plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
        }

        RiuPlotAxis newPlotAxis = RiuPlotAxis::defaultBottomForSummaryVectors();
        if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
        {
            newPlotAxis = plotWidget()->createNextPlotAxis( plotAxisType );
            addNewAxisProperties( newPlotAxis, "New Axis" );
        }
    }

    curve->setTopOrBottomAxisX( newPlotAxis );
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
