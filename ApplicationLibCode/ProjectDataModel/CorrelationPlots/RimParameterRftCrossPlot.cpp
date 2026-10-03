/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimParameterRftCrossPlot.h"

#include "RiaColorTables.h"
#include "RiaDefines.h"
#include "RiaPreferences.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderRftInterface.h"

#include "RigEnsembleParameter.h"
#include "RigStatisticsTools.h"

#include "RiaExtractionTools.h"

#include "Well/RigEclipseWellLogExtractor.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimObservedFmuRftData.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"
#include "RimWellLogRftCurve.h"
#include "RimWellPath.h"
#include "RimWellPlotTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuDockWidgetTools.h"
#include "RiuPlotCurve.h"
#include "RiuQwtCurveSelectorFilter.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtSymbol.h"

#include "cafPdmPointer.h"
#include "cafPdmUiComboBoxEditor.h"

#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_zoneitem.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"

#include <QMouseEvent>
#include <QPaintDevice>

#include <limits>
#include <numeric>

CAF_PDM_SOURCE_INIT( RimParameterRftCrossPlot, "ParameterRftCrossPlot" );

namespace caf
{
template <>
void caf::AppEnum<RimParameterRftCrossPlot::SampleMode>::setUp()
{
    addItem( RimParameterRftCrossPlot::SampleMode::ALL_SAMPLES, "ALL_SAMPLES", "All" );
    addItem( RimParameterRftCrossPlot::SampleMode::MEAN_PER_REALIZATION, "MEAN_PER_REALIZATION", "Mean per Realization" );
    setDefault( RimParameterRftCrossPlot::SampleMode::MEAN_PER_REALIZATION );
}
} // namespace caf

