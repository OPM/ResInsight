/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimWellPltPlot.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaPlotDefines.h"
#include "RiaQDateTimeTools.h"
#include "RiaWellNameComparer.h"

#include "RifReaderEclipseRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Well/RigAccWellFlowCalculator.h"
#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "RimDataSourceForRftPlt.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogChannel.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogLasFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafVecIjk.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <tuple>

CAF_PDM_SOURCE_INIT( RimWellPltPlot, "WellPltPlot" );

namespace caf
{
template <>
void caf::AppEnum<RimWellPlotTools::FlowType>::setUp()
{
    addItem( RimWellPlotTools::FlowType::FLOW_TYPE_PHASE_SPLIT, "PHASE_SPLIT", "Phase Split" );
    addItem( RimWellPlotTools::FlowType::FLOW_TYPE_TOTAL, "TOTAL", "Total Flow" );
    setDefault( RimWellPlotTools::FlowType::FLOW_TYPE_PHASE_SPLIT );
}

template <>
void caf::AppEnum<RimWellPlotTools::FlowPhase>::setUp()
{
    addItem( RimWellPlotTools::FlowPhase::FLOW_PHASE_OIL, "PHASE_OIL", "Oil" );
    addItem( RimWellPlotTools::FlowPhase::FLOW_PHASE_GAS, "PHASE_GAS", "Gas" );
    addItem( RimWellPlotTools::FlowPhase::FLOW_PHASE_WATER, "PHASE_WATER", "Water" );
    addItem( RimWellPlotTools::FlowPhase::FLOW_PHASE_TOTAL, "PHASE_TOTAL", "Total" );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char RimWellPltPlot::PLOT_NAME_QFORMAT_STRING[] = "PLT: %1";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPltPlot::RimWellPltPlot()
    : RimWellLogPlot()
{
    CAF_PDM_InitObject( "Well Allocation Plot", ":/WellFlowPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogPlot_OBSOLETE, "WellLog", "WellLog" );
    m_wellLogPlot_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_wellPathName, "WellName", "Well Name" );

    CAF_PDM_InitFieldNoDefault( &m_selectedSources, "SourcesInternal", "Sources Internal" );
    m_selectedSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue( false );
    m_selectedSources.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_selectedSourcesForIo, "Sources", "Sources" );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Time Steps" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedTimeSteps.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitField( &m_useStandardConditionCurves, "UseStandardConditionCurves", true, "Standard Volume" );
    CAF_PDM_InitField( &m_useReservoirConditionCurves, "UseReservoirConditionCurves", true, "Reservoir Volume" );

    CAF_PDM_InitFieldNoDefault( &m_phases, "Phases", "Phases" );
    m_phases.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_phases = std::vector<caf::AppEnum<RimWellPlotTools::FlowPhase>>( { RimWellPlotTools::FlowPhase::FLOW_PHASE_OIL,
                                                                         RimWellPlotTools::FlowPhase::FLOW_PHASE_GAS,
                                                                         RimWellPlotTools::FlowPhase::FLOW_PHASE_WATER } );
    m_phases.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_nameConfig->setCustomName( "PLT Plot" );
    setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );

    setAsPlotMdiWindow();
    m_doInitAfterLoad       = false;
    m_isOnLoad              = true;
    m_plotLegendsHorizontal = false;

