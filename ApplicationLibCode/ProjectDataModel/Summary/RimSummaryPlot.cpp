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
#include "RiaSummaryCurveAnalyzer.h"
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
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlotAxisFormatter.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryPlotFilterTextCurveSetEditor.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuTreeViewEventFilter.h"

#include "cvfColor3.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManager.h"

#include "qwt_abstract_legend.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_textlabel.h"
#include "qwt_scale_engine.h"

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
/// Internal types
//--------------------------------------------------------------------------------------------------
enum class ResampleAlgorithm
{
    NONE,
    DATA_DECIDES,
    PERIOD_END
};

struct CurveData
{
    QString                  name;
    RifEclipseSummaryAddress address;
    std::vector<double>      values;
};

class CurvesData
{
public:
    CurvesData()
        : resamplePeriod( RiaQDateTimeTools::DateTimePeriod::NONE )
    {
    }
    void clear()
    {
        caseIds.clear();
        timeSteps.clear();
        allCurveData.clear();
    }

    void addCurveData( const QString&             caseName,
                       const QString&             ensembleName,
                       const std::vector<time_t>& curvetimeSteps,
                       const CurveData&           curveData )
    {
        QString caseId            = createCaseId( caseName, ensembleName );
        size_t  existingCaseIndex = findCaseIndexForCaseId( caseId, curvetimeSteps.size() );

        if ( existingCaseIndex == cvf::UNDEFINED_SIZE_T )
        {
            caseIds.push_back( caseId );
            timeSteps.push_back( curvetimeSteps );
            allCurveData.push_back( { curveData } );
        }
        else
        {
            CVF_ASSERT( timeSteps[existingCaseIndex].size() == curveData.values.size() );

            allCurveData[existingCaseIndex].push_back( curveData );
        }
    }

    void addCurveDataNoSearch( const QString&                caseName,
                               const QString&                ensembleName,
                               const std::vector<time_t>&    curvetimeSteps,
                               const std::vector<CurveData>& curveDataVector )
    {
        QString caseId = createCaseId( caseName, ensembleName );

        caseIds.push_back( caseId );
        timeSteps.push_back( curvetimeSteps );
        allCurveData.push_back( curveDataVector );
    }

private:
    size_t findCaseIndexForCaseId( const QString& caseId, size_t timeStepCount )
    {
        size_t casePosInList = cvf::UNDEFINED_SIZE_T;

        for ( size_t i = 0; i < caseIds.size(); i++ )
        {
            if ( caseId == caseIds[i] && timeSteps[i].size() == timeStepCount ) casePosInList = i;
        }

        return casePosInList;
    }

    QString createCaseId( const QString& caseName, const QString& ensembleName )
    {
        QString caseId = caseName;
        if ( !ensembleName.isEmpty() ) caseId += QString( " (%1)" ).arg( ensembleName );

        return caseId;
    }

public:
    RiaQDateTimeTools::DateTimePeriod   resamplePeriod;
    std::vector<QString>                caseIds;
    std::vector<std::vector<time_t>>    timeSteps;
    std::vector<std::vector<CurveData>> allCurveData;
};

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
enum SummaryCurveType
{
    CURVE_TYPE_GRID     = 0x1,
    CURVE_TYPE_OBSERVED = 0x2
};

void populateSummaryCurvesData( std::vector<RimSummaryCurve*> curves, SummaryCurveType curveType, CurvesData* curvesData );
void populateTimeHistoryCurvesData( std::vector<RimGridTimeHistoryCurve*> curves, CurvesData* curvesData );
void populateAsciiDataCurvesData( std::vector<RimAsciiDataCurve*> curves, CurvesData* curvesData );

void prepareCaseCurvesForExport( RiaQDateTimeTools::DateTimePeriod period,
                                 ResampleAlgorithm                 algorithm,
                                 const CurvesData&                 inputCurvesData,
                                 CurvesData*                       resultCurvesData );