const QString RimParameterRftCrossPlot::CUSTOM_RANGE_FILTER_VALUE = "__CUSTOM_RANGE__";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterRftCrossPlot::RimParameterRftCrossPlot()
{
    CAF_PDM_InitObject( "Parameter RFT Cross Plot", ":/CorrelationCrossPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );
    CAF_PDM_InitField( &m_wellName, "WellName", QString(), "Well Name" );
    m_wellName.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeStep, "TimeStep", "Time Step" );
    m_selectedTimeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case (MD fallback)" );
    CAF_PDM_InitField( &m_useDepthRange, "UseDepthRange", false, "Filter by Depth Range" );
    CAF_PDM_InitField( &m_depthRangeMin, "DepthRangeMin", 0.0, "Min Depth (MD)" );
    CAF_PDM_InitField( &m_depthRangeMax, "DepthRangeMax", 5000.0, "Max Depth (MD)" );
    CAF_PDM_InitField( &m_formationFilter, "FormationFilter", QString(), "Depth Range Filter" );
    m_formationFilter.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString(), "Ensemble Parameter" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_sampleMode, "SampleMode", "Samples" );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "UseAutoPlotTitle", true, "Auto Title" );
    CAF_PDM_InitField( &m_description, "Description", QString( "RFT Cross Plot" ), "Title" );

    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size" );

    m_axisTitleFontSize = caf::FontTools::RelativeSize::Small;
    m_axisValueFontSize = caf::FontTools::RelativeSize::Small;

    m_showPlotLegends = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterRftCrossPlot::~RimParameterRftCrossPlot()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setWellName( const QString& wellName )
{
    m_wellName = wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setTimeStep( const QDateTime& timeStep )
{
    m_selectedTimeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setDepthRange( double minMd, double maxMd )
{
    m_depthRangeMin = minMd;
    m_depthRangeMax = maxMd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setFormationFilter( const QString& formationName )
{
    m_formationFilter = formationName;
    applyFormationFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setEnsembleParameter( const QString& paramName )
{
    m_ensembleParameter = paramName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setSampleMode( SampleMode sampleMode )
{
    m_sampleMode = sampleMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterRftCrossPlot::ensembleParameter() const
{
    return m_ensembleParameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterRftCrossPlot::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimParameterRftCrossPlot::selectedTimeStep() const
{
    return m_selectedTimeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimParameterRftCrossPlot::ensemble() const
{
    return m_ensemble();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimParameterRftCrossPlot::eclipseCase() const
{
    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimParameterRftCrossPlot::useDepthRange() const
{
    return m_useDepthRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimParameterRftCrossPlot::depthRangeMin() const
{
    return m_depthRangeMin();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimParameterRftCrossPlot::depthRangeMax() const
{
    return m_depthRangeMax();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterRftCrossPlot::formationFilter() const
{
    return m_formationFilter();
}

//--------------------------------------------------------------------------------------------------
/// Returns the formation name if the depth range filter is currently set to an actual formation
/// (as opposed to "None" or "Custom Range"), otherwise an empty string.
//--------------------------------------------------------------------------------------------------
QString RimParameterRftCrossPlot::selectedFormationName() const
{
    if ( m_formationFilter().isEmpty() || m_formationFilter() == CUSTOM_RANGE_FILTER_VALUE ) return QString();

    return m_formationFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterRftCrossPlot::SampleMode RimParameterRftCrossPlot::sampleMode() const
{
    return m_sampleMode();
}

//--------------------------------------------------------------------------------------------------
/// Looks up the MD depth range spanned by the selected formation across all observed FMU RFT data
/// sets for the current well/time step, and applies it as the depth range filter. Falls back to
/// leaving the depth range untouched if the formation is empty or no matching observed data exists.
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::applyFormationFilter()
{
    if ( m_formationFilter().isEmpty() || m_formationFilter() == CUSTOM_RANGE_FILTER_VALUE ) return;

    double minMd = std::numeric_limits<double>::max();
    double maxMd = -std::numeric_limits<double>::max();
    bool   found = false;

    for ( RimObservedFmuRftData* observedData : RimWellPlotTools::observedFmuRftDataForWell( m_wellName() ) )
    {
        auto range = observedData->formationDepthRange( m_wellName(), m_selectedTimeStep(), m_formationFilter() );
        if ( !range ) continue;

        minMd = std::min( minMd, range->first );
        maxMd = std::max( maxMd, range->second );
        found = true;
    }

    if ( !found ) return;

    m_depthRangeMin = minMd;
    m_depthRangeMax = maxMd;
    m_useDepthRange = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimParameterRftCrossPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RimParameterRftCrossPlot::computePressureSamplesPerCase( RimSummaryEnsemble*   ensemble,
                                                                                          const QString&        wellName,
                                                                                          const QDateTime&      timeStep,
                                                                                          RimEclipseResultCase* eclipseCase,
                                                                                          bool                  useDepthRange,
                                                                                          double                depthRangeMin,
                                                                                          double                depthRangeMax )
{
    if ( !ensemble || wellName.isEmpty() || !timeStep.isValid() ) return {};

    RigEclipseWellLogExtractor* extractor = nullptr;
    if ( eclipseCase )
    {
        RimWellPath* wellPath = RimProject::current()->wellPathFromSimWellName( wellName );
        extractor             = RiaExtractionTools::findOrCreateWellLogExtractor( wellPath, eclipseCase );
        if ( !extractor ) extractor = RiaExtractionTools::findOrCreateSimWellExtractor( eclipseCase, wellName, false, 0 );
    }

    // Simulated RFT readers (e.g. RifReaderOpmRft) do not expose an MD channel, and MD can only be
    // derived from well-path/grid intersections when an Eclipse case is available (see extractor
    // above). Without one, rftCurveDepthValues() falls back to TVD. Since the depth range filter is
    // always specified in MD, precompute the equivalent TVD range (from the well/time step's own
    // observed MD<->TVD relationship) so the filter still applies correctly to TVD-only data.
    std::optional<std::pair<double, double>> tvdFilterRange;
    if ( useDepthRange )
    {
        for ( RimObservedFmuRftData* observedData : RimWellPlotTools::observedFmuRftDataForWell( wellName ) )
        {
            tvdFilterRange = observedData->convertMdRangeToTvd( wellName, timeStep, depthRangeMin, depthRangeMax );
            if ( tvdFilterRange ) break;
        }
    }

    const auto& allCases = ensemble->allSummaryCases();

    std::vector<std::vector<double>> samplesPerCase;
    samplesPerCase.reserve( allCases.size() );

    for ( RimSummaryCase* summaryCase : allCases )
    {
        if ( !summaryCase )
        {
            samplesPerCase.emplace_back();
            continue;
        }

        RifReaderRftInterface* reader = summaryCase->rftReader();
        if ( !reader )
        {
            samplesPerCase.emplace_back();
            continue;
        }

        auto pressureAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        std::vector<double> pressures;
        reader->values( pressureAddress, &pressures );
        if ( pressures.empty() )
        {
            samplesPerCase.emplace_back();
            continue;
        }

        // Use the same depth values the RFT curves use for their depth axis, so the filter
        // operates on values consistent with what the user sees in the RFT plot.
        RiaDefines::DepthType depthType = RiaDefines::DepthType::MEASURED_DEPTH;
        std::vector<double>   depths    = RimWellLogRftCurve::rftCurveDepthValues( reader, wellName, timeStep, extractor, &depthType );

        // The MD-specified filter range only applies directly to MD depths; when the reader could
        // only supply TVD, use the TVD-converted range instead (if one could be computed).
        double rangeMin = depthRangeMin;
        double rangeMax = depthRangeMax;
        if ( depthType == RiaDefines::DepthType::TRUE_VERTICAL_DEPTH && tvdFilterRange )
        {
            rangeMin = tvdFilterRange->first;
            rangeMax = tvdFilterRange->second;
        }

        std::vector<double> samplesInRange;
        if ( useDepthRange )
        {
            if ( depths.size() != pressures.size() )
            {
                // Depth filter requested but no aligned depth data is available for this case;
                // exclude rather than silently return an unfiltered result.
                samplesPerCase.emplace_back();
                continue;
            }
            for ( size_t i = 0; i < depths.size(); ++i )
                if ( depths[i] >= rangeMin && depths[i] <= rangeMax ) samplesInRange.push_back( pressures[i] );
        }
        else
        {
            samplesInRange = pressures;
        }

        samplesPerCase.push_back( samplesInRange );
    }

    return samplesPerCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimParameterRftCrossPlot::computeMeanPressurePerCase( RimSummaryEnsemble*   ensemble,
                                                                          const QString&        wellName,
                                                                          const QDateTime&      timeStep,
                                                                          RimEclipseResultCase* eclipseCase,
                                                                          bool                  useDepthRange,
                                                                          double                depthRangeMin,
                                                                          double                depthRangeMax )
{
    const std::vector<std::vector<double>> samplesPerCase =
        computePressureSamplesPerCase( ensemble, wellName, timeStep, eclipseCase, useDepthRange, depthRangeMin, depthRangeMax );

    std::vector<double> pressurePerCase;
    pressurePerCase.reserve( samplesPerCase.size() );
    for ( const auto& samples : samplesPerCase )
    {
        if ( samples.empty() )
            pressurePerCase.push_back( std::numeric_limits<double>::infinity() );
        else
            pressurePerCase.push_back( std::accumulate( samples.begin(), samples.end(), 0.0 ) / samples.size() );
    }

    return pressurePerCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimParameterRftCrossPlot::CaseData> RimParameterRftCrossPlot::createCaseData() const
{
    if ( !m_ensemble() ) return {};
    if ( m_wellName().isEmpty() ) return {};
    if ( !m_selectedTimeStep().isValid() ) return {};
    if ( m_ensembleParameter().isEmpty() ) return {};

    RigEnsembleParameter parameter = m_ensemble->ensembleParameter( m_ensembleParameter );
    if ( !parameter.isNumeric() || !parameter.isValid() ) return {};

    const auto& allCases = m_ensemble->allSummaryCases();

    std::vector<CaseData> result;

    if ( m_sampleMode() == SampleMode::ALL_SAMPLES )
    {
        // One point per RFT sample within the depth range, instead of a single per-case mean.
        const std::vector<std::vector<double>> samplesPerCase = computePressureSamplesPerCase( m_ensemble(),
                                                                                               m_wellName(),
                                                                                               m_selectedTimeStep(),
                                                                                               m_eclipseCase(),
                                                                                               m_useDepthRange(),
                                                                                               m_depthRangeMin(),
                                                                                               m_depthRangeMax() );
        if ( samplesPerCase.size() != allCases.size() ) return {};

        for ( size_t caseIdx = 0; caseIdx < allCases.size(); ++caseIdx )
        {
            RimSummaryCase* summaryCase = allCases[caseIdx];
            if ( !summaryCase ) continue;
            if ( caseIdx >= static_cast<size_t>( parameter.values.size() ) ) continue;

            const double paramValue = parameter.values[caseIdx].toDouble();
            for ( double pressureValue : samplesPerCase[caseIdx] )
                result.push_back( { .parameterValue = paramValue, .pressureValue = pressureValue, .summaryCase = summaryCase } );
        }
    }
    else
    {
        const std::vector<double> pressurePerCase = computeMeanPressurePerCase( m_ensemble(),
                                                                                m_wellName(),
                                                                                m_selectedTimeStep(),
                                                                                m_eclipseCase(),
                                                                                m_useDepthRange(),
                                                                                m_depthRangeMin(),
                                                                                m_depthRangeMax() );

        if ( pressurePerCase.size() != allCases.size() ) return {};

        result.reserve( allCases.size() );

        for ( size_t caseIdx = 0; caseIdx < allCases.size(); ++caseIdx )
        {
            RimSummaryCase* summaryCase = allCases[caseIdx];
            if ( !summaryCase ) continue;
            if ( std::isinf( pressurePerCase[caseIdx] ) ) continue;
            if ( caseIdx >= static_cast<size_t>( parameter.values.size() ) ) continue;

            result.push_back( { .parameterValue = parameter.values[caseIdx].toDouble(),
                                .pressureValue  = pressurePerCase[caseIdx],
                                .summaryCase    = summaryCase } );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimParameterRftCrossPlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    const int axisTitleSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
    const int axisValueSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );

    const QString pressureLabel = m_sampleMode() == SampleMode::ALL_SAMPLES ? QString( "Pressure" ) : QString( "Mean Pressure" );
    const QString formationName = selectedFormationName();
    const QString depthLabel    = !formationName.isEmpty() ? QString( "%1 [%2]" ).arg( pressureLabel ).arg( formationName )
                                  : m_useDepthRange()
                                      ? QString( "%1 [MD %2 - %3]" ).arg( pressureLabel ).arg( m_depthRangeMin() ).arg( m_depthRangeMax() )
                                      : pressureLabel;

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), depthLabel );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), axisTitleSize, axisValueSize, false, Qt::AlignCenter );

    if ( m_yValueRange.has_value() )
    {
        m_plotWidget->setAxisRange( RiuPlotAxis::defaultLeft(), m_yValueRange->first, m_yValueRange->second );
    }

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), m_ensembleParameter() );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(), axisTitleSize, axisValueSize, false, Qt::AlignCenter );

    if ( m_xValueRange.has_value() )
    {
        m_plotWidget->setAxisRange( RiuPlotAxis::defaultBottom(), m_xValueRange->first, m_xValueRange->second );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterRftCrossPlot::asciiDataForPlotExport() const
{
    QString       asciiData;
    const QString pressureHeader = m_sampleMode() == SampleMode::ALL_SAMPLES ? "Pressure" : "Mean Pressure";
    asciiData += QString( "Realization\tParameter\t%1\n" ).arg( pressureHeader );
    for ( const auto& [paramValue, pressureValue, summaryCase] : createCaseData() )
    {
        asciiData += QString( "%1\t%2\t%3\n" ).arg( summaryCase->displayCaseName() ).arg( paramValue ).arg( pressureValue );
    }
    return asciiData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::detachAllCurves()
{
    if ( m_plotWidget ) m_plotWidget->qwtPlot()->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterRftCrossPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimParameterRftCrossPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_plotWidget ) m_plotWidget->render( paintDevice );
}

namespace
{
class CurveTracker : public QwtPlotPicker
{
public:
    CurveTracker( QwtPlot* plot )
        : QwtPlotPicker( plot->canvas() )
    {
        setStateMachine( new QwtPickerTrackerMachine() );
        setRubberBand( QwtPicker::NoRubberBand );
        setTrackerMode( QwtPicker::AlwaysOn );
    }

protected:
    QwtText trackerText( const QPoint& pos ) const override
    {
        double  minDistance = std::numeric_limits<double>::max();
        QString closestCurveLabel;

        for ( QwtPlotItem* item : plot()->itemList() )
        {
            if ( item->rtti() == QwtPlotItem::Rtti_PlotCurve )
            {
                auto   curve    = static_cast<QwtPlotCurve*>( item );
                double distance = std::numeric_limits<double>::max();
                curve->closestPoint( pos, &distance );

                if ( distance < minDistance )
                {
                    minDistance       = distance;
                    closestCurveLabel = curve->title().text();
                }
            }
        }

        if ( minDistance < 20.0 )
        {
            QwtText text( closestCurveLabel );
            text.setBackgroundBrush( QBrush( Qt::white ) );
            text.setColor( Qt::black );
            return text;
        }
        return QwtText();
    }
};

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimParameterRftCrossPlot::doCreatePlotViewWidget( QWidget* parent )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, parent );
        updatePlotTitle();
        new RiuContextMenuLauncher( m_plotWidget, { "RicShowPlotDataFeature" } );
    }

    if ( m_plotWidget )
    {
        new CurveTracker( m_plotWidget->qwtPlot() );

        caf::PdmPointer<RimParameterRftCrossPlot> self( this );
        new RiuQwtCurveSelectorFilter( m_plotWidget->qwtPlot(),
                                       [self]( const QPoint& pos ) -> const caf::PdmUiItem*
                                       { return self ? self->findClosestCase( pos ) : nullptr; } );
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::onLoadDataAndUpdate()
{
    updateDockWindowVisibility();

    if ( m_plotWidget )
    {
        createPoints();
        updateValueRanges();
        updateAxes();
        updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto* dataGroup = uiOrdering.addNewGroup( "Data Source" );
    dataGroup->add( &m_ensemble );
    dataGroup->add( &m_wellName );
    dataGroup->add( &m_selectedTimeStep );
    dataGroup->add( &m_eclipseCase );

    auto* depthGroup = uiOrdering.addNewGroup( "Depth Range" );
    depthGroup->add( &m_formationFilter );
    depthGroup->add( &m_depthRangeMin );
    depthGroup->add( &m_depthRangeMax );
    const bool customRangeSelected = m_formationFilter() == CUSTOM_RANGE_FILTER_VALUE;
    m_depthRangeMin.uiCapability()->setUiReadOnly( !customRangeSelected );
    m_depthRangeMax.uiCapability()->setUiReadOnly( !customRangeSelected );

    auto* crossPlotGroup = uiOrdering.addNewGroup( "Cross Plot Parameter" );
    crossPlotGroup->add( &m_ensembleParameter );
    crossPlotGroup->add( &m_sampleMode );

    auto* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->setCollapsedByDefault();
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );

    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_ensemble || changedField == &m_wellName )
    {
        // Reset time step to the first available one for the new ensemble/well combination
        std::set<QDateTime> timeSteps;
        if ( m_ensemble() && !m_wellName().isEmpty() )
        {
            for ( RimSummaryCase* summaryCase : m_ensemble->allSummaryCases() )
            {
                RifReaderRftInterface* reader = summaryCase->rftReader();
                if ( reader )
                {
                    for ( const QDateTime& dt : reader->availableTimeSteps( m_wellName() ) )
                        timeSteps.insert( dt );
                }
            }
        }
        m_selectedTimeStep = timeSteps.empty() ? QDateTime() : *timeSteps.begin();

        // The formation list depends on well/time step; clear the stale selection rather than risk
        // silently filtering by a formation name that no longer applies.
        m_formationFilter = QString();
        m_useDepthRange   = false;
    }
    else if ( changedField == &m_selectedTimeStep )
    {
        // The formation list and its depth range are specific to a given time step.
        m_formationFilter = QString();
        m_useDepthRange   = false;
    }
    else if ( changedField == &m_formationFilter )
    {
        if ( m_formationFilter().isEmpty() )
        {
            // "None": no depth filtering.
            m_useDepthRange = false;
        }
        else if ( m_formationFilter() == CUSTOM_RANGE_FILTER_VALUE )
        {
            // "Custom Range": keep the existing min/max values, now editable by the user.
            m_useDepthRange = true;
        }
        else
        {
            // A formation name: compute and apply its depth range.
            applyFormationFilter();
        }
    }

    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimParameterRftCrossPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_ensemble )
    {
        for ( RimSummaryEnsemble* ensemble : RimProject::current()->summaryEnsembles() )
        {
            if ( ensemble->isEnsemble() ) options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
        }
    }
    else if ( fieldNeedingOptions == &m_wellName )
    {
        std::set<QString> wellNames;
        if ( m_ensemble() )
        {
            for ( RimSummaryCase* summaryCase : m_ensemble->allSummaryCases() )
            {
                RifReaderRftInterface* reader = summaryCase->rftReader();
                if ( reader )
                {
                    for ( const QString& name : reader->wellNames() )
                        wellNames.insert( name );
                }
            }
        }
        for ( const QString& name : wellNames )
            options.push_back( caf::PdmOptionItemInfo( name, name ) );
    }
    else if ( fieldNeedingOptions == &m_selectedTimeStep )
    {
        std::set<QDateTime> timeSteps;
        if ( m_ensemble() && !m_wellName().isEmpty() )
        {
            for ( RimSummaryCase* summaryCase : m_ensemble->allSummaryCases() )
            {
                RifReaderRftInterface* reader = summaryCase->rftReader();
                if ( reader )
                {
                    for ( const QDateTime& dt : reader->availableTimeSteps( m_wellName() ) )
                        timeSteps.insert( dt );
                }
            }
        }
        for ( const QDateTime& dt : timeSteps )
            options.push_back( caf::PdmOptionItemInfo( dt.toString( "yyyy-MM-dd" ), dt ) );
    }
    else if ( fieldNeedingOptions == &m_formationFilter )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", QString() ) );
        options.push_back( caf::PdmOptionItemInfo( "Custom Range", CUSTOM_RANGE_FILTER_VALUE ) );

        std::set<QString> formationNames;
        if ( !m_wellName().isEmpty() && m_selectedTimeStep().isValid() )
        {
            for ( RimObservedFmuRftData* observedData : RimWellPlotTools::observedFmuRftDataForWell( m_wellName() ) )
            {
                for ( const QString& name : observedData->formationNames( m_wellName(), m_selectedTimeStep() ) )
                    formationNames.insert( name );
            }
        }
        for ( const QString& name : formationNames )
            options.push_back( caf::PdmOptionItemInfo( name, name ) );
    }
    else if ( fieldNeedingOptions == &m_eclipseCase )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", static_cast<RimEclipseResultCase*>( nullptr ) ) );
        for ( RimEclipseCase* c : RimProject::current()->eclipseCases() )
        {
            if ( auto* rc = dynamic_cast<RimEclipseResultCase*>( c ) )
                options.push_back( caf::PdmOptionItemInfo( rc->caseUserDescription(), rc ) );
        }
    }
    else if ( fieldNeedingOptions == &m_ensembleParameter )
    {
        if ( m_ensemble() )
        {
            const auto& allCases = m_ensemble->allSummaryCases();

            // Build mean RFT pressure per case if enough context is available for correlation sorting
            const bool          canComputeCorrelation = !m_wellName().isEmpty() && m_selectedTimeStep().isValid();
            std::vector<double> pressurePerCase;
            if ( canComputeCorrelation )
            {
                pressurePerCase = computeMeanPressurePerCase( m_ensemble(),
                                                              m_wellName(),
                                                              m_selectedTimeStep(),
                                                              m_eclipseCase(),
                                                              m_useDepthRange(),
                                                              m_depthRangeMin(),
                                                              m_depthRangeMax() );
            }

            // Compute correlation for each numeric parameter, then sort by abs value descending
            std::vector<std::pair<double, RigEnsembleParameter>> correlatedParams;
            for ( const auto& param : RimSummaryEnsembleTools::alphabeticEnsembleParameters( allCases ) )
            {
                if ( !param.isNumeric() ) continue;

                double absPearson = 0.0;
                if ( canComputeCorrelation && static_cast<size_t>( param.values.size() ) == allCases.size() )
                {
                    std::vector<double> paramValues, pressureValues;
                    for ( size_t i = 0; i < allCases.size(); ++i )
                    {
                        if ( std::isinf( pressurePerCase[i] ) ) continue;
                        paramValues.push_back( param.values[i].toDouble() );
                        pressureValues.push_back( pressurePerCase[i] );
                    }
                    if ( paramValues.size() >= 2 )
                    {
                        double r = RigStatisticsTools::pearsonCorrelation( paramValues, pressureValues );
                        if ( !std::isinf( r ) && !std::isnan( r ) ) absPearson = std::abs( r );
                    }
                }
                correlatedParams.emplace_back( absPearson, param );
            }

            std::stable_sort( correlatedParams.begin(),
                              correlatedParams.end(),
                              []( const auto& a, const auto& b ) { return a.first > b.first; } );

            for ( const auto& [corr, param] : correlatedParams )
                options.push_back( caf::PdmOptionItemInfo( param.uiName(), param.name ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::createPoints()
{
    detachAllCurves();

    addObservedPressureMarkers();

    caf::ColorTable colorTable = RiaColorTables::categoryPaletteColors();

    auto caseData = createCaseData();
    if ( caseData.empty() ) return;

    std::set<RimSummaryCase*> selectedSummaryCases;
    auto selectedTreeViewItems = RiuDockWidgetTools::selectedItemsInTreeView( RiuDockWidgetTools::plotMainWindowDataSourceTreeName() );
    for ( auto item : selectedTreeViewItems )
    {
        if ( auto summaryCase = dynamic_cast<RimSummaryCase*>( item ) )
        {
            selectedSummaryCases.insert( summaryCase );
        }
    }

    // createCaseData() groups all entries belonging to the same case consecutively (one entry per
    // case normally, or one entry per in-range RFT sample when sampleMode() is ALL_SAMPLES). Group
    // them into a single curve per case so each case gets one consistent color/legend entry even
    // when it contributes multiple points.
    int    idx = 0;
    size_t i   = 0;
    while ( i < caseData.size() )
    {
        RimSummaryCase*     summaryCase = caseData[i].summaryCase;
        std::vector<double> xValues;
        std::vector<double> yValues;
        while ( i < caseData.size() && caseData[i].summaryCase == summaryCase )
        {
            xValues.push_back( caseData[i].parameterValue );
            yValues.push_back( caseData[i].pressureValue );
            ++i;
        }

        auto* plotCurve = new RiuQwtPlotCurve;
        plotCurve->setSamplesValues( xValues, yValues );
        plotCurve->setStyle( QwtPlotCurve::NoCurve );

        const bool isSelected = selectedSummaryCases.contains( summaryCase );
        auto*      symbol     = new RiuQwtSymbol( isSelected ? RiuPlotCurveSymbol::SYMBOL_XCROSS : RiuPlotCurveSymbol::SYMBOL_ELLIPSE );
        symbol->setSize( 8, 8 );
        symbol->setColor( colorTable.cycledQColor( idx++ ) );
        plotCurve->setSymbol( symbol );

        plotCurve->setTitle( summaryCase->displayCaseName() );
        plotCurve->attach( m_plotWidget->qwtPlot() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Adds the mean observed RFT pressure as a solid horizontal reference line, and the observed
/// pressure error band (pressure +/- mean error) as dashed horizontal reference lines, across the
/// full width of the plot. Averages across all observed FMU RFT data sources available for the
/// current well, honoring the active depth range filter (if any).
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::addObservedPressureMarkers()
{
    if ( !m_plotWidget ) return;

    auto observed = observedPressureAndErrorForCurrentSelection();
    if ( !observed ) return;

    const double observedPressure      = observed->first;
    const double observedPressureError = observed->second;

    auto addHorizontalLine = [this]( double yValue, Qt::PenStyle penStyle, const QString& label )
    {
        auto* marker = new QwtPlotMarker();
        marker->setLineStyle( QwtPlotMarker::HLine );
        marker->setYValue( yValue );
        QPen pen( Qt::black );
        pen.setStyle( penStyle );
        pen.setWidth( 1 );
        marker->setLinePen( pen );

        if ( !label.isEmpty() )
        {
            QwtText text( label );
            text.setColor( Qt::black );
            marker->setLabel( text );
            marker->setLabelAlignment( Qt::AlignTop | Qt::AlignLeft );
        }

        // Markers are not included in automatic axis scaling, but updateValueRanges() extends
        // m_yValueRange to include the observed pressure/error values, so the explicit axis range
        // set in updateAxes() always keeps these lines visible.
        marker->setZ( 1000.0 );
        marker->attach( m_plotWidget->qwtPlot() );
    };

    addHorizontalLine( observedPressure, Qt::SolidLine, "Observed Pressure" );
    if ( observedPressureError > 0.0 )
    {
        addHorizontalLine( observedPressure - observedPressureError, Qt::DashLine, "" );
        addHorizontalLine( observedPressure + observedPressureError, Qt::DashLine, "" );

        // Transparent light pink background spanning the +/- error band around the observed pressure.
        QColor shadingColor( 255, 192, 203 ); // light pink
        shadingColor.setAlpha( 60 );

        auto* shading = new QwtPlotZoneItem();
        shading->setOrientation( Qt::Horizontal );
        shading->setInterval( observedPressure - observedPressureError, observedPressure + observedPressureError );
        shading->setPen( shadingColor, 0.0, Qt::NoPen );
        shading->setBrush( QBrush( shadingColor ) );
        shading->setZ( 999.0 );
        shading->attach( m_plotWidget->qwtPlot() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the mean observed RFT pressure and mean observed pressure error for the current well,
/// time step and depth range filter, averaged across all observed FMU RFT data sources available
/// for the well. Returns std::nullopt if no observed data is available for the current selection.
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>> RimParameterRftCrossPlot::observedPressureAndErrorForCurrentSelection() const
{
    if ( m_wellName().isEmpty() || !m_selectedTimeStep().isValid() ) return std::nullopt;

    double sumPressure      = 0.0;
    double sumPressureError = 0.0;
    int    count            = 0;

    for ( RimObservedFmuRftData* observedData : RimWellPlotTools::observedFmuRftDataForWell( m_wellName() ) )
    {
        auto observed =
            observedData->observedPressureAndError( m_wellName(), m_selectedTimeStep(), m_useDepthRange(), m_depthRangeMin(), m_depthRangeMax() );
        if ( !observed ) continue;

        sumPressure += observed->first;
        sumPressureError += observed->second;
        ++count;
    }

    if ( count == 0 ) return std::nullopt;

    return std::make_pair( sumPressure / count, sumPressureError / count );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::updatePlotTitle()
{
    if ( !m_plotWidget ) return;

    if ( m_useAutoPlotTitle && m_ensemble() )
    {
        const QString pressureLabel = m_sampleMode() == SampleMode::ALL_SAMPLES ? QString( "RFT Pressure (All Samples)" )
                                                                                : QString( "Mean RFT Pressure" );
        const QString formationName = selectedFormationName();

        if ( !formationName.isEmpty() )
        {
            m_description =
                QString( "%1 vs %2 [%3], %4" ).arg( m_ensembleParameter() ).arg( pressureLabel ).arg( formationName ).arg( m_ensemble->name() );
        }
        else if ( m_useDepthRange() )
        {
            m_description = QString( "%1 vs %2 [%3 - %4 m], %5" )
                                .arg( m_ensembleParameter() )
                                .arg( pressureLabel )
                                .arg( m_depthRangeMin() )
                                .arg( m_depthRangeMax() )
                                .arg( m_ensemble->name() );
        }
        else
        {
            m_description = QString( "%1 vs %2, %3" ).arg( m_ensembleParameter() ).arg( pressureLabel ).arg( m_ensemble->name() );
        }
    }

    m_plotWidget->setPlotTitle( m_description() );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle() );
    m_plotWidget->setPlotTitleFontSize( titleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::updateValueRanges()
{
    double xMin = std::numeric_limits<double>::infinity();
    double xMax = -std::numeric_limits<double>::infinity();
    double yMin = std::numeric_limits<double>::infinity();
    double yMax = -std::numeric_limits<double>::infinity();

    for ( const auto& [paramValue, pressureValue, summaryCase] : createCaseData() )
    {
        xMin = std::min( xMin, paramValue );
        xMax = std::max( xMax, paramValue );
        yMin = std::min( yMin, pressureValue );
        yMax = std::max( yMax, pressureValue );
    }

    // Ensure the observed pressure/error reference lines added by addObservedPressureMarkers() are
    // always within the Y axis range, even when they fall outside the ensemble pressure values.
    if ( auto observed = observedPressureAndErrorForCurrentSelection() )
    {
        const double observedPressure      = observed->first;
        const double observedPressureError = observed->second;

        yMin = std::min( yMin, observedPressure - observedPressureError );
        yMax = std::max( yMax, observedPressure + observedPressureError );

        if ( xMin == std::numeric_limits<double>::infinity() )
        {
            // No ensemble case data at all; still show the X axis as-is and size the Y axis around
            // the observed pressure only.
            m_xValueRange = std::nullopt;
            m_yValueRange = std::make_pair( yMin, yMax );
            return;
        }
    }
    else if ( xMin == std::numeric_limits<double>::infinity() )
    {
        m_xValueRange = std::nullopt;
        m_yValueRange = std::nullopt;
        return;
    }

    const double xRange = xMax - xMin;
    const double yRange = yMax - yMin;

    m_xValueRange = std::make_pair( xMin - xRange * 0.1, xMax + xRange * 0.1 );
    m_yValueRange = std::make_pair( yMin - yRange * 0.1, yMax + yRange * 0.1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimParameterRftCrossPlot::findClosestCase( const QPoint& canvasPos )
{
    auto caseData = createCaseData();

    std::vector<std::pair<double, double>> points;
    points.reserve( caseData.size() );
    for ( const auto& d : caseData )
        points.push_back( { d.parameterValue, d.pressureValue } );

    int idx = RiuQwtCurveSelectorFilter::closestPointIndex( m_plotWidget->qwtPlot(), canvasPos, points );
    return idx >= 0 ? caseData[idx].summaryCase : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::cleanupBeforeClose()
{
    detachAllCurves();

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}