    setAvailableDepthTypes( { RiaDefines::DepthTypeEnum::MEASURED_DEPTH } );
    setPlotTitleVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPltPlot::~RimWellPltPlot()
{
    removeMdiWindowFromMdiArea();

    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::setPlotXAxisTitles( RimWellLogTrack* plotTrack )
{
    std::set<RiaDefines::EclipseUnitSystem> presentUnitSystems;
    for ( const auto& source : m_selectedSources() )
    {
        auto systems = source.availableUnitSystems();
        for ( const auto& s : systems )
        {
            presentUnitSystems.insert( s );
        }
    }
    if ( presentUnitSystems.size() > 1 )
    {
        RiaLogging::errorInMessageBox( nullptr, "ResInsight PLT Plot", "Inconsistent units in PLT plot" );
    }

    if ( presentUnitSystems.empty() ) return;

    RiaDefines::EclipseUnitSystem unitSet = *presentUnitSystems.begin();

    QString axisTitle;
    if ( m_useReservoirConditionCurves )
        axisTitle += RimWellPlotTools::flowPlotAxisTitle( RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR, unitSet );
    if ( m_useReservoirConditionCurves && m_useStandardConditionCurves ) axisTitle += " | ";
    if ( m_useStandardConditionCurves )
        axisTitle += RimWellPlotTools::flowPlotAxisTitle( RimWellLogLasFile::WELL_FLOW_COND_STANDARD, unitSet );

    plotTrack->setPropertyValueAxisTitle( axisTitle );

#if 0
    QString unitText;
    for ( auto unitSet: presentUnitSystems )
    {
        switch ( unitSet )
        {
            case RiaEclipseUnitTools::UNITS_METRIC:
            unitText += "[Liquid Sm<sup>3</sup>/day], [Gas kSm<sup>3</sup>/day]";
            break;
            case RiaEclipseUnitTools::UNITS_FIELD:
            unitText += "[Liquid BBL/day], [Gas BOE/day]";
            break;
            case RiaEclipseUnitTools::UNITS_LAB:
            unitText += "[cm<sup>3</sup>/hr]";
            break;
            default:
            unitText += "(unknown unit)";
            break;
        }
    }

    plotTrack->setXAxisTitle("Surface Flow Rate " + unitText);
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::updateFormationsOnPlot() const
{
    if ( plotCount() > 0 )
    {
        RimProject*  proj     = RimProject::current();
        RimWellPath* wellPath = proj->wellPathByName( m_wellPathName );

        RimWellLogTrack* track = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( track )
        {
            RimCase* formationNamesCase = track->formationNamesCase();

            if ( !formationNamesCase )
            {
                /// Set default case. Todo : Use the first of the selected cases in the
                /// plot
                std::vector<RimCase*> cases = proj->allGridCases();
                if ( !cases.empty() )
                {
                    formationNamesCase = cases[0];
                }
            }

            track->setAndUpdateWellPathFormationNamesData( formationNamesCase, wellPath );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaRftPltCurveDefinition> RimWellPltPlot::selectedCurveDefs() const
{
    std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::pltPlotChannelTypes();

    return RimWellPlotTools::curveDefsFromTimesteps( RimWellPlotTools::simWellName( m_wellPathName ),
                                                     m_selectedTimeSteps.v(),
                                                     false,
                                                     m_selectedSources(),
                                                     channelTypesToUse );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigResultPointCalculator
{
public:
    RigResultPointCalculator() {}
    virtual ~RigResultPointCalculator() {}

    const std::vector<cvf::Vec3d>&         pipeBranchCLCoords() { return m_pipeBranchCLCoords; }
    const std::vector<RigWellResultPoint>& pipeBranchWellResultPoints() { return m_pipeBranchWellResultPoints; }
    const std::vector<double>&             pipeBranchMeasuredDepths() { return m_pipeBranchMeasuredDepths; }

protected:
    RigEclipseWellLogExtractor* findWellLogExtractor( const QString& wellPathName, RimEclipseResultCase* eclCase )
    {
        RimProject*                 proj              = RimProject::current();
        RimWellPath*                wellPath          = proj->wellPathByName( wellPathName );
        RimWellLogPlotCollection*   wellLogCollection = RimMainPlotCollection::current()->wellLogPlotCollection();
        RigEclipseWellLogExtractor* eclExtractor      = wellLogCollection->findOrCreateExtractor( wellPath, eclCase );

        return eclExtractor;
    }

    std::vector<cvf::Vec3d>         m_pipeBranchCLCoords;
    std::vector<RigWellResultPoint> m_pipeBranchWellResultPoints;
    std::vector<double>             m_pipeBranchMeasuredDepths;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

class RigRftResultPointCalculator : public RigResultPointCalculator
{
public:
    RigRftResultPointCalculator( const QString& wellPathName, RimEclipseResultCase* eclCase, QDateTime m_timeStep )
    {
        const auto wellNameForRft = RimWellPlotTools::simWellName( wellPathName );

        RifEclipseRftAddress gasRateAddress =
            RifEclipseRftAddress::createAddress( wellNameForRft, m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::GRAT );
        RifEclipseRftAddress oilRateAddress =
            RifEclipseRftAddress::createAddress( wellNameForRft, m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::ORAT );
        RifEclipseRftAddress watRateAddress =
            RifEclipseRftAddress::createAddress( wellNameForRft, m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::WRAT );

        std::vector<caf::VecIjk0> rftIndices = eclCase->rftReader()->cellIndices( wellNameForRft, m_timeStep );
        if ( rftIndices.empty() ) return;

        std::vector<double> gasRates;
        std::vector<double> oilRates;
        std::vector<double> watRates;
        eclCase->rftReader()->values( gasRateAddress, &gasRates );
        eclCase->rftReader()->values( oilRateAddress, &oilRates );
        eclCase->rftReader()->values( watRateAddress, &watRates );

        std::map<size_t, size_t> globCellIdxToIdxInRftFile;

        const RigMainGrid* mainGrid = eclCase->eclipseCaseData()->mainGrid();

        for ( size_t rftCellIdx = 0; rftCellIdx < rftIndices.size(); rftCellIdx++ )
        {
            caf::VecIjk0 ijkIndex                      = rftIndices[rftCellIdx];
            size_t       globalCellIndex               = mainGrid->cellIndexFromIJK( ijkIndex.i(), ijkIndex.j(), ijkIndex.k() );
            globCellIdxToIdxInRftFile[globalCellIndex] = rftCellIdx;
        }

        RigEclipseWellLogExtractor* eclExtractor = findWellLogExtractor( wellPathName, eclCase );
        if ( !eclExtractor ) return;

        std::vector<WellPathCellIntersectionInfo> intersections = eclExtractor->cellIntersectionInfosAlongWellPath();

        for ( size_t wpExIdx = 0; wpExIdx < intersections.size(); wpExIdx++ )
        {
            size_t globCellIdx = intersections[wpExIdx].globCellIndex;

            auto it = globCellIdxToIdxInRftFile.find( globCellIdx );
            if ( it == globCellIdxToIdxInRftFile.end() )
            {
                if ( !m_pipeBranchWellResultPoints.empty() && wpExIdx == ( intersections.size() - 1 ) )
                {
                    m_pipeBranchWellResultPoints.pop_back();
                }
                continue;
            }
            m_pipeBranchCLCoords.push_back( intersections[wpExIdx].startPoint );
            m_pipeBranchMeasuredDepths.push_back( intersections[wpExIdx].startMD );

            m_pipeBranchCLCoords.push_back( intersections[wpExIdx].endPoint );
            m_pipeBranchMeasuredDepths.push_back( intersections[wpExIdx].endMD );

            RigWellResultPoint resPoint;
            resPoint.setIsOpen( true );
            resPoint.setGridIndex( 0 ); // Always main grid
            resPoint.setGridCellIndex( globCellIdx ); // Shortcut, since we only have
                                                      // main grid results from RFT

            const double adjustedGasRate =
                RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents( eclCase->eclipseCaseData()->unitsType(), gasRates[it->second] );
            resPoint.setFlowData( -1.0, oilRates[it->second], adjustedGasRate, watRates[it->second] );

            m_pipeBranchWellResultPoints.push_back( resPoint );

            if ( wpExIdx < intersections.size() - 1 )
            {
                m_pipeBranchWellResultPoints.push_back( RigWellResultPoint() ); // Invalid res point describing the
                                                                                // "line" between the cells
            }
        }
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigSimWellResultPointCalculator : public RigResultPointCalculator
{
public:
    RigSimWellResultPointCalculator( const QString& wellPathName, RimEclipseResultCase* eclCase, QDateTime m_timeStep )
    {
        // Find timestep index from qdatetime

        const std::vector<QDateTime> timeSteps =
            eclCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->timeStepDates();
        size_t tsIdx = timeSteps.size();
        for ( tsIdx = 0; tsIdx < timeSteps.size(); ++tsIdx )
        {
            if ( timeSteps[tsIdx] == m_timeStep ) break;
        }

        if ( tsIdx >= timeSteps.size() ) return;

        // Build search map into simulation well data

        std::map<size_t, std::pair<size_t, size_t>> globCellIdxToIdxInSimWellBranch;

        const RimWellPath*    wellPath = RimWellPlotTools::wellPathByWellPathNameOrSimWellName( wellPathName );
        const RigSimWellData* simWell =
            wellPath != nullptr ? eclCase->eclipseCaseData()->findSimWellData( wellPath->associatedSimulationWellName() ) : nullptr;

        if ( !simWell ) return;

        if ( !simWell->hasWellResult( tsIdx ) ) return;

        const RigWellResultFrame* resFrame = simWell->wellResultFrame( tsIdx );

        const RigMainGrid* mainGrid = eclCase->eclipseCaseData()->mainGrid();

        for ( size_t brIdx = 0; brIdx < resFrame->wellResultBranches().size(); ++brIdx )
        {
            const std::vector<RigWellResultPoint> branchResPoints = resFrame->branchResultPointsFromBranchIndex( brIdx );
            for ( size_t wrpIdx = 0; wrpIdx < branchResPoints.size(); wrpIdx++ )
            {
                const RigGridBase* grid            = mainGrid->gridByIndex( branchResPoints[wrpIdx].gridIndex() );
                size_t             globalCellIndex = grid->reservoirCellIndex( branchResPoints[wrpIdx].cellIndex() );

                globCellIdxToIdxInSimWellBranch[globalCellIndex] = std::make_pair( brIdx, wrpIdx );
            }
        }

        RigEclipseWellLogExtractor* eclExtractor = findWellLogExtractor( wellPathName, eclCase );

        if ( !eclExtractor ) return;

        std::vector<WellPathCellIntersectionInfo> intersections = eclExtractor->cellIntersectionInfosAlongWellPath();

        for ( size_t wpExIdx = 0; wpExIdx < intersections.size(); wpExIdx++ )
        {
            size_t globCellIdx = intersections[wpExIdx].globCellIndex;

            auto it = globCellIdxToIdxInSimWellBranch.find( globCellIdx );
            if ( it == globCellIdxToIdxInSimWellBranch.end() )
            {
                if ( !m_pipeBranchWellResultPoints.empty() && wpExIdx == ( intersections.size() - 1 ) )
                {
                    m_pipeBranchWellResultPoints.pop_back();
                }
                continue;
            }

            m_pipeBranchCLCoords.push_back( intersections[wpExIdx].startPoint );
            m_pipeBranchMeasuredDepths.push_back( intersections[wpExIdx].startMD );

            m_pipeBranchCLCoords.push_back( intersections[wpExIdx].endPoint );
            m_pipeBranchMeasuredDepths.push_back( intersections[wpExIdx].endMD );

            const RigWellResultPoint resPoint = resFrame->branchResultPointsFromBranchIndex( it->second.first )[it->second.second];
            m_pipeBranchWellResultPoints.push_back( resPoint );

            if ( wpExIdx < intersections.size() - 1 )
            {
                m_pipeBranchWellResultPoints.push_back( RigWellResultPoint() ); // Invalid res point describing the
                                                                                // "line" between the cells
            }
        }
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::syncCurvesFromUiSelection()
{
    RimWellLogTrack* plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
    if ( !plotTrack ) return;

    const std::set<RiaRftPltCurveDefinition>& curveDefs = selectedCurveDefs();

    setPlotXAxisTitles( plotTrack );

    // Delete existing curves

    plotTrack->deleteAllCurves();

    int curveGroupId = 0;

    QString dateFormatString;
    {
        std::vector<QDateTime> allTimeSteps;
        for ( const RiaRftPltCurveDefinition& curveDefToAdd : curveDefs )
        {
            allTimeSteps.push_back( curveDefToAdd.timeStep() );
        }
        dateFormatString = RiaQDateTimeTools::createTimeFormatStringFromDates( allTimeSteps );
    }

    // Add curves
    for ( const RiaRftPltCurveDefinition& curveDefToAdd : curveDefs )
    {
        std::set<RimWellPlotTools::FlowPhase> selectedPhases = std::set<RimWellPlotTools::FlowPhase>( m_phases().begin(), m_phases().end() );

        RifDataSourceForRftPlt sourceDef = curveDefToAdd.address();
        QDateTime              timeStep  = curveDefToAdd.timeStep();

        std::unique_ptr<RigResultPointCalculator> resultPointCalc;

        QString curveName;

        {
            curveName += sourceDef.eclCase() ? sourceDef.eclCase()->caseUserDescription() : "";
            curveName += sourceDef.wellLogFile() ? sourceDef.wellLogFile()->name() : "";
            if ( sourceDef.sourceType() == RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA ) curveName += ", RFT";

            curveName += ", " + RiaQDateTimeTools::toStringUsingApplicationLocale( timeStep, dateFormatString );
        }

        RimEclipseResultCase* rimEclipseResultCase = dynamic_cast<RimEclipseResultCase*>( sourceDef.eclCase() );

        if ( sourceDef.sourceType() == RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA )
        {
            resultPointCalc = std::make_unique<RigRftResultPointCalculator>( m_wellPathName, rimEclipseResultCase, timeStep );
        }
        else if ( sourceDef.sourceType() == RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA )
        {
            resultPointCalc = std::make_unique<RigSimWellResultPointCalculator>( m_wellPathName, rimEclipseResultCase, timeStep );
        }

        RiaDefines::EclipseUnitSystem unitSet = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
        if ( rimEclipseResultCase )
        {
            unitSet = rimEclipseResultCase->eclipseCaseData()->unitsType();
        }

        if ( resultPointCalc != nullptr )
        {
            if ( !resultPointCalc->pipeBranchCLCoords().empty() )
            {
                if ( selectedPhases.count( RimWellPlotTools::FlowPhase::FLOW_PHASE_TOTAL ) && m_useReservoirConditionCurves() &&
                     sourceDef.sourceType() == RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA )
                {
                    RigAccWellFlowCalculator wfTotalAccumulator( resultPointCalc->pipeBranchCLCoords(),
                                                                 resultPointCalc->pipeBranchWellResultPoints(),
                                                                 resultPointCalc->pipeBranchMeasuredDepths(),
                                                                 true );

                    const std::vector<double>& depthValues = wfTotalAccumulator.pseudoLengthFromTop( 0 );

                    QString curveUnitText = RimWellPlotTools::flowUnitText( RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR, unitSet );

                    const std::vector<double> accFlow =
                        wfTotalAccumulator.accumulatedTracerFlowPrPseudoLength( RigFlowDiagDefines::flowTotalName(), 0 );
                    addStackedCurve( curveName + ", " + RigFlowDiagDefines::flowTotalName() + " " + curveUnitText,
                                     depthValues,
                                     accFlow,
                                     plotTrack,
                                     cvf::Color3f::DARK_GRAY,
                                     curveGroupId,
                                     false );
                    curveGroupId++;
                }

                if ( m_useStandardConditionCurves() )
                {
                    RigAccWellFlowCalculator wfPhaseAccumulator( resultPointCalc->pipeBranchCLCoords(),
                                                                 resultPointCalc->pipeBranchWellResultPoints(),
                                                                 resultPointCalc->pipeBranchMeasuredDepths(),
                                                                 false );

                    const std::vector<double>& depthValues = wfPhaseAccumulator.pseudoLengthFromTop( 0 );
                    std::vector<QString>       tracerNames = wfPhaseAccumulator.tracerNames();
                    for ( const QString& tracerName : tracerNames )
                    {
                        auto color = tracerName == RigFlowDiagDefines::flowOilName()     ? cvf::Color3f::DARK_GREEN
                                     : tracerName == RigFlowDiagDefines::flowGasName()   ? cvf::Color3f::DARK_RED
                                     : tracerName == RigFlowDiagDefines::flowWaterName() ? cvf::Color3f::BLUE
                                                                                         : cvf::Color3f::DARK_GRAY;

                        if ( ( tracerName == RigFlowDiagDefines::flowOilName() &&
                               selectedPhases.count( RimWellPlotTools::FlowPhase::FLOW_PHASE_OIL ) ) ||
                             ( tracerName == RigFlowDiagDefines::flowGasName() &&
                               selectedPhases.count( RimWellPlotTools::FlowPhase::FLOW_PHASE_GAS ) ) ||
                             ( tracerName == RigFlowDiagDefines::flowWaterName() &&
                               selectedPhases.count( RimWellPlotTools::FlowPhase::FLOW_PHASE_WATER ) ) )
                        {
                            RimWellPlotTools::FlowPhase flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_NONE;
                            if ( tracerName == RigFlowDiagDefines::flowOilName() )
                                flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_OIL;
                            else if ( tracerName == RigFlowDiagDefines::flowGasName() )
                                flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_GAS;
                            else if ( tracerName == RigFlowDiagDefines::flowWaterName() )
                                flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_WATER;
                            QString curveUnitText =
                                RimWellPlotTools::curveUnitText( RimWellLogLasFile::WELL_FLOW_COND_STANDARD, unitSet, flowPhase );

                            const std::vector<double>& accFlow = wfPhaseAccumulator.accumulatedTracerFlowPrPseudoLength( tracerName, 0 );
                            addStackedCurve( curveName + ", " + tracerName + " " + curveUnitText,
                                             depthValues,
                                             accFlow,
                                             plotTrack,
                                             color,
                                             curveGroupId,
                                             false );
                        }
                    }
                }
            }
        }
        else if ( sourceDef.sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE )
        {
            if ( sourceDef.wellLogFile() && sourceDef.wellLogFile()->wellLogData() )
            {
                RimWellLogLasFile::WellFlowCondition flowCondition = sourceDef.wellLogFile()->wellFlowRateCondition();

                if ( ( m_useStandardConditionCurves() && flowCondition == RimWellLogLasFile::WELL_FLOW_COND_STANDARD ) ||
                     ( m_useReservoirConditionCurves() && flowCondition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR ) )
                {
                    using ChannelValNameIdxTuple = std::tuple<double, QString, int>;

                    RigWellLogLasFile* wellLogData = sourceDef.wellLogFile()->wellLogData();

                    QStringList channelNames = wellLogData->wellLogChannelNames();

                    std::multiset<ChannelValNameIdxTuple> sortedChannels;
                    std::vector<std::vector<double>>      channelData;
                    channelData.resize( channelNames.size() );

                    for ( int chIdx = 0; chIdx < channelNames.size(); ++chIdx )
                    {
                        QString channelName = channelNames[chIdx];
                        channelData[chIdx]  = wellLogData->values( channelName );
                        if ( !channelData[chIdx].empty() )
                        {
                            sortedChannels.insert( ChannelValNameIdxTuple( -fabs( channelData[chIdx].front() ), channelName, chIdx ) );
                        }
                    }

                    std::vector<double> depthValues = wellLogData->depthValues();

                    RiaDefines::EclipseUnitSystem unitSystem = RiaDefines::fromDepthUnit( wellLogData->depthUnit() );

                    for ( const ChannelValNameIdxTuple& channelInfo : sortedChannels )
                    {
                        const auto& channelName = std::get<1>( channelInfo );
                        if ( selectedPhases.count( RimWellPlotTools::flowPhaseFromChannelName( channelName ) ) > 0 )
                        {
                            auto color = RimWellPlotTools::isOilFlowChannel( channelName )     ? cvf::Color3f::DARK_GREEN
                                         : RimWellPlotTools::isGasFlowChannel( channelName )   ? cvf::Color3f::DARK_RED
                                         : RimWellPlotTools::isWaterFlowChannel( channelName ) ? cvf::Color3f::BLUE
                                                                                               : cvf::Color3f::DARK_GRAY;

                            RimWellPlotTools::FlowPhase flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_NONE;
                            if ( RimWellPlotTools::isOilFlowChannel( channelName ) )
                                flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_OIL;
                            else if ( RimWellPlotTools::isGasFlowChannel( channelName ) )
                                flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_GAS;
                            else if ( RimWellPlotTools::isWaterFlowChannel( channelName ) )
                                flowPhase = RimWellPlotTools::FlowPhase::FLOW_PHASE_WATER;
                            QString curveUnitText = RimWellPlotTools::curveUnitText( flowCondition, unitSystem, flowPhase );

                            addStackedCurve( curveName + ", " + channelName + " " + curveUnitText,
                                             depthValues,
                                             channelData[std::get<2>( channelInfo )],
                                             plotTrack,
                                             color,
                                             curveGroupId,
                                             true );

                            // Total flow channel will end up first, so just increment the
                            // group idx to make the rest of the phases group together
                            if ( RimWellPlotTools::isTotalFlowChannel( channelName ) ) curveGroupId++;
                        }
                    }
                }
            }
        }
        curveGroupId++;
    }

    plotTrack->setAutoScalePropertyValuesEnabled( true );
    RimWellLogPlot::onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::addStackedCurve( const QString&             curveName,
                                      const std::vector<double>& depthValues,
                                      const std::vector<double>& accFlow,
                                      RimWellLogTrack*           plotTrack,
                                      cvf::Color3f               color,
                                      int                        curveGroupId,
                                      bool                       doFillCurve )
{
    RimWellFlowRateCurve* curve = new RimWellFlowRateCurve;
    curve->setFlowValuesPrDepthValue( curveName, depthType(), depthValues, accFlow );

    curve->setColor( color );
    curve->setGroupId( curveGroupId );

    if ( curveGroupId == 0 )
    {
        curve->setDoFillCurve( true );
        curve->setSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );
    }
    else
    {
        curve->setDoFillCurve( false );
        curve->setSymbol( RimSummaryCurveAppearanceCalculator::cycledSymbol( curveGroupId ) );
    }

    curve->setSymbolSkipDistance( 10 );
    plotTrack->addCurve( curve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::setCurrentWellName( const QString& currWellName )
{
    m_wellPathName = currWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char* RimWellPltPlot::plotNameFormatString()
{
    return PLOT_NAME_QFORMAT_STRING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPltPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimWellLogPlot::calculateValueOptions( fieldNeedingOptions );

    const QString simWellName = RimWellPlotTools::simWellName( m_wellPathName );

    if ( fieldNeedingOptions == &m_wellPathName )
    {
        calculateValueOptionsForWells( options );
    }
    else if ( fieldNeedingOptions == &m_selectedSources )
    {
        const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell( simWellName );
        std::set<RifDataSourceForRftPlt>         availableRftSources;

        for ( const auto& rftCase : rftCases )
        {
            std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::pltPlotChannelTypes();

            std::set<QDateTime> rftTimes = rftCase->rftReader()->availableTimeSteps( simWellName, channelTypesToUse );
            if ( !rftTimes.empty() )
            {
                availableRftSources.insert( RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA, rftCase ) );
            }
        }

        if ( !availableRftSources.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA ),
                                                                     true ) );

            for ( const auto& addr : availableRftSources )
            {
                auto item = caf::PdmOptionItemInfo( addr.eclCase()->caseUserDescription(), QVariant::fromValue( addr ) );
                item.setLevel( 1 );
                options.push_back( item );
            }
        }

        const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell( simWellName );
        if ( !gridCases.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA ),
                                                                     true ) );
        }

        for ( const auto& gridCase : gridCases )
        {
            auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA, gridCase );
            auto item = caf::PdmOptionItemInfo( gridCase->caseUserDescription(), QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }

        auto wellLogFiles = RimWellPlotTools::wellLogFilesContainingFlow( m_wellPathName );
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
                    auto item = caf::PdmOptionItemInfo( wellLogFile->name(), QVariant::fromValue( addr ) );
                    item.setLevel( 1 );
                    options.push_back( item );
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_selectedTimeSteps )
    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::pltPlotChannelTypes();

        RimWellPlotTools::calculateValueOptionsForTimeSteps( RimWellPlotTools::simWellName( m_wellPathName ),
                                                             m_selectedSources(),
                                                             channelTypesToUse,
                                                             options );
    }

    if ( fieldNeedingOptions == &m_phases )
    {
        options.push_back( caf::PdmOptionItemInfo( "Oil", RimWellPlotTools::FlowPhase::FLOW_PHASE_OIL ) );
        options.push_back( caf::PdmOptionItemInfo( "Gas", RimWellPlotTools::FlowPhase::FLOW_PHASE_GAS ) );
        options.push_back( caf::PdmOptionItemInfo( "Water", RimWellPlotTools::FlowPhase::FLOW_PHASE_WATER ) );
        options.push_back( caf::PdmOptionItemInfo( "Total", RimWellPlotTools::FlowPhase::FLOW_PHASE_TOTAL ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimWellLogPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellPathName )
    {
        m_nameConfig->setCustomName( QString( plotNameFormatString() ).arg( m_wellPathName ) );
    }

    if ( changedField == &m_wellPathName )
    {
        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( plotTrack )
        {
            plotTrack->deleteAllCurves();
            m_selectedSources.v().clear();
            m_selectedTimeSteps.v().clear();
            updateFormationsOnPlot();
        }
    }
    else if ( changedField == &m_selectedSources )
    {
        RimProject*  project  = RimProject::current();
        RimWellPath* wellPath = project->wellPathByName( m_wellPathName() );
        if ( wellPath && !wellPath->wellPathGeometry() )
        {
            for ( const RifDataSourceForRftPlt& address : m_selectedSources() )
            {
                if ( address.sourceType() == RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA ||
                     address.sourceType() == RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA )
                {
                    if ( !wellPath->wellPathGeometry() )
                    {
                        QString tmp = QString( "Display of Measured Depth (MD) for Grid or "
                                               "RFT curves is not possible without a "
                                               "well log path, and the curve will be hidden "
                                               "in this mode.\n\n" );

                        RiaLogging::errorInMessageBox( nullptr, "Grid/RFT curve without MD", tmp );

                        // Do not show multiple dialogs
                        break;
                    }
                }
            }
        }
    }

    if ( changedField == &m_selectedSources || changedField == &m_selectedTimeSteps )
    {
        updateFormationsOnPlot();
        syncSourcesIoFieldFromGuiField();
        syncCurvesFromUiSelection();

        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( plotTrack )
        {
            plotTrack->setAutoScalePropertyValuesEnabled( true );
        }
        updateZoom();
    }

    if ( changedField == &m_useStandardConditionCurves || changedField == &m_useReservoirConditionCurves || changedField == &m_phases )
    {
        syncCurvesFromUiSelection();

        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( plotTrack )
        {
            plotTrack->setAutoScalePropertyValuesEnabled( true );
        }
        updateZoom();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_wellPathName );

    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword( "Sources", "Sources" );
    sourcesGroup->add( &m_selectedSources );

    caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroupWithKeyword( "Time Steps", "TimeSteps" );
    timeStepsGroup->add( &m_selectedTimeSteps );

    caf::PdmUiGroup* flowGroup = uiOrdering.addNewGroupWithKeyword( "Curve Selection", "PhaseSelection" );
    flowGroup->add( &m_useStandardConditionCurves );
    flowGroup->add( &m_useReservoirConditionCurves );

    flowGroup->add( &m_phases );

    if ( plotCount() > 0 )
    {
        RimWellLogTrack* const track = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        if ( track )
        {
            track->uiOrderingForRftPltFormations( uiOrdering );
            track->uiOrderingForPropertyAxisSettings( uiOrdering );
            caf::PdmUiGroup* depthGroup = uiOrdering.addNewGroup( "Depth Axis Settings" );
            uiOrderingForDepthAxis( uiConfigName, *depthGroup );

            caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Plot Name" );
            nameGroup->setCollapsedByDefault();
            RimWellLogPlot::uiOrderingForAutoName( uiConfigName, *nameGroup );

            RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_phases )
    {
        caf::PdmUiTreeSelectionEditorAttribute* attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
        attrib->showTextFilter                         = false;
        attrib->showToggleAllCheckbox                  = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::initAfterRead()
{
    RimDepthTrackPlot::initAfterRead();

    if ( m_wellLogPlot_OBSOLETE )
    {
        RimWellLogPlot& wellLogPlot = dynamic_cast<RimWellLogPlot&>( *this );
        wellLogPlot                 = std::move( *m_wellLogPlot_OBSOLETE.value() );
        delete m_wellLogPlot_OBSOLETE;
        m_wellLogPlot_OBSOLETE = nullptr;
    }

    RimWellLogPlot::initAfterRead();

    // Postpone init until data has been loaded
    m_doInitAfterLoad = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::setupBeforeSave()
{
    syncSourcesIoFieldFromGuiField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::initAfterLoad()
{
    std::vector<RifDataSourceForRftPlt> selectedSources;
    for ( RimDataSourceForRftPlt* addr : m_selectedSourcesForIo )
    {
        if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.1.0" ) &&
             addr->address().sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE )
        {
            // In previous versions, the observed LAS files were not stored in the project file, but handled as a unity.
            // Now we have more observed data types, and need to show all LAS files individually for selection
            auto wellLogFiles = RimWellPlotTools::wellLogFilesContainingFlow( m_wellPathName );
            if ( !wellLogFiles.empty() )
            {
                for ( const auto& wellLogFile : wellLogFiles )
                {
                    if ( auto wellLogLasFile = dynamic_cast<RimWellLogLasFile*>( wellLogFile ) )
                    {
                        auto addr = RifDataSourceForRftPlt( wellLogLasFile );
                        selectedSources.push_back( addr );
                    }
                }
            }

            continue;
        }

        selectedSources.push_back( addr->address() );
    }

    m_selectedSources = selectedSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::syncSourcesIoFieldFromGuiField()
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
void RimWellPltPlot::calculateValueOptionsForWells( QList<caf::PdmOptionItemInfo>& options )
{
    auto wellPathsContainingFlowData = RimWellPlotTools::wellPathsContainingFlow();
    for ( const RimWellPath* const wellPath : wellPathsContainingFlowData )
    {
        const QString wellName = wellPath->name();
        options.push_back( caf::PdmOptionItemInfo( wellName, wellName ) );
    }

    options.push_back( caf::PdmOptionItemInfo( "None", "" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::onLoadDataAndUpdate()
{
    if ( m_doInitAfterLoad )
    {
        initAfterLoad();
        m_doInitAfterLoad = false;
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
    syncCurvesFromUiSelection();
}
