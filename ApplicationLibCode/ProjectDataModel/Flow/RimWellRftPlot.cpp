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
#include "RiaDateStringParser.h"
#include "RiaSimWellBranchTools.h"
#include "RiaStatisticsTools.h"

#include "RifReaderEclipseRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimObservedFmuRftData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCaseCollection.h"
#include "RimTools.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"
#include "RimWellPltPlot.h"

#include "RiuAbstractLegendFrame.h"
#include "RiuAbstractOverlayContentFrame.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <algorithm>
#include <iterator>
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
    CAF_PDM_InitObject( "RFT Plot", ":/RFTPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_showStatisticsCurves, "ShowStatisticsCurves", true, "Show Statistics Curves", "", "", "" );
    CAF_PDM_InitField( &m_showEnsembleCurves, "ShowEnsembleCurves", true, "Show Ensemble Curves", "", "", "" );
    CAF_PDM_InitField( &m_showErrorInObservedData, "ShowErrorObserved", true, "Show Observed Data Error", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogPlot_OBSOLETE, "WellLog", "Well Log", "", "", "" );
    m_wellLogPlot_OBSOLETE.uiCapability()->setUiHidden( true );
    m_wellLogPlot_OBSOLETE.xmlCapability()->setIOWritable( false );

    m_depthType = RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH;

    CAF_PDM_InitFieldNoDefault( &m_wellPathNameOrSimWellName, "WellName", "Well Name", "", "", "" );
    CAF_PDM_InitField( &m_branchIndex, "BranchIndex", 0, "Branch Index", "", "", "" );
    CAF_PDM_InitField( &m_branchDetection,
                       "BranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_selectedSources, "Sources", "Sources", "", "", "" );
    m_selectedSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSources.xmlCapability()->disableIO();
    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Time Steps", "", "", "" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.xmlCapability()->disableIO();
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedTimeSteps.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_wellPathCollection, "WellPathCollection", "Well Path Collection", "", "", "" );
    m_wellPathCollection.uiCapability()->setUiHidden( true );
    m_wellPathCollection.xmlCapability()->disableIO();
    m_wellPathCollection = RimProject::current()->activeOilField()->wellPathCollection();

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSets, "EnsembleCurveSets", "Ensemble Curve Sets", "", "", "" );

    // TODO: may want to support TRUE_VERTICAL_DEPTH_RKB in the future
    // It was developed for regular well log plots and requires some more work for RFT plots.
    setAvailableDepthTypes( { RiaDefines::DepthTypeEnum::MEASURED_DEPTH, RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH } );

    m_nameConfig->setCustomName( "RFT Plot" );
    m_plotLegendsHorizontal = false;

    setPlotTitleVisible( true );

    this->setAsPlotMdiWindow();
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

    RiuQwtSymbol::PointSymbolEnum currentSymbol = RiuQwtSymbol::SYMBOL_NONE;
    if ( curveDef.address().sourceType() != RifDataSourceForRftPlt::ENSEMBLE_RFT )
    {
        currentSymbol = m_timeStepSymbols[curveDef.timeStep()];
    }

    bool isObservedData = curveDef.address().sourceType() == RifDataSourceForRftPlt::OBSERVED ||
                          curveDef.address().sourceType() == RifDataSourceForRftPlt::OBSERVED_FMU_RFT;
    // Observed data
    lineStyle = isObservedData ? RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE
                               : RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID;

    curve->setSymbol( currentSymbol );
    curve->setLineStyle( lineStyle );
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
            formationNamesCase = track->formationNamesCase();

            if ( !formationNamesCase )
            {
                /// Set default case. Todo : Use the first of the selected cases in the plot
                std::vector<RimCase*> cases;
                proj->allCases( cases );

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
void RimWellRftPlot::applyInitialSelections()
{
    std::map<QString, QStringList> wellSources = findWellSources();
    if ( m_wellPathNameOrSimWellName == "None" && !wellSources.empty() )
    {
        m_wellPathNameOrSimWellName = wellSources.begin()->first;
    }

    std::vector<RifDataSourceForRftPlt> sourcesToSelect;
    const QString                       simWellName = associatedSimWellName();

    for ( RimEclipseResultCase* const rftCase : RimWellPlotTools::rftCasesForWell( simWellName ) )
    {
        sourcesToSelect.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::RFT, rftCase ) );
    }

    for ( RimEclipseResultCase* const gridCase : RimWellPlotTools::gridCasesForWell( simWellName ) )
    {
        sourcesToSelect.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::GRID, gridCase ) );
    }

    for ( RimSummaryCaseCollection* const ensemble : RimWellPlotTools::rftEnsemblesForWell( simWellName ) )
    {
        sourcesToSelect.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::ENSEMBLE_RFT, ensemble ) );
    }

    std::vector<RimWellLogFile*> wellLogFiles =
        RimWellPlotTools::wellLogFilesContainingPressure( m_wellPathNameOrSimWellName );
    if ( !wellLogFiles.empty() )
    {
        for ( RimWellLogFile* const wellLogFile : wellLogFiles )
        {
            sourcesToSelect.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED, wellLogFile ) );
        }
    }

    for ( RimObservedFmuRftData* const observedFmuRftData :
          RimWellPlotTools::observedFmuRftDataForWell( m_wellPathNameOrSimWellName ) )
    {
        sourcesToSelect.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED_FMU_RFT, observedFmuRftData ) );
    }

    m_selectedSources = sourcesToSelect;

    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse =
            RifEclipseRftAddress::rftPlotChannelTypes();

        auto relevantTimeSteps = RimWellPlotTools::calculateRelevantTimeStepsFromCases( m_wellPathNameOrSimWellName,
                                                                                        m_selectedSources,
                                                                                        channelTypesToUse );

        if ( !relevantTimeSteps.empty() )
        {
            std::vector<QDateTime> timeStepVector;

            // If we have RFT data from multiple sources, relevant time steps end up with a small number of time steps
            // If this is the case, pre-select all time steps
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

    bool dummy             = false;
    auto dataSourceOptions = calculateValueOptions( &m_selectedSources, &dummy );
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
    auto timeStepOptions = calculateValueOptions( &m_selectedTimeSteps, &dummy );
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
void RimWellRftPlot::updateEditorsFromCurves()
{
    std::set<RifDataSourceForRftPlt>                      selectedSources;
    std::set<QDateTime>                                   selectedTimeSteps;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> selectedTimeStepsMap;

    for ( const RiaRftPltCurveDefinition& curveDef : curveDefsFromCurves() )
    {
        if ( curveDef.address().sourceType() == RifDataSourceForRftPlt::OBSERVED )
            selectedSources.insert( RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED ) );
        else if ( curveDef.address().sourceType() == RifDataSourceForRftPlt::SUMMARY_RFT )
        {
            selectedSources.insert(
                RifDataSourceForRftPlt( RifDataSourceForRftPlt::ENSEMBLE_RFT, curveDef.address().ensemble() ) );
        }
        else
            selectedSources.insert( curveDef.address() );

        auto newTimeStepMap = std::map<QDateTime, std::set<RifDataSourceForRftPlt>>{
            { curveDef.timeStep(), std::set<RifDataSourceForRftPlt>{ curveDef.address() } } };
        RimWellPlotTools::addTimeStepsToMap( selectedTimeStepsMap, newTimeStepMap );
        selectedTimeSteps.insert( curveDef.timeStep() );
    }

    m_selectedSources   = std::vector<RifDataSourceForRftPlt>( selectedSources.begin(), selectedSources.end() );
    m_selectedTimeSteps = std::vector<QDateTime>( selectedTimeSteps.begin(), selectedTimeSteps.end() );
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

    // Delete curves
    plotTrack->deleteAllCurves();

    defineCurveColorsAndSymbols( allCurveDefs );

    std::set<RimSummaryCaseCollection*> ensemblesWithSummaryCurves;

    // Add new curves
    for ( const RiaRftPltCurveDefinition& curveDefToAdd : allCurveDefs )
    {
        if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::RFT )
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve( curve );

            auto rftCase = curveDefToAdd.address().eclCase();
            curve->setEclipseResultCase( dynamic_cast<RimEclipseResultCase*>( rftCase ) );

            RifEclipseRftAddress address( simWellName, curveDefToAdd.timeStep(), RifEclipseRftAddress::PRESSURE );
            curve->setRftAddress( address );
            curve->setZOrder( 1 );
            curve->setSimWellBranchData( m_branchDetection, m_branchIndex );

            applyCurveAppearance( curve );
        }
        else if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::OBSERVED_FMU_RFT )
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve( curve );

            auto observedFmuRftData = curveDefToAdd.address().observedFmuRftData();
            curve->setObservedFmuRftData( observedFmuRftData );
            RifEclipseRftAddress address( m_wellPathNameOrSimWellName,
                                          curveDefToAdd.timeStep(),
                                          RifEclipseRftAddress::PRESSURE );
            curve->setRftAddress( address );
            curve->setZOrder(
                RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_OBSERVED ) );
            applyCurveAppearance( curve );
        }
        else if ( m_showEnsembleCurves && curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SUMMARY_RFT )
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve( curve );
            auto rftCase = curveDefToAdd.address().summaryCase();
            curve->setSummaryCase( rftCase );
            curve->setEnsemble( curveDefToAdd.address().ensemble() );
            curve->setObservedFmuRftData(
                this->findObservedFmuData( m_wellPathNameOrSimWellName, curveDefToAdd.timeStep() ) );
            RifEclipseRftAddress address( m_wellPathNameOrSimWellName,
                                          curveDefToAdd.timeStep(),
                                          RifEclipseRftAddress::PRESSURE );
            curve->setRftAddress( address );
            curve->setZOrder( 1 );
            applyCurveAppearance( curve );

            bool isFirstSummaryCurveInEnsemble =
                ensemblesWithSummaryCurves.count( curveDefToAdd.address().ensemble() ) == 0u;
            curve->setShowInLegend( isFirstSummaryCurveInEnsemble );
            ensemblesWithSummaryCurves.insert( curveDefToAdd.address().ensemble() );
        }
        else if ( m_showStatisticsCurves && curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::ENSEMBLE_RFT )
        {
            RimSummaryCaseCollection*      ensemble = curveDefToAdd.address().ensemble();
            std::set<RifEclipseRftAddress> rftAddresses =
                ensemble->rftStatisticsReader()->eclipseRftAddresses( m_wellPathNameOrSimWellName,
                                                                      curveDefToAdd.timeStep() );
            for ( const auto& rftAddress : rftAddresses )
            {
                if ( rftAddress.wellLogChannel() == RifEclipseRftAddress::PRESSURE_P50 )
                {
                    // Default statistics curves are P10, P50, P90 and mean
                    // It is not common to use P50 for ensemble RFT, so skip display of P50 to avoid confusion with mean
                    // https://github.com/OPM/ResInsight/issues/5238

                    continue;
                }

                if ( rftAddress.wellLogChannel() != RifEclipseRftAddress::TVD )
                {
                    auto curve = new RimWellLogRftCurve();
                    plotTrack->addCurve( curve );
                    curve->setEnsemble( ensemble );
                    curve->setRftAddress( rftAddress );
                    curve->setObservedFmuRftData(
                        this->findObservedFmuData( m_wellPathNameOrSimWellName, curveDefToAdd.timeStep() ) );
                    curve->setZOrder(
                        RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_STAT_CURVE ) );
                    applyCurveAppearance( curve );
                    auto                        symbol   = statisticsCurveSymbolFromAddress( rftAddress );
                    RiuQwtSymbol::LabelPosition labelPos = statisticsLabelPosFromAddress( rftAddress );
                    curve->setSymbol( symbol );
                    curve->setSymbolLabelPosition( labelPos );
                    curve->setSymbolSize( curve->symbolSize() + 3 );
                    curve->setSymbolSkipDistance( 150 );
                    curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
                    QString uiText =
                        caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText( rftAddress.wellLogChannel() );
                    QString label = uiText.replace( ": Pressure", "" );
                    curve->setSymbolLabel( label );
                    curve->setLineThickness( 3 );
                }
            }
        }

        else if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::GRID )
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
                curve->setZOrder( 0 );

                applyCurveAppearance( curve );
            }
        }
        else if ( curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::OBSERVED )
        {
            RimWellLogFile* const wellLogFile = curveDefToAdd.address().wellLogFile();
            RimWellPath* const    wellPath    = RimWellPlotTools::wellPathFromWellLogFile( wellLogFile );
            if ( wellLogFile != nullptr )
            {
                RimWellLogFileChannel* pressureChannel = RimWellPlotTools::getPressureChannelFromWellFile( wellLogFile );
                auto                   curve           = new RimWellLogFileCurve();

                plotTrack->addCurve( curve );
                curve->setWellPath( wellPath );
                curve->setWellLogFile( wellLogFile );
                curve->setWellLogChannelName( pressureChannel->name() );
                curve->setZOrder( 2 );

                applyCurveAppearance( curve );
            }
        }
    }

    if ( depthType() == RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
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
        if ( addr.sourceType() == RifDataSourceForRftPlt::OBSERVED )
        {
            for ( RimWellLogFile* const wellLogFile :
                  RimWellPlotTools::wellLogFilesContainingPressure( m_wellPathNameOrSimWellName ) )
            {
                sources.push_back( RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED, wellLogFile ) );
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
QList<caf::PdmOptionItemInfo> RimWellRftPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                     bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimWellLogPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

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
        const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell( simWellName );
        if ( !rftCases.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::RFT ),
                                                                     true ) );

            for ( const auto& rftCase : rftCases )
            {
                auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::RFT, rftCase );
                auto item = caf::PdmOptionItemInfo( rftCase->caseUserDescription(), QVariant::fromValue( addr ) );
                item.setLevel( 1 );
                options.push_back( item );
            }
        }

        const std::vector<RimSummaryCaseCollection*> rftEnsembles =
            RimWellPlotTools::rftEnsemblesForWell( m_wellPathNameOrSimWellName );
        if ( !rftEnsembles.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::ENSEMBLE_RFT ),
                                                                     true ) );

            for ( RimSummaryCaseCollection* rftEnsemble : rftEnsembles )
            {
                auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::ENSEMBLE_RFT, rftEnsemble );
                auto item = caf::PdmOptionItemInfo( rftEnsemble->name(), QVariant::fromValue( addr ) );
                item.setLevel( 1 );
                options.push_back( item );
            }
        }

        const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell( simWellName );
        if ( !gridCases.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::GRID ),
                                                                     true ) );

            for ( const auto& gridCase : gridCases )
            {
                auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::GRID, gridCase );
                auto item = caf::PdmOptionItemInfo( gridCase->caseUserDescription(), QVariant::fromValue( addr ) );
                item.setLevel( 1 );
                options.push_back( item );
            }
        }

        if ( !RimWellPlotTools::wellLogFilesContainingPressure( m_wellPathNameOrSimWellName ).empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::OBSERVED ),
                                                                     true ) );

            auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED );
            auto item = caf::PdmOptionItemInfo( "Observed Data", QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }
        const std::vector<RimObservedFmuRftData*> observedFmuRftCases =
            RimWellPlotTools::observedFmuRftDataForWell( m_wellPathNameOrSimWellName );
        if ( !observedFmuRftCases.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::OBSERVED_FMU_RFT ),
                                                                     true ) );

            for ( const auto& observedFmuRftCase : observedFmuRftCases )
            {
                auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED_FMU_RFT, observedFmuRftCase );
                auto item = caf::PdmOptionItemInfo( observedFmuRftCase->name(), QVariant::fromValue( addr ) );
                item.setLevel( 1 );
                options.push_back( item );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_selectedTimeSteps )
    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse =
            RifEclipseRftAddress::rftPlotChannelTypes();

        RimWellPlotTools::calculateValueOptionsForTimeSteps( m_wellPathNameOrSimWellName,
                                                             selectedSourcesExpanded(),
                                                             channelTypesToUse,
                                                             options );
    }
    else if ( fieldNeedingOptions == &m_branchIndex )
    {
        auto simulationWellBranches = RiaSimWellBranchTools::simulationWellBranches( simWellName, m_branchDetection );

        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField( simulationWellBranches );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    RimWellLogPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellPathNameOrSimWellName )
    {
        m_nameConfig->setCustomName( QString( plotNameFormatString() ).arg( m_wellPathNameOrSimWellName ) );

        m_branchIndex = 0;

        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( plotTrack )
        {
            plotTrack->deleteAllCurves();
        }
        createEnsembleCurveSets();
        updateEditorsFromPreviousSelection();
        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
    }
    else if ( changedField == &m_branchIndex || changedField == &m_branchDetection )
    {
        const QString simWellName = associatedSimWellName();
        m_branchIndex = RiaSimWellBranchTools::clampBranchIndex( simWellName, m_branchIndex, m_branchDetection );

        createEnsembleCurveSets();
        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
    }
    else if ( changedField == &m_selectedSources || changedField == &m_selectedTimeSteps )
    {
        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_showStatisticsCurves || changedField == &m_showEnsembleCurves ||
              changedField == &m_showErrorInObservedData )
    {
        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
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
        for ( RimSummaryCaseCollection* selectedCurveSet : selectedEnsembles() )
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

    caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroupWithKeyword( "Time Steps", "TimeSteps" );
    timeStepsGroup->add( &m_selectedTimeSteps );

    if ( plotCount() > 0 )
    {
        RimWellLogTrack* const track = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( track )
        {
            track->uiOrderingForRftPltFormations( uiOrdering );
            track->uiOrderingForXAxisSettings( uiOrdering );
            caf::PdmUiGroup* depthGroup = uiOrdering.addNewGroup( "Depth Axis Settings" );
            uiOrderingForDepthAxis( uiConfigName, *depthGroup );

            caf::PdmUiGroup* plotLayoutGroup = uiOrdering.addNewGroup( "Plot Layout" );
            plotLayoutGroup->setCollapsedByDefault( true );
            RimWellLogPlot::uiOrderingForAutoName( uiConfigName, *plotLayoutGroup );
            RimWellLogPlot::uiOrderingForPlotLayout( uiConfigName, *plotLayoutGroup );
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
        const std::vector<RimSummaryCaseCollection*> rftEnsembles = RimWellPlotTools::rftEnsembles();
        // Ensemble RFT wells
        {
            for ( RimSummaryCaseCollection* summaryCaseColl : rftEnsembles )
            {
                std::set<QString> wellsWithRftData = summaryCaseColl->wellsWithRftData();
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
    }
    return wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::onLoadDataAndUpdate()
{
    if ( m_isOnLoad )
    {
        if ( plotCount() > 0 )
        {
            RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
            if ( plotTrack )
            {
                plotTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS );
            }
        }

        m_isOnLoad = false;
    }

    updateMdiWindowVisibility();
    updateFormationsOnPlot();

    if ( depthType() == RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
    {
        assignWellPathToExtractionCurves();
    }

    RimWellLogPlot::onLoadDataAndUpdate();
    createEnsembleCurveSets();
    updateEditorsFromCurves();

    // Update of curve color must happen here when loading data from project file, as the curve color is blended by the
    // background color. The background color is taken from the viewer.
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
void RimWellRftPlot::initAfterRead()
{
    if ( m_wellLogPlot_OBSOLETE )
    {
        RimWellLogPlot& wellLogPlot = dynamic_cast<RimWellLogPlot&>( *this );
        wellLogPlot                 = std::move( *m_wellLogPlot_OBSOLETE.value() );
        delete m_wellLogPlot_OBSOLETE;
        m_wellLogPlot_OBSOLETE = nullptr;
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
RiuQwtSymbol::PointSymbolEnum RimWellRftPlot::statisticsCurveSymbolFromAddress( const RifEclipseRftAddress& address )
{
    switch ( address.wellLogChannel() )
    {
        case RifEclipseRftAddress::PRESSURE_P10:
            return RiuQwtSymbol::SYMBOL_TRIANGLE;
        case RifEclipseRftAddress::PRESSURE_P50:
            return RiuQwtSymbol::SYMBOL_DOWN_TRIANGLE;
        case RifEclipseRftAddress::PRESSURE_P90:
            return RiuQwtSymbol::SYMBOL_LEFT_TRIANGLE;
        case RifEclipseRftAddress::PRESSURE_MEAN:
            return RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE;
    }
    return RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::LabelPosition RimWellRftPlot::statisticsLabelPosFromAddress( const RifEclipseRftAddress& address )
{
    switch ( address.wellLogChannel() )
    {
        case RifEclipseRftAddress::PRESSURE_P10:
            return RiuQwtSymbol::LabelLeftOfSymbol;
        case RifEclipseRftAddress::PRESSURE_P50:
            return RiuQwtSymbol::LabelAboveSymbol;
        case RifEclipseRftAddress::PRESSURE_P90:
            return RiuQwtSymbol::LabelRightOfSymbol;
        case RifEclipseRftAddress::PRESSURE_MEAN:
            return RiuQwtSymbol::LabelBelowSymbol;
    }
    return RiuQwtSymbol::LabelAboveSymbol;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellRftPlot::findCurveColor( RimWellLogCurve* curve )
{
    RiaRftPltCurveDefinition curveDef = RimWellPlotTools::curveDefFromCurve( curve );

    cvf::Color3f curveColor;
    if ( curveDef.address().sourceType() == RifDataSourceForRftPlt::SUMMARY_RFT )
    {
        RimWellRftEnsembleCurveSet* ensembleCurveSet = findEnsembleCurveSet( curveDef.address().ensemble() );
        if ( ensembleCurveSet && ensembleCurveSet->colorMode() == ColorMode::BY_ENSEMBLE_PARAM )
        {
            curveColor = ensembleCurveSet->caseColor( curveDef.address().summaryCase() );
        }
        else
        {
            RifDataSourceForRftPlt sourceAddress( RifDataSourceForRftPlt::ENSEMBLE_RFT, curveDef.address().ensemble() );
            curveColor = m_dataSourceColors[sourceAddress];
        }

        if ( m_showStatisticsCurves )
        {
            if ( plotByIndex( 0 ) && plotByIndex( 0 )->viewer() )
            {
                cvf::Color3f backgroundColor =
                    RiaColorTools::fromQColorTo3f( plotByIndex( 0 )->viewer()->canvasBackground().color() );
                curveColor = RiaColorTools::blendCvfColors( backgroundColor, curveColor, 1, 2 );
            }
        }
    }
    else
    {
        curveColor = m_dataSourceColors[curveDef.address()];
    }

    return curveColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::defineCurveColorsAndSymbols( const std::set<RiaRftPltCurveDefinition>& allCurveDefs )
{
    m_dataSourceColors.clear();
    m_timeStepSymbols.clear();

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
        auto ensemble_it =
            std::find_if( ensembles.begin(), ensembles.end(), [&curveSet]( const RimSummaryCaseCollection* ensemble ) {
                return curveSet->ensemble() == ensemble;
            } );
        if ( ensemble_it != ensembles.end() )
        {
            curveSet->initializeLegend();

            if ( viewer && curveSet->legendConfig()->showLegend() &&
                 curveSet->colorMode() == ColorMode::BY_ENSEMBLE_PARAM && !curveSet->currentEnsembleParameter().isEmpty() )
            {
                if ( !m_ensembleLegendFrames[curveSet] )
                {
                    auto m = new RiuDraggableOverlayFrame( viewer->canvas(), viewer->overlayMargins() );
                    m->setContentFrame( curveSet->legendConfig()->makeLegendFrame() );

                    m_ensembleLegendFrames[curveSet] = m;
                }

                viewer->addOverlayFrame( m_ensembleLegendFrames[curveSet] );
            }
        }
    }

    std::vector<cvf::Color3f> colorTable;
    RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector( &colorTable );

    std::vector<RiuQwtSymbol::PointSymbolEnum> symbolTable = { RiuQwtSymbol::SYMBOL_ELLIPSE,
                                                               RiuQwtSymbol::SYMBOL_RECT,
                                                               RiuQwtSymbol::SYMBOL_DIAMOND,
                                                               RiuQwtSymbol::SYMBOL_CROSS,
                                                               RiuQwtSymbol::SYMBOL_XCROSS,
                                                               RiuQwtSymbol::SYMBOL_STAR1 };

    // Add new curves
    for ( const RiaRftPltCurveDefinition& curveDefToAdd : allCurveDefs )
    {
        auto colorTableIndex  = m_dataSourceColors.size();
        auto symbolTableIndex = m_timeStepSymbols.size();

        const RifDataSourceForRftPlt& address      = curveDefToAdd.address();
        RifDataSourceForRftPlt        colorAddress = address;
        if ( address.sourceType() == RifDataSourceForRftPlt::SUMMARY_RFT )
        {
            colorAddress = RifDataSourceForRftPlt( RifDataSourceForRftPlt::ENSEMBLE_RFT, address.ensemble() );
        }

        if ( !m_dataSourceColors.count( colorAddress ) )
        {
            colorTableIndex                  = colorTableIndex % colorTable.size();
            m_dataSourceColors[colorAddress] = colorTable[colorTableIndex];
        }

        if ( address.sourceType() != RifDataSourceForRftPlt::ENSEMBLE_RFT )
        {
            if ( !m_timeStepSymbols.count( curveDefToAdd.timeStep() ) )
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
std::vector<RimSummaryCaseCollection*> RimWellRftPlot::selectedEnsembles() const
{
    std::vector<RimSummaryCaseCollection*> ensembleSets;
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
    const std::vector<RimSummaryCaseCollection*> rftEnsembles =
        RimWellPlotTools::rftEnsemblesForWell( m_wellPathNameOrSimWellName );

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
        m_ensembleCurveSets.removeChildObject( curveSet );
        delete curveSet;
    }

    // Then add curve set for any ensembles we haven't already added
    for ( RimSummaryCaseCollection* ensemble : rftEnsembles )
    {
        auto it = std::find_if( m_ensembleCurveSets.begin(),
                                m_ensembleCurveSets.end(),
                                [ensemble]( const RimWellRftEnsembleCurveSet* curveSet ) {
                                    return curveSet->ensemble() == ensemble;
                                } );
        if ( it == m_ensembleCurveSets.end() )
        {
            RimWellRftEnsembleCurveSet* curveSet = new RimWellRftEnsembleCurveSet;
            curveSet->setEnsemble( ensemble );
            m_ensembleCurveSets.push_back( curveSet );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftEnsembleCurveSet* RimWellRftPlot::findEnsembleCurveSet( RimSummaryCaseCollection* ensemble ) const
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
