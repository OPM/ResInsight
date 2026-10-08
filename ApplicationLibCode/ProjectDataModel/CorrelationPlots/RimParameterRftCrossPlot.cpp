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

#include "RifReaderRftInterface.h"

#include "RigEnsembleParameter.h"
#include "RigStatisticsTools.h"

#include "Formations/RimWellFormationsCollection.h"
#include "Formations/RimWellFormationsFile.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRftCorrelationReportPlot.h"
#include "RimRftCrossPlotTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuDockWidgetTools.h"
#include "RiuPlotCurve.h"
#include "RiuQwtCurveSelectorFilter.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtSymbol.h"

#include "cafPdmPointer.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

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
#include <map>
#include <numeric>

namespace caf
{
template <>
void caf::AppEnum<RimParameterRftCrossPlot::SamplingMode>::setUp()
{
    addItem( RimParameterRftCrossPlot::SamplingMode::ALL_SAMPLES, "ALL_SAMPLES", "All Samples" );
    addItem( RimParameterRftCrossPlot::SamplingMode::MEAN_PER_REALIZATION, "MEAN_PER_REALIZATION", "Mean per Realization" );
    setDefault( RimParameterRftCrossPlot::SamplingMode::MEAN_PER_REALIZATION );
}
} // namespace caf

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
    CAF_PDM_InitField( &m_filterMode,
                       "FilterMode",
                       RimRftCrossPlotTools::DepthFilterModeEnum( RimRftCrossPlotTools::DepthFilterMode::NONE ),
                       "Depth Filter" );
    CAF_PDM_InitField( &m_depthRangeMin, "DepthRangeMin", 0.0, "Min Depth" );
    CAF_PDM_InitField( &m_depthRangeMax, "DepthRangeMax", 5000.0, "Max Depth" );
    CAF_PDM_InitFieldNoDefault( &m_wellFormations, "WellFormations", "Well Formations File" );
    CAF_PDM_InitFieldNoDefault( &m_selectedZones, "SelectedZones", "Zones" );
    m_selectedZones.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_depthType, "DepthType", caf::AppEnum<RiaDefines::DepthType>( RiaDefines::DepthType::MEASURED_DEPTH ), "Depth Type" );
    m_depthType.uiCapability()->setUiHidden( true ); // driven by the parent RimRftCorrelationReportPlot
    CAF_PDM_InitField( &m_samplingMode, "SamplingMode", SamplingModeEnum( SamplingMode::MEAN_PER_REALIZATION ), "Sampling" );
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
void RimParameterRftCrossPlot::setDepthType( RiaDefines::DepthType depthType )
{
    m_depthType = depthType;
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
void RimParameterRftCrossPlot::setWellFormations( RimWellFormationsFile* wellFormationsFile )
{
    m_wellFormations = wellFormationsFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setFilterMode( RimRftCrossPlotTools::DepthFilterMode filterMode )
{
    m_filterMode = filterMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setSelectedZones( const std::vector<QString>& zones )
{
    m_selectedZones = zones;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::setZoneColors( const std::map<QString, QColor>& zoneColors )
{
    m_zoneColors = zoneColors;
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
RiaDefines::DepthType RimParameterRftCrossPlot::depthType() const
{
    return m_depthType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftCrossPlotTools::DepthFilterMode RimParameterRftCrossPlot::filterMode() const
{
    return m_filterMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimParameterRftCrossPlot::selectedZones() const
{
    return m_selectedZones();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellFormationsFile* RimParameterRftCrossPlot::wellFormationsFile() const
{
    return m_wellFormations();
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
std::vector<RimRftCrossPlotTools::DepthInterval> RimParameterRftCrossPlot::depthIntervals() const
{
    return RimRftCrossPlotTools::buildDepthIntervals( m_filterMode(),
                                                      m_depthRangeMin(),
                                                      m_depthRangeMax(),
                                                      m_wellFormations(),
                                                      m_wellName(),
                                                      m_selectedZones(),
                                                      m_depthType() );
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
    result.reserve( allCases.size() );

    if ( m_samplingMode() == SamplingMode::ALL_SAMPLES )
    {
        const std::vector<std::vector<double>> samplesPerCase = RimRftCrossPlotTools::computePressureSamplesPerCase( m_ensemble(),
                                                                                                                     m_wellName(),
                                                                                                                     m_selectedTimeStep(),
                                                                                                                     m_eclipseCase(),
                                                                                                                     depthIntervals(),
                                                                                                                     m_depthType() );

        if ( samplesPerCase.size() != allCases.size() ) return {};

        for ( size_t caseIdx = 0; caseIdx < allCases.size(); ++caseIdx )
        {
            RimSummaryCase* summaryCase = allCases[caseIdx];
            if ( !summaryCase ) continue;
            if ( caseIdx >= static_cast<size_t>( parameter.values.size() ) ) continue;

            const double parameterValue = parameter.values[caseIdx].toDouble();
            for ( double pressureValue : samplesPerCase[caseIdx] )
                result.push_back( { .parameterValue = parameterValue, .pressureValue = pressureValue, .summaryCase = summaryCase } );
        }
    }
    else
    {
        const std::vector<double> pressurePerCase = RimRftCrossPlotTools::computeMeanPressurePerCase( m_ensemble(),
                                                                                                      m_wellName(),
                                                                                                      m_selectedTimeStep(),
                                                                                                      m_eclipseCase(),
                                                                                                      depthIntervals(),
                                                                                                      m_depthType() );

        if ( pressurePerCase.size() != allCases.size() ) return {};

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

    const QString pressureLabel = m_samplingMode() == SamplingMode::ALL_SAMPLES ? "Pressure" : "Mean Pressure";
    const QString filterDescription =
        RimRftCrossPlotTools::depthFilterDescription( m_filterMode(), m_depthType(), m_depthRangeMin(), m_depthRangeMax(), m_selectedZones() );
    const QString depthLabel = filterDescription.isEmpty() ? pressureLabel
                                                           : QString( "%1 [%2]" ).arg( pressureLabel ).arg( filterDescription );

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
    const QString pressureLabel = m_samplingMode() == SamplingMode::ALL_SAMPLES ? "Pressure" : "Mean Pressure";
    asciiData += QString( "Realization\tParameter\t%1\n" ).arg( pressureLabel );
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
/// Adds the depth filter fields; used by the parent report plot, not shown in the cross plot's own editor.
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::appendDepthFilterUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    auto* depthGroup =
        uiOrdering.addNewGroup( QString( "Depth Range (%1)" ).arg( RimRftCrossPlotTools::depthTypeAbbreviation( m_depthType() ) ) );
    depthGroup->add( &m_filterMode );
    depthGroup->add( &m_depthRangeMin );
    depthGroup->add( &m_depthRangeMax );
    depthGroup->add( &m_wellFormations );
    depthGroup->add( &m_selectedZones );

    const bool useRange = m_filterMode() == RimRftCrossPlotTools::DepthFilterMode::DEPTH_RANGE;
    const bool useZones = m_filterMode() == RimRftCrossPlotTools::DepthFilterMode::ZONES;
    m_depthRangeMin.uiCapability()->setUiHidden( !useRange );
    m_depthRangeMax.uiCapability()->setUiHidden( !useRange );
    m_wellFormations.uiCapability()->setUiHidden( !useZones );
    m_selectedZones.uiCapability()->setUiHidden( !useZones || !m_wellFormations() );
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

    auto* crossPlotGroup = uiOrdering.addNewGroup( "Cross Plot Parameter" );
    crossPlotGroup->add( &m_ensembleParameter );
    crossPlotGroup->add( &m_samplingMode );

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

    if ( changedField == &m_wellName || changedField == &m_wellFormations )
    {
        // The selected zones belong to the previous well/formations file; clear them so stale zone
        // names are not silently applied as a filter.
        m_selectedZones = std::vector<QString>();
    }

    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    // The parent report plot syncs the other sub plots and the track zone highlight from this plot
    if ( auto* reportPlot = firstAncestorOrThisOfType<RimRftCorrelationReportPlot>() )
        reportPlot->loadDataAndUpdate();
    else
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
    else if ( fieldNeedingOptions == &m_wellFormations )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", static_cast<RimWellFormationsFile*>( nullptr ) ) );
        auto* project = RimProject::current();
        if ( project && project->activeOilField() && project->activeOilField()->wellFormationsCollection() )
        {
            for ( RimWellFormationsFile* file : project->activeOilField()->wellFormationsCollection()->wellFormationsFiles() )
                options.push_back( caf::PdmOptionItemInfo( file->shortName(), file, false, file->uiCapability()->uiIconProvider() ) );
        }
    }
    else if ( fieldNeedingOptions == &m_filterMode )
    {
        using DepthFilterMode = RimRftCrossPlotTools::DepthFilterMode;
        for ( auto mode : { DepthFilterMode::NONE, DepthFilterMode::DEPTH_RANGE, DepthFilterMode::ZONES } )
        {
            options.push_back( caf::PdmOptionItemInfo( RimRftCrossPlotTools::DepthFilterModeEnum::uiText( mode ), mode ) );
        }
    }
    else if ( fieldNeedingOptions == &m_selectedZones )
    {
        if ( m_wellFormations() )
        {
            for ( const QString& zoneName : m_wellFormations->zoneNames( m_wellName() ) )
                options.push_back( caf::PdmOptionItemInfo( zoneName, zoneName ) );
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
                pressurePerCase = RimRftCrossPlotTools::computeMeanPressurePerCase( m_ensemble(),
                                                                                    m_wellName(),
                                                                                    m_selectedTimeStep(),
                                                                                    m_eclipseCase(),
                                                                                    depthIntervals(),
                                                                                    m_depthType() );
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

    // Group points by summary case so "All Samples" mode draws one curve (and color) per
    // realization, with all its samples, instead of one curve per individual sample.
    std::vector<RimSummaryCase*>                                                   caseOrder;
    std::map<RimSummaryCase*, std::pair<std::vector<double>, std::vector<double>>> pointsPerCase;
    for ( const auto& [paramValue, pressureValue, summaryCase] : caseData )
    {
        auto it = pointsPerCase.find( summaryCase );
        if ( it == pointsPerCase.end() )
        {
            caseOrder.push_back( summaryCase );
            it = pointsPerCase.emplace( summaryCase, std::make_pair( std::vector<double>{}, std::vector<double>{} ) ).first;
        }
        it->second.first.push_back( paramValue );
        it->second.second.push_back( pressureValue );
    }

    int idx = 0;
    for ( RimSummaryCase* summaryCase : caseOrder )
    {
        const auto& [xValues, yValues] = pointsPerCase[summaryCase];

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

    attachObservedPressure();
}

//--------------------------------------------------------------------------------------------------
/// Draws every observed pressure within the selected depth intervals as a solid horizontal line,
/// with dashed lines and a shaded band at +/- the observed error. Only the first line is labelled.
//--------------------------------------------------------------------------------------------------
void RimParameterRftCrossPlot::attachObservedPressure()
{
    const auto observedPressures =
        RimRftCrossPlotTools::computeObservedPressures( m_wellName(),
                                                        m_selectedTimeStep(),
                                                        depthIntervals(),
                                                        m_depthType(),
                                                        RimRftCrossPlotTools::buildAllZoneIntervals( m_wellFormations(),
                                                                                                     m_wellName(),
                                                                                                     m_depthType() ) );
    if ( observedPressures.empty() ) return;

    auto attachLine = [this]( double value, Qt::PenStyle style, const QString& label, const QColor& color )
    {
        auto* marker = new QwtPlotMarker();
        marker->setLineStyle( QwtPlotMarker::HLine );
        marker->setYValue( value );
        QPen pen( color );
        pen.setStyle( style );
        pen.setWidth( 1 );
        marker->setLinePen( pen );

        if ( !label.isEmpty() )
        {
            QwtText text( label );
            text.setColor( Qt::black );
            marker->setLabel( text );
            marker->setLabelAlignment( Qt::AlignTop | Qt::AlignLeft );
        }

        marker->setZ( 1000.0 );
        marker->attach( m_plotWidget->qwtPlot() );
    };

    bool isFirst = true;
    for ( const auto& observed : observedPressures )
    {
        // Use the formation color when known; otherwise fall back to black lines and no shaded band
        std::optional<QColor> zoneColor;
        if ( auto it = m_zoneColors.find( observed.zoneName ); it != m_zoneColors.end() && it->second.isValid() ) zoneColor = it->second;
        QColor lineColor = zoneColor ? *zoneColor : QColor( Qt::black );
        lineColor.setAlpha( 255 );

        QString label;
        if ( observed.count > 1 )
            label = QString( "Observed Pressure (average of %1 observations)" ).arg( observed.count );
        else if ( isFirst )
            label = "Observed Pressure";

        attachLine( observed.pressure, Qt::SolidLine, label, lineColor );
        isFirst = false;

        if ( observed.rangeMax <= observed.rangeMin ) continue;

        attachLine( observed.rangeMin, Qt::DashLine, "", lineColor );
        attachLine( observed.rangeMax, Qt::DashLine, "", lineColor );

        if ( !zoneColor ) continue;

        const QColor shadingColor = *zoneColor; // alpha matches the track's formation shading

        auto* shading = new QwtPlotZoneItem();
        shading->setOrientation( Qt::Horizontal );
        shading->setInterval( observed.rangeMin, observed.rangeMax );
        shading->setPen( shadingColor, 0.0, Qt::NoPen );
        shading->setBrush( QBrush( shadingColor ) );
        shading->setZ( 999.0 );
        shading->attach( m_plotWidget->qwtPlot() );
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
        const QString filterDescription =
            RimRftCrossPlotTools::depthFilterDescription( m_filterMode(), m_depthType(), m_depthRangeMin(), m_depthRangeMax(), m_selectedZones() );
        if ( !filterDescription.isEmpty() )
        {
            m_description =
                QString( "%1 vs RFT Pressure [%2], %3" ).arg( m_ensembleParameter() ).arg( filterDescription ).arg( m_ensemble->name() );
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

    for ( const auto& observed :
          RimRftCrossPlotTools::computeObservedPressures( m_wellName(), m_selectedTimeStep(), depthIntervals(), m_depthType() ) )
    {
        yMin = std::min( yMin, observed.rangeMin );
        yMax = std::max( yMax, observed.rangeMax );
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
