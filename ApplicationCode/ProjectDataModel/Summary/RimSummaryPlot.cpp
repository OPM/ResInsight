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

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaFieldHandleTools.h"
#include "RiaSummaryCurveAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "SummaryPlotCommands/RicSummaryCurveCreator.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "PlotTemplates/RimPlotTemplateFolderItem.h"
#include "RimAsciiDataCurve.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimMultiPlotWindow.h"
#include "RimPlotAxisProperties.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlotAxisFormatter.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryPlotFilterTextCurveSetEditor.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuTreeViewEventFilter.h"

#include "cvfColor3.h"

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
        : resamplePeriod( DateTimePeriod::NONE )
    {
    }
    void clear()
    {
        resamplePeriod = DateTimePeriod::NONE;
        caseNames.clear();
        timeSteps.clear();
        allCurveData.clear();
    }

    DateTimePeriod                      resamplePeriod;
    std::vector<QString>                caseNames;
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

void prepareCaseCurvesForExport( DateTimePeriod    period,
                                 ResampleAlgorithm algorithm,
                                 const CurvesData& inputCurvesData,
                                 CurvesData*       resultCurvesData );

void       appendToExportDataForCase( QString&                      out,
                                      const std::vector<time_t>&    timeSteps,
                                      const std::vector<CurveData>& curveData );
