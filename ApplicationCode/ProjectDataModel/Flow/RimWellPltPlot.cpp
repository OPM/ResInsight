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

#include "RimWellPltPlot.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaDateStringParser.h"
#include "RiaQDateTimeTools.h"
#include "RiaWellNameComparer.h"

#include "RifReaderEclipseRft.h"

#include "RigAccWellFlowCalculator.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"

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
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafVecIjk.h"

#include <QMessageBox>

#include <algorithm>
#include <iterator>
#include <tuple>

CAF_PDM_SOURCE_INIT( RimWellPltPlot, "WellPltPlot" );

namespace caf
{
template <>
void caf::AppEnum<FlowType>::setUp()
{
    addItem( FLOW_TYPE_PHASE_SPLIT, "PHASE_SPLIT", "Phase Split" );
    addItem( FLOW_TYPE_TOTAL, "TOTAL", "Total Flow" );
    setDefault( FLOW_TYPE_PHASE_SPLIT );
}

template <>
void caf::AppEnum<FlowPhase>::setUp()
{
    addItem( FLOW_PHASE_OIL, "PHASE_OIL", "Oil" );
    addItem( FLOW_PHASE_GAS, "PHASE_GAS", "Gas" );
    addItem( FLOW_PHASE_WATER, "PHASE_WATER", "Water" );
    addItem( FLOW_PHASE_TOTAL, "PHASE_TOTAL", "Total" );
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
    CAF_PDM_InitObject( "Well Allocation Plot", ":/WellFlowPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_showPlotTitle_OBSOLETE, "ShowPlotTitle", false, "Show Plot Title", "", "", "" );
    m_showPlotTitle_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_wellLogPlot_OBSOLETE, "WellLog", "WellLog", "", "", "" );
    m_wellLogPlot_OBSOLETE.uiCapability()->setUiHidden( true );
    m_wellLogPlot_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_wellPathName, "WellName", "Well Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_selectedSources, "SourcesInternal", "Sources Internal", "", "", "" );
    m_selectedSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue( false );
    m_selectedSources.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_selectedSourcesForIo, "Sources", "Sources", "", "", "" );
    m_selectedSourcesForIo.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Time Steps", "", "", "" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedTimeSteps.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitField( &m_useStandardConditionCurves, "UseStandardConditionCurves", true, "Standard Volume", "", "", "" );
    CAF_PDM_InitField( &m_useReservoirConditionCurves, "UseReservoirConditionCurves", true, "Reservoir Volume", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_phases, "Phases", "Phases", "", "", "" );
    m_phases.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_phases = std::vector<caf::AppEnum<FlowPhase>>( {FLOW_PHASE_OIL, FLOW_PHASE_GAS, FLOW_PHASE_WATER} );
    m_phases.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_nameConfig->setCustomName( "PLT Plot" );

    this->setAsPlotMdiWindow();
    m_doInitAfterLoad       = false;
    m_isOnLoad              = true;
    m_plotLegendsHorizontal = false;

    setAvailableDepthTypes( {RiaDefines::MEASURED_DEPTH} );
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
    std::set<RiaEclipseUnitTools::UnitSystem> presentUnitSystems;
    for ( const RifDataSourceForRftPlt& source : m_selectedSources.v() )
    {
        if ( source.eclCase() ) presentUnitSystems.insert( source.eclCase()->eclipseCaseData()->unitsType() );
        if ( source.wellLogFile() )
        {
            if ( source.wellLogFile()->wellLogFileData() )
            {
                // Todo: Handle different units in the relevant las channels
                switch ( source.wellLogFile()->wellLogFileData()->depthUnit() )
                {
                    case RiaDefines::UNIT_METER:
                        presentUnitSystems.insert( RiaEclipseUnitTools::UNITS_METRIC );
                        break;
                    case RiaDefines::UNIT_FEET:
                        presentUnitSystems.insert( RiaEclipseUnitTools::UNITS_FIELD );
                        break;
                    case RiaDefines::UNIT_NONE:
                        presentUnitSystems.insert( RiaEclipseUnitTools::UNITS_UNKNOWN );
                        break;
                }
            }
        }
    }

    if ( presentUnitSystems.size() > 1 )
    {
        QMessageBox::warning( nullptr, "ResInsight PLT Plot", "Inconsistent units in PLT plot" );
    }

    if ( presentUnitSystems.empty() ) return;

