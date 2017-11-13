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
#include "RiaWellNameComparer.h"
#include "RifReaderEclipseRft.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"
#include "RigWellPath.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimOilField.h"
#include "RimProject.h"
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
#include "RimWellFlowRateCurve.h"
#include "RiuWellPltPlot.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include <tuple>
#include <algorithm>
#include <iterator>
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"
#include "RigWellLogExtractor.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "cafVecIjk.h"
#include "RigAccWellFlowCalculator.h"
#include "RimSummaryCurveAppearanceCalculator.h"


CAF_PDM_SOURCE_INIT(RimWellPltPlot, "WellPltPlot"); 

namespace caf
{
template<>
void caf::AppEnum< FlowType>::setUp()
{
    addItem(FLOW_TYPE_TOTAL, "TOTAL", "Total Flow");
    addItem(FLOW_TYPE_PHASE_SPLIT, "PHASE_SPLIT", "Phase Split");
}

template<>
void caf::AppEnum< FlowPhase>::setUp()
{
    addItem(FLOW_PHASE_OIL, "PHASE_OIL", "Oil");
    addItem(FLOW_PHASE_GAS, "PHASE_GAS", "Gas");
    addItem(FLOW_PHASE_WATER, "PHASE_WATER", "Water");
}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char RimWellPltPlot::PLOT_NAME_QFORMAT_STRING[] = "PLT: %1";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPltPlot::RimWellPltPlot()
{
    CAF_PDM_InitObject("Well Allocation Plot", ":/WellAllocPlot16x16.png", "", "");

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("PLT Plot"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogPlot, "WellLog", "WellLog", "", "", "");
    m_wellLogPlot.uiCapability()->setUiHidden(true);
    m_wellLogPlot = new RimWellLogPlot();
    m_wellLogPlot->setDepthType(RimWellLogPlot::MEASURED_DEPTH);
    m_wellLogPlot.uiCapability()->setUiTreeHidden(true);
    m_wellLogPlot.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellPathName, "WellName", "WellName", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSources, "SourcesInternal", "SourcesInternal", "", "", "");
    m_selectedSources.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSources.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue(false);
    m_selectedSources.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_selectedSourcesForIo, "Sources", "Sources", "", "", "");
    m_selectedSourcesForIo.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_selectedTimeSteps, "TimeSteps", "TimeSteps", "", "", "");
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedTimeSteps.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_phaseSelectionMode, "PhaseSelectionMode", "Mode", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_phases, "Phases", "Phases", "", "", "");
    m_phases.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_phases = std::vector<caf::AppEnum<FlowPhase>>({ FLOW_PHASE_OIL, FLOW_PHASE_GAS, FLOW_PHASE_WATER });

    this->setAsPlotMdiWindow();
    m_doInitAfterLoad = false;
    m_isOnLoad = true;
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
void RimWellPltPlot::deleteViewWidget()
{
    if (m_wellLogPlotWidget)
    {
        m_wellLogPlotWidget->deleteLater();
        m_wellLogPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//void RimWellPltPlot::applyCurveAppearance(RimWellLogCurve* newCurve)
//{
//    const std::pair<RifWellRftAddress, QDateTime>& newCurveDef = curveDefFromCurve(newCurve);
//
//    std::vector<cvf::Color3f> colorTable;
//    RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector(&colorTable);
//
//    std::vector<RimPlotCurve::PointSymbolEnum> symbolTable =
//    {
//        RimPlotCurve::SYMBOL_ELLIPSE,
//        RimPlotCurve::SYMBOL_RECT,
//        RimPlotCurve::SYMBOL_DIAMOND,
//        RimPlotCurve::SYMBOL_TRIANGLE,
//        RimPlotCurve::SYMBOL_CROSS,
//        RimPlotCurve::SYMBOL_XCROSS
//    };
//
//    // State variables
//    static size_t  defaultColorTableIndex = 0;
//    static size_t  defaultSymbolTableIndex = 0;
//
//    cvf::Color3f                    currentColor;
//    RimPlotCurve::PointSymbolEnum   currentSymbol = symbolTable.front();
//    RimPlotCurve::LineStyleEnum     currentLineStyle = RimPlotCurve::STYLE_SOLID;
//    bool                            isCurrentColorSet = false;
//    bool                            isCurrentSymbolSet = false;
//
//    std::set<cvf::Color3f>                  assignedColors;
//    std::set<RimPlotCurve::PointSymbolEnum> assignedSymbols;
//
//    // Used colors and symbols
//    for (RimWellLogCurve* const curve : m_wellLogPlot->trackByIndex(0)->curvesVector())
//    {
//        if (curve == newCurve) continue;
//
//        std::pair<RifWellRftAddress, QDateTime> cDef = curveDefFromCurve(curve);
//        if (cDef.first == newCurveDef.first)
//        {
//            currentColor = curve->color();
//            isCurrentColorSet = true;
//        }
//        if (cDef.second == newCurveDef.second)
//        {
//            currentSymbol = curve->symbol();
//            isCurrentSymbolSet = true;
//        }
//        assignedColors.insert(curve->color());
//        assignedSymbols.insert(curve->symbol());
//    }
//
//    // Assign color
//    if (!isCurrentColorSet)
//    {
//        for(const auto& color : colorTable)
//        {
//            if (assignedColors.count(color) == 0)
//            {
//                currentColor = color;
//                isCurrentColorSet = true;
//                break;
//            }
//        }
//        if (!isCurrentColorSet)
//        {
//            currentColor = colorTable[defaultColorTableIndex];
//            if (++defaultColorTableIndex == colorTable.size())
//                defaultColorTableIndex = 0;
//
//        }
//    }
//
//    // Assign symbol
//    if (!isCurrentSymbolSet)
//    {
//        for (const auto& symbol : symbolTable)
//        {
//            if (assignedSymbols.count(symbol) == 0)
//            {
//                currentSymbol = symbol;
//                isCurrentSymbolSet = true;
//                break;
//            }
//        }
//        if (!isCurrentSymbolSet)
//        {
//            currentSymbol = symbolTable[defaultSymbolTableIndex];
//            if (++defaultSymbolTableIndex == symbolTable.size())
//                defaultSymbolTableIndex = 0;
//        }
//    }
//
//    // Observed data
//    currentLineStyle = newCurveDef.first.sourceType() == RftSourceType::OBSERVED
//        ? RimPlotCurve::STYLE_NONE : RimPlotCurve::STYLE_SOLID;
//
//    newCurve->setColor(currentColor);
//    newCurve->setSymbol(currentSymbol);
//    newCurve->setLineStyle(currentLineStyle);
//}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::updateSelectedTimeStepsFromSelectedSources()
{
    std::vector<QDateTime> newTimeStepsSelections;
    std::vector<RifWellRftAddress> selectedSourcesVector = m_selectedSources();
    auto selectedSources = std::set<RifWellRftAddress>(selectedSourcesVector.begin(), selectedSourcesVector.end());

    for (const QDateTime& timeStep : m_selectedTimeSteps())
    {
        if(m_timeStepsToAddresses.count(timeStep) > 0)
        {
            std::vector<RifWellRftAddress> intersectVector;
            const std::set<RifWellRftAddress>& addresses = m_timeStepsToAddresses[timeStep];
            std::set_intersection(selectedSources.begin(), selectedSources.end(),
                                  addresses.begin(), addresses.end(), std::inserter(intersectVector, intersectVector.end()));
            if(intersectVector.size() > 0)
            {
                newTimeStepsSelections.push_back(timeStep);
            }
        }
    }
    m_selectedTimeSteps = newTimeStepsSelections;
}
#endif
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::setPlotXAxisTitles(RimWellLogTrack* plotTrack)
{
    std::vector<RimEclipseCase*> cases = eclipseCases();

    RiaEclipseUnitTools::UnitSystem unitSet = !cases.empty() ? 
        cases.front()->eclipseCaseData()->unitsType() : 
        RiaEclipseUnitTools::UNITS_UNKNOWN;

    QString unitText;
    switch (unitSet)
    {
    case RiaEclipseUnitTools::UNITS_METRIC:
        unitText = "[Liquid Sm<sup>3</sup>/day], [Gas kSm<sup>3</sup>/day]";
        break;
    case RiaEclipseUnitTools::UNITS_FIELD:
        unitText = "[Liquid BBL/day], [Gas BOE/day]";
        break;
    case RiaEclipseUnitTools::UNITS_LAB:
        unitText = "[cm<sup>3</sup>/hr]";
        break;
    default:
        unitText = "(unknown unit)";
        break;

    }
    plotTrack->setXAxisTitle("Surface Flow Rate " + unitText);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimWellPltPlot::eclipseCases() const
{
    std::vector<RimEclipseCase*> cases;
    RimProject* proj = RiaApplication::instance()->project();
    for (const auto& oilField : proj->oilFields)
    {
        for (const auto& eclCase : oilField->analysisModels()->cases)
        {
            cases.push_back(eclCase);
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::updateFormationsOnPlot() const
{
    if (m_wellLogPlot->trackCount() > 0)
    {
        RimProject* proj = RiaApplication::instance()->project();
        RimWellPath* wellPath = proj->wellPathByName(m_wellPathName);

        RimCase* formationNamesCase = m_wellLogPlot->trackByIndex(0)->formationNamesCase();

        if ( !formationNamesCase )
        {
            /// Set default case. Todo : Use the first of the selected cases in the plot
            std::vector<RimCase*> cases;
            proj->allCases(cases);

            if ( !cases.empty() )
            {
                formationNamesCase = cases[0];
            }
        }
        
        m_wellLogPlot->trackByIndex(0)->setAndUpdateWellPathFormationNamesData(formationNamesCase, wellPath);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
//void RimWellPltPlot::applyInitialSelections()
//{
//    std::vector<std::tuple<RimEclipseResultCase*, bool, bool>> eclCaseTuples = eclipseCasesForWell(m_wellName);
//
//    std::vector<RifWellRftAddress> sourcesToSelect;
//    std::map<QDateTime, std::set<RifWellRftAddress>> rftTimeStepsMap;
//    std::map<QDateTime, std::set<RifWellRftAddress>> observedTimeStepsMap;
//    std::map<QDateTime, std::set<RifWellRftAddress>> gridTimeStepsMap;
//
//    for(RimEclipseResultCase* const rftCase : rftCasesFromEclipseCases(eclCaseTuples))
//    {
//        sourcesToSelect.push_back(RifWellRftAddress(RftSourceType::RFT, rftCase));
//        addTimeStepsToMap(rftTimeStepsMap, timeStepsFromRftCase(rftCase));
//    }
//    
//    for (RimEclipseResultCase* const gridCase : gridCasesFromEclipseCases(eclCaseTuples))
//    {
//        sourcesToSelect.push_back(RifWellRftAddress(RftSourceType::GRID, gridCase));
//        addTimeStepsToMap(gridTimeStepsMap, timeStepsFromGridCase(gridCase));
//    }
//    
//    std::vector<RimWellLogFile*> wellLogFiles = wellLogFilesContainingFlow(m_wellName);
//    if(wellLogFiles.size() > 0)
//    {
//        sourcesToSelect.push_back(RifWellRftAddress(RftSourceType::OBSERVED));
//        for (RimWellLogFile* const wellLogFile : wellLogFiles)
//        {
//            addTimeStepsToMap(observedTimeStepsMap, timeStepsFromWellLogFile(wellLogFile));
//        }
//    }
//
//    m_selectedSources = sourcesToSelect;
//    
//    std::set<QDateTime> timeStepsToSelect;
//    for (const std::pair<QDateTime, std::set<RifWellRftAddress>>& dateTimePair : rftTimeStepsMap)
//    {
//        timeStepsToSelect.insert(dateTimePair.first);
//    }
//    for (const std::pair<QDateTime, std::set<RifWellRftAddress>>& dateTimePair : observedTimeStepsMap)
//    {
//        timeStepsToSelect.insert(dateTimePair.first);
//    }
//    if (gridTimeStepsMap.size() > 0)
//        timeStepsToSelect.insert((*gridTimeStepsMap.begin()).first);
//
//    m_selectedTimeSteps = std::vector<QDateTime>(timeStepsToSelect.begin(), timeStepsToSelect.end());
//
//    syncCurvesFromUiSelection();
//}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::updateWidgetTitleWindowTitle()
{
    updateMdiWindowTitle();

    if (m_wellLogPlotWidget)
    {
        if (m_showPlotTitle)
        {
            m_wellLogPlotWidget->showTitle(m_userName);
        }
        else
        {
            m_wellLogPlotWidget->hideTitle();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set < std::pair<RifWellRftAddress, QDateTime>> RimWellPltPlot::selectedCurveDefs() const
{
    std::set<std::pair<RifWellRftAddress, QDateTime>> curveDefs;
    const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell(RimWellPlotTools::simWellName(m_wellPathName));
    const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell(RimWellPlotTools::simWellName(m_wellPathName));
    const QString simWellName = RimWellPlotTools::simWellName(m_wellPathName);

    for (const QDateTime& timeStep : m_selectedTimeSteps())
    {
        for (const RifWellRftAddress& addr : selectedSources())
        {
            if (addr.sourceType() == RifWellRftAddress::RFT)
            {
                for (RimEclipseResultCase* const rftCase : rftCases)
                {
                    const std::set<QDateTime>& timeSteps = RimWellPlotTools::timeStepsFromRftCase(rftCase, simWellName);
                    if (timeSteps.count(timeStep) > 0)
                    {
                        curveDefs.insert(std::make_pair(addr, timeStep));
                    }
                }
            }
            else if (addr.sourceType() == RifWellRftAddress::GRID)
            {
                for (RimEclipseResultCase* const gridCase : gridCases)
                {
                    const std::set<QDateTime>& timeSteps = RimWellPlotTools::timeStepsFromGridCase(gridCase);
                    if (timeSteps.count(timeStep) > 0)
                    {
                        curveDefs.insert(std::make_pair(addr, timeStep));
                    }
                }
            }
            else
            if (addr.sourceType() == RifWellRftAddress::OBSERVED)
            {
                if (addr.wellLogFile() != nullptr)
                {
                    const QDateTime& wellLogFileTimeStep = RimWellPlotTools::timeStepFromWellLogFile(addr.wellLogFile());
                    if (wellLogFileTimeStep == timeStep)
                    {
                        curveDefs.insert(std::make_pair(RifWellRftAddress(RifWellRftAddress::OBSERVED, addr.wellLogFile()), timeStep));
                    }
                }
            }
        }
    }
    return curveDefs;
}
#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::pair<RifWellRftAddress, QDateTime>> RimWellPltPlot::curveDefsFromCurves() const
{
    std::set<std::pair<RifWellRftAddress, QDateTime>> curveDefs;

    RimWellLogTrack* const plotTrack = m_wellLogPlot->trackByIndex(0);
    for (RimWellLogCurve* const curve : plotTrack->curvesVector())
    {
        curveDefs.insert(curveDefFromCurve(curve));
    }
    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<RifWellRftAddress, QDateTime> RimWellPltPlot::curveDefFromCurve(const RimWellLogCurve* curve) const
{
    const RimWellLogRftCurve* rftCurve = dynamic_cast<const RimWellLogRftCurve*>(curve);
    const RimWellLogExtractionCurve* gridCurve = dynamic_cast<const RimWellLogExtractionCurve*>(curve);
    const RimWellLogFileCurve* wellLogFileCurve = dynamic_cast<const RimWellLogFileCurve*>(curve);

    //if (rftCurve != nullptr)
    //{
    //    RimEclipseResultCase* rftCase = dynamic_cast<RimEclipseResultCase*>(rftCurve->eclipseResultCase());
    //    if (rftCase != nullptr)
    //    {
    //        const RifEclipseRftAddress rftAddress = rftCurve->rftAddress();
    //        const QDateTime timeStep = rftAddress.timeStep();
    //        return std::make_pair(RifWellRftAddress(RftSourceType::RFT, rftCase), timeStep);
    //    }
    //}
    //else if (gridCurve != nullptr)
    //{
    //    RimEclipseResultCase* gridCase = dynamic_cast<RimEclipseResultCase*>(gridCurve->rimCase());
    //    if (gridCase != nullptr)
    //    {
    //        size_t timeStepIndex = gridCurve->currentTimeStep();
    //        const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsMap = timeStepsFromGridCase(gridCase);
    //        auto timeStepsVector = std::vector<std::pair<QDateTime, std::set<RifWellRftAddress>>>(
    //            timeStepsMap.begin(), timeStepsMap.end());
    //        if (timeStepIndex < timeStepsMap.size())
    //        {
    //            return std::make_pair(RifWellRftAddress(RftSourceType::GRID, gridCase),
    //                                  timeStepsVector[timeStepIndex].first);
    //        }
    //    }
    //}
    //else
    if (wellLogFileCurve != nullptr)
    {
        const RimWellPath* const wellPath = wellLogFileCurve->wellPath();
        RimWellLogFile* const wellLogFile = wellLogFileCurve->wellLogFile();

        if (wellLogFile != nullptr)
        {
            const QDateTime date = wellLogFile->date();

            if (date.isValid())
            {
                return std::make_pair(RifWellRftAddress(RifWellRftAddress::OBSERVED, wellLogFile), date);
            }
        }
    }
    return std::make_pair(RifWellRftAddress(), QDateTime());
}
#endif
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

class RigResultPointCalculator
{
public:
    RigResultPointCalculator() {}
    virtual ~RigResultPointCalculator() {}

    const std::vector <cvf::Vec3d>&         pipeBranchCLCoords()         { return m_pipeBranchCLCoords;         }
    const std::vector <RigWellResultPoint>& pipeBranchWellResultPoints() { return m_pipeBranchWellResultPoints; }
    const std::vector <double>&             pipeBranchMeasuredDepths()   { return m_pipeBranchMeasuredDepths;   }

protected:

    RigEclipseWellLogExtractor*  findWellLogExtractor(const QString& wellPathName,
                                                      RimEclipseResultCase* eclCase)
    {
        RimProject* proj = RiaApplication::instance()->project();
        RimWellPath* wellPath = proj->wellPathByName(wellPathName);
        RimWellLogPlotCollection* wellLogCollection = proj->mainPlotCollection()->wellLogPlotCollection();
        RigEclipseWellLogExtractor* eclExtractor = wellLogCollection->findOrCreateExtractor(wellPath, eclCase);

        return eclExtractor;
    }


    std::vector <cvf::Vec3d>          m_pipeBranchCLCoords;
    std::vector <RigWellResultPoint>  m_pipeBranchWellResultPoints;
    std::vector <double>              m_pipeBranchMeasuredDepths;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

class RigRftResultPointCalculator : public RigResultPointCalculator
{
public:
    RigRftResultPointCalculator(const QString& wellPathName, 
                                RimEclipseResultCase* eclCase,
                                QDateTime m_timeStep)
    {

        RifEclipseRftAddress gasRateAddress(RimWellPlotTools::simWellName(wellPathName), m_timeStep, RifEclipseRftAddress::GRAT);
        RifEclipseRftAddress oilRateAddress(RimWellPlotTools::simWellName(wellPathName), m_timeStep, RifEclipseRftAddress::ORAT);
        RifEclipseRftAddress watRateAddress(RimWellPlotTools::simWellName(wellPathName), m_timeStep, RifEclipseRftAddress::WRAT);

        std::vector<caf::VecIjk> rftIndices;
        eclCase->rftReader()->cellIndices(gasRateAddress, &rftIndices);
        if (rftIndices.empty()) eclCase->rftReader()->cellIndices(oilRateAddress, &rftIndices);
        if (rftIndices.empty()) eclCase->rftReader()->cellIndices(watRateAddress, &rftIndices);
        if (rftIndices.empty()) return;

        std::vector<double> gasRates;
        std::vector<double> oilRates;
        std::vector<double> watRates;
        eclCase->rftReader()->values(gasRateAddress, &gasRates);
        eclCase->rftReader()->values(oilRateAddress, &oilRates);
        eclCase->rftReader()->values(watRateAddress, &watRates);

        std::map<size_t, size_t> globCellIdxToIdxInRftFile;

        const RigMainGrid* mainGrid = eclCase->eclipseCaseData()->mainGrid();

        for (size_t rftCellIdx = 0; rftCellIdx < rftIndices.size(); rftCellIdx++)
        {
            caf::VecIjk ijkIndex = rftIndices[rftCellIdx];
            size_t globalCellIndex = mainGrid->cellIndexFromIJK(ijkIndex.i(), ijkIndex.j(), ijkIndex.k());
            globCellIdxToIdxInRftFile[globalCellIndex] = rftCellIdx;
        }

        RigEclipseWellLogExtractor* eclExtractor = findWellLogExtractor(wellPathName, eclCase);
        if (!eclExtractor) return;

        std::vector<CellIntersectionInfo> intersections = eclExtractor->intersectionInfo();

        for (size_t wpExIdx = 0; wpExIdx < intersections.size(); wpExIdx++)
        {
            size_t globCellIdx = intersections[wpExIdx].globCellIndex;

            auto it = globCellIdxToIdxInRftFile.find(globCellIdx);
            if (it == globCellIdxToIdxInRftFile.end())
            {
                if (wpExIdx == (intersections.size() - 1))
                {
                    m_pipeBranchWellResultPoints.pop_back();
                }
                continue;
            }
            m_pipeBranchCLCoords.push_back(intersections[wpExIdx].startPoint);
            m_pipeBranchMeasuredDepths.push_back(intersections[wpExIdx].startMD);

            m_pipeBranchCLCoords.push_back(intersections[wpExIdx].endPoint);
            m_pipeBranchMeasuredDepths.push_back(intersections[wpExIdx].endMD);

            RigWellResultPoint resPoint;
            resPoint.m_isOpen = true;
            resPoint.m_gridIndex = 0; // Always main grid
            resPoint.m_gridCellIndex = globCellIdx; // Shortcut, since we only have main grid results from RFT

            resPoint.m_gasRate   =   RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents(eclCase->eclipseCaseData()->unitsType(),  
                                                                                                    gasRates[it->second]);
            resPoint.m_oilRate   = oilRates[it->second];  
            resPoint.m_waterRate = watRates[it->second];

            m_pipeBranchWellResultPoints.push_back(resPoint);
        
            if ( wpExIdx < intersections.size() - 1 )
            {
                m_pipeBranchWellResultPoints.push_back(RigWellResultPoint()); // Invalid res point describing the "line" between the cells
            }
        }
    }


};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigSimWellResultPointCalculator: public RigResultPointCalculator
{
public:
    RigSimWellResultPointCalculator(const QString& wellPathName, 
                                    RimEclipseResultCase* eclCase,
                                    QDateTime m_timeStep)
    {
        // Find timestep index from qdatetime

        const std::vector<QDateTime> timeSteps = eclCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->timeStepDates();
        size_t tsIdx = timeSteps.size();
        for ( tsIdx = 0; tsIdx < timeSteps.size(); ++tsIdx) { if (timeSteps[tsIdx] == m_timeStep) break; }

        if (tsIdx >= timeSteps.size()) return;

        // Build search map into simulation well data
         
        std::map<size_t, std::pair<size_t, size_t> > globCellIdxToIdxInSimWellBranch;

        const RimWellPath* wellPath = RimWellPlotTools::wellPathByWellPathNameOrSimWellName(wellPathName);
        const RigSimWellData* simWell = wellPath != nullptr ? eclCase->eclipseCaseData()->findSimWellData(wellPath->associatedSimulationWell()) : nullptr;

        if (!simWell) return;

        if (!simWell->hasWellResult(tsIdx)) return;

        const RigWellResultFrame & resFrame = simWell->wellResultFrame(tsIdx);

        const RigMainGrid* mainGrid = eclCase->eclipseCaseData()->mainGrid();

        for (size_t brIdx = 0; brIdx < resFrame.m_wellResultBranches.size(); ++brIdx)
        {
            const std::vector<RigWellResultPoint>& branchResPoints =  resFrame.m_wellResultBranches[brIdx].m_branchResultPoints;
            for ( size_t wrpIdx = 0; wrpIdx < branchResPoints.size(); wrpIdx++ )
            {
                const RigGridBase* grid = mainGrid->gridByIndex(branchResPoints[wrpIdx].m_gridIndex);
                size_t globalCellIndex = grid->reservoirCellIndex(branchResPoints[wrpIdx].m_gridCellIndex);

                globCellIdxToIdxInSimWellBranch[globalCellIndex] = std::make_pair(brIdx, wrpIdx);
            }
        }

        RigEclipseWellLogExtractor* eclExtractor = findWellLogExtractor(wellPathName, eclCase);

        if (!eclExtractor) return;
        
        std::vector<CellIntersectionInfo> intersections = eclExtractor->intersectionInfo();

        for (size_t wpExIdx = 0; wpExIdx < intersections.size(); wpExIdx++)
        {
            size_t globCellIdx = intersections[wpExIdx].globCellIndex;

            auto it = globCellIdxToIdxInSimWellBranch.find(globCellIdx);
            if (it == globCellIdxToIdxInSimWellBranch.end())
            {
                if (wpExIdx == (intersections.size() - 1))
                {
                    m_pipeBranchWellResultPoints.pop_back();
                }
                continue;
            }

            m_pipeBranchCLCoords.push_back(intersections[wpExIdx].startPoint);
            m_pipeBranchMeasuredDepths.push_back(intersections[wpExIdx].startMD);

            m_pipeBranchCLCoords.push_back(intersections[wpExIdx].endPoint);
            m_pipeBranchMeasuredDepths.push_back(intersections[wpExIdx].endMD);

            const RigWellResultPoint& resPoint = resFrame.m_wellResultBranches[it->second.first].m_branchResultPoints[it->second.second];

            m_pipeBranchWellResultPoints.push_back(resPoint);
            if ( wpExIdx < intersections.size() - 1 )
            {
                m_pipeBranchWellResultPoints.push_back(RigWellResultPoint()); // Invalid res point describing the "line" between the cells
            }
        }
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::syncCurvesFromUiSelection()
{
    RimWellLogTrack* plotTrack = m_wellLogPlot->trackByIndex(0);
    const std::set<std::pair<RifWellRftAddress, QDateTime>>& curveDefs = selectedCurveDefs();

    setPlotXAxisTitles(plotTrack);

    // Delete existing curves
    const auto& curves = plotTrack->curvesVector();
    for (const auto& curve : curves)
    {
        plotTrack->removeCurve(curve);
    }

    int curveGroupId = 0;

    RimProject* proj = RiaApplication::instance()->project();
    RimWellPath* wellPath = RimWellPlotTools::wellPathByWellPathNameOrSimWellName(m_wellPathName);

    // Add curves
    for (const std::pair<RifWellRftAddress, QDateTime>& curveDefToAdd : curveDefs)
    {
        std::set<FlowPhase> selectedPhases = m_phaseSelectionMode == FLOW_TYPE_PHASE_SPLIT ?
            std::set<FlowPhase>(m_phases().begin(), m_phases().end()) :
            std::set<FlowPhase>({ FLOW_PHASE_TOTAL });
        
        RifWellRftAddress sourceDef = curveDefToAdd.first;
        QDateTime timeStep = curveDefToAdd.second;

        std::unique_ptr<RigResultPointCalculator> resultPointCalc;

        QString curveName;
        {
            curveName += sourceDef.eclCase() ? sourceDef.eclCase()->caseUserDescription() : "";
            curveName += sourceDef.wellLogFile() ? sourceDef.wellLogFile()->name() : "";
            if ( sourceDef.sourceType() == RifWellRftAddress::RFT ) curveName += ", RFT";
            curveName += ", " + timeStep.toString();
        }

        if ( sourceDef.sourceType() == RifWellRftAddress::RFT )
        {
            resultPointCalc.reset(new RigRftResultPointCalculator(m_wellPathName,
                                                                  dynamic_cast<RimEclipseResultCase*>(sourceDef.eclCase()),
                                                                  timeStep));
        }
        else if (sourceDef.sourceType() == RifWellRftAddress::GRID)
        {
            resultPointCalc.reset(new RigSimWellResultPointCalculator(m_wellPathName,
                                                                      dynamic_cast<RimEclipseResultCase*>(sourceDef.eclCase()),
                                                                      timeStep));

        }

        if (resultPointCalc != nullptr)
        {
             if ( resultPointCalc->pipeBranchCLCoords().size() )
             {
                
                 RigAccWellFlowCalculator wfAccumulator(resultPointCalc->pipeBranchCLCoords(),
                                                        resultPointCalc->pipeBranchWellResultPoints(),
                                                        resultPointCalc->pipeBranchMeasuredDepths(),
                                                        false); // m_phaseSelectionMode() != FLOW_TYPE_PHASE_SPLIT); The total flow is reservoir conditions must be careful

                 const std::vector<double>& depthValues = wfAccumulator.pseudoLengthFromTop(0);
                 std::vector<QString> tracerNames = wfAccumulator.tracerNames();
                 for ( const QString& tracerName: tracerNames )
                 {
                     auto color = tracerName == RIG_FLOW_OIL_NAME   ? cvf::Color3f::DARK_GREEN :
                                  tracerName == RIG_FLOW_GAS_NAME   ? cvf::Color3f::DARK_RED :
                                  tracerName == RIG_FLOW_WATER_NAME ? cvf::Color3f::BLUE :
                                  cvf::Color3f::DARK_GRAY;

                     if (    tracerName == RIG_FLOW_OIL_NAME  && selectedPhases.count(FLOW_PHASE_OIL) 
                          || tracerName == RIG_FLOW_GAS_NAME  && selectedPhases.count(FLOW_PHASE_GAS)
                          || tracerName == RIG_FLOW_WATER_NAME  && selectedPhases.count(FLOW_PHASE_WATER)
                          || tracerName == RIG_FLOW_TOTAL_NAME &&  selectedPhases.count(FLOW_PHASE_TOTAL) )
                     {
                         const std::vector<double> accFlow = wfAccumulator.accumulatedTracerFlowPrPseudoLength(tracerName, 0);
                         addStackedCurve(curveName + ", " + tracerName, 
                                         depthValues, 
                                         accFlow, 
                                         plotTrack, 
                                         color, 
                                         curveGroupId, 
                                         false);
                     }
                 }
             }
        }
        else if ( sourceDef.sourceType() == RifWellRftAddress::OBSERVED )
        {
            RimWellLogFile* const wellLogFile = sourceDef.wellLogFile();
            if ( wellLogFile )
            {
                RigWellLogFile* rigWellLogFile = wellLogFile->wellLogFile();

                if ( rigWellLogFile )
                {
                    for (RimWellLogFileChannel* channel : RimWellPlotTools::getFlowChannelsFromWellFile(wellLogFile))
                    {
                        const auto& channelName = channel->name();
                        if (selectedPhases.count(RimWellPlotTools::flowPhaseFromChannelName(channelName)) > 0)
                        {
                            auto color = RimWellPlotTools::isOilFlowChannel(channelName) ? cvf::Color3f::DARK_GREEN :
                                         RimWellPlotTools::isGasFlowChannel(channelName) ? cvf::Color3f::DARK_RED :
                                         RimWellPlotTools::isWaterFlowChannel(channelName) ? cvf::Color3f::BLUE :
                                         cvf::Color3f::DARK_GRAY;

                            addStackedCurve(curveName + ", " + channelName, 
                                            rigWellLogFile->depthValues(), 
                                            rigWellLogFile->values(channelName),
                                            plotTrack, 
                                            color,
                                            curveGroupId, 
                                            true);
                        }
                    }
                }
            }
        }
        curveGroupId++;
    }
    m_wellLogPlot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::addStackedCurve(const QString& curveName,
                                     const std::vector<double>& depthValues,
                                     const std::vector<double>& accFlow,
                                     RimWellLogTrack* plotTrack,
                                     cvf::Color3f color,
                                     int curveGroupId,
                                     bool doFillCurve)
{
    RimWellFlowRateCurve* curve = new RimWellFlowRateCurve;
    curve->setFlowValuesPrDepthValue(curveName, depthValues, accFlow);

    curve->setColor(color);
    curve->setGroupId(curveGroupId);
    curve->setDoFillCurve(doFillCurve);
    curve->setSymbol(RimSummaryCurveAppearanceCalculator::cycledSymbol(curveGroupId));

    plotTrack->addCurve(curve);

    curve->loadDataAndUpdate(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPltPlot::isOnlyGridSourcesSelected() const
{
    const std::vector<RifWellRftAddress>& selSources = m_selectedSources();
    return std::find_if(selSources.begin(), selSources.end(), [](const RifWellRftAddress& addr)
    {
        return addr.sourceType() == RifWellRftAddress::RFT || addr.sourceType() == RifWellRftAddress::OBSERVED;
    }) == selSources.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPltPlot::isAnySourceAddressSelected(const std::set<RifWellRftAddress>& addresses) const
{
    const std::vector<RifWellRftAddress>& selectedSourcesVector = m_selectedSources();
    const auto selectedSources = std::set<RifWellRftAddress>(selectedSourcesVector.begin(), selectedSourcesVector.end());
    std::vector<RifWellRftAddress> intersectVector;

    std::set_intersection(selectedSources.begin(), selectedSources.end(),
                          addresses.begin(), addresses.end(), std::inserter(intersectVector, intersectVector.end()));
    return intersectVector.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifWellRftAddress> RimWellPltPlot::selectedSources() const
{
    std::vector<RifWellRftAddress> sources;
    for (const RifWellRftAddress& addr : m_selectedSources())
    {
        if (addr.sourceType() == RifWellRftAddress::OBSERVED)
        {
            for (RimWellLogFile* const wellLogFile : RimWellPlotTools::wellLogFilesContainingFlow(m_wellPathName))
            {
                sources.push_back(RifWellRftAddress(RifWellRftAddress::OBSERVED, wellLogFile));
            }
        }
        else
            sources.push_back(addr);
    }
    return sources;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifWellRftAddress> RimWellPltPlot::selectedSourcesAndTimeSteps() const
{
    std::vector<RifWellRftAddress> sources;
    for (const RifWellRftAddress& addr : m_selectedSources())
    {
        if (addr.sourceType() == RifWellRftAddress::OBSERVED)
        {
            for (const QDateTime& timeStep : m_selectedTimeSteps())
            {
                for (const RifWellRftAddress& address : m_timeStepsToAddresses.at(timeStep))
                {
                    sources.push_back(address);
                }
            }
        }
        else
            sources.push_back(addr);
    }
    return sources;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellPltPlot::viewWidget()
{
    return m_wellLogPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::zoomAll()
{
    m_wellLogPlot()->zoomAll();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RimWellPltPlot::wellLogPlot() const
{
    return m_wellLogPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::setCurrentWellName(const QString& currWellName)
{
    m_wellPathName = currWellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPltPlot::currentWellName() const
{
    return m_wellPathName;
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
QList<caf::PdmOptionItemInfo> RimWellPltPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    const QString simWellName = RimWellPlotTools::simWellName(m_wellPathName);

    if (fieldNeedingOptions == &m_wellPathName)
    {
        calculateValueOptionsForWells(options);
    }
    else if (fieldNeedingOptions == &m_selectedSources)
    {
        std::set<RifWellRftAddress> optionAddresses;

        const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell(simWellName);
        if (rftCases.size() > 0)
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(RifWellRftAddress::sourceTypeUiText(RifWellRftAddress::RFT), true));
        }

        for (const auto& rftCase : rftCases)
        {
            auto addr = RifWellRftAddress(RifWellRftAddress::RFT, rftCase);
            auto item = caf::PdmOptionItemInfo(rftCase->caseUserDescription(), QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
        }

        const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell(simWellName);
        if (gridCases.size() > 0)
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(RifWellRftAddress::sourceTypeUiText(RifWellRftAddress::GRID), true));
        }

        for (const auto& gridCase : gridCases)
        {
            auto addr = RifWellRftAddress(RifWellRftAddress::GRID, gridCase);
            auto item = caf::PdmOptionItemInfo(gridCase->caseUserDescription(), QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
        }

        if (RimWellPlotTools::wellLogFilesContainingFlow(m_wellPathName).size() > 0)
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(RifWellRftAddress::sourceTypeUiText(RifWellRftAddress::OBSERVED), true));

            auto addr = RifWellRftAddress(RifWellRftAddress::OBSERVED);
            auto item = caf::PdmOptionItemInfo("Observed Data", QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
            optionAddresses.insert(addr);
        }
    }
    else if (fieldNeedingOptions == &m_selectedTimeSteps)
    {
        calculateValueOptionsForTimeSteps(m_wellPathName, options);
    }

    if (fieldNeedingOptions == &m_phaseSelectionMode)
    {
    }
    else if (fieldNeedingOptions == &m_phases)
    {
        options.push_back(caf::PdmOptionItemInfo("Oil", FLOW_PHASE_OIL));
        options.push_back(caf::PdmOptionItemInfo("Gas", FLOW_PHASE_GAS));
        options.push_back(caf::PdmOptionItemInfo("Water", FLOW_PHASE_WATER));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_wellPathName)
    {
        setDescription(QString(plotNameFormatString()).arg(m_wellPathName));
    }

    if (changedField == &m_wellPathName)
    {
        RimWellLogTrack* const plotTrack = m_wellLogPlot->trackByIndex(0);
        for (RimWellLogCurve* const curve : plotTrack->curvesVector())
        {
            plotTrack->removeCurve(curve);
        }
        m_timeStepsToAddresses.clear();
        updateFormationsOnPlot();
    }
    else if (changedField == &m_selectedSources)
    {
      
    }

    if (changedField == &m_selectedSources ||
        changedField == &m_selectedTimeSteps)
    {
        updateFormationsOnPlot();
        syncSourcesIoFieldFromGuiField();
        syncCurvesFromUiSelection();
    }
    else if (changedField == &m_showPlotTitle)
    {
        //m_wellLogPlot->setShowDescription(m_showPlotTitle);
    }

    if (changedField == &m_phaseSelectionMode ||
        changedField == &m_phases)
    {
        syncCurvesFromUiSelection();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimWellPltPlot::snapshotWindowContent()
{
    QImage image;

    if (m_wellLogPlotWidget)
    {
        QPixmap pix = QPixmap::grabWidget(m_wellLogPlotWidget);
        image = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    const QString simWellName = RimWellPlotTools::simWellName(m_wellPathName);

    uiOrdering.add(&m_userName);
    uiOrdering.add(&m_wellPathName);
    
    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword("Sources", "Sources");
    sourcesGroup->add(&m_selectedSources);

    caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroupWithKeyword("Time Steps", "TimeSteps");
    timeStepsGroup->add(&m_selectedTimeSteps);

    caf::PdmUiGroup* flowGroup = uiOrdering.addNewGroupWithKeyword("Phase Selection", "PhaseSelection");
    flowGroup->add(&m_phaseSelectionMode);

    if (m_phaseSelectionMode == FLOW_TYPE_PHASE_SPLIT)
    {
        flowGroup->add(&m_phases);
    }

    //uiOrdering.add(&m_showPlotTitle);

    if (m_wellLogPlot && m_wellLogPlot->trackCount() > 0)
    {
        RimWellLogTrack* track = m_wellLogPlot->trackByIndex(0);

        track->uiOrderingForShowFormationNamesAndCase(uiOrdering);

        caf::PdmUiGroup* legendAndAxisGroup = uiOrdering.addNewGroup("Legend and Axis");
        legendAndAxisGroup->setCollapsedByDefault(true);

        m_wellLogPlot->uiOrderingForPlot(*legendAndAxisGroup);

        track->uiOrderingForVisibleXRange(*legendAndAxisGroup);

        m_wellLogPlot->uiOrderingForVisibleDepthRange(*legendAndAxisGroup);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_phases)
    {
        caf::PdmUiTreeSelectionEditorAttribute* attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*> (attribute);
        attrib->showTextFilter = false;
        attrib->showToggleAllCheckbox = false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::initAfterRead()
{
    RimViewWindow::initAfterRead();

    // Postpone init until data has been loaded
    m_doInitAfterLoad = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::setupBeforeSave()
{
    syncCurvesFromUiSelection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::initAfterLoad()
{
    std::set<RifWellRftAddress> selectedSources;
    for (RimRftAddress* addr : m_selectedSourcesForIo)
    {
        if (addr->address().sourceType() == RifWellRftAddress::OBSERVED)
        {
            selectedSources.insert(RifWellRftAddress(RifWellRftAddress::OBSERVED));
        }
        else
        {
            selectedSources.insert(addr->address());
        }
    }
    m_selectedSources = std::vector<RifWellRftAddress>(selectedSources.begin(), selectedSources.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::syncSourcesIoFieldFromGuiField()
{
    m_selectedSourcesForIo.clear();
    for (const RifWellRftAddress& addr : selectedSourcesAndTimeSteps())
    {
        m_selectedSourcesForIo.push_back(new RimRftAddress(addr));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::updateTimeStepsToAddresses(const std::vector<RifWellRftAddress>& addressesToKeep)
{
    for (auto& timeStepPair : m_timeStepsToAddresses)
    {
        std::vector<RifWellRftAddress> addressesToDelete;
        std::set<RifWellRftAddress> keepAddresses = std::set<RifWellRftAddress>(addressesToKeep.begin(), addressesToKeep.end());
        std::set<RifWellRftAddress>& currentAddresses = timeStepPair.second;

        std::set_difference(currentAddresses.begin(), currentAddresses.end(),
                            keepAddresses.begin(), keepAddresses.end(),
                            std::inserter(addressesToDelete, addressesToDelete.end()));

        for (const auto& addr : addressesToDelete)
        {
            currentAddresses.erase(addr);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options)
{
    RimProject * proj = RiaApplication::instance()->project();

    if (proj != nullptr)
    {
        std::set<QString> wellNames;
       
        // Observed wells
        for (const RimWellPath* const wellPath : proj->allWellPaths())
        {
            wellNames.insert(wellPath->name());
        }

        for (const auto& wellName : wellNames)
        {
            options.push_back(caf::PdmOptionItemInfo(wellName, wellName));
        }
    }

    if (options.size() == 0)
    {
        options.push_back(caf::PdmOptionItemInfo("None", "None"));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::calculateValueOptionsForTimeSteps(const QString& wellPathNameOrSimWellName, QList<caf::PdmOptionItemInfo>& options)
{
    const QString simWellName = RimWellPlotTools::simWellName(m_wellPathName);
    std::map<QDateTime, std::set<RifWellRftAddress>> displayTimeStepsMap, obsAndRftTimeStepsMap, gridTimeStepsMap;
    const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell(simWellName);
    const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell(simWellName);

    // First update timeSteps to Address 'cache'
    std::vector<RifWellRftAddress> selSources = selectedSources();
    updateTimeStepsToAddresses(selectedSources());

    for (const RifWellRftAddress& selection : selSources)
    {
        if (selection.sourceType() == RifWellRftAddress::RFT)
        {
            for (RimEclipseResultCase* const rftCase : rftCases)
            {
                RimWellPlotTools::addTimeStepsToMap(obsAndRftTimeStepsMap, RimWellPlotTools::timeStepsMapFromRftCase(rftCase, simWellName));
            }
        }
        else if (selection.sourceType() == RifWellRftAddress::GRID)
        {
            for (RimEclipseResultCase* const gridCase : gridCases)
            {
                RimWellPlotTools::addTimeStepsToMap(gridTimeStepsMap, RimWellPlotTools::timeStepsMapFromGridCase(gridCase));
            }
        }
        else if (selection.sourceType() == RifWellRftAddress::OBSERVED)
        {
            if (selection.wellLogFile() != nullptr)
            {
                RimWellPlotTools::addTimeStepsToMap(obsAndRftTimeStepsMap, RimWellPlotTools::timeStepsMapFromWellLogFile(selection.wellLogFile()));
            }
        }
    }

    if (isOnlyGridSourcesSelected())
    {
        displayTimeStepsMap = gridTimeStepsMap;
    }
    else
    {
        const auto gridTimeStepsVector = std::vector<std::pair<QDateTime, std::set<RifWellRftAddress>>>(gridTimeStepsMap.begin(), gridTimeStepsMap.end());

        for (const std::pair<QDateTime, std::set<RifWellRftAddress>>& timeStepPair : obsAndRftTimeStepsMap)
        {
            const std::map<QDateTime, std::set<RifWellRftAddress>>& adjTimeSteps = RimWellPlotTools::adjacentTimeSteps(gridTimeStepsVector, timeStepPair);
            RimWellPlotTools::addTimeStepsToMap(displayTimeStepsMap, adjTimeSteps);
        }

        // Add the first grid time step (from the total grid time steps list)
        if (gridTimeStepsVector.size() > 0)
        {
            RimWellPlotTools::addTimeStepToMap(displayTimeStepsMap, gridTimeStepsVector.front());
        }

        // Add already selected time steps
        for (const QDateTime& timeStep : m_selectedTimeSteps())
        {
            if (m_timeStepsToAddresses.count(timeStep) > 0)
            {
                const std::set<RifWellRftAddress> sourceAddresses = m_timeStepsToAddresses[timeStep];
                if (isAnySourceAddressSelected(sourceAddresses))
                {
                    RimWellPlotTools::addTimeStepToMap(displayTimeStepsMap, std::make_pair(timeStep, m_timeStepsToAddresses[timeStep]));
                }
            }
        }
    }

    RimWellPlotTools::addTimeStepsToMap(m_timeStepsToAddresses, displayTimeStepsMap);

    // Create vector of all time steps
    std::vector<QDateTime> allTimeSteps;
    for (const std::pair<QDateTime, std::set<RifWellRftAddress>>& timeStepPair : m_timeStepsToAddresses)
    {
        allTimeSteps.push_back(timeStepPair.first);
    }

    const QString dateFormatString = RimTools::createTimeFormatStringFromDates(allTimeSteps);
    for (const std::pair<QDateTime, std::set<RifWellRftAddress>>& timeStepPair : displayTimeStepsMap)
    {
        options.push_back(caf::PdmOptionItemInfo(timeStepPair.first.toString(dateFormatString), timeStepPair.first));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::setDescription(const QString& description)
{
    m_userName = description;

    updateWidgetTitleWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPltPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::onLoadDataAndUpdate()
{
    if (m_doInitAfterLoad)
    {
        initAfterLoad();
        m_doInitAfterLoad = false;
    }

    if (m_isOnLoad)
    {
        if (m_wellLogPlot->trackCount() > 0)
        {
            m_wellLogPlot->trackByIndex(0)->setShowFormations(true);
        }
        m_isOnLoad = false;
    }

    updateMdiWindowVisibility();
    updateFormationsOnPlot();
    syncCurvesFromUiSelection();
    m_wellLogPlot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellPltPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_wellLogPlotWidget = new RiuWellPltPlot(this, mainWindowParent);
    return m_wellLogPlotWidget;
}