void       appendToExportData( QString& out, const std::vector<CurvesData>& curvesData, bool showTimeAsLongString );
CurvesData concatCurvesData( const std::vector<CurvesData>& curvesData );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot::RimSummaryPlot()
    : RimPlot()
{
    CAF_PDM_InitObject( "Summary Plot", ":/SummaryPlotLight16x16.png", "", "" );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Plot Title", "", "", "" );
    m_showPlotTitle.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title", "", "", "" );

    CAF_PDM_InitField( &m_description, "PlotDescription", QString( "Summary Plot" ), "Name", "", "", "" );

    CAF_PDM_InitField( &m_normalizeCurveYValues, "normalizeCurveYValues", false, "Normalize all curves", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCurveCollection, "SummaryCurveCollection", "", "", "", "" );
    m_summaryCurveCollection.uiCapability()->setUiTreeHidden( true );
    m_summaryCurveCollection = new RimSummaryCurveCollection;

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSetCollection, "EnsembleCurveSetCollection", "", "", "", "" );
    m_ensembleCurveSetCollection.uiCapability()->setUiTreeHidden( true );
    m_ensembleCurveSetCollection.uiCapability()->setUiHidden( true );
    m_ensembleCurveSetCollection = new RimEnsembleCurveSetCollection();

    CAF_PDM_InitFieldNoDefault( &m_gridTimeHistoryCurves, "GridTimeHistoryCurves", "", "", "", "" );
    m_gridTimeHistoryCurves.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_asciiDataCurves, "AsciiDataCurves", "", "", "", "" );
    m_asciiDataCurves.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_leftYAxisProperties, "LeftYAxisProperties", "Left Y Axis", "", "", "" );
    m_leftYAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_leftYAxisProperties = new RimPlotAxisProperties;
    m_leftYAxisProperties->setNameAndAxis( "Left Y-Axis", QwtPlot::yLeft );

    CAF_PDM_InitFieldNoDefault( &m_rightYAxisProperties, "RightYAxisProperties", "Right Y Axis", "", "", "" );
    m_rightYAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_rightYAxisProperties = new RimPlotAxisProperties;
    m_rightYAxisProperties->setNameAndAxis( "Right Y-Axis", QwtPlot::yRight );

    CAF_PDM_InitFieldNoDefault( &m_bottomAxisProperties, "BottomAxisProperties", "Bottom X Axis", "", "", "" );
    m_bottomAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_bottomAxisProperties = new RimPlotAxisProperties;
    m_bottomAxisProperties->setNameAndAxis( "Bottom X-Axis", QwtPlot::xBottom );

    CAF_PDM_InitFieldNoDefault( &m_timeAxisProperties, "TimeAxisProperties", "Time Axis", "", "", "" );
    m_timeAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_timeAxisProperties = new RimSummaryTimeAxisProperties;

    CAF_PDM_InitFieldNoDefault( &m_textCurveSetEditor,
                                "SummaryPlotFilterTextCurveSetEditor",
                                "Text Filter Curve Creator",
                                "",
                                "",
                                "" );
    m_textCurveSetEditor.uiCapability()->setUiTreeHidden( true );
    m_textCurveSetEditor = new RimSummaryPlotFilterTextCurveSetEditor;

    // Obsolete fields
    CAF_PDM_InitField( &m_isAutoZoom_OBSOLETE, "AutoZoom", true, "Auto Zoom", "", "", "" );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &m_isAutoZoom_OBSOLETE );
    CAF_PDM_InitField( &m_showLegend_OBSOLETE, "ShowLegend", true, "Legend", "", "", "" );
    m_showLegend_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitFieldNoDefault( &m_curveFilters_OBSOLETE, "SummaryCurveFilters", "", "", "", "" );
    m_curveFilters_OBSOLETE.uiCapability()->setUiTreeHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_summaryCurves_OBSOLETE, "SummaryCurves", "", "", "", "" );
    m_summaryCurves_OBSOLETE.uiCapability()->setUiTreeHidden( true );

    m_isCrossPlot = false;
    m_isDraggable = true;

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

    m_summaryCurves_OBSOLETE.deleteAllChildObjects();
    m_curveFilters_OBSOLETE.deleteAllChildObjects();
    delete m_summaryCurveCollection;
    delete m_ensembleCurveSetCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::showPlotTitle() const
{
    return m_showPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::setShowPlotTitle( bool showTitle )
{
    m_showPlotTitle = showTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateAxes()
{
    updateYAxis( RiaDefines::PLOT_AXIS_LEFT );
    updateYAxis( RiaDefines::PLOT_AXIS_RIGHT );

    if ( m_isCrossPlot )
    {
        updateBottomXAxis();
    }
    else
    {
        updateTimeAxis();
    }

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
    return asciiDataForSummaryPlotExport( DateTimePeriod::YEAR, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlot::asciiDataForSummaryPlotExport( DateTimePeriod resamplingPeriod, bool showTimeAsLongString ) const
{
    QString                      out;
    RiaTimeHistoryCurveResampler resampler;

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
        appendToExportData( out, {summaryCurvesObsData}, showTimeAsLongString );

        std::vector<CurvesData> exportData( 2 );

        // Summary grid data for export
        prepareCaseCurvesForExport( resamplingPeriod,
                                    ResampleAlgorithm::DATA_DECIDES,
                                    summaryCurvesGridData,
                                    &exportData[0] );

        // Time history data for export
        prepareCaseCurvesForExport( resamplingPeriod, ResampleAlgorithm::PERIOD_END, timeHistoryCurvesData, &exportData[1] );

        // Export resampled summary and time history data
        appendToExportData( out, exportData, showTimeAsLongString );
    }

    // Pasted observed data
    {
        CurvesData asciiCurvesData;
        populateAsciiDataCurvesData( m_asciiDataCurves.childObjects(), &asciiCurvesData );

        appendToExportData( out, {asciiCurvesData}, showTimeAsLongString );
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
        RimEnsembleCurveSet* foundCurveSet = m_ensembleCurveSetCollection->findRimCurveSetFromQwtCurve( qwtCurve );

        if ( foundCurveSet )
        {
            m_ensembleCurveSetCollection->setCurrentSummaryCurveSet( foundCurveSet );

            return foundCurveSet;
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
        allCurveDefs.insert( RiaSummaryCurveDefinition( curve->summaryCaseY(), curve->summaryAddressY() ) );
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
        QString data = sourceSummaryPlot.yAxisPropertiesLeftOrRight( RiaDefines::PLOT_AXIS_LEFT )->writeObjectToXmlString();
        yAxisPropertiesLeftOrRight( RiaDefines::PLOT_AXIS_LEFT )
            ->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
    }

    {
        QString data = sourceSummaryPlot.yAxisPropertiesLeftOrRight( RiaDefines::PLOT_AXIS_RIGHT )->writeObjectToXmlString();
        yAxisPropertiesLeftOrRight( RiaDefines::PLOT_AXIS_RIGHT )
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
    m_plotWidget->setLegendVisible( m_showPlotLegends && isMdiWindow() );

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
        m_plotInfoLabel->attach( m_plotWidget );
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
    std::vector<RimSummaryCurve*> summaryCurves                = summaryAndEnsembleCurves();
    size_t                        resamplableSummaryCurveCount = std::count_if( summaryCurves.begin(),
                                                         summaryCurves.end(),
                                                         []( RimSummaryCurve* curve ) {
                                                             return curve->summaryCaseY()
                                                                        ? !curve->summaryCaseY()->isObservedData()
                                                                        : false;
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
bool RimSummaryPlot::hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
{
    if ( fontSettingType == RiaDefines::PLOT_FONT && m_plotWidget )
    {
        for ( auto plotAxis : allPlotAxes() )
        {
            if ( plotAxis->titleFontSize() != defaultFontSize || plotAxis->valuesFontSize() != defaultFontSize )
            {
                return true;
            }
        }

        if ( m_legendFontSize() != defaultFontSize )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                    int                         oldFontSize,
                                    int                         fontSize,
                                    bool                        forceChange /*= false*/ )
{
    bool anyChange = false;

    if ( fontSettingType == RiaDefines::PLOT_FONT && m_plotWidget )
    {
        for ( auto plotAxis : allPlotAxes() )
        {
            if ( forceChange || plotAxis->titleFontSize() == oldFontSize )
            {
                plotAxis->setTitleFontSize( fontSize );
                anyChange = true;
            }
            if ( forceChange || plotAxis->valuesFontSize() == oldFontSize )
            {
                plotAxis->setValuesFontSize( fontSize );
                anyChange = true;
            }
        }

        if ( forceChange || m_legendFontSize() == oldFontSize )
        {
            m_legendFontSize = fontSize;
            anyChange        = true;
        }

        if ( anyChange ) loadDataAndUpdate();
    }
    return anyChange;
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

    QwtPlot::Axis qwtAxis = QwtPlot::yLeft;
    if ( plotAxis == RiaDefines::PLOT_AXIS_LEFT )
    {
        qwtAxis = QwtPlot::yLeft;
    }
    else
    {
        qwtAxis = QwtPlot::yRight;
    }

    RimPlotAxisProperties* yAxisProperties = yAxisPropertiesLeftOrRight( plotAxis );
    if ( yAxisProperties->isActive() && hasVisibleCurvesForAxis( plotAxis ) )
    {
        m_plotWidget->enableAxis( qwtAxis, true );

        std::set<QString> timeHistoryQuantities;

        for ( auto c : visibleTimeHistoryCurvesForAxis( plotAxis ) )
        {
            timeHistoryQuantities.insert( c->quantityName() );
        }

        RimSummaryPlotAxisFormatter calc( yAxisProperties,
                                          visibleSummaryCurvesForAxis( plotAxis ),
                                          visibleAsciiDataCurvesForAxis( plotAxis ),
                                          timeHistoryQuantities );
        calc.applyAxisPropertiesToPlot( m_plotWidget );
    }
    else
    {
        m_plotWidget->enableAxis( qwtAxis, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomForAxis( RiaDefines::PlotAxis plotAxis )
{
    if ( plotAxis == RiaDefines::PLOT_AXIS_BOTTOM )
    {
        if ( m_isCrossPlot )
        {
            if ( m_bottomAxisProperties->isAutoZoom() )
            {
                m_plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
            }
            else
            {
                m_plotWidget->setAxisScale( QwtPlot::xBottom,
                                            m_bottomAxisProperties->visibleRangeMin(),
                                            m_bottomAxisProperties->visibleRangeMax() );
            }
        }
        else
        {
            if ( m_timeAxisProperties->isAutoZoom() )
            {
                m_plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
            }
            else
            {
                m_plotWidget->setAxisScale( QwtPlot::xBottom,
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
            m_plotWidget->setAxisIsLogarithmic( yAxisProps->qwtPlotAxisType(), yAxisProps->isLogarithmicScaleEnabled );

            if ( yAxisProps->isLogarithmicScaleEnabled )
            {
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

                m_plotWidget->setAxisScale( yAxisProps->qwtPlotAxisType(), min, max );
            }
            else
            {
                m_plotWidget->setAxisAutoScale( yAxisProps->qwtPlotAxisType(), true );
            }
        }
        else
        {
            m_plotWidget->setAxisScale( yAxisProps->qwtPlotAxisType(),
                                        yAxisProps->visibleRangeMin(),
                                        yAxisProps->visibleRangeMax() );
        }

        m_plotWidget->axisScaleEngine( yAxisProps->qwtPlotAxisType() )
            ->setAttribute( QwtScaleEngine::Inverted, yAxisProps->isAxisInverted() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryPlot::visibleSummaryCurvesForAxis( RiaDefines::PlotAxis plotAxis ) const
{
    std::vector<RimSummaryCurve*> curves;

    if ( plotAxis == RiaDefines::PLOT_AXIS_BOTTOM )
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

    if ( leftOrRightPlotAxis == RiaDefines::PLOT_AXIS_LEFT )
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
            if ( c->yAxis() == plotAxis || plotAxis == RiaDefines::PLOT_AXIS_BOTTOM )
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
            if ( c->yAxis() == plotAxis || plotAxis == RiaDefines::PLOT_AXIS_BOTTOM )
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
        m_plotWidget->enableAxis( QwtPlot::xBottom, false );

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

    m_plotWidget->enableAxis( QwtPlot::xBottom, true );

    {
        Qt::AlignmentFlag alignment = Qt::AlignCenter;
        if ( m_timeAxisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            alignment = Qt::AlignRight;
        }

        m_plotWidget->setAxisFontsAndAlignment( QwtPlot::xBottom,
                                                m_timeAxisProperties->titleFontSize(),
                                                m_timeAxisProperties->valuesFontSize(),
                                                true,
                                                alignment );
        m_plotWidget->setAxisTitleText( QwtPlot::xBottom, m_timeAxisProperties->title() );
        m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, m_timeAxisProperties->showTitle );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateBottomXAxis()
{
    if ( !m_plotWidget ) return;

    QwtPlot::Axis qwtAxis = QwtPlot::xBottom;

    RimPlotAxisProperties* bottomAxisProperties = m_bottomAxisProperties();

    if ( bottomAxisProperties->isActive() )
    {
        m_plotWidget->enableAxis( qwtAxis, true );

        std::set<QString> timeHistoryQuantities;

        RimSummaryPlotAxisFormatter calc( bottomAxisProperties,
                                          visibleSummaryCurvesForAxis( RiaDefines::PLOT_AXIS_BOTTOM ),
                                          visibleAsciiDataCurvesForAxis( RiaDefines::PLOT_AXIS_BOTTOM ),
                                          timeHistoryQuantities );
        calc.applyAxisPropertiesToPlot( m_plotWidget );
    }
    else
    {
        m_plotWidget->enableAxis( qwtAxis, false );
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

        if ( m_plotWidget )
        {
            curve->setParentQwtPlotAndReplot( m_plotWidget );
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

        if ( m_plotWidget )
        {
            curve->setParentQwtPlotNoReplot( m_plotWidget );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::deleteCurve( RimSummaryCurve* curve )
{
    deleteCurves( {curve} );
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
                                 m_plotWidget )
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
        curve->setParentQwtPlotAndReplot( m_plotWidget );
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
        curve->setParentQwtPlotNoReplot( m_plotWidget );
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
        curve->setParentQwtPlotAndReplot( m_plotWidget );
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

    if ( changedField == &m_showWindow )
    {
        updateWindowVisibility();
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
    }

    if ( changedField == &m_normalizeCurveYValues )
    {
        this->loadDataAndUpdate();
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
    if ( uiConfigName == RicSummaryCurveCreator::CONFIGURATION_NAME )
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

    updateWindowVisibility();

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
        m_plotWidget->setLegendVisible( m_showPlotLegends && isMdiWindow() );
        m_plotWidget->setLegendFontSize( m_legendFontSize() );
        m_plotWidget->updateLegend();
    }
    this->updateAxes();

    m_textCurveSetEditor->updateTextFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateZoomInQwt()
{
    if ( m_plotWidget )
    {
        updateZoomForAxis( RiaDefines::PLOT_AXIS_BOTTOM );
        updateZoomForAxis( RiaDefines::PLOT_AXIS_LEFT );
        updateZoomForAxis( RiaDefines::PLOT_AXIS_RIGHT );

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

    QwtInterval leftAxis  = m_plotWidget->axisRange( QwtPlot::yLeft );
    QwtInterval rightAxis = m_plotWidget->axisRange( QwtPlot::yRight );
    QwtInterval timeAxis  = m_plotWidget->axisRange( QwtPlot::xBottom );

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
    return {m_timeAxisProperties, m_bottomAxisProperties, m_leftYAxisProperties, m_rightYAxisProperties};
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
void RimSummaryPlot::doRemoveFromCollection()
{
    RimSummaryPlotCollection* summaryCollection = nullptr;
    this->firstAncestorOrThisOfType( summaryCollection );
    if ( summaryCollection )
    {
        summaryCollection->removeSummaryPlot( this );
        summaryCollection->updateAllRequiredEditors();
    }
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
void RimSummaryPlot::setDraggable( bool draggable )
{
    m_isDraggable = draggable;
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
        mainOptions->add( &m_showPlotLegends );
        if ( m_showPlotLegends() )
        {
            mainOptions->add( &m_legendFontSize );
        }
    }

    mainOptions->add( &m_normalizeCurveYValues );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryPlot::createViewWidget( QWidget* mainWindowParent )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuSummaryQwtPlot( this, mainWindowParent );
        m_plotWidget->setDraggable( m_isDraggable );

        for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
        {
            curve->setParentQwtPlotNoReplot( m_plotWidget );
        }

        for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
        {
            curve->setParentQwtPlotNoReplot( m_plotWidget );
        }

        if ( m_summaryCurveCollection )
        {
            m_summaryCurveCollection->setParentQwtPlotAndReplot( m_plotWidget );
        }

        if ( m_ensembleCurveSetCollection )
        {
            m_ensembleCurveSetCollection->setParentQwtPlotAndReplot( m_plotWidget );
        }
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

    // Move summary curves from obsolete storage to the new curve collection
    std::vector<RimSummaryCurve*> curvesToMove;

    for ( auto& curveFilter : m_curveFilters_OBSOLETE )
    {
        const auto& tmpCurves = curveFilter->curves();
        curvesToMove.insert( curvesToMove.end(), tmpCurves.begin(), tmpCurves.end() );
        curveFilter->clearCurvesWithoutDelete();
    }
    m_curveFilters_OBSOLETE.clear();

    curvesToMove.insert( curvesToMove.end(), m_summaryCurves_OBSOLETE.begin(), m_summaryCurves_OBSOLETE.end() );
    m_summaryCurves_OBSOLETE.clear();

    for ( const auto& curve : curvesToMove )
    {
        m_summaryCurveCollection->addCurve( curve );
    }

    if ( !m_isAutoZoom_OBSOLETE() )
    {
        setAutoScaleXEnabled( false );
        setAutoScaleYEnabled( false );
    }

    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfType( proj );
    if ( proj )
    {
        if ( proj->isProjectFileVersionEqualOrOlderThan( "2017.0.0" ) )
        {
            m_useAutoPlotTitle = false;
        }
    }

    if ( m_showLegend_OBSOLETE() )
    {
        m_showPlotLegends = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::updateMdiWindowTitle()
{
    if ( m_plotWidget )
    {
        if ( isMdiWindow() )
        {
            QString plotTitle = description();

            m_plotWidget->setWindowTitle( plotTitle );

            if ( m_showPlotTitle )
            {
                m_plotWidget->setTitle( plotTitle );
            }
            else
            {
                m_plotWidget->setTitle( "" );
            }
        }
        else
        {
            m_plotWidget->setTitle( "" );
        }
        m_plotWidget->scheduleReplot();
    }
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
void RimSummaryPlot::updateWindowVisibility()
{
    if ( isMdiWindow() )
    {
        updateMdiWindowVisibility();
    }
    else
    {
        updateParentLayout();
    }
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlot::doUpdateLayout()
{
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
RimSummaryPlotSourceStepping* RimSummaryPlot::sourceSteppingObjectForKeyEventHandling() const
{
    caf::PdmObjectHandle* selectedObj = dynamic_cast<caf::PdmObjectHandle*>(
        caf::SelectionManager::instance()->selectedItem() );
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
void populateTimeHistoryCurvesData( std::vector<RimGridTimeHistoryCurve*> curves, CurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->caseNames.clear();
    curvesData->timeSteps.clear();
    curvesData->allCurveData.clear();

    for ( RimGridTimeHistoryCurve* curve : curves )
    {
        if ( !curve->isCurveVisible() ) continue;
        QString curveCaseName = curve->caseName();

        size_t casePosInList = cvf::UNDEFINED_SIZE_T;
        for ( size_t i = 0; i < curvesData->caseNames.size(); i++ )
        {
            if ( curveCaseName == curvesData->caseNames[i] ) casePosInList = i;
        }

        CurveData curveData = {curve->curveExportDescription(), RifEclipseSummaryAddress(), curve->yValues()};

        if ( casePosInList == cvf::UNDEFINED_SIZE_T )
        {
            curvesData->caseNames.push_back( curveCaseName );
            curvesData->timeSteps.push_back( curve->timeStepValues() );
            curvesData->allCurveData.push_back( std::vector<CurveData>( {curveData} ) );
        }
        else
        {
            curvesData->allCurveData[casePosInList].push_back( curveData );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void populateAsciiDataCurvesData( std::vector<RimAsciiDataCurve*> curves, CurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->caseNames.clear();
    curvesData->timeSteps.clear();
    curvesData->allCurveData.clear();

    for ( RimAsciiDataCurve* curve : curves )
    {
        if ( !curve->isCurveVisible() ) continue;

        size_t casePosInList = cvf::UNDEFINED_SIZE_T;

        CurveData curveData = {curve->curveExportDescription(), RifEclipseSummaryAddress(), curve->yValues()};

        if ( casePosInList == cvf::UNDEFINED_SIZE_T )
        {
            curvesData->caseNames.push_back( "" );
            curvesData->timeSteps.push_back( curve->timeSteps() );
            curvesData->allCurveData.push_back( std::vector<CurveData>( {curveData} ) );
        }
        else
        {
            curvesData->allCurveData[casePosInList].push_back( curveData );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void populateSummaryCurvesData( std::vector<RimSummaryCurve*> curves, SummaryCurveType curveType, CurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->caseNames.clear();
    curvesData->timeSteps.clear();
    curvesData->allCurveData.clear();

    for ( RimSummaryCurve* curve : curves )
    {
        bool isObservedCurve = curve->summaryCaseY() ? curve->summaryCaseY()->isObservedData() : false;

        if ( !curve->isCurveVisible() ) continue;
        if ( isObservedCurve && ( curveType & CURVE_TYPE_OBSERVED ) == 0 ) continue;
        if ( !isObservedCurve && ( curveType & CURVE_TYPE_GRID ) == 0 ) continue;

        QString curveCaseName = curve->summaryCaseY()->caseName();

        size_t casePosInList = cvf::UNDEFINED_SIZE_T;
        for ( size_t i = 0; i < curvesData->caseNames.size(); i++ )
        {
            if ( curveCaseName == curvesData->caseNames[i] ) casePosInList = i;
        }

        CurveData curveData = {curve->curveExportDescription(), curve->summaryAddressY(), curve->valuesY()};
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

        if ( casePosInList == cvf::UNDEFINED_SIZE_T )
        {
            auto curveDataList = std::vector<CurveData>( {curveData} );
            if ( hasErrorData ) curveDataList.push_back( errorCurveData );

            curvesData->caseNames.push_back( curveCaseName );
            curvesData->timeSteps.push_back( curve->timeStepsY() );
            curvesData->allCurveData.push_back( curveDataList );
        }
        else
        {
            curvesData->allCurveData[casePosInList].push_back( curveData );
            if ( hasErrorData ) curvesData->allCurveData[casePosInList].push_back( errorCurveData );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void prepareCaseCurvesForExport( DateTimePeriod    period,
                                 ResampleAlgorithm algorithm,
                                 const CurvesData& inputCurvesData,
                                 CurvesData*       resultCurvesData )
{
    RiaTimeHistoryCurveResampler resampler;

    resultCurvesData->clear();

    if ( period != DateTimePeriod::NONE )
    {
        // Prepare result data
        resultCurvesData->resamplePeriod = period;

        for ( size_t i = 0; i < inputCurvesData.caseNames.size(); i++ )
        {
            // Shortcuts to input data
            auto& caseName      = inputCurvesData.caseNames[i];
            auto& caseTimeSteps = inputCurvesData.timeSteps[i];
            auto& caseCurveData = inputCurvesData.allCurveData[i];

            // Prepare result data
            resultCurvesData->caseNames.push_back( caseName );
            resultCurvesData->allCurveData.push_back( std::vector<CurveData>() );

            for ( auto& curveDataItem : caseCurveData )
            {
                resampler.setCurveData( curveDataItem.values, caseTimeSteps );

                if ( curveDataItem.address.hasAccumulatedData() || algorithm == ResampleAlgorithm::PERIOD_END )
                {
                    resampler.resampleAndComputePeriodEndValues( period );
                }
                else
                {
                    resampler.resampleAndComputeWeightedMeanValues( period );
                }

                auto cd                       = curveDataItem;
                cd.values                     = resampler.resampledValues();
                auto& currResultCurveDataList = resultCurvesData->allCurveData[i];
                currResultCurveDataList.push_back( cd );
            }

            resultCurvesData->timeSteps.push_back( resampler.resampledTimeSteps() );
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
        out += QDateTime::fromTime_t( timeSteps[j] ).toUTC().toString( "yyyy-MM-dd hh:mm:ss " );

        for ( size_t k = 0; k < curveData.size(); k++ ) // curves
        {
            QString valueText;
            if ( j < curveData[k].values.size() )
            {
                valueText = QString::number( curveData[k].values[j], 'g', 6 );
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

    if ( data.resamplePeriod != DateTimePeriod::NONE )
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

        auto allTimeSteps = RiaTimeHistoryCurveResampler::timeStepsFromTimeRange( data.resamplePeriod,
                                                                                  minTimeStep,
                                                                                  maxTimeStep );

        out += "\n\n";
        out += "Date and time";
        for ( size_t i = 0; i < data.caseNames.size(); i++ )
        {
            for ( size_t j = 0; j < data.allCurveData[i].size(); j++ )
            {
                out += "\t" + data.allCurveData[i][j].name;
            }
        }
        out += "\n";

        std::vector<size_t> currIndexes( data.caseNames.size() );
        for ( auto& i : currIndexes )
            i = 0;

        for ( auto timeStep : allTimeSteps )
        {
            QDateTime timseStepUtc = QDateTime::fromTime_t( timeStep ).toUTC();
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
                    case DateTimePeriod::NONE:
                        timeText = timseStepUtc.toString( "yyyy-MM-dd hh:mm:ss " );
                        break;
                    case DateTimePeriod::DAY:
                        timeText = oneDayEarlier.toString( "yyyy-MM-dd " );
                        break;
                    case DateTimePeriod::WEEK:
                    {
                        timeText       = oneDayEarlier.toString( "yyyy" );
                        int weekNumber = oneDayEarlier.date().weekNumber();
                        timeText += QString( "-W%1" ).arg( weekNumber, 2, 10, zeroChar );
                        break;
                    }
                    case DateTimePeriod::MONTH:
                        timeText = oneDayEarlier.toString( "yyyy-MM" );
                        break;
                    case DateTimePeriod::QUARTER:
                    {
                        int quarterNumber = oneDayEarlier.date().month() / 3;
                        timeText          = oneDayEarlier.toString( "yyyy" );
                        timeText += QString( "-Q%1" ).arg( quarterNumber );
                        break;
                    }
                    case DateTimePeriod::HALFYEAR:
                    {
                        int halfYearNumber = oneDayEarlier.date().month() / 6;
                        timeText           = oneDayEarlier.toString( "yyyy" );
                        timeText += QString( "-H%1" ).arg( halfYearNumber );
                        break;
                    }
                    case DateTimePeriod::YEAR:
                        timeText = oneDayEarlier.toString( "yyyy" );
                        break;
                    case DateTimePeriod::DECADE:
                        timeText = oneDayEarlier.toString( "yyyy" );
                        break;
                }
            }
            out += timeText;

            for ( size_t i = 0; i < data.caseNames.size(); i++ ) // cases
            {
                // Check is time step exists in curr case
                size_t& currIndex   = currIndexes[i];
                bool timeStepExists = currIndex < data.timeSteps[i].size() && timeStep == data.timeSteps[i][currIndex];

                for ( size_t j = 0; j < data.allCurveData[i].size(); j++ ) // vectors
                {
                    QString valueText;
                    if ( timeStepExists )
                    {
                        valueText = QString::number( data.allCurveData[i][j].values[currIndex], 'g', 6 );
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
        for ( size_t i = 0; i < data.caseNames.size(); i++ )
        {
            out += "\n\n";
            if ( !data.caseNames[i].isEmpty() )
            {
                out += "Case: " + data.caseNames[i];
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

    DateTimePeriod period = curvesData.front().resamplePeriod;
    CurvesData     resultCurvesData;

    resultCurvesData.resamplePeriod = period;

    for ( auto curvesDataItem : curvesData )
    {
        if ( curvesDataItem.caseNames.empty() ) continue;

        CVF_ASSERT( curvesDataItem.resamplePeriod == period );

        resultCurvesData.caseNames.insert( resultCurvesData.caseNames.end(),
                                           curvesDataItem.caseNames.begin(),
                                           curvesDataItem.caseNames.end() );
        resultCurvesData.timeSteps.insert( resultCurvesData.timeSteps.end(),
                                           curvesDataItem.timeSteps.begin(),
                                           curvesDataItem.timeSteps.end() );
        resultCurvesData.allCurveData.insert( resultCurvesData.allCurveData.end(),
                                              curvesDataItem.allCurveData.begin(),
                                              curvesDataItem.allCurveData.end() );
    }
    return resultCurvesData;
}