void       appendToExportDataForCase( QString& out, const std::vector<time_t>& timeSteps, const std::vector<CurveData>& curveData );
void       appendToExportData( QString& out, const std::vector<CurvesData>& curvesData, bool showTimeAsLongString );
CurvesData concatCurvesData( const std::vector<CurvesData>& curvesData );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot()
    : RimQwtPlot()
{
    CAF_PDM_InitScriptableObject( "Summary Plot", ":/SummaryPlotLight16x16.png", "", "A Summary Plot" );

    CAF_PDM_InitScriptableField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title" );
    CAF_PDM_InitScriptableField( &m_description, "PlotDescription", QString( "Summary Plot" ), "Name" );
    CAF_PDM_InitScriptableField( &m_normalizeCurveYValues, "normalizeCurveYValues", false, "Normalize all curves" );

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

    CAF_PDM_InitFieldNoDefault( &m_leftYAxisProperties, "LeftYAxisProperties", "Left Y Axis" );
    m_leftYAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_leftYAxisProperties = new RimPlotAxisProperties;
    m_leftYAxisProperties->setNameAndAxis( "Left Y-Axis", RiaDefines::PlotAxis::PLOT_AXIS_LEFT );

    CAF_PDM_InitFieldNoDefault( &m_rightYAxisProperties, "RightYAxisProperties", "Right Y Axis" );
    m_rightYAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_rightYAxisProperties = new RimPlotAxisProperties;
    m_rightYAxisProperties->setNameAndAxis( "Right Y-Axis", RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

    CAF_PDM_InitFieldNoDefault( &m_bottomAxisProperties, "BottomAxisProperties", "Bottom X Axis" );
    m_bottomAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_bottomAxisProperties = new RimPlotAxisProperties;
    m_bottomAxisProperties->setNameAndAxis( "Bottom X-Axis", RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );

    connectAxisSignals( m_leftYAxisProperties() );
    connectAxisSignals( m_rightYAxisProperties() );
    connectAxisSignals( m_bottomAxisProperties() );

    CAF_PDM_InitFieldNoDefault( &m_timeAxisProperties, "TimeAxisProperties", "Time Axis" );
    m_timeAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_timeAxisProperties = new RimSummaryTimeAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_textCurveSetEditor, "SummaryPlotFilterTextCurveSetEditor", "Text Filter Curve Creator" );
    m_textCurveSetEditor.uiCapability()->setUiTreeHidden( true );
    m_textCurveSetEditor = new RimSummaryPlotFilterTextCurveSetEditor;

    m_isCrossPlot = false;

    m_nameHelperAllCurves.reset( new RimSummaryPlotNameHelper );

    setPlotInfoLabel( "Filters Active" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::~RimSummaryPlot()
{
    removeMdiWindowFromMdiArea();

    cleanupBeforeClose();

    delete m_summaryCurveCollection;
    delete m_ensembleCurveSetCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxes()
{
    updateYAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    updateYAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

    if ( m_timeAxisProperties() && m_plotWidget )
    {
        m_plotWidget->updateAnnotationObjects( m_timeAxisProperties() );
    }
    if ( m_leftYAxisProperties() && m_plotWidget )
    {
        m_plotWidget->updateAnnotationObjects( m_leftYAxisProperties() );
    }

    if ( m_isCrossPlot )
    {
        updateBottomXAxis();
    }
    else
    {
        updateTimeAxis();
    }

    m_plotWidget->scheduleReplot();

    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::isLogarithmicScaleEnabled( RiaDefines::PlotAxis plotAxis ) const
{
    return yAxisPropertiesLeftOrRight( plotAxis )->isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties* RimSummaryPlot::timeAxisProperties()
{
    return m_timeAxisProperties();
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

    if ( firstCurve && firstCurve->timeStepsY().size() > 0 )
    {
        return firstCurve->timeStepsY()[0];
    }
    else
        return time_t( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimSummaryPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::asciiDataForPlotExport() const
{
    return asciiDataForSummaryPlotExport( RiaQDateTimeTools::DateTimePeriod::YEAR, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::asciiDataForSummaryPlotExport( RiaQDateTimeTools::DateTimePeriod resamplingPeriod,
                                                       bool                              showTimeAsLongString ) const
{
    QString out;

    // Summary and time history (from grid) curves
    {
        std::vector<RimSummaryCurve*> curves;
        this->descendantsIncludingThisOfType( curves );

        CurvesData summaryCurvesGridData;
        CurvesData summaryCurvesObsData;
        populateSummaryCurvesData( curves, CURVE_TYPE_GRID, &summaryCurvesGridData );
        populateSummaryCurvesData( curves, CURVE_TYPE_OBSERVED, &summaryCurvesObsData );

        CurvesData timeHistoryCurvesData;
        populateTimeHistoryCurvesData( m_gridTimeHistoryCurves.childObjects(), &timeHistoryCurvesData );

        // Export observed data
        appendToExportData( out, { summaryCurvesObsData }, showTimeAsLongString );

        std::vector<CurvesData> exportData( 2 );

        // Summary grid data for export
        prepareCaseCurvesForExport( resamplingPeriod, ResampleAlgorithm::DATA_DECIDES, summaryCurvesGridData, &exportData[0] );

        // Time history data for export
        prepareCaseCurvesForExport( resamplingPeriod, ResampleAlgorithm::PERIOD_END, timeHistoryCurvesData, &exportData[1] );

        // Export resampled summary and time history data
        appendToExportData( out, exportData, showTimeAsLongString );
    }

    // Pasted observed data
    {
        CurvesData asciiCurvesData;
        populateAsciiDataCurvesData( m_asciiDataCurves.childObjects(), &asciiCurvesData );

        appendToExportData( out, { asciiCurvesData }, showTimeAsLongString );
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimSummaryPlot::findPdmObjectFromQwtCurve( const QwtPlotCurve* qwtCurve ) const
{
    for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    {
        if ( curve->qwtPlotCurve() == qwtCurve )
        {
            return curve;
        }
    }

    for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    {
        if ( curve->qwtPlotCurve() == qwtCurve )
        {
            return curve;
        }
    }

    if ( m_summaryCurveCollection )
    {
        RimSummaryCurve* foundCurve = m_summaryCurveCollection->findRimCurveFromQwtCurve( qwtCurve );

        if ( foundCurve )
        {
            m_summaryCurveCollection->setCurrentSummaryCurve( foundCurve );

            return foundCurve;
        }
    }

    if ( m_ensembleCurveSetCollection )
    {
        RimSummaryCurve* foundCurve = m_ensembleCurveSetCollection->findRimCurveFromQwtCurve( qwtCurve );

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
        itemToSelect = m_leftYAxisProperties;
    }
    else if ( axis == QwtPlot::yRight )
    {
        itemToSelect = m_rightYAxisProperties;
    }
    else if ( axis == QwtPlot::xBottom )
    {
        if ( m_isCrossPlot )
        {
            itemToSelect = m_bottomAxisProperties;
        }
        else
        {
            itemToSelect = m_timeAxisProperties;
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
std::vector<RimSummaryCurve*> RimSummaryPlot::visibleStackedSummaryCurvesForAxis( RiaDefines::PlotAxis plotAxis )
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
std::vector<RimEnsembleCurveSet*> RimSummaryPlot::curveSets() const
{
    return ensembleCurveSetCollection()->curveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updatePlotTitle()
{
    updateNameHelperWithCurveData( m_nameHelperAllCurves.get() );

    if ( m_useAutoPlotTitle )
    {
        m_description = m_nameHelperAllCurves->plotTitle();
    }

    updateCurveNames();
    updateMdiWindowTitle();

    if ( m_plotWidget )
    {
        QString plotTitle = description();
        m_plotWidget->setPlotTitle( plotTitle );
        m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimSummaryPlotNameHelper* RimSummaryPlot::activePlotTitleHelperAllCurves() const
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
    {
        QString data =
            sourceSummaryPlot.yAxisPropertiesLeftOrRight( RiaDefines::PlotAxis::PLOT_AXIS_LEFT )->writeObjectToXmlString();
        yAxisPropertiesLeftOrRight( RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
            ->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
    }

    {
        QString data =
            sourceSummaryPlot.yAxisPropertiesLeftOrRight( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )->writeObjectToXmlString();
        yAxisPropertiesLeftOrRight( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
            ->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAll()
{
    if ( m_plotWidget )
    {
        updatePlotTitle();
        m_plotWidget->updateLegend();
        updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateLegend()
{
    if ( m_plotWidget )
    {
        m_plotWidget->setInternalLegendVisible( m_showPlotLegends && !isSubPlot() );
    }

    reattachAllCurves();
    if ( m_plotWidget )
    {
        m_plotWidget->updateLegend();
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

    m_plotInfoLabel.reset( new QwtPlotTextLabel() );
    m_plotInfoLabel->setText( qwtText );
    m_plotInfoLabel->setMargin( 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::showPlotInfoLabel( bool show )
{
    if ( show )
        m_plotInfoLabel->attach( m_plotWidget->qwtPlot() );
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
void RimSummaryPlot::updateYAxis( RiaDefines::PlotAxis plotAxis )
{
    if ( !m_plotWidget ) return;

    RimPlotAxisProperties* yAxisProperties = yAxisPropertiesLeftOrRight( plotAxis );
    if ( yAxisProperties->isActive() && hasVisibleCurvesForAxis( plotAxis ) )
    {
        m_plotWidget->enableAxis( plotAxis, true );

        std::set<QString> timeHistoryQuantities;

        for ( auto c : visibleTimeHistoryCurvesForAxis( plotAxis ) )
        {
            timeHistoryQuantities.insert( c->quantityName() );
        }

        RimSummaryPlotAxisFormatter calc( yAxisProperties,
                                          visibleSummaryCurvesForAxis( plotAxis ),
                                          {},
                                          visibleAsciiDataCurvesForAxis( plotAxis ),
                                          timeHistoryQuantities );
        calc.applyAxisPropertiesToPlot( m_plotWidget );
    }
    else
    {
        m_plotWidget->enableAxis( plotAxis, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomForAxis( RiaDefines::PlotAxis plotAxis )
{
    if ( plotAxis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
    {
        if ( m_isCrossPlot )
        {
            if ( m_bottomAxisProperties->isAutoZoom() )
            {
                m_plotWidget->setAxisAutoScale( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, true );
            }
            else
            {
                m_plotWidget->setAxisScale( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM,
                                            m_bottomAxisProperties->visibleRangeMin(),
                                            m_bottomAxisProperties->visibleRangeMax() );
            }
        }
        else
        {
            if ( m_timeAxisProperties->isAutoZoom() )
            {
                m_plotWidget->setAxisAutoScale( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, true );
            }
            else
            {
                m_plotWidget->setAxisScale( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM,
                                            m_timeAxisProperties->visibleRangeMin(),
                                            m_timeAxisProperties->visibleRangeMax() );
            }
        }
    }
    else
    {
        RimPlotAxisProperties* yAxisProps = yAxisPropertiesLeftOrRight( plotAxis );

        if ( yAxisProps->isAutoZoom() )
        {
            if ( yAxisProps->isLogarithmicScaleEnabled )
            {
                m_plotWidget->setAxisScaleType( yAxisProps->plotAxisType(), RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC );

                std::vector<const QwtPlotCurve*> plotCurves;

                for ( RimSummaryCurve* c : visibleSummaryCurvesForAxis( plotAxis ) )
                {
                    plotCurves.push_back( c->qwtPlotCurve() );
                }

                for ( RimGridTimeHistoryCurve* c : visibleTimeHistoryCurvesForAxis( plotAxis ) )
                {
                    plotCurves.push_back( c->qwtPlotCurve() );
                }

                for ( RimAsciiDataCurve* c : visibleAsciiDataCurvesForAxis( plotAxis ) )
                {
                    plotCurves.push_back( c->qwtPlotCurve() );
                }

                double                        min, max;
                RimPlotAxisLogRangeCalculator calc( QwtPlot::yLeft, plotCurves );
                calc.computeAxisRange( &min, &max );

                if ( yAxisProps->isAxisInverted() )
                {
                    std::swap( min, max );
                }

                m_plotWidget->setAxisScale( yAxisProps->plotAxisType(), min, max );
            }
            else
            {
                m_plotWidget->setAxisAutoScale( yAxisProps->plotAxisType(), true );
            }
        }
        else
        {
            m_plotWidget->setAxisScale( yAxisProps->plotAxisType(),
                                        yAxisProps->visibleRangeMin(),
                                        yAxisProps->visibleRangeMax() );
        }

        m_plotWidget->setAxisInverted( yAxisProps->plotAxisType(), yAxisProps->isAxisInverted() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::visibleSummaryCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const
{
    std::vector<RimSummaryCurve*> curves;

    if ( plotAxis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
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
bool RimSummaryPlot::hasVisibleCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const
{
    if ( visibleSummaryCurvesForAxis( plotAxis ).size() > 0 )
    {
        return true;
    }

    if ( visibleTimeHistoryCurvesForAxis( plotAxis ).size() > 0 )
    {
        return true;
    }

    if ( visibleAsciiDataCurvesForAxis( plotAxis ).size() > 0 )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimSummaryPlot::yAxisPropertiesLeftOrRight( RiaDefines::PlotAxis leftOrRightPlotAxis ) const
{
    RimPlotAxisProperties* yAxisProps = nullptr;

    if ( leftOrRightPlotAxis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
    {
        yAxisProps = m_leftYAxisProperties();
    }
    else
    {
        yAxisProps = m_rightYAxisProperties();
    }

    CVF_ASSERT( yAxisProps );

    return yAxisProps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridTimeHistoryCurve*> RimSummaryPlot::visibleTimeHistoryCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const
{
    std::vector<RimGridTimeHistoryCurve*> curves;

    for ( auto c : m_gridTimeHistoryCurves )
    {
        if ( c->isCurveVisible() )
        {
            if ( c->yAxis() == plotAxis || plotAxis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
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
std::vector<RimAsciiDataCurve*> RimSummaryPlot::visibleAsciiDataCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const
{
    std::vector<RimAsciiDataCurve*> curves;

    for ( auto c : m_asciiDataCurves )
    {
        if ( c->isCurveVisible() )
        {
            if ( c->yAxis() == plotAxis || plotAxis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
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
void RimSummaryPlot::updateTimeAxis()
{
    if ( !m_plotWidget ) return;

    if ( !m_timeAxisProperties->isActive() )
    {
        m_plotWidget->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, false );

        return;
    }

    if ( m_timeAxisProperties->timeMode() == RimSummaryTimeAxisProperties::DATE )
    {
        RiaQDateTimeTools::DateFormatComponents dateComponents = m_timeAxisProperties->dateComponents();
        RiaQDateTimeTools::TimeFormatComponents timeComponents = m_timeAxisProperties->timeComponents();

        QString dateFormat = m_timeAxisProperties->dateFormat();
        QString timeFormat = m_timeAxisProperties->timeFormat();

        m_plotWidget->useDateBasedTimeAxis( dateFormat, timeFormat, dateComponents, timeComponents );
    }
    else
    {
        m_plotWidget->useTimeBasedTimeAxis();
    }

    m_plotWidget->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, true );

    {
        Qt::AlignmentFlag alignment = Qt::AlignCenter;
        if ( m_timeAxisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            alignment = Qt::AlignRight;
        }

        m_plotWidget->setAxisFontsAndAlignment( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM,
                                                m_timeAxisProperties->titleFontSize(),
                                                m_timeAxisProperties->valuesFontSize(),
                                                true,
                                                alignment );
        m_plotWidget->setAxisTitleText( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, m_timeAxisProperties->title() );
        m_plotWidget->setAxisTitleEnabled( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, m_timeAxisProperties->showTitle );

        {
            RimSummaryTimeAxisProperties::LegendTickmarkCount tickmarkCountEnum =
                m_timeAxisProperties->majorTickmarkCount();

            int maxTickmarkCount = 8;

            switch ( tickmarkCountEnum )
            {
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

            m_plotWidget->setAxisMaxMajor( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, maxTickmarkCount );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateBottomXAxis()
{
    if ( !m_plotWidget ) return;

    RimPlotAxisProperties* bottomAxisProperties = m_bottomAxisProperties();

    if ( bottomAxisProperties->isActive() )
    {
        m_plotWidget->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, true );

        std::set<QString> timeHistoryQuantities;

        RimSummaryPlotAxisFormatter calc( bottomAxisProperties,
                                          visibleSummaryCurvesForAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM ),
                                          {},
                                          visibleAsciiDataCurvesForAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM ),
                                          timeHistoryQuantities );
        calc.applyAxisPropertiesToPlot( m_plotWidget );
    }
    else
    {
        m_plotWidget->enableAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, false );
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
        RimTimeAxisAnnotation* annotation = new RimTimeAxisAnnotation;
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
        RimTimeAxisAnnotation* annotation = new RimTimeAxisAnnotation;
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
    {
        axisProps->removeAllAnnotations();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveAndUpdate( RimSummaryCurve* curve )
{
    if ( curve )
    {
        m_summaryCurveCollection->addCurve( curve );
        connectCurveSignals( curve );
        if ( m_plotWidget )
        {
            curve->setParentQwtPlotAndReplot( m_plotWidget->qwtPlot() );
            this->updateAxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::addCurveNoUpdate( RimSummaryCurve* curve )
{
    if ( curve )
    {
        m_summaryCurveCollection->addCurve( curve );
        connectCurveSignals( curve );
        if ( m_plotWidget )
        {
            curve->setParentQwtPlotNoReplot( m_plotWidget->qwtPlot() );
        }
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
        connectCurveSignals( curve );
        if ( m_plotWidget )
        {
            curve->setParentQwtPlotNoReplot( m_plotWidget->qwtPlot() );
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
                                 m_plotWidget && curveSet->legendFrame() )
                            {
                                m_plotWidget->removeOverlayFrame( curveSet->legendFrame() );
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
    if ( m_plotWidget )
    {
        curve->setParentQwtPlotAndReplot( m_plotWidget->qwtPlot() );
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
    if ( m_plotWidget )
    {
        curve->setParentQwtPlotNoReplot( m_plotWidget->qwtPlot() );
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
    if ( m_plotWidget )
    {
        curve->setParentQwtPlotAndReplot( m_plotWidget->qwtPlot() );
        this->updateAxes();
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
    updateStackedCurveDataForAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    updateStackedCurveDataForAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

    if ( m_plotWidget )
    {
        reattachAllCurves();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateStackedCurveDataForAxis( RiaDefines::PlotAxis plotAxis )
{
    std::map<RiaDefines::PhaseType, size_t> curvePhaseCount;

    auto stackedCurves = visibleStackedSummaryCurvesForAxis( plotAxis );

    // Reset all curves
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
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimSummaryPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
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
        caf::PdmUiTreeOrdering* axisFolder = uiTreeOrdering.add( "Axes", ":/Axes16x16.png" );

        if ( m_isCrossPlot )
        {
            axisFolder->add( &m_bottomAxisProperties );
        }
        else
        {
            axisFolder->add( &m_timeAxisProperties );
        }
        axisFolder->add( &m_leftYAxisProperties );
        axisFolder->add( &m_rightYAxisProperties );

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

    if ( m_plotWidget )
    {
        m_plotWidget->setInternalLegendVisible( m_showPlotLegends && !isSubPlot() );
        m_plotWidget->setLegendFontSize( legendFontSize() );
        m_plotWidget->updateLegend();
    }
    this->updateAxes();

    m_textCurveSetEditor->updateTextFilter();

    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomInQwt()
{
    if ( m_plotWidget )
    {
        updateZoomForAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
        updateZoomForAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
        updateZoomForAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

        m_plotWidget->updateAxes();
        updateZoomFromQwt();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomFromQwt()
{
    if ( !m_plotWidget ) return;

    QwtInterval leftAxis  = m_plotWidget->axisRange( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    QwtInterval rightAxis = m_plotWidget->axisRange( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );
    QwtInterval timeAxis  = m_plotWidget->axisRange( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );

    m_leftYAxisProperties->visibleRangeMax = leftAxis.maxValue();
    m_leftYAxisProperties->visibleRangeMin = leftAxis.minValue();
    m_leftYAxisProperties->updateConnectedEditors();

    m_rightYAxisProperties->visibleRangeMax = rightAxis.maxValue();
    m_rightYAxisProperties->visibleRangeMin = rightAxis.minValue();
    m_rightYAxisProperties->updateConnectedEditors();

    if ( m_isCrossPlot )
    {
        m_bottomAxisProperties->visibleRangeMax = timeAxis.maxValue();
        m_bottomAxisProperties->visibleRangeMin = timeAxis.minValue();
        m_bottomAxisProperties->updateConnectedEditors();
    }
    else
    {
        m_timeAxisProperties->setVisibleRangeMin( timeAxis.minValue() );
        m_timeAxisProperties->setVisibleRangeMax( timeAxis.maxValue() );
        m_timeAxisProperties->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimPlotAxisPropertiesInterface*> RimSummaryPlot::allPlotAxes() const
{
    return { m_timeAxisProperties, m_bottomAxisProperties, m_leftYAxisProperties, m_rightYAxisProperties };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::cleanupBeforeClose()
{
    detachAllPlotItems();

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
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
    if ( m_plotWidget )
    {
        m_plotWidget->scheduleReplot();
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
void RimSummaryPlot::setAsCrossPlot()
{
    m_isCrossPlot = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::onPlotZoomed()
{
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );
    updateZoomFromQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !m_isCrossPlot )
    {
        caf::PdmUiGroup* textCurveFilterGroup = uiOrdering.addNewGroup( "Text-Based Curve Creation" );
        m_textCurveSetEditor->uiOrdering( uiConfigName, *textCurveFilterGroup );
    }

    caf::PdmUiGroup* mainOptions = uiOrdering.addNewGroup( "General Plot Options" );
    mainOptions->setCollapsedByDefault( true );

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
RiuQwtPlotWidget* RimSummaryPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuSummaryQwtPlot( this, mainWindowParent );

        for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
        {
            curve->setParentQwtPlotNoReplot( m_plotWidget->qwtPlot() );
        }

        for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
        {
            curve->setParentQwtPlotNoReplot( m_plotWidget->qwtPlot() );
        }

        if ( m_summaryCurveCollection )
        {
            m_summaryCurveCollection->setParentQwtPlotAndReplot( m_plotWidget->qwtPlot() );
        }

        if ( m_ensembleCurveSetCollection )
        {
            m_ensembleCurveSetCollection->setParentQwtPlotAndReplot( m_plotWidget->qwtPlot() );
        }

        this->connect( m_plotWidget, SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        updatePlotTitle();
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::initAfterRead()
{
    RimViewWindow::initAfterRead();

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
        m_summaryCurveCollection->detachQwtCurves();
    }

    m_ensembleCurveSetCollection->detachQwtCurves();

    for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    {
        curve->detachQwtCurve();
    }

    for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    {
        curve->detachQwtCurve();
    }

    m_plotInfoLabel->detach();
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
            c->updateCurveNameNoLegendUpdate();
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
        m_summaryCurveCollection->reattachQwtCurves();
    }

    m_ensembleCurveSetCollection->reattachQwtCurves();

    for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    {
        curve->reattachQwtCurve();
    }

    for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    {
        curve->reattachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::handleKeyPressEvent( QKeyEvent* keyEvent )
{
    if ( !keyEvent ) return;

    if ( RiuTreeViewEventFilter::activateFeatureFromKeyEvent( keyEvent ) )
    {
        return;
    }

    RimSummaryPlotSourceStepping* sourceStepping = sourceSteppingObjectForKeyEventHandling();
    if ( !sourceStepping ) return;

    if ( keyEvent->key() == Qt::Key_PageUp )
    {
        if ( keyEvent->modifiers() & Qt::ShiftModifier )
        {
            sourceStepping->applyPrevCase();

            keyEvent->accept();
        }
        else if ( keyEvent->modifiers() & Qt::ControlModifier )
        {
            sourceStepping->applyPrevOtherIdentifier();

            keyEvent->accept();
        }
        else
        {
            sourceStepping->applyPrevQuantity();

            keyEvent->accept();
        }
    }
    else if ( keyEvent->key() == Qt::Key_PageDown )
    {
        if ( keyEvent->modifiers() & Qt::ShiftModifier )
        {
            sourceStepping->applyNextCase();

            keyEvent->accept();
        }
        else if ( keyEvent->modifiers() & Qt::ControlModifier )
        {
            sourceStepping->applyNextOtherIdentifier();

            keyEvent->accept();
        }
        else
        {
            sourceStepping->applyNextQuantity();

            keyEvent->accept();
        }
    }
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
    caf::PdmObjectHandle* selectedObj =
        dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selectedObj )
    {
        RimEnsembleCurveSetCollection* ensembleCurveSetColl = nullptr;
        selectedObj->firstAncestorOrThisOfType( ensembleCurveSetColl );

        if ( ensembleCurveSetColl )
        {
            return ensembleCurveSetCollection()->sourceSteppingObject();
        }
    }

    return summaryCurveCollection()->sourceSteppingObject( RimSummaryPlotSourceStepping::Y_AXIS );
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
    if ( m_isCrossPlot )
    {
        m_bottomAxisProperties->setAutoZoom( enabled );
    }
    else
    {
        m_timeAxisProperties->setAutoZoom( enabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setAutoScaleYEnabled( bool enabled )
{
    m_leftYAxisProperties->setAutoZoom( enabled );
    m_rightYAxisProperties->setAutoZoom( enabled );
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
void populateTimeHistoryCurvesData( std::vector<RimGridTimeHistoryCurve*> curves, CurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->clear();

    for ( RimGridTimeHistoryCurve* curve : curves )
    {
        if ( !curve->isCurveVisible() ) continue;
        QString curveCaseName = curve->caseName();

        CurveData curveData = { curve->curveExportDescription(), RifEclipseSummaryAddress(), curve->yValues() };

        curvesData->addCurveData( curveCaseName, "", curve->timeStepValues(), curveData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void populateAsciiDataCurvesData( std::vector<RimAsciiDataCurve*> curves, CurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->clear();

    for ( RimAsciiDataCurve* curve : curves )
    {
        if ( !curve->isCurveVisible() ) continue;

        CurveData curveData = { curve->curveExportDescription(), RifEclipseSummaryAddress(), curve->yValues() };

        curvesData->addCurveDataNoSearch( "", "", curve->timeSteps(), { curveData } );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void populateSummaryCurvesData( std::vector<RimSummaryCurve*> curves, SummaryCurveType curveType, CurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->clear();

    for ( RimSummaryCurve* curve : curves )
    {
        bool isObservedCurve = curve->summaryCaseY() ? curve->summaryCaseY()->isObservedData() : false;

        if ( !curve->isCurveVisible() ) continue;
        if ( isObservedCurve && ( curveType & CURVE_TYPE_OBSERVED ) == 0 ) continue;
        if ( !isObservedCurve && ( curveType & CURVE_TYPE_GRID ) == 0 ) continue;
        if ( !curve->summaryCaseY() ) continue;

        QString curveCaseName = curve->summaryCaseY()->displayCaseName();
        QString ensembleName;
        if ( curve->curveDefinitionY().ensemble() )
        {
            ensembleName = curve->curveDefinitionY().ensemble()->name();
        }

        CurveData curveData = { curve->curveExportDescription(), curve->summaryAddressY(), curve->valuesY() };
        CurveData errorCurveData;

        // Error data
        auto errorValues  = curve->errorValuesY();
        bool hasErrorData = !errorValues.empty();

        if ( hasErrorData )
        {
            errorCurveData.name    = curve->curveExportDescription( curve->errorSummaryAddressY() );
            errorCurveData.address = curve->errorSummaryAddressY();
            errorCurveData.values  = errorValues;
        }

        auto curveDataList = std::vector<CurveData>( { curveData } );
        if ( hasErrorData ) curveDataList.push_back( errorCurveData );
        if ( curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
        {
            // We have calculated data, and it we cannot assume identical time axis
            curvesData->addCurveDataNoSearch( curveCaseName, ensembleName, curve->timeStepsY(), curveDataList );
        }
        else
        {
            for ( auto cd : curveDataList )
            {
                curvesData->addCurveData( curveCaseName, ensembleName, curve->timeStepsY(), cd );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void prepareCaseCurvesForExport( RiaQDateTimeTools::DateTimePeriod period,
                                 ResampleAlgorithm                 algorithm,
                                 const CurvesData&                 inputCurvesData,
                                 CurvesData*                       resultCurvesData )
{
    RiaTimeHistoryCurveResampler resampler;

    resultCurvesData->clear();

    if ( period != RiaQDateTimeTools::DateTimePeriod::NONE )
    {
        // Prepare result data
        resultCurvesData->resamplePeriod = period;

        for ( size_t i = 0; i < inputCurvesData.caseIds.size(); i++ )
        {
            // Shortcuts to input data
            auto& caseId        = inputCurvesData.caseIds[i];
            auto& caseTimeSteps = inputCurvesData.timeSteps[i];
            auto& caseCurveData = inputCurvesData.allCurveData[i];

            // Prepare result data

            for ( auto& curveDataItem : caseCurveData )
            {
                resampler.setCurveData( curveDataItem.values, caseTimeSteps );

                if ( RiaSummaryTools::hasAccumulatedData( curveDataItem.address ) ||
                     algorithm == ResampleAlgorithm::PERIOD_END )
                {
                    resampler.resampleAndComputePeriodEndValues( period );
                }
                else
                {
                    resampler.resampleAndComputeWeightedMeanValues( period );
                }

                auto cd   = curveDataItem;
                cd.values = resampler.resampledValues();

                resultCurvesData->addCurveData( caseId, "", resampler.resampledTimeSteps(), cd );
            }
        }
    }
    else
    {
        *resultCurvesData = inputCurvesData;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void appendToExportDataForCase( QString& out, const std::vector<time_t>& timeSteps, const std::vector<CurveData>& curveData )
{
    for ( size_t j = 0; j < timeSteps.size(); j++ ) // time steps & data points
    {
        if ( j == 0 )
        {
            out += "Date and time";
            for ( size_t k = 0; k < curveData.size(); k++ ) // curves
            {
                out += "\t" + ( curveData[k].name );
            }
        }
        out += "\n";
        out += QDateTime::fromSecsSinceEpoch( timeSteps[j] ).toUTC().toString( "yyyy-MM-dd hh:mm:ss " );

        for ( size_t k = 0; k < curveData.size(); k++ ) // curves
        {
            QString valueText;
            if ( j < curveData[k].values.size() )
            {
                valueText = QString::number( curveData[k].values[j], 'g', RimSummaryPlot::precision() );
            }
            out += "\t" + valueText.rightJustified( 13 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void appendToExportData( QString& out, const std::vector<CurvesData>& curvesData, bool showTimeAsLongString )
{
    CurvesData data = concatCurvesData( curvesData );

    if ( data.resamplePeriod != RiaQDateTimeTools::DateTimePeriod::NONE )
    {
        time_t minTimeStep = std::numeric_limits<time_t>::max();
        time_t maxTimeStep = 0;

        for ( auto& timeSteps : data.timeSteps )
        {
            if ( !timeSteps.empty() )
            {
                if ( timeSteps.front() < minTimeStep ) minTimeStep = timeSteps.front();
                if ( timeSteps.back() > maxTimeStep ) maxTimeStep = timeSteps.back();
            }
        }

        auto allTimeSteps =
            RiaTimeHistoryCurveResampler::timeStepsFromTimeRange( data.resamplePeriod, minTimeStep, maxTimeStep );

        out += "\n\n";
        out += "Date and time";
        for ( size_t i = 0; i < data.caseIds.size(); i++ )
        {
            for ( size_t j = 0; j < data.allCurveData[i].size(); j++ )
            {
                out += "\t" + data.allCurveData[i][j].name;
            }
        }
        out += "\n";

        std::vector<size_t> currIndexes( data.caseIds.size() );
        for ( auto& i : currIndexes )
            i = 0;

        for ( auto timeStep : allTimeSteps )
        {
            QDateTime timseStepUtc = QDateTime::fromSecsSinceEpoch( timeStep ).toUTC();
            QString   timeText;

            if ( showTimeAsLongString )
            {
                timeText = timseStepUtc.toString( "yyyy-MM-dd hh:mm:ss " );
            }
            else
            {
                // Subtract one day to make sure the period is reported using the previous period as label
                QDateTime oneDayEarlier = timseStepUtc.addDays( -1 );

                QChar zeroChar( 48 );

                switch ( data.resamplePeriod )
                {
                    default:
                        // Fall through to NONE
                    case RiaQDateTimeTools::DateTimePeriod::NONE:
                        timeText = timseStepUtc.toString( "yyyy-MM-dd hh:mm:ss " );
                        break;
                    case RiaQDateTimeTools::DateTimePeriod::DAY:
                        timeText = oneDayEarlier.toString( "yyyy-MM-dd " );
                        break;
                    case RiaQDateTimeTools::DateTimePeriod::WEEK:
                    {
                        timeText       = oneDayEarlier.toString( "yyyy" );
                        int weekNumber = oneDayEarlier.date().weekNumber();
                        timeText += QString( "-W%1" ).arg( weekNumber, 2, 10, zeroChar );
                        break;
                    }
                    case RiaQDateTimeTools::DateTimePeriod::MONTH:
                        timeText = oneDayEarlier.toString( "yyyy-MM" );
                        break;
                    case RiaQDateTimeTools::DateTimePeriod::QUARTER:
                    {
                        int quarterNumber = oneDayEarlier.date().month() / 3;
                        timeText          = oneDayEarlier.toString( "yyyy" );
                        timeText += QString( "-Q%1" ).arg( quarterNumber );
                        break;
                    }
                    case RiaQDateTimeTools::DateTimePeriod::HALFYEAR:
                    {
                        int halfYearNumber = oneDayEarlier.date().month() / 6;
                        timeText           = oneDayEarlier.toString( "yyyy" );
                        timeText += QString( "-H%1" ).arg( halfYearNumber );
                        break;
                    }
                    case RiaQDateTimeTools::DateTimePeriod::YEAR:
                        timeText = oneDayEarlier.toString( "yyyy" );
                        break;
                    case RiaQDateTimeTools::DateTimePeriod::DECADE:
                        timeText = oneDayEarlier.toString( "yyyy" );
                        break;
                }
            }
            out += timeText;

            for ( size_t i = 0; i < data.caseIds.size(); i++ ) // cases
            {
                // Check is time step exists in curr case
                size_t& currIndex   = currIndexes[i];
                bool timeStepExists = currIndex < data.timeSteps[i].size() && timeStep == data.timeSteps[i][currIndex];

                for ( size_t j = 0; j < data.allCurveData[i].size(); j++ ) // vectors
                {
                    QString valueText;
                    if ( timeStepExists )
                    {
                        valueText =
                            QString::number( data.allCurveData[i][j].values[currIndex], 'g', RimSummaryPlot::precision() );
                    }
                    else
                    {
                        valueText = "NULL";
                    }
                    out += "\t" + valueText.rightJustified( 13 );
                }

                if ( timeStepExists && currIndex < data.timeSteps[i].size() ) currIndex++;
            }
            out += "\n";
        }
    }
    else
    {
        for ( size_t i = 0; i < data.caseIds.size(); i++ )
        {
            out += "\n\n";
            if ( !data.caseIds[i].isEmpty() )
            {
                out += "Case: " + data.caseIds[i];
                out += "\n";
            }

            appendToExportDataForCase( out, data.timeSteps[i], data.allCurveData[i] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CurvesData concatCurvesData( const std::vector<CurvesData>& curvesData )
{
    CVF_ASSERT( !curvesData.empty() );

    RiaQDateTimeTools::DateTimePeriod period = curvesData.front().resamplePeriod;
    CurvesData                        resultCurvesData;

    resultCurvesData.resamplePeriod = period;

    for ( auto curvesDataItem : curvesData )
    {
        if ( curvesDataItem.caseIds.empty() ) continue;

        CVF_ASSERT( curvesDataItem.resamplePeriod == period );

        resultCurvesData.caseIds.insert( resultCurvesData.caseIds.end(),
                                         curvesDataItem.caseIds.begin(),
                                         curvesDataItem.caseIds.end() );
        resultCurvesData.timeSteps.insert( resultCurvesData.timeSteps.end(),
                                           curvesDataItem.timeSteps.begin(),
                                           curvesDataItem.timeSteps.end() );
        resultCurvesData.allCurveData.insert( resultCurvesData.allCurveData.end(),
                                              curvesDataItem.allCurveData.begin(),
                                              curvesDataItem.allCurveData.end() );
    }
    return resultCurvesData;
}
