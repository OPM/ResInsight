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

CAF_PDM_SOURCE_INIT(RimWellPltPlot, "WellPltPlot"); 

namespace caf
{
template<>
void caf::AppEnum< RimWellPltPlot::FlowType>::setUp()
{
    addItem(RimWellPltPlot::FLOW_TYPE_TOTAL, "TOTAL", "Total Flow");
    addItem(RimWellPltPlot::FLOW_TYPE_PHASE_SPLIT, "PHASE_SPLIT", "Phase Split");
}

template<>
void caf::AppEnum< RimWellPltPlot::FlowPhase>::setUp()
{
    addItem(RimWellPltPlot::PHASE_OIL, "PHASE_OIL", "Oil");
    addItem(RimWellPltPlot::PHASE_GAS, "PHASE_GAS", "Gas");
    addItem(RimWellPltPlot::PHASE_WATER, "PHASE_WATER", "Water");
}
}

const QString RimWellPltPlot::OIL_CHANNEL_NAME = "QOZT";
const QString RimWellPltPlot::GAS_CHANNEL_NAME = "QGZT";
const QString RimWellPltPlot::WATER_CHANNEL_NAME = "QWZT";
const QString RimWellPltPlot::TOTAL_CHANNEL_NAME = "QTZT";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::set<QString> RimWellPltPlot::FLOW_DATA_NAMES = { 
    OIL_CHANNEL_NAME,
    GAS_CHANNEL_NAME,
    WATER_CHANNEL_NAME,
    TOTAL_CHANNEL_NAME };
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

    CAF_PDM_InitFieldNoDefault(&m_wellName, "WellName", "WellName", "", "", "");
    CAF_PDM_InitField(&m_branchIndex, "BranchIndex", 0, "BranchIndex", "", "", "");

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
    m_phases = std::vector<caf::AppEnum<FlowPhase>>({ FlowPhase::PHASE_OIL, FlowPhase::PHASE_GAS, FlowPhase::PHASE_WATER });

    this->setAsPlotMdiWindow();
    m_doInitAfterLoad = false;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPltPlot::FlowPhase RimWellPltPlot::flowPhaseFromChannelName(const QString& channelName)
{
    if (QString::compare(channelName, OIL_CHANNEL_NAME, Qt::CaseInsensitive) == 0) return PHASE_OIL;
    if (QString::compare(channelName, GAS_CHANNEL_NAME, Qt::CaseInsensitive) == 0) return PHASE_GAS;
    if (QString::compare(channelName, WATER_CHANNEL_NAME, Qt::CaseInsensitive) == 0) return PHASE_WATER;
    if (QString::compare(channelName, TOTAL_CHANNEL_NAME, Qt::CaseInsensitive) == 0) return PHASE_TOTAL;
    return PHASE_NONE;
}

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
    if (m_selectedTimeSteps().empty())
    {
        for (size_t i = 0; i < m_wellLogPlot->trackCount(); i++)
        {
            m_wellLogPlot->trackByIndex(i)->updateFormationNamesData(nullptr, RimWellLogTrack::WELL_PATH, nullptr, QString(), 0);
        }
        return;
    }

    RimProject* proj = RiaApplication::instance()->project();
    RimOilField* oilField = proj->activeOilField();

    RimWellPathCollection* wellPathCollection = oilField->wellPathCollection();
    RimWellPath* wellPath = wellPathCollection->wellPathByName(m_wellName);

    RimWellLogTrack::TrajectoryType trajectoryType;

    if (wellPath)
    {
        trajectoryType = RimWellLogTrack::WELL_PATH;
    }
    else
    {
        trajectoryType = RimWellLogTrack::SIMULATION_WELL;
    }

    RimCase* rimCase = nullptr;
    std::vector<RimCase*> cases;
    proj->allCases(cases);
    if (!cases.empty())
    {
        rimCase = cases[0];
    }

    if (m_wellLogPlot->trackCount() > 0)
    {
        m_wellLogPlot->trackByIndex(0)->updateFormationNamesData(rimCase, trajectoryType, wellPath, m_wellName, m_branchIndex);
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
std::vector<RimWellLogFile*> RimWellPltPlot::wellLogFilesContainingFlow(const QString& wellName) const
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const project = RiaApplication::instance()->project();

    for (const auto& oilField : project->oilFields)
    {
        auto wellPathsVector = std::vector<RimWellPath*>(oilField->wellPathCollection()->wellPaths.begin(), oilField->wellPathCollection()->wellPaths.end());

        for (const auto& wellPath : wellPathsVector)
        {
            bool hasPressure = false;
            const std::vector<RimWellLogFile*> files = wellPath->wellLogFiles();

            for (RimWellLogFile* const file : files)
            {
                size_t timeStepCount = timeStepsFromWellLogFile(file).size();

                if (timeStepCount == 0) continue;
                if (QString::compare(file->wellName(), wellName) != 0) continue;

                if (hasFlowData(file))
                {
                    wellLogFiles.push_back(file);
                }
            }
        }
    }
    return wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFileChannel*> RimWellPltPlot::getFlowChannelsFromWellFile(const RimWellLogFile* wellLogFile) const
{
    std::vector<RimWellLogFileChannel*> channels;
    if(wellLogFile != nullptr)
    {
        for (RimWellLogFileChannel* const channel : wellLogFile->wellLogChannels())
        {
            if (isFlowChannel(channel))
            {
                channels.push_back(channel);
            }
        }
    }
    return channels;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPltPlot::wellPathFromWellLogFile(const RimWellLogFile* wellLogFile) const
{
    RimProject* const project = RiaApplication::instance()->project();
    for (const auto& oilField : project->oilFields)
    {
        auto wellPaths = std::vector<RimWellPath*>(oilField->wellPathCollection()->wellPaths.begin(), oilField->wellPathCollection()->wellPaths.end());

        for (const auto& wellPath : wellPaths)
        {
            for (RimWellLogFile* const file : wellPath->wellLogFiles())
            {
                if (file == wellLogFile)
                {
                    return wellPath;
                }
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::tuple<RimEclipseResultCase*, bool/*hasPressure*/, bool /*hasRftData*/>> 
RimWellPltPlot::eclipseCasesForWell(const QString& wellName) const
{
    std::vector<std::tuple<RimEclipseResultCase*, bool, bool>> cases;
    const RimProject* const project = RiaApplication::instance()->project();

    for (const auto& oilField : project->oilFields)
    {
        const RimEclipseCaseCollection* const eclCaseColl = oilField->analysisModels();
        for (RimEclipseCase* eCase : eclCaseColl->cases())
        {
            auto eclCase = dynamic_cast<RimEclipseResultCase*>(eCase);
            if (eclCase != nullptr)
            {
                RigEclipseCaseData* const eclipseCaseData = eclCase->eclipseCaseData();
                for (const cvf::ref<RigSimWellData>& wellResult : eclipseCaseData->wellResults())
                {
                    if (QString::compare(wellResult->m_wellName, wellName) == 0)
                    {
                        bool hasRftData = eclCase->rftReader() != nullptr;
                        cases.push_back(std::make_tuple(eclCase, true, hasRftData));
                        break;
                    }
                }
            }
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> 
RimWellPltPlot::gridCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const
{
    std::vector<RimEclipseResultCase*> cases;
    for (const std::tuple<RimEclipseResultCase*, bool, bool>& eclCaseTuple : eclipseCasesTuple)
    {
        bool hasPressureData = std::get<1>(eclCaseTuple);
        size_t timeStepCount = timeStepsFromGridCase(std::get<0>(eclCaseTuple)).size();
        if (hasPressureData && timeStepCount > 0)
        {
            cases.push_back(std::get<0>(eclCaseTuple));
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> 
RimWellPltPlot::rftCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const
{
    std::vector<RimEclipseResultCase*> cases;
    for (const std::tuple<RimEclipseResultCase*, bool, bool>& eclCaseTuple : eclipseCasesTuple)
    {
        bool hasRftData = std::get<2>(eclCaseTuple);
        size_t timeStepCount = timeStepsFromRftCase(std::get<0>(eclCaseTuple)).size();
        if (hasRftData && timeStepCount > 0)
        {
            cases.push_back(std::get<0>(eclCaseTuple));
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifWellRftAddress>> RimWellPltPlot::timeStepsFromRftCase(RimEclipseResultCase* rftCase) const
{
    std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMap;
    RifReaderEclipseRft* const reader = rftCase->rftReader();
    if (reader != nullptr)
    {
        for (const QDateTime& timeStep : reader->availableTimeSteps(m_wellName, RifEclipseRftAddress::PRESSURE))
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RifWellRftAddress>()));
            }
            timeStepsMap[timeStep].insert(RifWellRftAddress(RifWellRftAddress::RFT, rftCase));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifWellRftAddress>> RimWellPltPlot::timeStepsFromGridCase(RimEclipseCase* gridCase) const
{
    const RigEclipseCaseData* const eclipseCaseData = gridCase->eclipseCaseData();

    std::map<QDateTime, std::set<RifWellRftAddress> > timeStepsMap;
    {
        for (const QDateTime& timeStep : eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->timeStepDates())
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RifWellRftAddress>()));
            }
            timeStepsMap[timeStep].insert(RifWellRftAddress(RifWellRftAddress::GRID, gridCase));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifWellRftAddress> > RimWellPltPlot::timeStepsFromWellLogFile(RimWellLogFile* wellLogFile) const
{
    std::map<QDateTime, std::set<RifWellRftAddress> > timeStepsMap;

    QDateTime timeStep = wellLogFile->date();

    if (timeStepsMap.count(timeStep) == 0)
    {
        timeStepsMap.insert(std::make_pair(timeStep, std::set<RifWellRftAddress>()));
    }
    timeStepsMap[timeStep].insert(RifWellRftAddress(RifWellRftAddress::OBSERVED, wellLogFile));

    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifWellRftAddress>> 
RimWellPltPlot::adjacentTimeSteps(const std::vector<std::pair<QDateTime, std::set<RifWellRftAddress>>>& allTimeSteps,
                                  const std::pair<QDateTime, std::set<RifWellRftAddress>>& searchTimeStepPair)
{
    std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMap;

    if (allTimeSteps.size() > 0)
    {
        auto itr = std::find_if(allTimeSteps.begin(), allTimeSteps.end(), 
                                [searchTimeStepPair](const std::pair<QDateTime, std::set<RifWellRftAddress>>& dt)
        {
            return dt.first > searchTimeStepPair.first;
        });

        auto itrEnd = itr != allTimeSteps.end() ? itr + 1 : itr;

        for (itr = itrEnd - 1; itr != allTimeSteps.begin() && (*itr).first >= searchTimeStepPair.first; itr--);
        auto itrFirst = itr;

        timeStepsMap.insert(itrFirst, itrEnd);
    }

    // Add searched time step in case it is not included
    addTimeStepToMap(timeStepsMap, searchTimeStepPair);

    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPltPlot::mapContainsTimeStep(const std::map<QDateTime, std::set<RifWellRftAddress>>& map, const QDateTime& timeStep)
{
    return std::find_if(map.begin(), map.end(), [timeStep](const std::pair<QDateTime, std::set<RifWellRftAddress>>& pair)
    {
        return pair.first == timeStep;
    }) != map.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set < std::pair<RifWellRftAddress, QDateTime>> RimWellPltPlot::selectedCurveDefs() const
{
    std::set<std::pair<RifWellRftAddress, QDateTime>> curveDefs;
    const std::vector<std::tuple<RimEclipseResultCase*,bool,bool>>& eclipseCases = eclipseCasesForWell(m_wellName);
    const std::vector<RimEclipseResultCase*> rftCases = rftCasesFromEclipseCases(eclipseCases);
    const std::vector<RimEclipseResultCase*> gridCases = gridCasesFromEclipseCases(eclipseCases);

    for (const QDateTime& timeStep : m_selectedTimeSteps())
    {
        for (const RifWellRftAddress& addr : selectedSources())
        {
            if (addr.sourceType() == RifWellRftAddress::RFT)
            {
                for (RimEclipseResultCase* const rftCase : rftCases)
                {
                    const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsMap = timeStepsFromRftCase(rftCase);
                    if (mapContainsTimeStep(timeStepsMap , timeStep))
                    {
                        curveDefs.insert(std::make_pair(addr, timeStep));
                    }
                }
            }
            else if (addr.sourceType() == RifWellRftAddress::GRID)
            {
                for (RimEclipseResultCase* const gridCase : gridCases)
                {
                    const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsMap = timeStepsFromGridCase(gridCase);
                    if (mapContainsTimeStep(timeStepsMap, timeStep))
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
                    const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsMap = timeStepsFromWellLogFile(addr.wellLogFile());
                    if (mapContainsTimeStep(timeStepsMap, timeStep))
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

    RigEclipseWellLogExtractor*  findWellLogExtractor(const QString& wellName,
                                                      int branchIndex,
                                                      RimEclipseResultCase* eclCase)
    {
        RimProject* proj = RiaApplication::instance()->project();
        RimWellPath* wellPath = proj->wellPathFromSimulationWell(wellName, branchIndex);
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
    RigRftResultPointCalculator(const QString& wellName, 
                                int branchIndex,
                                RimEclipseResultCase* eclCase,
                                QDateTime m_timeStep)
    {

        RifEclipseRftAddress gasRateAddress(wellName, m_timeStep, RifEclipseRftAddress::GRAT);
        RifEclipseRftAddress oilRateAddress(wellName, m_timeStep, RifEclipseRftAddress::ORAT);
        RifEclipseRftAddress watRateAddress(wellName, m_timeStep, RifEclipseRftAddress::WRAT);

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

        RigEclipseWellLogExtractor* eclExtractor = findWellLogExtractor(wellName, branchIndex, eclCase);
        if (!eclExtractor) return;

        std::vector<CellIntersectionInfo> intersections = eclExtractor->intersectionInfo();

        for (size_t wpExIdx = 0; wpExIdx < intersections.size(); wpExIdx++)
        {
            size_t globCellIdx = intersections[wpExIdx].globCellIndex;

            auto it = globCellIdxToIdxInRftFile.find(globCellIdx);
            if (it == globCellIdxToIdxInRftFile.end()) continue;

            m_pipeBranchCLCoords.push_back(intersections[wpExIdx].startPoint);
            m_pipeBranchMeasuredDepths.push_back(intersections[wpExIdx].startMD);

            m_pipeBranchCLCoords.push_back(intersections[wpExIdx].endPoint);
            m_pipeBranchMeasuredDepths.push_back(intersections[wpExIdx].endMD);

            RigWellResultPoint resPoint;
            resPoint.m_isOpen = true;
            resPoint.m_gridIndex = 0; // Always main grod
            resPoint.m_gridCellIndex = globCellIdx; // Shortcut, since we only have main grid results from RFT

            resPoint.m_oilRate   = gasRates[it->second];  
            resPoint.m_gasRate   = oilRates[it->second];  
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
    RigSimWellResultPointCalculator(const QString& wellName, 
                                    int branchIndex,
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

        const RigSimWellData* simWell = eclCase->eclipseCaseData()->findSimWellData(wellName);

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

        RigEclipseWellLogExtractor* eclExtractor = findWellLogExtractor(wellName, branchIndex, eclCase);

        if (!eclExtractor) return;
        
        std::vector<CellIntersectionInfo> intersections = eclExtractor->intersectionInfo();

        for (size_t wpExIdx = 0; wpExIdx < intersections.size(); wpExIdx++)
        {
            size_t globCellIdx = intersections[wpExIdx].globCellIndex;

            auto it = globCellIdxToIdxInSimWellBranch.find(globCellIdx);
            if (it == globCellIdxToIdxInSimWellBranch.end()) continue;

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
    RimWellPath* wellPath = proj->wellPathFromSimulationWell(m_wellName());
    RimWellLogPlotCollection* wellLogCollection = proj->mainPlotCollection()->wellLogPlotCollection();


    // Add curves
    for (const std::pair<RifWellRftAddress, QDateTime>& curveDefToAdd : curveDefs)
    {
        std::set<FlowPhase> selectedPhases = m_phaseSelectionMode == FLOW_TYPE_PHASE_SPLIT ?
            std::set<FlowPhase>(m_phases().begin(), m_phases().end()) :
            std::set<FlowPhase>({ PHASE_TOTAL });
        
        RifWellRftAddress sourceDef = curveDefToAdd.first;
        QDateTime timeStep = curveDefToAdd.second;

        std::unique_ptr<RigResultPointCalculator> resultPointCalc;
        QString curveName;
        curveName += sourceDef.eclCase() ? sourceDef.eclCase()->caseUserDescription() : "";
        curveName += sourceDef.wellLogFile() ? sourceDef.wellLogFile()->name() : "";
        if (sourceDef.sourceType() == RifWellRftAddress::RFT) curveName += ", RFT";
        curveName += ", " + timeStep.toString();

        if ( sourceDef.sourceType() == RifWellRftAddress::RFT )
        {
            resultPointCalc.reset(new RigRftResultPointCalculator(m_wellName(),
                                                                  m_branchIndex(),
                                                                  dynamic_cast<RimEclipseResultCase*>(sourceDef.eclCase()),
                                                                  timeStep));
        }
        else if (sourceDef.sourceType() == RifWellRftAddress::GRID)
        {
            resultPointCalc.reset(new RigSimWellResultPointCalculator(m_wellName(),
                                                                      m_branchIndex(),
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
                                                        0.0);

                 const std::vector<double>& depthValues = wfAccumulator.pseudoLengthFromTop(0);
                 std::vector<QString> tracerNames = wfAccumulator.tracerNames();
                 for ( const QString& tracerName: tracerNames )
                 {
                     auto color = tracerName == RIG_FLOW_OIL_NAME   ? cvf::Color3f::DARK_GREEN :
                                  tracerName == RIG_FLOW_GAS_NAME   ? cvf::Color3f::DARK_RED :
                                  tracerName == RIG_FLOW_WATER_NAME ? cvf::Color3f::BLUE :
                                  cvf::Color3f::DARK_GRAY;

                     if (    tracerName == RIG_FLOW_OIL_NAME  && selectedPhases.count(PHASE_OIL) 
                          || tracerName == RIG_FLOW_GAS_NAME  && selectedPhases.count(PHASE_GAS)
                          || tracerName == RIG_FLOW_WATER_NAME  && selectedPhases.count(PHASE_WATER) )
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
            if(wellLogFile!= nullptr)
            {
                RigWellLogFile* rigWellLogFile = wellLogFile->wellLogFile();

                if (rigWellLogFile != nullptr)
                {
                    for (RimWellLogFileChannel* channel : getFlowChannelsFromWellFile(wellLogFile))
                    {
                        const auto& channelName = channel->name();
                        if (selectedPhases.count(flowPhaseFromChannelName(channelName)) > 0)
                        {
                            auto color = channelName == OIL_CHANNEL_NAME   ? cvf::Color3f::DARK_GREEN :
                                         channelName == GAS_CHANNEL_NAME   ? cvf::Color3f::DARK_RED :
                                         channelName == WATER_CHANNEL_NAME ? cvf::Color3f::BLUE :
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
            for (RimWellLogFile* const wellLogFile : wellLogFilesContainingFlow(m_wellName))
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
    m_wellName = currWellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPltPlot::currentWellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellPltPlot::branchIndex() const
{
    return m_branchIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPltPlot::hasFlowData(const RimWellLogFile* wellLogFile)
{
    for (RimWellLogFileChannel* const wellLogChannel : wellLogFile->wellLogChannels())
    {
        if (isFlowChannel(wellLogChannel)) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPltPlot::hasFlowData(RimWellPath* wellPath)
{
    for (RimWellLogFile* const wellLogFile : wellPath->wellLogFiles())
    {
        if (hasFlowData(wellLogFile))
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPltPlot::isFlowChannel(RimWellLogFileChannel* channel)
{
    return FLOW_DATA_NAMES.count(channel->name()) > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPltPlot::hasFlowData(RimEclipseResultCase* gridCase)
{
    const RigEclipseCaseData* const eclipseCaseData = gridCase->eclipseCaseData();

    for (const QString& channelName : FLOW_DATA_NAMES)
    {
        size_t resultIndex = eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, channelName);

        if (resultIndex != cvf::UNDEFINED_SIZE_T) return true;
    }
    return false;
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

    if (fieldNeedingOptions == &m_wellName)
    {
        calculateValueOptionsForWells(options);
    }
    else if (fieldNeedingOptions == &m_selectedSources)
    {
        std::set<RifWellRftAddress> optionAddresses;

        const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCases = eclipseCasesForWell(m_wellName);

        const std::vector<RimEclipseResultCase*> rftCases = rftCasesFromEclipseCases(eclipseCases);
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

        const std::vector<RimEclipseResultCase*> gridCases = gridCasesFromEclipseCases(eclipseCases);
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

        if (wellLogFilesContainingFlow(m_wellName).size() > 0)
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
        calculateValueOptionsForTimeSteps(m_wellName, options);
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        RimProject* proj = RiaApplication::instance()->project();

        size_t branchCount = proj->simulationWellBranches(m_wellName).size();

        for (int bIdx = 0; bIdx < static_cast<int>(branchCount); ++bIdx)
        {
            options.push_back(caf::PdmOptionItemInfo("Branch " + QString::number(bIdx + 1), QVariant::fromValue(bIdx)));
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", -1));
        }
    }

    if (fieldNeedingOptions == &m_phaseSelectionMode)
    {
    }
    else if (fieldNeedingOptions == &m_phases)
    {
        options.push_back(caf::PdmOptionItemInfo("Oil", PHASE_OIL));
        options.push_back(caf::PdmOptionItemInfo("Gas", PHASE_GAS));
        options.push_back(caf::PdmOptionItemInfo("Water", PHASE_WATER));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_wellName)
    {
        setDescription(QString(plotNameFormatString()).arg(m_wellName));
    }

    if (changedField == &m_wellName || changedField == &m_branchIndex)
    {
        RimWellLogTrack* const plotTrack = m_wellLogPlot->trackByIndex(0);
        for (RimWellLogCurve* const curve : plotTrack->curvesVector())
        {
            plotTrack->removeCurve(curve);
        }
        m_timeStepsToAddresses.clear();
    }
    else if (changedField == &m_selectedSources)
    {
        // Update time steps selections based on source selections
        updateSelectedTimeStepsFromSelectedSources();
    }

    if (changedField == &m_selectedSources ||
        changedField == &m_selectedTimeSteps)
    {
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
    uiOrdering.add(&m_userName);
    uiOrdering.add(&m_wellName);
    
    RimProject* proj = RiaApplication::instance()->project();
    if (proj->simulationWellBranches(m_wellName).size() > 1)
    {
        uiOrdering.add(&m_branchIndex);
    }

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
void RimWellPltPlot::addTimeStepToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap,
                                       const std::pair<QDateTime, std::set<RifWellRftAddress>>& timeStepToAdd)
{
    auto timeStepMapToAdd = std::map<QDateTime, std::set<RifWellRftAddress>> { timeStepToAdd };
    addTimeStepsToMap(destMap, timeStepMapToAdd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPltPlot::addTimeStepsToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap, 
                                       const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsToAdd)
{
    for (const auto& timeStepPair : timeStepsToAdd)
    {
        if (timeStepPair.first.isValid())
        {
            if (destMap.count(timeStepPair.first) == 0)
            {
                destMap.insert(std::make_pair(timeStepPair.first, std::set<RifWellRftAddress>()));
            }
            auto addresses = timeStepPair.second;
            destMap[timeStepPair.first].insert(addresses.begin(), addresses.end());
        }
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
        const std::vector<QString> simWellNames = proj->simulationWellNames();
        auto wellNames = std::set<QString>(simWellNames.begin(), simWellNames.end());

        // Observed wells
        for (const auto& oilField : proj->oilFields())
        {
            auto wellPathColl = oilField->wellPathCollection();
            for (const auto& wellPath : wellPathColl->wellPaths)
            {
                wellNames.insert(wellPath->name());
            }
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
void RimWellPltPlot::calculateValueOptionsForTimeSteps(const QString& wellName, QList<caf::PdmOptionItemInfo>& options)
{
    std::map<QDateTime, std::set<RifWellRftAddress>> displayTimeStepsMap, obsAndRftTimeStepsMap, gridTimeStepsMap;
    const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCases = eclipseCasesForWell(wellName);
    const std::vector<RimEclipseResultCase*> rftCases = rftCasesFromEclipseCases(eclipseCases);
    const std::vector<RimEclipseResultCase*> gridCases = gridCasesFromEclipseCases(eclipseCases);

    // First update timeSteps to Address 'cache'
    std::vector<RifWellRftAddress> selSources = selectedSources();
    updateTimeStepsToAddresses(selectedSources());

    for (const RifWellRftAddress& selection : selSources)
    {
        if (selection.sourceType() == RifWellRftAddress::RFT)
        {
            for (RimEclipseResultCase* const rftCase : rftCases)
            {
                addTimeStepsToMap(obsAndRftTimeStepsMap, timeStepsFromRftCase(rftCase));
            }
        }
        else if (selection.sourceType() == RifWellRftAddress::GRID)
        {
            for (RimEclipseResultCase* const gridCase : gridCases)
            {
                addTimeStepsToMap(gridTimeStepsMap, timeStepsFromGridCase(gridCase));
            }
        }
        else
        if (selection.sourceType() == RifWellRftAddress::OBSERVED)
        {
            if (selection.wellLogFile() != nullptr)
            {
                addTimeStepsToMap(obsAndRftTimeStepsMap, timeStepsFromWellLogFile(selection.wellLogFile()));
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
            const std::map<QDateTime, std::set<RifWellRftAddress>>& adjTimeSteps = adjacentTimeSteps(gridTimeStepsVector, timeStepPair);
            addTimeStepsToMap(displayTimeStepsMap, adjTimeSteps);
        }

        // Add the first grid time step (from the total grid time steps list)
        if (gridTimeStepsVector.size() > 0)
        {
            addTimeStepToMap(displayTimeStepsMap, gridTimeStepsVector.front());
        }

        // Add already selected time steps
        for (const QDateTime& timeStep : m_selectedTimeSteps())
        {
            if (m_timeStepsToAddresses.count(timeStep) > 0)
            {
                const std::set<RifWellRftAddress> sourceAddresses = m_timeStepsToAddresses[timeStep];
                if (isAnySourceAddressSelected(sourceAddresses))
                {
                    addTimeStepToMap(displayTimeStepsMap, std::make_pair(timeStep, m_timeStepsToAddresses[timeStep]));
                }
            }
        }
    }

    addTimeStepsToMap(m_timeStepsToAddresses, displayTimeStepsMap);

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

    updateMdiWindowVisibility();
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
