/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimWellRftPlot.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaPlotDefines.h"
#include "RiaQDateTimeTools.h"
#include "RiaSimWellBranchTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RifReaderEclipseRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RimDataSourceForRftPlt.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimObservedFmuRftData.h"
#include "RimPressureDepthData.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimReloadCaseTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"
#include "RimTools.h"
#include "RimWellLogChannel.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogLasFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"
#include "RimWellRftEnsembleCurveSet.h"

#include "RiuAbstractLegendFrame.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuQwtCurveSelectorFilter.h"

#include "RiuPlotCurve.h"
#include "RiuPlotItem.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmPointer.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafSelectionManager.h"

#include "qwt_plot.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <tuple>

CAF_PDM_SOURCE_INIT( RimWellRftPlot, "WellRftPlot" );

using ColorMode = RimEnsembleCurveSetColorManager::ColorMode;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char RimWellRftPlot::PLOT_NAME_QFORMAT_STRING[] = "RFT: %1";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftPlot::RimWellRftPlot()
    : RimWellLogPlot()
{
    CAF_PDM_InitObject( "RFT Plot", ":/RFTPlot16x16.png" );

    CAF_PDM_InitField( &m_showStatisticsCurves, "ShowStatisticsCurves", true, "Show Statistics Curves" );
    CAF_PDM_InitField( &m_showEnsembleCurves, "ShowEnsembleCurves", true, "Show Ensemble Curves" );
    CAF_PDM_InitField( &m_showErrorInObservedData, "ShowErrorObserved", true, "Show Observed Data Error" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogPlot_OBSOLETE, "WellLog", "Well Log" );
    m_wellLogPlot_OBSOLETE.xmlCapability()->setIOWritable( false );

    m_depthType = RiaDefines::DepthType::TRUE_VERTICAL_DEPTH;

    CAF_PDM_InitFieldNoDefault( &m_wellPathNameOrSimWellName, "WellName", "Well Name" );
    CAF_PDM_InitField( &m_branchIndex, "BranchIndex", 0, "Branch Index" );
    CAF_PDM_InitField( &m_branchDetection,
                       "BranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_selectedSources, "Sources", "Sources" );
    m_selectedSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSources.xmlCapability()->disableIO();
    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_selectedSourcesForIo, "SourcesForIo", "SourcesForIo" );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Time Steps" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
    m_selectedTimeSteps.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSets, "EnsembleCurveSets", "Ensemble Curve Sets" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSetEclipseCase,
                                "EclipseResultCase",
                                "Grid Model For MD",
                                "Grid model used to compute measured depth using well path geometry" );

    // TODO: may want to support TRUE_VERTICAL_DEPTH_RKB in the future
    // It was developed for regular well log plots and requires some more work for RFT plots.
    setAvailableDepthTypes( { RiaDefines::DepthType::MEASURED_DEPTH, RiaDefines::DepthType::TRUE_VERTICAL_DEPTH } );

    m_nameConfig->setCustomName( "RFT Plot" );
    setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );

    m_plotLegendsHorizontal = false;

    setPlotTitleVisible( true );

    setAsPlotMdiWindow();
    m_isOnLoad = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftPlot::~RimWellRftPlot()
{
    removeMdiWindowFromMdiArea();

    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::applyCurveAppearance( RimWellLogCurve* curve )
{
    applyCurveColor( curve );

    RiaRftPltCurveDefinition              curveDef  = RimWellPlotTools::curveDefFromCurve( curve );
    RiuQwtPlotCurveDefines::LineStyleEnum lineStyle = RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID;

    RiuPlotCurveSymbol::PointSymbolEnum currentSymbol = RiuPlotCurveSymbol::SYMBOL_NONE;
    if ( curveDef.address().sourceType() != RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT )
    {
        currentSymbol = m_timeStepSymbols[curveDef.timeStep()];
    }

    bool isObservedData = curveDef.address().sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE ||
                          curveDef.address().sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT;
    // Observed data
    lineStyle = isObservedData ? RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE : RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID;

    curve->setSymbol( currentSymbol );
    curve->setLineStyle( lineStyle );

    // Dim ensemble member curves that do not belong to the highlighted curve set.
    // Statistics curves (ENSEMBLE_RFT) are intentionally excluded.
    if ( m_highlightedCurveSet && curveDef.address().sourceType() != RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT )
    {
        auto curveSet = findEnsembleCurveSet( curveDef.address().ensemble() );
        if ( curveSet && curveSet != m_highlightedCurveSet ) curve->setCurveColorOpacity( 0.1f );
    }

    // Highlight the selected realization using a contrast color and raised z-order.
    // Other ensemble member curves have their z-order restored to normal so this does not
    // compete with the ensemble-level opacity-dim mechanism.
    // Statistics curves (ENSEMBLE_RFT) and standalone case curves are never changed.
    if ( curveDef.address().sourceType() == RifDataSourceForRftPlt::SourceType::SUMMARY_RFT && curveDef.address().ensemble() )
    {
        auto* rftCurve = dynamic_cast<RimWellLogRftCurve*>( curve );
        if ( rftCurve )
        {
            if ( m_selectedRealization && rftCurve->summaryCase() == m_selectedRealization )
            {
                curve->setColor( RiaColorTools::fromQColorTo3f( QColor( "#e8572a" ) ) );
                curve->setLineThickness( 3 );
                curve->setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_HIGHLIGHTED_CURVE ) );
            }
            else
            {
                curve->setLineThickness( 1 );
                curve->setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_CURVE ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::applyCurveColor( RimWellLogCurve* curve )
{
    cvf::Color3f color = findCurveColor( curve );

    curve->setColor( color );
    curve->setSymbolEdgeColor( color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateFormationsOnPlot() const
{
    if ( plotCount() > 0 )
    {
        RimProject*  proj     = RimProject::current();
        RimWellPath* wellPath = proj->wellPathByName( m_wellPathNameOrSimWellName );

        RimCase*         formationNamesCase = nullptr;
        RimWellLogTrack* track              = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( track )
        {
            formationNamesCase = track->formationCase();

            if ( !formationNamesCase )
            {
                /// Set default case. Todo : Use the first of the selected cases in the plot
                std::vector<RimCase*> cases = proj->allGridCases();
                if ( !cases.empty() )
                {
                    formationNamesCase = cases[0];
                }
            }

            if ( wellPath )
            {
                track->setAndUpdateWellPathFormationNamesData( formationNamesCase, wellPath );
            }
            else
            {
                track->setAndUpdateSimWellFormationNamesAndBranchData( formationNamesCase,
                                                                       associatedSimWellName(),
                                                                       m_branchIndex,
                                                                       m_branchDetection );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellRftPlot::associatedSimWellName() const
{
    return RimWellPlotTools::simWellName( m_wellPathNameOrSimWellName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setOrInitializeDataSources( const std::vector<RifDataSourceForRftPlt>& sourcesToSelect )
{
    std::map<QString, QStringList> wellSources = findWellSources();
    if ( m_wellPathNameOrSimWellName() == "None" && !wellSources.empty() )
    {
        m_wellPathNameOrSimWellName = wellSources.begin()->first;
    }

    std::vector<RifDataSourceForRftPlt> dataSources;
    const QString                       simWellName = associatedSimWellName();

    if ( !sourcesToSelect.empty() )
    {
        // If the selection is provided, use it directly
        dataSources = sourcesToSelect;
    }
    else
    {
        // If no selection is provided, build the selection based on available data sources

        for ( RimEclipseResultCase* const rftCase : RimWellPlotTools::rftCasesForWell( simWellName ) )
        {
            dataSources.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA, rftCase ) );
        }

        for ( RimEclipseResultCase* const gridCase : RimWellPlotTools::gridCasesForWell( simWellName ) )
        {
            dataSources.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA, gridCase ) );
        }

        for ( RimSummaryEnsemble* const ensemble : RimWellPlotTools::rftEnsemblesForWell( simWellName ) )
        {
            dataSources.push_back( RifDataSourceForRftPlt( ensemble ) );
        }

        for ( auto singleCase : RiaSummaryTools::singleTopLevelSummaryCases() )
        {
            dataSources.push_back( RifDataSourceForRftPlt( singleCase, nullptr, nullptr ) );
        }
    }

    std::vector<RimWellLogFile*> wellLogFiles = RimWellPlotTools::wellLogFilesContainingPressure( m_wellPathNameOrSimWellName );
    if ( !wellLogFiles.empty() )
    {
        for ( RimWellLogFile* const wellLogFile : wellLogFiles )
        {
            if ( auto wellLogLasFile = dynamic_cast<RimWellLogLasFile*>( wellLogFile ) )
            {
                dataSources.push_back( RifDataSourceForRftPlt( wellLogLasFile ) );
            }
        }
    }

    for ( RimObservedFmuRftData* const observedFmuRftData : RimWellPlotTools::observedFmuRftDataForWell( m_wellPathNameOrSimWellName ) )
    {
        dataSources.push_back( RifDataSourceForRftPlt( observedFmuRftData ) );
    }

    m_selectedSources = dataSources;

    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::rftPlotChannelTypes();

        auto relevantTimeSteps =
            RimWellPlotTools::calculateRelevantTimeStepsFromCases( m_wellPathNameOrSimWellName, m_selectedSources, channelTypesToUse );

        if ( !relevantTimeSteps.empty() )
        {
            std::vector<QDateTime> timeStepVector;

            // If we have RFT data from multiple sources, relevant time steps end up with a small number of time steps
            // If this is the case, preselect all time steps
            const size_t maxCountToPreSelect = 3;
            if ( relevantTimeSteps.size() <= maxCountToPreSelect )
            {
                for ( const auto& item : relevantTimeSteps )
                    timeStepVector.push_back( item.first );
            }
            else
            {
                // If only one RFT source is available, we might get a large number of time steps causing performance
                // issues Only select the first available time step
                timeStepVector.push_back( relevantTimeSteps.begin()->first );
            }

            m_selectedTimeSteps = timeStepVector;
        }
    }

    createEnsembleCurveSets();
    syncCurvesFromUiSelection();

    m_isInitialized = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateEditorsFromPreviousSelection()
{
    std::set<RifDataSourceForRftPlt> previousSources( m_selectedSources().begin(), m_selectedSources().end() );
    std::set<QDateTime>              previousTimeSteps( m_selectedTimeSteps().begin(), m_selectedTimeSteps().end() );

    m_selectedSources.v().clear();
    m_selectedTimeSteps.v().clear();

    auto dataSourceOptions = calculateValueOptions( &m_selectedSources );
    for ( const auto& dataSourceOption : dataSourceOptions )
    {
        if ( dataSourceOption.level() == 1 )
        {
            RifDataSourceForRftPlt dataSource = dataSourceOption.value().value<RifDataSourceForRftPlt>();
            if ( previousSources.count( dataSource ) )
            {
                m_selectedSources.v().push_back( dataSource );
            }
        }
    }

    // This has to happen after the m_selectedSources is filled
    // because the available time steps is dependent on the selected sources.
    auto timeStepOptions = calculateValueOptions( &m_selectedTimeSteps );
    for ( const auto& timeStepOption : timeStepOptions )
    {
        QDateTime timeStep = timeStepOption.value().toDateTime();
        if ( previousTimeSteps.count( timeStep ) )
        {
            m_selectedTimeSteps.v().push_back( timeStep );
        }
    }

    if ( m_selectedTimeSteps.v().empty() && !timeStepOptions.empty() )
    {
        QDateTime timeStep = timeStepOptions.first().value().toDateTime();
        m_selectedTimeSteps.v().push_back( timeStep );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setSelectedSourcesFromCurves()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2025.04.3" ) )
    {
        std::set<RifDataSourceForRftPlt>                      selectedSources;
        std::set<QDateTime>                                   selectedTimeSteps;
        std::map<QDateTime, std::set<RifDataSourceForRftPlt>> selectedTimeStepsMap;

        for ( const RiaRftPltCurveDefinition& curveDef : curveDefsFromCurves() )
        {
            if ( curveDef.address().sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE )
            {
                selectedSources.insert( RifDataSourceForRftPlt( curveDef.address().wellLogFile() ) );
            }
            else if ( ( curveDef.address().sourceType() == RifDataSourceForRftPlt::SourceType::SUMMARY_RFT ) && curveDef.address().ensemble() )
            {
                selectedSources.insert( RifDataSourceForRftPlt( curveDef.address().ensemble() ) );
            }
            else
                selectedSources.insert( curveDef.address() );

            auto newTimeStepMap = std::map<QDateTime, std::set<RifDataSourceForRftPlt>>{
                { curveDef.timeStep(), std::set<RifDataSourceForRftPlt>{ curveDef.address() } } };
            RimWellPlotTools::addTimeStepsToMap( selectedTimeStepsMap, newTimeStepMap );
            selectedTimeSteps.insert( curveDef.timeStep() );
        }

        // Storage of time steps to the project file was changed in 2025.04.3
        m_selectedSources   = std::vector<RifDataSourceForRftPlt>( selectedSources.begin(), selectedSources.end() );
        m_selectedTimeSteps = std::vector<QDateTime>( selectedTimeSteps.begin(), selectedTimeSteps.end() );

        return;
    }

    std::vector<RifDataSourceForRftPlt> selectedSources;
    for ( RimDataSourceForRftPlt* addr : m_selectedSourcesForIo )
    {
        selectedSources.push_back( addr->address() );
    }

    m_selectedSources = selectedSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::syncCurvesFromUiSelection()
{
    RimWellLogTrack* plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
    if ( !plotTrack ) return;

    std::set<RiaRftPltCurveDefinition> allCurveDefs    = selectedCurveDefs();
    std::set<RiaRftPltCurveDefinition> curveDefsInPlot = curveDefsFromCurves();

    std::set<RimWellLogCurve*>         curvesToDelete;
    std::set<RiaRftPltCurveDefinition> newCurveDefs;

    if ( allCurveDefs.size() < curveDefsInPlot.size() )
    {
        // Determine which curves to delete from plot
        std::set<RiaRftPltCurveDefinition> deleteCurveDefs;

        std::set_difference( curveDefsInPlot.begin(),
                             curveDefsInPlot.end(),
                             allCurveDefs.begin(),
                             allCurveDefs.end(),
                             std::inserter( deleteCurveDefs, deleteCurveDefs.end() ) );

        for ( RimWellLogCurve* const curve : plotTrack->curves() )
        {
            RiaRftPltCurveDefinition curveDef = RimWellPlotTools::curveDefFromCurve( curve );
            if ( deleteCurveDefs.count( curveDef ) > 0 )
            {
                curvesToDelete.insert( curve );
            }
        }
    }
    else
    {
        // Determine which curves are new since last time
        std::set_difference( allCurveDefs.begin(),
                             allCurveDefs.end(),
                             curveDefsInPlot.begin(),
                             curveDefsInPlot.end(),
                             std::inserter( newCurveDefs, newCurveDefs.end() ) );
    }

    updateCurvesInPlot( allCurveDefs, newCurveDefs, curvesToDelete );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaRftPltCurveDefinition> RimWellRftPlot::selectedCurveDefs() const
{
    std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::rftPlotChannelTypes();

    return RimWellPlotTools::curveDefsFromTimesteps( m_wellPathNameOrSimWellName,
                                                     m_selectedTimeSteps.v(),
                                                     true,
                                                     selectedSourcesExpanded(),
                                                     channelTypesToUse );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaRftPltCurveDefinition> RimWellRftPlot::curveDefsFromCurves() const
{
    std::set<RiaRftPltCurveDefinition> curveDefs;

    RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
    if ( plotTrack )
    {
        for ( RimWellLogCurve* const curve : plotTrack->curves() )
        {
            curveDefs.insert( RimWellPlotTools::curveDefFromCurve( curve ) );
        }
    }
    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateCurvesInPlot( const std::set<RiaRftPltCurveDefinition>& allCurveDefs,
                                         const std::set<RiaRftPltCurveDefinition>& curveDefsToAdd,
                                         const std::set<RimWellLogCurve*>&         curvesToDelete )
{
    const QString simWellName = associatedSimWellName();

    RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
    if ( !plotTrack ) return;

    if ( plotTrack->plotWidget() )
    {
        detachAndDeleteLegendCurves();
    }

    // Delete curves
    plotTrack->deleteAllCurves();

    defineCurveColorsAndSymbols( allCurveDefs );

    // Sort legend items by project tree order (index in m_ensembleCurveSets), then by date/time
    auto curveSetsForLegendCmp = [this]( const std::pair<RimWellRftEnsembleCurveSet*, QDateTime>& a,
                                         const std::pair<RimWellRftEnsembleCurveSet*, QDateTime>& b ) -> bool
    {
        auto ia = m_ensembleCurveSets.indexOf( a.first );
        auto ib = m_ensembleCurveSets.indexOf( b.first );
        if ( ia != ib ) return ia < ib;
        return a.second < b.second;
    };
    std::set<std::pair<RimWellRftEnsembleCurveSet*, QDateTime>, decltype( curveSetsForLegendCmp )> curveSetsForLegend( curveSetsForLegendCmp );

    // Add new curves
    for ( const RiaRftPltCurveDefinition& curveDefToAdd : allCurveDefs )
    {
        if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA )
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve( curve );

            auto rftCase = curveDefToAdd.address().eclCase();
            curve->setEclipseCase( rftCase );

            RifEclipseRftAddress address = RifEclipseRftAddress::createAddress( simWellName,
                                                                                curveDefToAdd.timeStep(),
                                                                                RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
            curve->setRftAddress( address );
            curve->setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_NON_OBSERVED ) );
            curve->setSimWellBranchData( m_branchDetection, m_branchIndex );

            applyCurveAppearance( curve );
        }
        else if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT )
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve( curve );

            if ( auto observedFmuRftData = curveDefToAdd.address().observedFmuRftData() )
            {
                curve->setErrorBarsVisible( m_showErrorInObservedData );
                curve->setObservedFmuRftData( observedFmuRftData );
            }
            else if ( auto pressureDepthData = curveDefToAdd.address().pressureDepthData() )
            {
                curve->setPressureDepthData( pressureDepthData );
            }

            RifEclipseRftAddress address = RifEclipseRftAddress::createAddress( m_wellPathNameOrSimWellName,
                                                                                curveDefToAdd.timeStep(),
                                                                                RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
            curve->setRftAddress( address );
            curve->setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_OBSERVED ) );
            applyCurveAppearance( curve );
        }
        else if ( ( !curveDefToAdd.address().ensemble() || m_showEnsembleCurves ) &&
                  curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SourceType::SUMMARY_RFT )
        {
            // A summary case address can optionally contain an Eclipse case used to compute the TVD/MD for a well path
            // https://github.com/OPM/ResInsight/issues/10501
            auto eclipeCase = curveDefToAdd.address().eclCase();
            if ( curveDefToAdd.address().ensemble() )
            {
                auto curveSet = findEnsembleCurveSet( curveDefToAdd.address().ensemble() );
                if ( curveSet )
                {
                    eclipeCase = curveSet->eclipseCase();
                    curveSetsForLegend.insert( { curveSet, curveDefToAdd.timeStep() } );
                }
            }

            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve( curve );
            auto summaryCase = curveDefToAdd.address().summaryCase();
            curve->setSummaryCase( summaryCase );
            curve->setEnsemble( curveDefToAdd.address().ensemble() );
            curve->setObservedFmuRftData( findObservedFmuData( m_wellPathNameOrSimWellName, curveDefToAdd.timeStep() ) );
            RifEclipseRftAddress address = RifEclipseRftAddress::createAddress( m_wellPathNameOrSimWellName,
                                                                                curveDefToAdd.timeStep(),
                                                                                RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
            curve->setRftAddress( address );
            curve->setEclipseCase( eclipeCase );

            double zValue = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_CURVE );
            if ( !curveDefToAdd.address().ensemble() )
            {
                zValue = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_NON_OBSERVED );
            }
            curve->setZOrder( zValue );

            applyCurveAppearance( curve );

            if ( curveDefToAdd.address().ensemble() )
            {
                curve->setShowInLegend( false );
            }
        }
        else if ( m_showStatisticsCurves && curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT )
        {
            RimSummaryEnsemble* ensemble = curveDefToAdd.address().ensemble();

            if ( auto curveSet = findEnsembleCurveSet( ensemble ) )
            {
                std::set<RifEclipseRftAddress> rftAddresses =
                    curveSet->statisticsEclipseRftReader()->eclipseRftAddresses( m_wellPathNameOrSimWellName, curveDefToAdd.timeStep() );
                for ( const auto& rftAddress : rftAddresses )
                {
                    if ( rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P50 )
                    {
                        // Default statistics curves are P10, P50, P90 and mean
                        // It is not common to use P50 for ensemble RFT, so skip display of P50 to avoid confusion with mean
                        // https://github.com/OPM/ResInsight/issues/5238

                        continue;
                    }

                    if ( rftAddress.wellLogChannel() != RifEclipseRftAddress::RftWellLogChannelType::TVD )
                    {
                        auto curve = new RimWellLogRftCurve();
                        plotTrack->addCurve( curve );
                        curve->setEnsemble( ensemble );
                        curve->setEclipseCase( curveSet->eclipseCase() );
                        curve->setRftAddress( rftAddress );
                        curve->setObservedFmuRftData( findObservedFmuData( m_wellPathNameOrSimWellName, curveDefToAdd.timeStep() ) );
                        curve->setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_STAT_CURVE ) );
                        applyCurveAppearance( curve );

                        auto symbol = m_timeStepSymbols[curveDefToAdd.timeStep()];
                        curve->setSymbol( symbol );
                        auto size = curve->symbolSize();
                        curve->setSymbolSize( size + 2 );
                        curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
                        curve->setLineThickness( 3 );
                        curve->setShowInLegend( false );

                        curveSetsForLegend.insert( { curveSet, curveDefToAdd.timeStep() } );
                    }
                }
            }
        }

        else if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA )
        {
            auto curve = new RimWellLogExtractionCurve();
            plotTrack->addCurve( curve );

            cvf::Color3f curveColor = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f( plotTrack->curveCount() );
            curve->setColor( curveColor );
            curve->setFromSimulationWellName( simWellName, m_branchIndex, m_branchDetection );

            // Fetch cases and time steps
            auto gridCase = curveDefToAdd.address().eclCase();
            if ( gridCase != nullptr )
            {
                // Case
                curve->setCase( gridCase );
                curve->setEclipseResultVariable( "PRESSURE" );

                // Time step

                std::vector<QDateTime> timeSteps =
                    gridCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->timeStepDates();
                int currentTimeStepIndex = -1;
                for ( size_t tsIdx = 0; tsIdx < timeSteps.size(); ++tsIdx )
                {
                    if ( timeSteps[tsIdx] == curveDefToAdd.timeStep() )
                    {
                        currentTimeStepIndex = static_cast<int>( tsIdx );
                        break;
                    }
                }

                curve->setCurrentTimeStep( currentTimeStepIndex );
                curve->setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_CURVE ) );

                applyCurveAppearance( curve );
            }
        }
        else if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE )
        {
            RimWellLogLasFile* const wellLogFile = curveDefToAdd.address().wellLogFile();
            RimWellPath* const       wellPath    = RimWellPlotTools::wellPathFromWellLogFile( wellLogFile );
            if ( wellLogFile != nullptr )
            {
                RimWellLogChannel* pressureChannel = RimWellPlotTools::getPressureChannelFromWellFile( wellLogFile );
                auto               curve           = new RimWellLogLasFileCurve();

                plotTrack->addCurve( curve );
                curve->setWellPath( wellPath );
                curve->setWellLog( wellLogFile );
                curve->setWellLogChannelName( pressureChannel->name() );
                curve->setZOrder( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_NON_OBSERVED ) );

                applyCurveAppearance( curve );
            }
        }
    }

    if ( auto qwtWidget = dynamic_cast<RiuQwtPlotWidget*>( plotTrack->plotWidget() ) )
    {
        // Install click-to-select filter once (guard against repeated calls to updateCurvesInPlot)
        auto*       canvas         = qwtWidget->qwtPlot()->canvas();
        const char* filterPropName = "rftCurveSelectorInstalled";
        if ( !canvas->property( filterPropName ).toBool() )
        {
            caf::PdmPointer<RimWellRftPlot> self( this );
            new RiuQwtCurveSelectorFilter( qwtWidget->qwtPlot(),
                                           [self]( const QPoint& pos ) -> const caf::PdmUiItem*
                                           { return self ? self->findClosestRealization( pos ) : nullptr; } );
            canvas->setProperty( filterPropName, true );
        }

        // Connect legend item clicks to select the corresponding ensemble curve set in the project tree.
        // Disconnect previous connection first to avoid duplicates (non-QObject lambda connections).
        // Use a guarded PdmPointer to avoid use-after-free if the signal fires after this object is destroyed.
        QObject::disconnect( m_legendClickedConnection );
        caf::PdmPointer<RimWellRftPlot> self( this );
        m_legendClickedConnection = QObject::connect( qwtWidget,
                                                      &RiuQwtPlotWidget::plotItemSelected,
                                                      [self]( std::shared_ptr<RiuPlotItem> item, bool toggle, int idx )
                                                      {
                                                          if ( self ) self->onLegendItemClicked( item, toggle, idx );
                                                      } );

        // Create curves with no content to display in the curve legend section. Ensures a consistent legend for both ensemble and
        // statistics curves.

        auto formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( m_selectedTimeSteps() );
        for ( const auto& [curveSet, dateTime] : curveSetsForLegend )
        {
            auto riuCurve = qwtWidget->createPlotCurve( nullptr, "" );
            riuCurve->attachToPlot( qwtWidget );
            riuCurve->setVisibleInLegend( true );

            QStringList titleItems;
            titleItems.push_back( curveSet->ensembleName() );
            titleItems.push_back( dateTime.toString( formatString ) );
            titleItems.push_back( m_wellPathNameOrSimWellName );
            QString title = titleItems.join( ", " );
            riuCurve->setTitle( title );

            auto color          = RiaColorTools::toQColor( curveSet->curveColor( nullptr, nullptr ) );
            int  curveThickness = 3;
            riuCurve->setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID,
                                     RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT,
                                     curveThickness,
                                     color );

            auto symbolType = m_timeStepSymbols[dateTime];
            auto symbol     = riuCurve->createSymbol( symbolType );
            symbol->setSize( 8, 8 );
            symbol->setColor( color );

            riuCurve->setSymbol( symbol );

            m_legendPlotCurves.push_back( riuCurve );
            m_legendCurveToEnsembleCurveSet[riuCurve] = curveSet;
        }
    }

    if ( depthType() == RiaDefines::DepthType::MEASURED_DEPTH )
    {
        assignWellPathToExtractionCurves();
    }

    RimWellLogPlot::onLoadDataAndUpdate();
    if ( plotTrack->curveCount() )
    {
        zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifDataSourceForRftPlt> RimWellRftPlot::selectedSourcesExpanded() const
{
    std::vector<RifDataSourceForRftPlt> sources;
    for ( const RifDataSourceForRftPlt& addr : m_selectedSources() )
    {
        if ( addr.sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE )
        {
            for ( RimWellLogFile* const wellLogFile : RimWellPlotTools::wellLogFilesContainingPressure( m_wellPathNameOrSimWellName ) )
            {
                if ( auto wellLogLasFile = dynamic_cast<RimWellLogLasFile*>( wellLogFile ) )
                {
                    sources.push_back( RifDataSourceForRftPlt( wellLogLasFile ) );
                }
            }
        }
        else
            sources.push_back( addr );
    }
    return sources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimWellRftPlot::simWellOrWellPathName() const
{
    return m_wellPathNameOrSimWellName.v();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setSimWellOrWellPathName( const QString& currWellName )
{
    m_wellPathNameOrSimWellName = currWellName;
    if ( m_wellPathNameOrSimWellName().isEmpty() )
    {
        m_wellPathNameOrSimWellName = "None";
    }
    m_nameConfig->setCustomName( QString( plotNameFormatString() ).arg( m_wellPathNameOrSimWellName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellRftPlot::branchIndex() const
{
    return m_branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char* RimWellRftPlot::plotNameFormatString()
{
    return PLOT_NAME_QFORMAT_STRING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::deleteCurvesAssosicatedWithObservedData( const RimObservedFmuRftData* observedFmuRftData )
{
    for ( auto plot : plots() )
    {
        RimWellLogTrack* const track = dynamic_cast<RimWellLogTrack*>( plot );
        if ( track )
        {
            auto curves = track->curves();
            for ( auto curve : curves )
            {
                RimWellLogRftCurve* rftCurve = dynamic_cast<RimWellLogRftCurve*>( curve );
                if ( rftCurve && rftCurve->observedFmuRftData() == observedFmuRftData )
                {
                    track->removeCurve( rftCurve );
                    delete rftCurve;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellRftPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimWellLogPlot::calculateValueOptions( fieldNeedingOptions );

    const QString simWellName = associatedSimWellName();

    if ( fieldNeedingOptions == &m_wellPathNameOrSimWellName )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );

        std::map<QString, QStringList> wellSources = findWellSources();
        for ( const auto& wellName : wellSources )
        {
            const QStringList& tags   = wellName.second;
            QString            uiText = wellName.first;
            if ( !tags.empty() )
            {
                uiText += QString( " (%1)" ).arg( wellName.second.join( ", " ) );
            }

            options.push_back( caf::PdmOptionItemInfo( uiText, wellName.first ) );
        }
    }
    else if ( fieldNeedingOptions == &m_selectedSources )
    {
        options = calculateValueOptionsForSources();
    }
    else if ( fieldNeedingOptions == &m_selectedTimeSteps )
    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::rftPlotChannelTypes();

        RimWellPlotTools::calculateValueOptionsForTimeSteps( m_wellPathNameOrSimWellName, selectedSourcesExpanded(), channelTypesToUse, options );
    }
    else if ( fieldNeedingOptions == &m_branchIndex )
    {
        auto simulationWellBranches = RiaSimWellBranchTools::simulationWellBranches( simWellName, m_branchDetection );

        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField( simulationWellBranches );
    }
    else if ( fieldNeedingOptions == &m_ensembleCurveSetEclipseCase )
    {
        RimTools::caseOptionItems( &options );

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellRftPlot::calculateValueOptionsForSources() const
{
    QList<caf::PdmOptionItemInfo> options;

    const QString simWellName = associatedSimWellName();

    const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell( simWellName );
    if ( !rftCases.empty() )
    {
        options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                     RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA ),
                                                                 true ) );

        for ( const auto& rftCase : rftCases )
        {
            auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA, rftCase );
            auto item = caf::PdmOptionItemInfo( rftCase->caseUserDescription(), QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }
    }

    const std::vector<RimSummaryEnsemble*> rftEnsembles = RimWellPlotTools::rftEnsemblesForWell( m_wellPathNameOrSimWellName );
    if ( !rftEnsembles.empty() )
    {
        options.push_back(
            caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText( RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT ),
                                                  true ) );

        for ( RimSummaryEnsemble* rftEnsemble : rftEnsembles )
        {
            auto addr = RifDataSourceForRftPlt( rftEnsemble );
            auto item = caf::PdmOptionItemInfo( rftEnsemble->name(), QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }
    }

    auto singleCases = RiaSummaryTools::singleTopLevelSummaryCases();
    if ( !singleCases.empty() )
    {
        options.push_back(
            caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText( RifDataSourceForRftPlt::SourceType::SUMMARY_RFT ),
                                                  true ) );
        for ( auto summaryCase : singleCases )
        {
            if ( summaryCase->rftReader() && summaryCase->rftReader()->wellNames().contains( m_wellPathNameOrSimWellName ) )
            {
                auto eclipeGridModel = RimReloadCaseTools::gridModelFromSummaryCase( summaryCase );
                auto parentEnsemble  = summaryCase->firstAncestorOrThisOfType<RimSummaryEnsemble>();
                auto addr            = RifDataSourceForRftPlt( summaryCase, parentEnsemble, eclipeGridModel );

                auto item = caf::PdmOptionItemInfo( summaryCase->displayCaseName(), QVariant::fromValue( addr ) );
                item.setLevel( 1 );
                options.push_back( item );
            }
        }
    }

    const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell( simWellName );
    if ( !gridCases.empty() )
    {
        options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                     RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA ),
                                                                 true ) );

        for ( const auto& gridCase : gridCases )
        {
            auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA, gridCase );
            auto item = caf::PdmOptionItemInfo( gridCase->caseUserDescription(), QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }
    }

    auto wellLogFiles = RimWellPlotTools::wellLogFilesContainingPressure( m_wellPathNameOrSimWellName );
    if ( !wellLogFiles.empty() )
    {
        options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                     RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE ),
                                                                 true ) );

        for ( const auto& wellLogFile : wellLogFiles )
        {
            if ( auto wellLogLasFile = dynamic_cast<RimWellLogLasFile*>( wellLogFile ) )
            {
                auto addr = RifDataSourceForRftPlt( wellLogLasFile );
                auto item = caf::PdmOptionItemInfo( "Observed Data", QVariant::fromValue( addr ) );
                item.setLevel( 1 );
                options.push_back( item );
            }
        }
    }
    const std::vector<RimObservedFmuRftData*> observedFmuRftCases = RimWellPlotTools::observedFmuRftDataForWell( m_wellPathNameOrSimWellName );
    if ( !observedFmuRftCases.empty() )
    {
        options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                     RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT ),
                                                                 true ) );

        for ( const auto& observedFmuRftCase : observedFmuRftCases )
        {
            auto addr = RifDataSourceForRftPlt( observedFmuRftCase );
            auto item = caf::PdmOptionItemInfo( observedFmuRftCase->name(), QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }
    }
    const std::vector<RimPressureDepthData*> pressureDepthData = RimWellPlotTools::pressureDepthDataForWell( m_wellPathNameOrSimWellName );
    if ( !pressureDepthData.empty() )
    {
        options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                     RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT ),
                                                                 true ) );

        for ( const auto& pd : pressureDepthData )
        {
            auto addr = RifDataSourceForRftPlt( pd );
            auto item = caf::PdmOptionItemInfo( pd->name(), QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimWellLogPlot::fieldChangedByUi( changedField, oldValue, newValue );

    bool rebuildEnsembleCurveSets = false;
    bool updateEditors            = false;

    if ( changedField == &m_wellPathNameOrSimWellName )
    {
        m_nameConfig->setCustomName( QString( plotNameFormatString() ).arg( m_wellPathNameOrSimWellName ) );
        m_branchIndex = 0;

        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( plotTrack )
        {
            plotTrack->deleteAllCurves();
        }
        updateEditorsFromPreviousSelection();
        rebuildEnsembleCurveSets = true;
    }
    else if ( changedField == &m_branchIndex || changedField == &m_branchDetection )
    {
        const QString simWellName = associatedSimWellName();
        m_branchIndex             = RiaSimWellBranchTools::clampBranchIndex( simWellName, m_branchIndex, m_branchDetection );
        rebuildEnsembleCurveSets  = true;
    }
    else if ( changedField == &m_selectedSources || changedField == &m_selectedTimeSteps )
    {
        rebuildEnsembleCurveSets = true;
        updateEditors            = true;
    }
    else if ( changedField == &m_showStatisticsCurves || changedField == &m_showEnsembleCurves || changedField == &m_showErrorInObservedData )
    {
        // No extra action needed — handled by the common tail below.
    }
    else if ( changedField == &m_ensembleCurveSetEclipseCase )
    {
        for ( RimWellRftEnsembleCurveSet* curveSet : m_ensembleCurveSets() )
        {
            curveSet->setEclipseCase( m_ensembleCurveSetEclipseCase );
        }
        rebuildEnsembleCurveSets = true;
    }
    else
    {
        return;
    }

    if ( rebuildEnsembleCurveSets )
    {
        createEnsembleCurveSets();
    }
    updateFormationsOnPlot();
    syncCurvesFromUiSelection();

    if ( updateEditors )
    {
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( RimWellRftEnsembleCurveSet* curveSet : m_ensembleCurveSets() )
    {
        bool isSelected = false;
        for ( RimSummaryEnsemble* selectedCurveSet : selectedEnsembles() )
        {
            if ( curveSet->ensemble() == selectedCurveSet )
            {
                isSelected = true;
                break;
            }
        }
        if ( isSelected )
        {
            uiTreeOrdering.add( curveSet );
        }
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_wellPathNameOrSimWellName );
    uiOrdering.add( &m_showStatisticsCurves );
    uiOrdering.add( &m_showEnsembleCurves );
    uiOrdering.add( &m_showErrorInObservedData );

    bool ensembleDataSelected = !selectedEnsembles().empty();
    m_showStatisticsCurves.uiCapability()->setUiReadOnly( !ensembleDataSelected );
    m_showEnsembleCurves.uiCapability()->setUiReadOnly( !ensembleDataSelected );

    RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromWellName( &uiOrdering,
                                                                            m_wellPathNameOrSimWellName,
                                                                            m_branchDetection,
                                                                            m_branchIndex );

    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword( "Sources", "Sources" );
    sourcesGroup->add( &m_selectedSources );

    if ( !m_ensembleCurveSets.empty() )
    {
        uiOrdering.add( &m_ensembleCurveSetEclipseCase );
    }

    caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroupWithKeyword( "Time Steps", "TimeSteps" );
    timeStepsGroup->add( &m_selectedTimeSteps );

    if ( plotCount() > 0 )
    {
        RimWellLogTrack* const track = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( track )
        {
            track->uiOrderingForRftPltFormations( uiOrdering );
            track->uiOrderingForPropertyAxisSettings( uiOrdering );
            caf::PdmUiGroup* depthGroup = uiOrdering.addNewGroup( "Depth Axis Settings" );
            uiOrderingForDepthAxis( uiConfigName, *depthGroup );

            caf::PdmUiGroup* plotLayoutGroup = uiOrdering.addNewGroup( "Plot Layout" );
            plotLayoutGroup->setCollapsedByDefault();
            RimWellLogPlot::uiOrderingForAutoName( uiConfigName, *plotLayoutGroup );
            RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );
            plotLayoutGroup->add( &m_depthOrientation );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QStringList> RimWellRftPlot::findWellSources()
{
    std::map<QString /*value*/, QStringList /*uitext*/> wellNames;

    RimProject* proj = RimProject::current();

    if ( proj != nullptr )
    {
        const std::vector<QString> simWellNames = proj->simulationWellNames();
        std::set<QString>          simWellsAssociatedWithWellPath;

        // Observed wells
        for ( RimWellPath* const wellPath : proj->allWellPaths() )
        {
            wellNames[wellPath->name()].push_back( "Well Path" );

            if ( !wellPath->associatedSimulationWellName().isEmpty() )
            {
                simWellsAssociatedWithWellPath.insert( wellPath->associatedSimulationWellName() );
            }
        }

        // Sim wells not associated with well path
        for ( const QString& simWellName : simWellNames )
        {
            if ( simWellsAssociatedWithWellPath.count( simWellName ) == 0 )
            {
                wellNames[simWellName].push_back( "Sim.Well" );
            }
        }

        auto singleCases = RiaSummaryTools::singleTopLevelSummaryCases();
        for ( auto summaryCase : singleCases )
        {
            if ( auto rftReader = summaryCase->rftReader() )
            {
                for ( const QString& wellName : rftReader->wellNames() )
                {
                    wellNames[wellName].push_back( "Summary" );
                }
            }
        }

        const std::vector<RimSummaryEnsemble*> rftEnsembles = RimWellPlotTools::rftEnsembles();
        // Ensemble RFT wells
        {
            for ( RimSummaryEnsemble* summaryCaseColl : rftEnsembles )
            {
                std::set<QString> wellsWithRftData = RimSummaryEnsembleTools::wellsWithRftData( summaryCaseColl->allSummaryCases() );
                for ( const QString& wellName : wellsWithRftData )
                {
                    wellNames[wellName].push_back( "Ensemble" );
                }
            }
        }

        // Observed FMU RFT wells
        const std::vector<RimObservedFmuRftData*> allRftFmuData = RimWellPlotTools::observedFmuRftData();
        if ( !allRftFmuData.empty() )
        {
            for ( RimObservedFmuRftData* rftFmuData : allRftFmuData )
            {
                for ( const QString& wellName : rftFmuData->wells() )
                {
                    wellNames[wellName].push_back( "Observed" );
                }
            }
        }

        const std::vector<RimPressureDepthData*> pressureDepthData = RimWellPlotTools::pressureDepthData();
        for ( const auto& pd : pressureDepthData )
        {
            for ( const auto& wellName : pd->wellNames() )
            {
                wellNames[wellName].push_back( "Observed" );
            }
        }
    }
    return wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::onLoadDataAndUpdate()
{
    if ( !m_isInitialized )
    {
        // TODO: m_selectedSources and m_selectedTimeSteps are not stored in the project file, so we need to set them here
        // to ensure that the plot is initialized with the correct sources and time steps. Refactor m_selectedSources and
        // m_selectedTimeSteps to be stored in the project file, and disable storing of curves to the project file. This is a temporary
        // solution to ensure that the plot is initialized correctly setSelectedSourcesFromCurves();
        //
        // This function call was previously in initAfterRead, and triggered opening of the grid model. Moved here to allow 3D views to
        // trigger open of grid model

        setSelectedSourcesFromCurves();
        m_isInitialized = true;
    }

    if ( m_isOnLoad )
    {
        if ( plotCount() > 0 )
        {
            RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
            if ( plotTrack )
            {
                plotTrack->setAnnotationType( RiaDefines::RegionAnnotationType::FORMATION_ANNOTATIONS );
            }
        }

        m_isOnLoad = false;
    }

    updateMdiWindowVisibility();
    updateFormationsOnPlot();

    if ( depthType() == RiaDefines::DepthType::MEASURED_DEPTH )
    {
        assignWellPathToExtractionCurves();
    }

    RimWellLogPlot::onLoadDataAndUpdate();
    createEnsembleCurveSets();

    // Update of curve color must happen here when loading data from project file, as the curve color is blended by
    // the background color. The background color is taken from the viewer.
    RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );

    if ( plotTrack && plotTrack->viewer() )
    {
        syncCurvesFromUiSelection();
        for ( auto c : plotTrack->curves() )
        {
            applyCurveColor( c );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setupBeforeSave()
{
    syncSourcesIoFieldFromGuiField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::initAfterRead()
{
    if ( m_wellLogPlot_OBSOLETE )
    {
        RimWellLogPlot& wellLogPlot = dynamic_cast<RimWellLogPlot&>( *this );
        wellLogPlot                 = std::move( *m_wellLogPlot_OBSOLETE.value() );
        delete m_wellLogPlot_OBSOLETE;
        m_wellLogPlot_OBSOLETE = nullptr;
    }

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2025.04.3" ) )
    {
        std::vector<cvf::Color3f> colorTable;
        RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector( &colorTable );

        size_t colorIndex = 0;
        for ( auto e : m_ensembleCurveSets )
        {
            e->setCurveColor( colorTable[colorIndex % colorTable.size()] );
        }
    }

    RimWellLogPlot::initAfterRead();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::showErrorBarsForObservedData() const
{
    return m_showErrorInObservedData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::onLegendDefinitionChanged()
{
    syncCurvesFromUiSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::useUndoRedoForFieldChanged()
{
    // m_selectedSources use data types that are not compatible with caf
    // consider rewrite to caf object using ptrfield instead of pdmpointer

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::deleteViewWidget()
{
    // Required to detach curves before view widget is deleted. The Qwt plot curves are implicitly deleted when the view widget is deleted.
    detachAndDeleteLegendCurves();

    // Disconnect before the widget is deleted to prevent the lambda from firing after this object is destroyed.
    QObject::disconnect( m_legendClickedConnection );
    m_legendClickedConnection = {};

    RimDepthTrackPlot::deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::assignWellPathToExtractionCurves()
{
    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathByName( m_wellPathNameOrSimWellName );

    if ( wellPath )
    {
        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( plotTrack )
        {
            for ( RimWellLogCurve* curve : plotTrack->curves() )
            {
                auto extractionCurve = dynamic_cast<RimWellLogExtractionCurve*>( curve );
                if ( extractionCurve )
                {
                    extractionCurve->setTrajectoryType( RimWellLogExtractionCurve::WELL_PATH );
                    extractionCurve->setWellPath( wellPath );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::syncSourcesIoFieldFromGuiField()
{
    m_selectedSourcesForIo.deleteChildren();

    for ( const RifDataSourceForRftPlt& addr : m_selectedSources() )
    {
        m_selectedSourcesForIo.push_back( new RimDataSourceForRftPlt( addr ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::PointSymbolEnum RimWellRftPlot::statisticsCurveSymbolFromAddress( const RifEclipseRftAddress& address )
{
    switch ( address.wellLogChannel() )
    {
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P10:
            return RiuPlotCurveSymbol::SYMBOL_TRIANGLE;
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P50:
            return RiuPlotCurveSymbol::SYMBOL_DOWN_TRIANGLE;
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P90:
            return RiuPlotCurveSymbol::SYMBOL_LEFT_TRIANGLE;
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_MEAN:
            return RiuPlotCurveSymbol::SYMBOL_RIGHT_TRIANGLE;
    }
    return RiuPlotCurveSymbol::SYMBOL_RIGHT_TRIANGLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::LabelPosition RimWellRftPlot::statisticsLabelPosFromAddress( const RifEclipseRftAddress& address )
{
    switch ( address.wellLogChannel() )
    {
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P10:
            return RiuPlotCurveSymbol::LabelLeftOfSymbol;
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P50:
            return RiuPlotCurveSymbol::LabelAboveSymbol;
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P90:
            return RiuPlotCurveSymbol::LabelRightOfSymbol;
        case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_MEAN:
            return RiuPlotCurveSymbol::LabelBelowSymbol;
    }
    return RiuPlotCurveSymbol::LabelAboveSymbol;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellRftPlot::findCurveColor( RimWellLogCurve* curve )
{
    RiaRftPltCurveDefinition curveDef = RimWellPlotTools::curveDefFromCurve( curve );

    if ( RimWellRftEnsembleCurveSet* ensembleCurveSet = findEnsembleCurveSet( curveDef.address().ensemble() ) )
    {
        return ensembleCurveSet->curveColor( curveDef.address().ensemble(), curveDef.address().summaryCase() );
    }

    return m_dataSourceColors[curveDef.address()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::defineCurveColorsAndSymbols( const std::set<RiaRftPltCurveDefinition>& allCurveDefs )
{
    // Clear all ensemble legends
    RiuQwtPlotWidget* viewer = nullptr;
    RimWellLogTrack*  track  = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
    if ( track ) viewer = track->viewer();

    for ( const auto& ensembleLegendPair : m_ensembleLegendFrames )
    {
        if ( viewer && ensembleLegendPair.second )
        {
            viewer->removeOverlayFrame( ensembleLegendPair.second );
            delete ensembleLegendPair.second;
        }
    }

    auto ensembles = selectedEnsembles();
    for ( RimWellRftEnsembleCurveSet* curveSet : m_ensembleCurveSets() )
    {
        CAF_ASSERT( curveSet );
        auto ensemble_it = std::find_if( ensembles.begin(),
                                         ensembles.end(),
                                         [&curveSet]( const RimSummaryEnsemble* ensemble ) { return curveSet->ensemble() == ensemble; } );
        if ( ensemble_it != ensembles.end() )
        {
            if ( viewer && curveSet->legendConfig() )
            {
                if ( !m_ensembleLegendFrames[curveSet] )
                {
                    auto m = new RiuDraggableOverlayFrame( viewer->getParentForOverlay(), viewer->overlayMargins() );
                    m->setContentFrame( curveSet->legendConfig()->makeLegendFrame() );

                    m_ensembleLegendFrames[curveSet] = m;
                }

                viewer->addOverlayFrame( m_ensembleLegendFrames[curveSet] );
            }
        }
    }

    std::vector<cvf::Color3f> colorTable;
    RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector( &colorTable );

    std::vector<RiuPlotCurveSymbol::PointSymbolEnum> symbolTable = { RiuPlotCurveSymbol::SYMBOL_ELLIPSE,
                                                                     RiuPlotCurveSymbol::SYMBOL_RECT,
                                                                     RiuPlotCurveSymbol::SYMBOL_DIAMOND,
                                                                     RiuPlotCurveSymbol::SYMBOL_CROSS,
                                                                     RiuPlotCurveSymbol::SYMBOL_XCROSS,
                                                                     RiuPlotCurveSymbol::SYMBOL_STAR1 };

    // Add new curves
    for ( const RiaRftPltCurveDefinition& curveDefToAdd : allCurveDefs )
    {
        auto colorTableIndex  = m_dataSourceColors.size();
        auto symbolTableIndex = m_timeStepSymbols.size();

        RifDataSourceForRftPlt address = curveDefToAdd.address();
        if ( address.ensemble() )
        {
            // Strip the summary case from the address, so that all curves in the ensemble will get the same color
            address = RifDataSourceForRftPlt( address.ensemble() );
        }

        if ( !m_dataSourceColors.contains( address ) )
        {
            colorTableIndex             = colorTableIndex % colorTable.size();
            m_dataSourceColors[address] = colorTable[colorTableIndex];
        }

        {
            if ( !m_timeStepSymbols.contains( curveDefToAdd.timeStep() ) )
            {
                symbolTableIndex                            = symbolTableIndex % symbolTable.size();
                m_timeStepSymbols[curveDefToAdd.timeStep()] = symbolTable[symbolTableIndex];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> RimWellRftPlot::selectedEnsembles() const
{
    std::vector<RimSummaryEnsemble*> ensembleSets;
    for ( const RifDataSourceForRftPlt& dataSource : m_selectedSources() )
    {
        if ( dataSource.ensemble() != nullptr )
        {
            ensembleSets.push_back( dataSource.ensemble() );
        }
    }
    return ensembleSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::createEnsembleCurveSets()
{
    const std::vector<RimSummaryEnsemble*> rftEnsembles = RimWellPlotTools::rftEnsemblesForWell( m_wellPathNameOrSimWellName );

    // First delete any curve sets not belonging to the given rftEnsembles
    std::vector<RimWellRftEnsembleCurveSet*> curveSetsToDelete;
    for ( RimWellRftEnsembleCurveSet* curveSet : m_ensembleCurveSets() )
    {
        if ( std::find( rftEnsembles.begin(), rftEnsembles.end(), curveSet->ensemble() ) == rftEnsembles.end() )
        {
            curveSetsToDelete.push_back( curveSet );
        }
    }

    for ( RimWellRftEnsembleCurveSet* curveSet : curveSetsToDelete )
    {
        m_ensembleCurveSets.removeChild( curveSet );
        delete curveSet;
    }

    std::vector<cvf::Color3f> colorTable;
    RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector( &colorTable );

    // Then add curve set for any ensembles we haven't already added
    for ( RimSummaryEnsemble* ensemble : rftEnsembles )
    {
        auto it = std::find_if( m_ensembleCurveSets.begin(),
                                m_ensembleCurveSets.end(),
                                [ensemble]( const RimWellRftEnsembleCurveSet* curveSet ) { return curveSet->ensemble() == ensemble; } );
        if ( it == m_ensembleCurveSets.end() )
        {
            RimWellRftEnsembleCurveSet* curveSet = new RimWellRftEnsembleCurveSet;
            curveSet->setEnsemble( ensemble );
            auto index = m_ensembleCurveSets.size();
            auto color = colorTable.at( index % colorTable.size() );
            curveSet->setCurveColor( color );
            m_ensembleCurveSets.push_back( curveSet );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::detachAndDeleteLegendCurves()
{
    for ( auto c : m_legendPlotCurves )
    {
        c->detach();
        delete c;
    }

    m_legendPlotCurves.clear();
    m_legendCurveToEnsembleCurveSet.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::cleanupLegendCurves()
{
    detachAndDeleteLegendCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicCreateRftCorrelationReportFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftEnsembleCurveSet* RimWellRftPlot::selectedEnsembleCurveSet() const
{
    if ( auto selected = caf::SelectionManager::instance()->selectedItemOfType<RimWellRftEnsembleCurveSet>() )
    {
        for ( auto cs : m_ensembleCurveSets )
        {
            if ( cs == selected ) return selected;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::onSelectionManagerSelectionChanged( const std::set<int>& /*changedSelectionLevels*/ )
{
    auto newCurveSet = selectedEnsembleCurveSet();
    if ( newCurveSet != m_highlightedCurveSet )
    {
        m_highlightedCurveSet = newCurveSet;
        loadDataAndUpdate();
    }

    auto* newRealization = caf::SelectionManager::instance()->selectedItemOfType<RimSummaryCase>();
    if ( newRealization != m_selectedRealization )
    {
        m_selectedRealization = newRealization;
        highlightSelectedRealization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimWellRftPlot::findClosestRealization( const QPoint& canvasPos )
{
    auto* track = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
    if ( !track ) return nullptr;

    double          minDist     = 15.0; // pixel threshold
    RimSummaryCase* closestCase = nullptr;
    for ( RimWellLogCurve* curve : track->curves() )
    {
        auto* rftCurve = dynamic_cast<RimWellLogRftCurve*>( curve );
        if ( !rftCurve || !rftCurve->summaryCase() || !rftCurve->ensemble() ) continue;
        auto* qwtCurve = dynamic_cast<RiuQwtPlotCurve*>( rftCurve->plotCurve() );
        if ( !qwtCurve ) continue;
        double dist = std::numeric_limits<double>::max();
        qwtCurve->closestPoint( canvasPos, &dist );
        if ( dist < minDist )
        {
            minDist     = dist;
            closestCase = rftCurve->summaryCase();
        }
    }

    // Toggle highlight: clicking the selected realization again clears it.
    m_selectedRealization = ( closestCase != m_selectedRealization ) ? closestCase : nullptr;
    highlightSelectedRealization();

    return closestCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::highlightSelectedRealization()
{
    auto* track = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
    if ( !track ) return;

    for ( RimWellLogCurve* curve : track->curves() )
    {
        applyCurveAppearance( curve );
        curve->updateCurveAppearance();
    }

    if ( track->plotWidget() ) track->plotWidget()->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::onLegendItemClicked( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int /*sampleIndex*/ )
{
    auto wrapper = dynamic_cast<RiuQwtPlotItem*>( plotItem.get() );
    if ( !wrapper ) return;

    auto qwtCurve = dynamic_cast<RiuQwtPlotCurve*>( wrapper->qwtPlotItem() );
    if ( !qwtCurve ) return;

    auto it = m_legendCurveToEnsembleCurveSet.find( qwtCurve );
    if ( it != m_legendCurveToEnsembleCurveSet.end() ) RiuPlotMainWindowTools::selectOrToggleObject( it->second, toggle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftEnsembleCurveSet* RimWellRftPlot::findEnsembleCurveSet( RimSummaryEnsemble* ensemble ) const
{
    for ( RimWellRftEnsembleCurveSet* curveSet : m_ensembleCurveSets() )
    {
        if ( ensemble == curveSet->ensemble() )
        {
            return curveSet;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::rebuildCurves()
{
    for ( auto c : m_ensembleCurveSets )
    {
        c->clearEnsembleStatistics();
    }

    createEnsembleCurveSets();
    updateFormationsOnPlot();
    syncCurvesFromUiSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimWellRftPlot::selectedTimeSteps() const
{
    return m_selectedTimeSteps();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setSelectedTimeSteps( const std::vector<QDateTime>& timeSteps )
{
    m_selectedTimeSteps = timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::initializeDataSources( RimWellRftPlot* source )
{
    for ( auto curveSet : m_ensembleCurveSets )
    {
        // Clear the ensemble statistics before applying initial selections
        // This is necessary to ensure that the statistics are recalculated based on the initial selections
        curveSet->clearEnsembleStatistics();
    }

    if ( source )
    {
        setOrInitializeDataSources( source->m_selectedSources );
    }
    else
    {
        // If no source is provided, initialize with empty sources
        setOrInitializeDataSources( {} );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::variant<RimSummaryCase*, RimSummaryEnsemble*> RimWellRftPlot::dataSource() const
{
    // Return the first selected ensemble, if any
    // If no ensemble is selected, return the first selected summary case, if any

    for ( const auto& source : m_selectedSources() )
    {
        if ( source.ensemble() ) return source.ensemble();
        if ( source.summaryCase() ) return source.summaryCase();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedFmuRftData* RimWellRftPlot::findObservedFmuData( const QString& wellPathName, const QDateTime& timeStep ) const
{
    auto allObservedDataForWell = RimWellPlotTools::observedFmuRftDataForWell( wellPathName );
    for ( auto observedData : allObservedDataForWell )
    {
        if ( observedData->rftReader()->availableTimeSteps( wellPathName ).count( timeStep ) )
        {
            return observedData;
        }
    }
    return nullptr;
}