    RiaEclipseUnitTools::UnitSystem unitSet = *presentUnitSystems.begin();

    QString axisTitle;
    if ( m_useReservoirConditionCurves )
        axisTitle += RimWellPlotTools::flowPlotAxisTitle( RimWellLogFile::WELL_FLOW_COND_RESERVOIR, unitSet );
    if ( m_useReservoirConditionCurves && m_useStandardConditionCurves ) axisTitle += " | ";
    if ( m_useStandardConditionCurves )
        axisTitle += RimWellPlotTools::flowPlotAxisTitle( RimWellLogFile::WELL_FLOW_COND_STANDARD, unitSet );

    plotTrack->setXAxisTitle( axisTitle );

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
        RimProject*  proj     = RiaApplication::instance()->project();
        RimWellPath* wellPath = proj->wellPathByName( m_wellPathName );

        RimWellLogTrack* track = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        CAF_ASSERT( track );
        if ( track )
        {
            RimCase* formationNamesCase = track->formationNamesCase();

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
                                                     selectedSourcesExpanded(),
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

    const std::vector<cvf::Vec3d>& pipeBranchCLCoords()
    {
        return m_pipeBranchCLCoords;
    }
    const std::vector<RigWellResultPoint>& pipeBranchWellResultPoints()
    {
        return m_pipeBranchWellResultPoints;
    }
    const std::vector<double>& pipeBranchMeasuredDepths()
    {
        return m_pipeBranchMeasuredDepths;
    }

protected:
    RigEclipseWellLogExtractor* findWellLogExtractor( const QString& wellPathName, RimEclipseResultCase* eclCase )
    {
        RimProject*                 proj              = RiaApplication::instance()->project();
        RimWellPath*                wellPath          = proj->wellPathByName( wellPathName );
        RimWellLogPlotCollection*   wellLogCollection = proj->mainPlotCollection()->wellLogPlotCollection();
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
        RifEclipseRftAddress gasRateAddress( RimWellPlotTools::simWellName( wellPathName ),
                                             m_timeStep,
                                             RifEclipseRftAddress::GRAT );
        RifEclipseRftAddress oilRateAddress( RimWellPlotTools::simWellName( wellPathName ),
                                             m_timeStep,
                                             RifEclipseRftAddress::ORAT );
        RifEclipseRftAddress watRateAddress( RimWellPlotTools::simWellName( wellPathName ),
                                             m_timeStep,
                                             RifEclipseRftAddress::WRAT );

        std::vector<caf::VecIjk> rftIndices;
        eclCase->rftReader()->cellIndices( gasRateAddress, &rftIndices );
        if ( rftIndices.empty() ) eclCase->rftReader()->cellIndices( oilRateAddress, &rftIndices );
        if ( rftIndices.empty() ) eclCase->rftReader()->cellIndices( watRateAddress, &rftIndices );
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
            caf::VecIjk ijkIndex        = rftIndices[rftCellIdx];
            size_t      globalCellIndex = mainGrid->cellIndexFromIJK( ijkIndex.i(), ijkIndex.j(), ijkIndex.k() );
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
            resPoint.m_isOpen        = true;
            resPoint.m_gridIndex     = 0; // Always main grid
            resPoint.m_gridCellIndex = globCellIdx; // Shortcut, since we only have main grid results from RFT

            resPoint.m_gasRate =
                RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents( eclCase->eclipseCaseData()->unitsType(),
                                                                                gasRates[it->second] );
            resPoint.m_oilRate   = oilRates[it->second];
            resPoint.m_waterRate = watRates[it->second];

            m_pipeBranchWellResultPoints.push_back( resPoint );

            if ( wpExIdx < intersections.size() - 1 )
            {
                m_pipeBranchWellResultPoints.push_back(
                    RigWellResultPoint() ); // Invalid res point describing the "line" between the cells
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
            eclCase->eclipseCaseData()->results( RiaDefines::MATRIX_MODEL )->timeStepDates();
        size_t tsIdx = timeSteps.size();
        for ( tsIdx = 0; tsIdx < timeSteps.size(); ++tsIdx )
        {
            if ( timeSteps[tsIdx] == m_timeStep ) break;
        }

        if ( tsIdx >= timeSteps.size() ) return;

        // Build search map into simulation well data

        std::map<size_t, std::pair<size_t, size_t>> globCellIdxToIdxInSimWellBranch;

        const RimWellPath*    wellPath = RimWellPlotTools::wellPathByWellPathNameOrSimWellName( wellPathName );
        const RigSimWellData* simWell  = wellPath != nullptr ? eclCase->eclipseCaseData()->findSimWellData(
                                                                  wellPath->associatedSimulationWellName() )
                                                            : nullptr;

        if ( !simWell ) return;

        if ( !simWell->hasWellResult( tsIdx ) ) return;

        const RigWellResultFrame& resFrame = simWell->wellResultFrame( tsIdx );

        const RigMainGrid* mainGrid = eclCase->eclipseCaseData()->mainGrid();

        for ( size_t brIdx = 0; brIdx < resFrame.m_wellResultBranches.size(); ++brIdx )
        {
            const std::vector<RigWellResultPoint>& branchResPoints = resFrame.m_wellResultBranches[brIdx].m_branchResultPoints;
            for ( size_t wrpIdx = 0; wrpIdx < branchResPoints.size(); wrpIdx++ )
            {
                const RigGridBase* grid = mainGrid->gridByIndex( branchResPoints[wrpIdx].m_gridIndex );
                size_t globalCellIndex  = grid->reservoirCellIndex( branchResPoints[wrpIdx].m_gridCellIndex );

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

            const RigWellResultPoint& resPoint =
                resFrame.m_wellResultBranches[it->second.first].m_branchResultPoints[it->second.second];

            m_pipeBranchWellResultPoints.push_back( resPoint );
            if ( wpExIdx < intersections.size() - 1 )
            {
                m_pipeBranchWellResultPoints.push_back(
                    RigWellResultPoint() ); // Invalid res point describing the "line" between the cells
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

    CAF_ASSERT( plotTrack );
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
        std::set<FlowPhase> selectedPhases = std::set<FlowPhase>( m_phases().begin(), m_phases().end() );

        RifDataSourceForRftPlt sourceDef = curveDefToAdd.address();
        QDateTime              timeStep  = curveDefToAdd.timeStep();

        std::unique_ptr<RigResultPointCalculator> resultPointCalc;

        QString curveName;

        {
            curveName += sourceDef.eclCase() ? sourceDef.eclCase()->caseUserDescription() : "";
            curveName += sourceDef.wellLogFile() ? sourceDef.wellLogFile()->name() : "";
            if ( sourceDef.sourceType() == RifDataSourceForRftPlt::RFT ) curveName += ", RFT";

            curveName += ", " + RiaQDateTimeTools::toStringUsingApplicationLocale( timeStep, dateFormatString );
        }

        RimEclipseResultCase* rimEclipseResultCase = dynamic_cast<RimEclipseResultCase*>( sourceDef.eclCase() );

        if ( sourceDef.sourceType() == RifDataSourceForRftPlt::RFT )
        {
            resultPointCalc.reset( new RigRftResultPointCalculator( m_wellPathName, rimEclipseResultCase, timeStep ) );
        }
        else if ( sourceDef.sourceType() == RifDataSourceForRftPlt::GRID )
        {
            resultPointCalc.reset( new RigSimWellResultPointCalculator( m_wellPathName, rimEclipseResultCase, timeStep ) );
        }

        RiaEclipseUnitTools::UnitSystem unitSet = RiaEclipseUnitTools::UNITS_UNKNOWN;
        if ( rimEclipseResultCase )
        {
            unitSet = rimEclipseResultCase->eclipseCaseData()->unitsType();
        }

        if ( resultPointCalc != nullptr )
        {
            if ( !resultPointCalc->pipeBranchCLCoords().empty() )
            {
                if ( selectedPhases.count( FLOW_PHASE_TOTAL ) && m_useReservoirConditionCurves() &&
                     sourceDef.sourceType() == RifDataSourceForRftPlt::GRID )
                {
                    RigAccWellFlowCalculator wfTotalAccumulator( resultPointCalc->pipeBranchCLCoords(),
                                                                 resultPointCalc->pipeBranchWellResultPoints(),
                                                                 resultPointCalc->pipeBranchMeasuredDepths(),
                                                                 true );

                    const std::vector<double>& depthValues = wfTotalAccumulator.pseudoLengthFromTop( 0 );

                    QString curveUnitText = RimWellPlotTools::flowUnitText( RimWellLogFile::WELL_FLOW_COND_RESERVOIR,
                                                                            unitSet );

                    const std::vector<double> accFlow =
                        wfTotalAccumulator.accumulatedTracerFlowPrPseudoLength( RIG_FLOW_TOTAL_NAME, 0 );
                    addStackedCurve( curveName + ", " + RIG_FLOW_TOTAL_NAME + " " + curveUnitText,
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
                        auto color = tracerName == RIG_FLOW_OIL_NAME
                                         ? cvf::Color3f::DARK_GREEN
                                         : tracerName == RIG_FLOW_GAS_NAME
                                               ? cvf::Color3f::DARK_RED
                                               : tracerName == RIG_FLOW_WATER_NAME ? cvf::Color3f::BLUE
                                                                                   : cvf::Color3f::DARK_GRAY;

                        if ( tracerName == RIG_FLOW_OIL_NAME && selectedPhases.count( FLOW_PHASE_OIL ) ||
                             tracerName == RIG_FLOW_GAS_NAME && selectedPhases.count( FLOW_PHASE_GAS ) ||
                             tracerName == RIG_FLOW_WATER_NAME && selectedPhases.count( FLOW_PHASE_WATER ) )
                        {
                            FlowPhase flowPhase = FLOW_PHASE_NONE;
                            if ( tracerName == RIG_FLOW_OIL_NAME )
                                flowPhase = FLOW_PHASE_OIL;
                            else if ( tracerName == RIG_FLOW_GAS_NAME )
                                flowPhase = FLOW_PHASE_GAS;
                            else if ( tracerName == RIG_FLOW_WATER_NAME )
                                flowPhase = FLOW_PHASE_WATER;
                            QString curveUnitText = RimWellPlotTools::curveUnitText( RimWellLogFile::WELL_FLOW_COND_STANDARD,
                                                                                     unitSet,
                                                                                     flowPhase );

                            const std::vector<double>& accFlow =
                                wfPhaseAccumulator.accumulatedTracerFlowPrPseudoLength( tracerName, 0 );
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
        else if ( sourceDef.sourceType() == RifDataSourceForRftPlt::OBSERVED )
        {
            if ( sourceDef.wellLogFile() && sourceDef.wellLogFile()->wellLogFileData() )
            {
                RimWellLogFile::WellFlowCondition flowCondition = sourceDef.wellLogFile()->wellFlowRateCondition();

                if ( ( m_useStandardConditionCurves() && flowCondition == RimWellLogFile::WELL_FLOW_COND_STANDARD ) ||
                     ( m_useReservoirConditionCurves() && flowCondition == RimWellLogFile::WELL_FLOW_COND_RESERVOIR ) )
                {
                    using ChannelValNameIdxTuple = std::tuple<double, QString, int>;

                    RigWellLogFile* wellLogFileData = sourceDef.wellLogFile()->wellLogFileData();

                    QStringList channelNames = wellLogFileData->wellLogChannelNames();

                    std::multiset<ChannelValNameIdxTuple> sortedChannels;
                    std::vector<std::vector<double>>      channelData;
                    channelData.resize( channelNames.size() );

                    for ( int chIdx = 0; chIdx < channelNames.size(); ++chIdx )
                    {
                        QString channelName = channelNames[chIdx];
                        channelData[chIdx]  = wellLogFileData->values( channelName );
                        if ( !channelData[chIdx].empty() )
                        {
                            sortedChannels.insert(
                                ChannelValNameIdxTuple( -fabs( channelData[chIdx].front() ), channelName, chIdx ) );
                        }
                    }

                    std::vector<double> depthValues = wellLogFileData->depthValues();

                    RiaEclipseUnitTools::UnitSystem unitSystem = RiaEclipseUnitTools::UNITS_UNKNOWN;
                    {
                        RiaDefines::DepthUnitType depthUnit = wellLogFileData->depthUnit();
                        if ( depthUnit == RiaDefines::UNIT_FEET ) unitSystem = RiaEclipseUnitTools::UNITS_FIELD;
                        if ( depthUnit == RiaDefines::UNIT_METER ) unitSystem = RiaEclipseUnitTools::UNITS_METRIC;
                    }

                    for ( const ChannelValNameIdxTuple& channelInfo : sortedChannels )
                    {
                        const auto& channelName = std::get<1>( channelInfo );
                        if ( selectedPhases.count( RimWellPlotTools::flowPhaseFromChannelName( channelName ) ) > 0 )
                        {
                            auto color = RimWellPlotTools::isOilFlowChannel( channelName )
                                             ? cvf::Color3f::DARK_GREEN
                                             : RimWellPlotTools::isGasFlowChannel( channelName )
                                                   ? cvf::Color3f::DARK_RED
                                                   : RimWellPlotTools::isWaterFlowChannel( channelName )
                                                         ? cvf::Color3f::BLUE
                                                         : cvf::Color3f::DARK_GRAY;

                            FlowPhase flowPhase = FLOW_PHASE_NONE;
                            if ( RimWellPlotTools::isOilFlowChannel( channelName ) )
                                flowPhase = FLOW_PHASE_OIL;
                            else if ( RimWellPlotTools::isGasFlowChannel( channelName ) )
                                flowPhase = FLOW_PHASE_GAS;
                            else if ( RimWellPlotTools::isWaterFlowChannel( channelName ) )
                                flowPhase = FLOW_PHASE_WATER;
                            QString curveUnitText = RimWellPlotTools::curveUnitText( flowCondition, unitSystem, flowPhase );

                            addStackedCurve( curveName + ", " + channelName + " " + curveUnitText,
                                             depthValues,
                                             channelData[std::get<2>( channelInfo )],
                                             plotTrack,
                                             color,
                                             curveGroupId,
                                             true );

                            // Total flow channel will end up first, so just increment the group
                            // idx to make the rest of the phases group together
                            if ( RimWellPlotTools::isTotalFlowChannel( channelName ) ) curveGroupId++;
                        }
                    }
                }
            }
        }
        curveGroupId++;
    }

    plotTrack->setAutoScaleXEnabled( true );
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
        curve->setSymbol( RiuQwtSymbol::SYMBOL_NONE );
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
std::vector<RifDataSourceForRftPlt> RimWellPltPlot::selectedSourcesExpanded() const
{
    std::vector<RifDataSourceForRftPlt> sources;
    for ( const RifDataSourceForRftPlt& addr : m_selectedSources() )
    {
        if ( addr.sourceType() == RifDataSourceForRftPlt::OBSERVED )
        {
            for ( RimWellLogFile* const wellLogFile : RimWellPlotTools::wellLogFilesContainingFlow( m_wellPathName ) )
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
QList<caf::PdmOptionItemInfo> RimWellPltPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                     bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimWellLogPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    const QString simWellName = RimWellPlotTools::simWellName( m_wellPathName );

    if ( fieldNeedingOptions == &m_wellPathName )
    {
        calculateValueOptionsForWells( options );
    }
    else if ( fieldNeedingOptions == &m_selectedSources )
    {
        std::set<RifDataSourceForRftPlt> optionAddresses;

        const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell( simWellName );
        std::set<RifDataSourceForRftPlt>         availableRftSources;

        for ( const auto& rftCase : rftCases )
        {
            std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse =
                RifEclipseRftAddress::pltPlotChannelTypes();

            std::set<QDateTime> rftTimes = rftCase->rftReader()->availableTimeSteps( simWellName, channelTypesToUse );
            if ( !rftTimes.empty() )
            {
                availableRftSources.insert( RifDataSourceForRftPlt( RifDataSourceForRftPlt::RFT, rftCase ) );
            }
        }

        if ( !availableRftSources.empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::RFT ),
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
                                                                         RifDataSourceForRftPlt::GRID ),
                                                                     true ) );
        }

        for ( const auto& gridCase : gridCases )
        {
            auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::GRID, gridCase );
            auto item = caf::PdmOptionItemInfo( gridCase->caseUserDescription(), QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
        }

        if ( !RimWellPlotTools::wellLogFilesContainingFlow( m_wellPathName ).empty() )
        {
            options.push_back( caf::PdmOptionItemInfo::createHeader( RifDataSourceForRftPlt::sourceTypeUiText(
                                                                         RifDataSourceForRftPlt::OBSERVED ),
                                                                     true ) );

            auto addr = RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED );
            auto item = caf::PdmOptionItemInfo( "Observed Data", QVariant::fromValue( addr ) );
            item.setLevel( 1 );
            options.push_back( item );
            optionAddresses.insert( addr );
        }
    }
    else if ( fieldNeedingOptions == &m_selectedTimeSteps )
    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse =
            RifEclipseRftAddress::pltPlotChannelTypes();

        RimWellPlotTools::calculateValueOptionsForTimeSteps( RimWellPlotTools::simWellName( m_wellPathName ),
                                                             selectedSourcesExpanded(),
                                                             channelTypesToUse,
                                                             options );
    }

    if ( fieldNeedingOptions == &m_phases )
    {
        options.push_back( caf::PdmOptionItemInfo( "Oil", FLOW_PHASE_OIL ) );
        options.push_back( caf::PdmOptionItemInfo( "Gas", FLOW_PHASE_GAS ) );
        options.push_back( caf::PdmOptionItemInfo( "Water", FLOW_PHASE_WATER ) );
        options.push_back( caf::PdmOptionItemInfo( "Total", FLOW_PHASE_TOTAL ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    RimWellLogPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellPathName )
    {
        setMultiPlotTitle( QString( plotNameFormatString() ).arg( m_wellPathName ) );
    }

    if ( changedField == &m_wellPathName )
    {
        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        CAF_ASSERT( plotTrack );
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
        RimProject*  project  = RiaApplication::instance()->project();
        RimWellPath* wellPath = project->wellPathByName( m_wellPathName() );
        if ( wellPath && !wellPath->wellPathGeometry() )
        {
            for ( const RifDataSourceForRftPlt& address : m_selectedSources() )
            {
                if ( address.sourceType() == RifDataSourceForRftPlt::RFT ||
                     address.sourceType() == RifDataSourceForRftPlt::GRID )
                {
                    if ( !wellPath->wellPathGeometry() )
                    {
                        QString tmp = QString(
                            "Display of Measured Depth (MD) for Grid or RFT curves is not possible without a "
                            "well log path, and the curve will be hidden in this mode.\n\n" );

                        QMessageBox::warning( nullptr, "Grid/RFT curve without MD", tmp );

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
        CAF_ASSERT( plotTrack );
        if ( plotTrack )
        {
            plotTrack->setAutoScaleXEnabled( true );
        }
        updateZoom();
    }

    if ( changedField == &m_useStandardConditionCurves || changedField == &m_useReservoirConditionCurves ||
         changedField == &m_phases )
    {
        syncCurvesFromUiSelection();

        RimWellLogTrack* const plotTrack = dynamic_cast<RimWellLogTrack*>( plotByIndex( 0 ) );
        CAF_ASSERT( plotTrack );
        if ( plotTrack )
        {
            plotTrack->setAutoScaleXEnabled( true );
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
        CAF_ASSERT( track );
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
void RimWellPltPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                            QString                    uiConfigName,
                                            caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_phases )
    {
        caf::PdmUiTreeSelectionEditorAttribute* attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>(
            attribute );
        attrib->showTextFilter        = false;
        attrib->showToggleAllCheckbox = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::initAfterRead()
{
    RimViewWindow::initAfterRead();

    if ( m_wellLogPlot_OBSOLETE )
    {
        RimWellLogPlot& wellLogPlot = dynamic_cast<RimWellLogPlot&>( *this );
        wellLogPlot                 = std::move( *m_wellLogPlot_OBSOLETE.value() );
        delete m_wellLogPlot_OBSOLETE;
        m_wellLogPlot_OBSOLETE = nullptr;
    }

    if ( m_showPlotTitle_OBSOLETE() && !m_showPlotWindowTitle() )
    {
        m_showPlotWindowTitle = m_showPlotTitle_OBSOLETE();
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
        selectedSources.push_back( addr->address() );
    }
    m_selectedSources = selectedSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::syncSourcesIoFieldFromGuiField()
{
    m_selectedSourcesForIo.clear();

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
    RimProject* proj = RiaApplication::instance()->project();

    if ( proj != nullptr )
    {
        // Observed wells
        for ( const RimWellPath* const wellPath : proj->allWellPaths() )
        {
            const QString wellName = wellPath->name();

            if ( wellPath->wellPathGeometry() || RimWellPlotTools::hasFlowData( wellPath ) )
                options.push_back( caf::PdmOptionItemInfo( wellName, wellName ) );
        }
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
            CAF_ASSERT( plotTrack );
            if ( plotTrack )
            {
                plotTrack->setAnnotationType( RiuPlotAnnotationTool::FORMATION_ANNOTATIONS );
            }
        }
        m_isOnLoad = false;
    }

    updateMdiWindowVisibility();
    updateFormationsOnPlot();
    syncCurvesFromUiSelection();
}
