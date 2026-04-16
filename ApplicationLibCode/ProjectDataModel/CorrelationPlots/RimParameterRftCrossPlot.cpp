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
#include "RiaPreferences.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderRftInterface.h"

#include "RigEnsembleParameter.h"
#include "RigStatisticsTools.h"

#include "RiaExtractionTools.h"

#include "Well/RigEclipseWellLogExtractor.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"
#include "RimWellPath.h"

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
#include "qwt_scale_map.h"
#include "qwt_text.h"

#include <QMouseEvent>
#include <QPaintDevice>

#include <limits>
#include <numeric>

CAF_PDM_SOURCE_INIT( RimParameterRftCrossPlot, "ParameterRftCrossPlot" );

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
    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString(), "Ensemble Parameter" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

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
void RimParameterRftCrossPlot::setEnsembleParameter( const QString& paramName )
{
    m_ensembleParameter = paramName;
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
RiuQwtPlotWidget* RimParameterRftCrossPlot::viewer()
{
    return m_plotWidget;
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
    if ( !ensemble || wellName.isEmpty() || !timeStep.isValid() ) return {};

    RigEclipseWellLogExtractor* extractor = nullptr;
    if ( eclipseCase )
    {
        RimWellPath* wellPath = RimProject::current()->wellPathFromSimWellName( wellName );
        extractor             = RiaExtractionTools::findOrCreateWellLogExtractor( wellPath, eclipseCase );
        if ( !extractor ) extractor = RiaExtractionTools::findOrCreateSimWellExtractor( eclipseCase, wellName, false, 0 );
    }

    const auto& allCases = ensemble->allSummaryCases();

    std::vector<double> pressurePerCase;
    pressurePerCase.reserve( allCases.size() );

    for ( RimSummaryCase* summaryCase : allCases )
    {
        if ( !summaryCase )
        {
            pressurePerCase.push_back( std::numeric_limits<double>::infinity() );
            continue;
        }

        RifReaderRftInterface* reader = summaryCase->rftReader();
        if ( !reader )
        {
            pressurePerCase.push_back( std::numeric_limits<double>::infinity() );
            continue;
        }

        auto pressureAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        std::vector<double> pressures;
        reader->values( pressureAddress, &pressures );
        if ( pressures.empty() )
        {
            pressurePerCase.push_back( std::numeric_limits<double>::infinity() );
            continue;
        }

        auto mdAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::MD );
        std::vector<double> depths;
        reader->values( mdAddress, &depths );
        if ( depths.empty() && extractor ) depths = reader->computeMeasuredDepth( wellName, timeStep, extractor );
        // If depths are still empty after the fallback (no MD channel and no extractor), depth range
        // filtering is not possible for this case; fall through to use all pressures unfiltered.

        std::vector<double> samplesInRange;
        if ( useDepthRange && depths.size() == pressures.size() )
        {
            for ( size_t i = 0; i < depths.size(); ++i )
                if ( depths[i] >= depthRangeMin && depths[i] <= depthRangeMax ) samplesInRange.push_back( pressures[i] );
        }
        else
        {
            samplesInRange = pressures;
        }

        if ( samplesInRange.empty() )
            pressurePerCase.push_back( std::numeric_limits<double>::infinity() );
        else
            pressurePerCase.push_back( std::accumulate( samplesInRange.begin(), samplesInRange.end(), 0.0 ) / samplesInRange.size() );
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

    const std::vector<double> pressurePerCase = computeMeanPressurePerCase( m_ensemble(),
                                                                            m_wellName(),
                                                                            m_selectedTimeStep(),
                                                                            m_eclipseCase(),
                                                                            m_useDepthRange(),
                                                                            m_depthRangeMin(),
                                                                            m_depthRangeMax() );

    if ( pressurePerCase.size() != allCases.size() ) return {};

    std::vector<CaseData> result;
    result.reserve( allCases.size() );

    for ( size_t caseIdx = 0; caseIdx < allCases.size(); ++caseIdx )
    {
        RimSummaryCase* summaryCase = allCases[caseIdx];
        if ( !summaryCase ) continue;
        if ( std::isinf( pressurePerCase[caseIdx] ) ) continue;
        if ( caseIdx >= static_cast<size_t>( parameter.values.size() ) ) continue;

        result.push_back(
            { .parameterValue = parameter.values[caseIdx].toDouble(), .pressureValue = pressurePerCase[caseIdx], .summaryCase = summaryCase } );
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

    const QString depthLabel = m_useDepthRange() ? QString( "Mean Pressure [MD %1 - %2]" ).arg( m_depthRangeMin() ).arg( m_depthRangeMax() )
                                                 : QString( "Mean Pressure" );

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
    QString asciiData;
    asciiData += "Realization\tParameter\tMean Pressure\n";
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
    updateMdiWindowVisibility();

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
    depthGroup->add( &m_useDepthRange );
    depthGroup->add( &m_depthRangeMin );
    depthGroup->add( &m_depthRangeMax );
    m_depthRangeMin.uiCapability()->setUiReadOnly( !m_useDepthRange() );
    m_depthRangeMax.uiCapability()->setUiReadOnly( !m_useDepthRange() );

    auto* crossPlotGroup = uiOrdering.addNewGroup( "Cross Plot Parameter" );
    crossPlotGroup->add( &m_ensembleParameter );

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

    int idx = 0;
    for ( const auto& [paramValue, pressureValue, summaryCase] : caseData )
    {
        auto* plotCurve = new RiuQwtPlotCurve;
        plotCurve->setSamplesValues( { paramValue }, { pressureValue } );
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
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::updatePlotTitle()
{
    if ( !m_plotWidget ) return;

    if ( m_useAutoPlotTitle && m_ensemble() )
    {
        if ( m_useDepthRange() )
        {
            m_description = QString( "%1 vs RFT Pressure [%2 - %3 m], %4" )
                                .arg( m_ensembleParameter() )
                                .arg( m_depthRangeMin() )
                                .arg( m_depthRangeMax() )
                                .arg( m_ensemble->name() );
        }
        else
        {
            m_description = QString( "%1 vs RFT Pressure, %2" ).arg( m_ensembleParameter() ).arg( m_ensemble->name() );
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

    if ( xMin == std::numeric_limits<double>::infinity() )
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
