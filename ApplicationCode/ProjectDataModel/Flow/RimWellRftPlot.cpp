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

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaDateStringParser.h"
#include "RifReaderEclipseRft.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSingleWellResultsData.h"
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
#include "RiuWellRftPlot.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include <tuple>
#include <algorithm>
#include <iterator>

CAF_PDM_SOURCE_INIT(RimWellRftPlot, "WellRftPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char RimWellRftPlot::PRESSURE_DATA_NAME[] = "PRESSURE";
const char RimWellRftPlot::PLOT_NAME_QFORMAT_STRING[] = "RFT: %1";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellRftPlot::RimWellRftPlot()
{
    CAF_PDM_InitObject("Well Allocation Plot", ":/WellAllocPlot16x16.png", "", "");

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("RFT Plot"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogPlot, "WellLog", "WellLog", "", "", "");
    m_wellLogPlot.uiCapability()->setUiHidden(true);
    m_wellLogPlot = new RimWellLogPlot();

    CAF_PDM_InitFieldNoDefault(&m_wellName, "WellName", "WellName", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_branchIndex, "BranchIndex", "BranchIndex", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSources, "Sources", "Sources", "", "", "");
    m_selectedSources.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSources.xmlCapability()->disableIO();
    m_selectedSources.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_selectedTimeSteps, "TimeSteps", "TimeSteps", "", "", "");
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedTimeSteps.xmlCapability()->disableIO();
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedTimeSteps.uiCapability()->setAutoAddingOptionFromValue(false);

    this->setAsPlotMdiWindow();
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
void RimWellRftPlot::deleteViewWidget()
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
void RimWellRftPlot::applyCurveAppearance(RimWellLogCurve* newCurve)
{
    const std::pair<RimWellRftAddress, QDateTime>& newCurveDef = curveDefFromCurve(newCurve);

    std::vector<cvf::Color3f> colorTable;
    RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector(&colorTable);

    std::vector<RimPlotCurve::PointSymbolEnum> symbolTable =
    {
        RimPlotCurve::SYMBOL_NONE,
        RimPlotCurve::SYMBOL_ELLIPSE,
        RimPlotCurve::SYMBOL_RECT,
        RimPlotCurve::SYMBOL_DIAMOND,
        RimPlotCurve::SYMBOL_TRIANGLE,
        RimPlotCurve::SYMBOL_CROSS,
        RimPlotCurve::SYMBOL_XCROSS
    };

    cvf::Color3f                    currentColor;
    RimPlotCurve::PointSymbolEnum   currentSymbol = RimPlotCurve::SYMBOL_NONE;
    RimPlotCurve::LineStyleEnum     currentLineStyle = RimPlotCurve::STYLE_SOLID;
    bool                            isCurrentColorSet = false;
    bool                            isCurrentSymbolSet = false;

    std::set<cvf::Color3f>                  assignedColors;
    std::set<RimPlotCurve::PointSymbolEnum> assignedSymbols;

    // Used colors and symbols
    for (const auto& curve : m_wellLogPlot->trackByIndex(0)->curvesVector())
    {
        if (curve == newCurve) continue;

        auto cDef = curveDefFromCurve(curve);
        if (cDef.first == newCurveDef.first)
        {
            currentColor = curve->color();
            isCurrentColorSet = true;
        }
        if (cDef.second == newCurveDef.second)
        {
            currentSymbol = curve->symbol();
            isCurrentSymbolSet = true;
        }
        assignedColors.insert(curve->color());
        assignedSymbols.insert(curve->symbol());
    }

    // Assign color
    if (!isCurrentColorSet)
    {
        for(const auto& color : colorTable)
        {
            if (assignedColors.count(color) == 0)
            {
                currentColor = color;
                isCurrentColorSet = true;
                break;
            }
        }
        if (!isCurrentColorSet)
        {
            currentColor = colorTable.front();
        }
    }

    // Assign symbol
    if (!isCurrentSymbolSet)
    {
        for (const auto& symbol : symbolTable)
        {
            if (symbol == RimPlotCurve::SYMBOL_NONE) continue;

            if (assignedSymbols.count(symbol) == 0)
            {
                currentSymbol = symbol;
                isCurrentSymbolSet = true;
                break;
            }
        }
        if (!isCurrentSymbolSet)
        {
            currentSymbol = symbolTable.front();
        }
    }

    // Observed data
    currentLineStyle = newCurveDef.first.sourceType() == RftSourceType::OBSERVED
        ? RimPlotCurve::STYLE_NONE : RimPlotCurve::STYLE_SOLID;

    newCurve->setColor(currentColor);
    newCurve->setSymbol(currentSymbol);
    newCurve->setLineStyle(currentLineStyle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateEditorsFromCurves()
{
    std::set<RimWellRftAddress>                         selectedSources;
    std::set<QDateTime>                                 selectedTimeSteps;
    std::map<QDateTime, std::set<RimWellRftAddress>>    selectedTimeStepsMap;

    const auto& curveDefs = curveDefsFromCurves();
    for (const auto& curveDef : curveDefs)
    {
        selectedSources.insert(curveDef.first);

        auto newTimeStepMap = std::map<QDateTime, std::set<RimWellRftAddress>>
        {
            { curveDef.second, std::set<RimWellRftAddress> { curveDef.first} }
        };
        addTimeStepsToMap(selectedTimeStepsMap, newTimeStepMap);
        selectedTimeSteps.insert(curveDef.second);
    }

    m_selectedSources = std::vector<RimWellRftAddress>(selectedSources.begin(), selectedSources.end());
    m_selectedTimeSteps = std::vector<QDateTime>(selectedTimeSteps.begin(), selectedTimeSteps.end());
    addTimeStepsToMap(m_timeStepsToAddresses, selectedTimeStepsMap);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateWidgetTitleWindowTitle()
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
void RimWellRftPlot::syncCurvesFromUiSelection()
{
    auto plotTrack = m_wellLogPlot->trackByIndex(0);
    const auto& allCurveDefs = selectedCurveDefs();
    const auto& curveDefsInPlot = curveDefsFromCurves();
    std::set<RimWellLogCurve*> curvesToDelete;
    std::set<std::pair<RimWellRftAddress, QDateTime>> newCurveDefs;

    if (allCurveDefs.size() < curveDefsInPlot.size())
    {
        // Determine which curves to delete from plot
        std::set<std::pair<RimWellRftAddress, QDateTime>> deleteCurveDefs;
        std::set_difference(curveDefsInPlot.begin(), curveDefsInPlot.end(),
                            allCurveDefs.begin(), allCurveDefs.end(),
                            std::inserter(deleteCurveDefs, deleteCurveDefs.end()));

        for (const auto& curve : plotTrack->curvesVector())
        {
            std::pair<RimWellRftAddress, QDateTime> curveDef = curveDefFromCurve(curve);
            if (deleteCurveDefs.count(curveDef) > 0)
                curvesToDelete.insert(curve);
        }
    }
    else
    {
        // Determine which curves are new since last time
        std::set_difference(allCurveDefs.begin(), allCurveDefs.end(),
                            curveDefsInPlot.begin(), curveDefsInPlot.end(),
                            std::inserter(newCurveDefs, newCurveDefs.end()));
    }
    updateCurvesInPlot(allCurveDefs, newCurveDefs, curvesToDelete);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellRftPlot::wellPathsContainingPressure(const QString& wellName) const
{
    std::vector<RimWellPath*> wellPaths;
    auto project = RiaApplication::instance()->project();

    for (RimOilField* oilField : project->oilFields)
    {
        auto wellPathColl = oilField->wellPathCollection();
        auto wellPathsVector = std::vector<RimWellPath*>(wellPathColl->wellPaths().begin(), wellPathColl->wellPaths().end());
        size_t timeStepCount = timeStepsFromWellPaths(wellPathsVector).size();

        if (timeStepCount == 0) continue;

        for (const auto& wellPath : wellPathColl->wellPaths)
        {
            bool hasPressure = false;
            const auto& wellLogFile = wellPath->wellLogFile();

            if (QString::compare(wellLogFile->wellName(), wellName) != 0) continue;

            if (hasPressureData(wellLogFile))
                wellPaths.push_back(wellPath);
        }
    }
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFileChannel*> RimWellRftPlot::getPressureChannelsFromWellPath(const RimWellPath* wellPath) const
{
    std::vector<RimWellLogFileChannel*> channels;

    auto wellLogFile = wellPath->wellLogFile();
    for (const auto& channel : wellLogFile->wellLogChannels())
    {
        // Todo: add more criterias
        if (hasPressureData(channel))
        {
            channels.push_back(channel);
        }
    }
    return channels;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimWellRftPlot::eclipseCaseFromCaseId(int caseId)
{
    const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCases = eclipseCasesContainingPressure(m_wellName);
    auto itr = std::find_if(eclipseCases.begin(), eclipseCases.end(), 
                            [caseId](std::tuple<RimEclipseResultCase*,bool,bool> eclCase) { return std::get<0>(eclCase)->caseId == caseId; });
    return itr != eclipseCases.end() ? std::get<0>(*itr) : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellRftPlot::wellPathForObservedData(const QString& wellName, const QDateTime& date) const
{
    auto wellPaths = wellPathsContainingPressure(wellName);
    for (const auto& wellPath : wellPaths)
    {
        auto wellLogFile = wellPath->wellLogFile();
        auto wName = wellLogFile->wellName();
        auto wDate = RiaDateStringParser::parseDateString(wellLogFile->date());
        if (wName == wellName && wDate == date)
        {
            return wellPath;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::tuple<RimEclipseResultCase*, bool/*hasPressure*/, bool /*hasRftData*/>> 
RimWellRftPlot::eclipseCasesContainingPressure(const QString& wellName) const
{
    std::vector<std::tuple<RimEclipseResultCase*, bool, bool>> cases;
    auto project = RiaApplication::instance()->project();

    for (RimOilField* oilField : project->oilFields)
    {
        auto eclCaseColl = oilField->analysisModels();
        for (RimEclipseCase* eCase : eclCaseColl->cases())
        {
            auto eclCase = dynamic_cast<RimEclipseResultCase*>(eCase);
            if (eclCase != nullptr)
            {
                auto eclipseCaseData = eclCase->eclipseCaseData();
                for (const auto& wellResult : eclipseCaseData->wellResults())
                {
                    if (QString::compare(wellResult->m_wellName, wellName) == 0)
                    {
                        bool hasPressure = hasPressureData(eclCase);
                        bool hasRftData = eclCase->rftReader() != nullptr;
                        cases.push_back(std::make_tuple(eclCase, hasPressure, hasRftData));
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
RimWellRftPlot::gridCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const
{
    std::vector<RimEclipseResultCase*> cases;
    for (const auto& eclCaseTuple : eclipseCasesTuple)
    {
        bool hasPressureData = std::get<1>(eclCaseTuple);
        size_t timeStepCount = timeStepsFromGridCase(std::get<0>(eclCaseTuple)).size();
        if (timeStepCount > 0)
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
RimWellRftPlot::rftCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const
{
    std::vector<RimEclipseResultCase*> cases;
    for (const auto& eclCaseTuple : eclipseCasesTuple)
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
std::map<QDateTime, std::set<RimWellRftAddress>> RimWellRftPlot::timeStepsFromRftCase(RimEclipseResultCase* rftCase) const
{
    std::map<QDateTime, std::set<RimWellRftAddress>> timeStepsMap;
    auto reader = rftCase->rftReader();
    if (reader != nullptr)
    {
        for (const auto& timeStep : reader->availableTimeSteps(m_wellName, RifEclipseRftAddress::PRESSURE))
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RimWellRftAddress>()));
            }
            timeStepsMap[timeStep].insert(RimWellRftAddress(RftSourceType::RFT, rftCase->caseId));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RimWellRftAddress>> RimWellRftPlot::timeStepsFromGridCase(const RimEclipseCase* gridCase) const
{
    auto eclipseCaseData = gridCase->eclipseCaseData();
    size_t resultIndex = eclipseCaseData != nullptr ? 
        eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, PRESSURE_DATA_NAME) :
        cvf::UNDEFINED_SIZE_T;

    std::map<QDateTime, std::set<RimWellRftAddress>> timeStepsMap;
    if (resultIndex != cvf::UNDEFINED_SIZE_T)
    {
        for (const auto& timeStep : eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->timeStepDates(resultIndex))
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RimWellRftAddress>()));
            }
            timeStepsMap[timeStep].insert(RimWellRftAddress(RftSourceType::GRID, gridCase->caseId));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RimWellRftAddress>> RimWellRftPlot::timeStepsFromWellPaths(const std::vector<RimWellPath*> wellPaths) const
{
    std::map<QDateTime, std::set<RimWellRftAddress>> timeStepsMap;
    for (const auto& wellPath : wellPaths)
    {
        auto wellLogFile = wellPath->wellLogFile();
        auto timeStep = RiaDateStringParser::parseDateString(wellLogFile->date());

        if (timeStepsMap.count(timeStep) == 0)
        {
            timeStepsMap.insert(std::make_pair(timeStep, std::set<RimWellRftAddress>()));
        }
        timeStepsMap[timeStep].insert(RimWellRftAddress(RftSourceType::OBSERVED));
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RimWellRftAddress>> 
RimWellRftPlot::adjacentTimeSteps(const std::vector<std::pair<QDateTime, std::set<RimWellRftAddress>>>& allTimeSteps,
                                  const std::pair<QDateTime, std::set<RimWellRftAddress>>& searchTimeStepPair)
{
    std::map<QDateTime, std::set<RimWellRftAddress>> timeStepsMap;

    if (allTimeSteps.size() > 0)
    {
        auto itr = std::find_if(allTimeSteps.begin(), allTimeSteps.end(), 
                                [searchTimeStepPair](const std::pair<QDateTime, std::set<RimWellRftAddress>>& dt)
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
bool RimWellRftPlot::mapContainsTimeStep(const std::map<QDateTime, std::set<RimWellRftAddress>>& map, const QDateTime& timeStep)
{
    return std::find_if(map.begin(), map.end(), [timeStep](const std::pair<QDateTime, std::set<RimWellRftAddress>>& pair)
    {
        return pair.first == timeStep;
    }) != map.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set < std::pair<RimWellRftAddress, QDateTime>> RimWellRftPlot::selectedCurveDefs() const
{
    std::set<std::pair<RimWellRftAddress, QDateTime>> curveDefs;
    const std::vector<std::tuple<RimEclipseResultCase*,bool,bool>>& eclipseCases = eclipseCasesContainingPressure(m_wellName);
    auto rftCases = rftCasesFromEclipseCases(eclipseCases);
    auto gridCases = gridCasesFromEclipseCases(eclipseCases);
    auto wellPaths = wellPathsContainingPressure(m_wellName);

    for (const auto& timeStep : m_selectedTimeSteps())
    {
        for (const auto& rftAddr : m_selectedSources())
        {
            if (rftAddr.sourceType() == RftSourceType::RFT)
            {
                for (const auto& rftCase : rftCases)
                {
                    const auto& timeSteps = timeStepsFromRftCase(rftCase);
                    if (mapContainsTimeStep(timeSteps, timeStep))
                    {
                        curveDefs.insert(std::make_pair(rftAddr, timeStep));
                    }
                }
            }
            else if (rftAddr.sourceType() == RftSourceType::GRID)
            {
                for (const auto& gridCase : gridCases)
                {
                    const auto& timeSteps = timeStepsFromGridCase(gridCase);
                    if (mapContainsTimeStep(timeSteps, timeStep))
                    {
                        curveDefs.insert(std::make_pair(rftAddr, timeStep));
                    }
                }
            }
            else if (rftAddr.sourceType() == RftSourceType::OBSERVED)
            {
                const auto& timeSteps = timeStepsFromWellPaths(wellPaths);
                if (mapContainsTimeStep(timeSteps, timeStep))
                {
                    curveDefs.insert(std::make_pair(rftAddr, timeStep));
                }
            }
        }
    }
    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::pair<RimWellRftAddress, QDateTime>> RimWellRftPlot::curveDefsFromCurves() const
{
    std::set<std::pair<RimWellRftAddress, QDateTime>> curveDefs;

    auto plotTrack = m_wellLogPlot->trackByIndex(0);
    for (const auto& curve : plotTrack->curvesVector())
    {
        curveDefs.insert(curveDefFromCurve(curve));
    }
    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<RimWellRftAddress, QDateTime> RimWellRftPlot::curveDefFromCurve(const RimWellLogCurve* curve) const
{
    const RimWellLogRftCurve* rftCurve = dynamic_cast<const RimWellLogRftCurve*>(curve);
    const RimWellLogExtractionCurve* gridCurve = dynamic_cast<const RimWellLogExtractionCurve*>(curve);
    const RimWellLogFileCurve* wellLogFileCurve = dynamic_cast<const RimWellLogFileCurve*>(curve);

    if (rftCurve != nullptr)
    {
        const RimEclipseResultCase* rftCase = dynamic_cast<const RimEclipseResultCase*>(rftCurve->eclipseResultCase());
        if (rftCase != nullptr)
        {
            auto rftAddress = rftCurve->rftAddress();
            auto timeStep = rftAddress.timeStep();
            return std::make_pair(RimWellRftAddress(RftSourceType::RFT, rftCase->caseId), timeStep);
        }
    }
    else if (gridCurve != nullptr)
    {
        const RimEclipseResultCase* gridCase = dynamic_cast<const RimEclipseResultCase*>(gridCurve->rimCase());
        if (gridCase != nullptr)
        {
            size_t timeStepIndex = gridCurve->currentTimeStep();
            auto timeStepsMap = timeStepsFromGridCase(gridCase);
            auto timeStepsVector = std::vector<std::pair<QDateTime, std::set<RimWellRftAddress>>>(
                timeStepsMap.begin(), timeStepsMap.end());
            if (timeStepIndex < timeStepsMap.size())
            {
                return std::make_pair(RimWellRftAddress(RftSourceType::GRID, gridCase->caseId),
                                      timeStepsVector[timeStepIndex].first);
            }
        }
    }
    else if (wellLogFileCurve != nullptr)
    {
        const auto& wellPath = wellLogFileCurve->wellPath();
        const auto& wellLogFile = wellPath->wellLogFile();
        const auto date = RiaDateStringParser::parseDateString(wellLogFile->date());

        if (date.isValid())
        {
            return std::make_pair(RimWellRftAddress(RftSourceType::OBSERVED), date);
        }
    }
    return std::make_pair(RimWellRftAddress(RftSourceType::NONE), QDateTime());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateCurvesInPlot(const std::set<std::pair<RimWellRftAddress, QDateTime>>& allCurveDefs,
                                        const std::set<std::pair<RimWellRftAddress, QDateTime>>& curveDefsToAdd,
                                        const std::set<RimWellLogCurve*>& curvesToDelete)
{
    auto plotTrack = m_wellLogPlot->trackByIndex(0);

    // Delete curves
    for (const auto& curve : curvesToDelete)
    {
        plotTrack->removeCurve(curve);
    }

    // Add new curves
    for (const auto& curveDefToAdd : curveDefsToAdd)
    {
        if (curveDefToAdd.first.sourceType() == RftSourceType::RFT)
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve(curve);

            auto rftCase = eclipseCaseFromCaseId(curveDefToAdd.first.caseId());
            curve->setEclipseResultCase(dynamic_cast<RimEclipseResultCase*>(rftCase));

            RifEclipseRftAddress address(m_wellName, curveDefToAdd.second, RifEclipseRftAddress::PRESSURE);
            curve->setRftAddress(address);

            applyCurveAppearance(curve);
            curve->loadDataAndUpdate(true);
        }
        else if (curveDefToAdd.first.sourceType() == RftSourceType::GRID)
        {
            auto curve = new RimWellLogExtractionCurve();
            plotTrack->addCurve(curve);

            cvf::Color3f curveColor = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f(plotTrack->curveCount());
            curve->setColor(curveColor);
            curve->setFromSimulationWellName(m_wellName, m_branchIndex);

            // Fetch cases and time steps
            auto gridCase = eclipseCaseFromCaseId(curveDefToAdd.first.caseId());
            if (gridCase != nullptr)
            {
                // Case
                curve->setCase(gridCase);

                // Result definition
                RimEclipseResultDefinition* resultDef = new RimEclipseResultDefinition();
                resultDef->setResultVariable(PRESSURE_DATA_NAME);
                curve->setEclipseResultDefinition(resultDef);

                // Time step
                auto timeSteps = timeStepsFromGridCase(gridCase);
                auto currentTimeStepIt = std::find_if(timeSteps.begin(), timeSteps.end(), 
                                                      [curveDefToAdd](std::pair<QDateTime, std::set<RimWellRftAddress>> pair) {return pair.first == curveDefToAdd.second; });
                auto currentTimeStep = std::distance(timeSteps.begin(), currentTimeStepIt);
                curve->setCurrentTimeStep(currentTimeStep);

                applyCurveAppearance(curve);
                curve->loadDataAndUpdate(false);
            }
        }
        else if (curveDefToAdd.first.sourceType() == RftSourceType::OBSERVED)
        {
            auto wellPath = wellPathForObservedData(m_wellName, curveDefToAdd.second);
            if (wellPath != nullptr)
            {
                auto wellLogFile = wellPath->wellLogFile();
                auto pressureChannels = getPressureChannelsFromWellPath(wellPath);
                auto curve = new RimWellLogFileCurve();
                plotTrack->addCurve(curve);
                curve->setWellPath(wellPath);
                curve->setWellLogChannelName(pressureChannels.front()->name());

                applyCurveAppearance(curve);
                curve->loadDataAndUpdate(true);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::isOnlyGridSourcesSelected() const
{
    const auto& selSources = m_selectedSources();
    return std::find_if(selSources.begin(), selSources.end(), [](const RimWellRftAddress& addr)
    {
        return addr.sourceType() == RftSourceType::RFT || addr.sourceType() == RftSourceType::OBSERVED;
    }) == selSources.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::isAnySourceAddressSelected(const std::set<RimWellRftAddress>& addresses) const
{
    const auto& selectedSourcesVector = m_selectedSources();
    const auto selectedSources = std::set<RimWellRftAddress>(selectedSourcesVector.begin(), selectedSourcesVector.end());
    std::vector<RimWellRftAddress> intersectVector;

    std::set_intersection(selectedSources.begin(), selectedSources.end(),
                          addresses.begin(), addresses.end(), std::inserter(intersectVector, intersectVector.end()));
    return intersectVector.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellRftPlot::viewWidget()
{
    return m_wellLogPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::zoomAll()
{
    m_wellLogPlot()->zoomAll();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RimWellRftPlot::wellLogPlot() const
{
    return m_wellLogPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setCurrentWellName(const QString& currWellName)
{
    m_wellName = currWellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellRftPlot::currentWellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::hasPressureData(RimWellLogFile* wellLogFile)
{
    auto wellLogChannels = wellLogFile->wellLogChannels();
    for (const auto& wellLogChannel : wellLogChannels)
    {
        if (hasPressureData(wellLogChannel)) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::hasPressureData(RimWellLogFileChannel* channel)
{
    // Todo: read pressure channel names from config/defines
    return QString::compare(channel->name(), PRESSURE_DATA_NAME) == 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::hasPressureData(RimEclipseResultCase* gridCase)
{
    auto eclipseCaseData = gridCase->eclipseCaseData();
    size_t resultIndex = eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->
        findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, PRESSURE_DATA_NAME);
    return resultIndex != cvf::UNDEFINED_SIZE_T;
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
QList<caf::PdmOptionItemInfo> RimWellRftPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_wellName)
    {
        calculateValueOptionsForWells(options);
    }
    else if (fieldNeedingOptions == &m_selectedSources)
    {
        const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCases = eclipseCasesContainingPressure(m_wellName);

        auto rftCases = rftCasesFromEclipseCases(eclipseCases);
        if (rftCases.size() > 0)
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(RimWellRftAddress::sourceTypeUiText(RftSourceType::RFT), true));
        }
        for (const auto& rftCase : rftCases)
        {
            auto addr = RimWellRftAddress(RftSourceType::RFT, rftCase->caseId);
            auto item = caf::PdmOptionItemInfo(rftCase->caseUserDescription(), QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
        }

        auto gridCases = gridCasesFromEclipseCases(eclipseCases);
        if (gridCases.size() > 0)
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(RimWellRftAddress::sourceTypeUiText(RftSourceType::GRID), true));
        }
        for (const auto& gridCase : gridCases)
        {
            auto addr = RimWellRftAddress(RftSourceType::GRID, gridCase->caseId);
            auto item = caf::PdmOptionItemInfo(gridCase->caseUserDescription(), QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
        }

        if (wellPathsContainingPressure(m_wellName).size() > 0)
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(RimWellRftAddress::sourceTypeUiText(RftSourceType::OBSERVED), true));

            auto addr = RimWellRftAddress(RftSourceType::OBSERVED);
            auto item = caf::PdmOptionItemInfo(addr.uiText(), QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
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

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_wellName)
    {
        setDescription(QString(plotNameFormatString()).arg(m_wellName));
    }

    if (changedField == &m_wellName || changedField == &m_branchIndex)
    {
        auto plotTrack = m_wellLogPlot->trackByIndex(0);
        for (const auto& curve : plotTrack->curvesVector())
        {
            plotTrack->removeCurve(curve);
        }
        m_timeStepsToAddresses.clear();
        updateEditorsFromCurves();
    }
    else if (changedField == &m_selectedSources ||
             changedField == &m_selectedTimeSteps)
    {
        syncCurvesFromUiSelection();
    }
    else if (changedField == &m_showPlotTitle)
    {
        //m_wellLogPlot->setShowDescription(m_showPlotTitle);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimWellRftPlot::snapshotWindowContent()
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
void RimWellRftPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
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

    //uiOrdering.add(&m_showPlotTitle);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::addTimeStepToMap(std::map<QDateTime, std::set<RimWellRftAddress>>& destMap,
                                       const std::pair<QDateTime, std::set<RimWellRftAddress>>& timeStepToAdd)
{
    auto timeStepMapToAdd = std::map<QDateTime, std::set<RimWellRftAddress>>
    {
        timeStepToAdd
    };
    addTimeStepsToMap(destMap, timeStepMapToAdd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::addTimeStepsToMap(std::map<QDateTime, std::set<RimWellRftAddress>>& destMap, 
                                       const std::map<QDateTime, std::set<RimWellRftAddress>>& timeStepsToAdd)
{
    for (const auto& timeStepPair : timeStepsToAdd)
    {
        if (timeStepPair.first.isValid())
        {
            if (destMap.count(timeStepPair.first) == 0)
            {
                destMap.insert(std::make_pair(timeStepPair.first, std::set<RimWellRftAddress>()));
            }
            auto addresses = timeStepPair.second;
            destMap[timeStepPair.first].insert(addresses.begin(), addresses.end());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options)
{
    RimProject * proj = RiaApplication::instance()->project();

    if (proj != nullptr)
    {
        const auto simWellNames = proj->simulationWellNames();
        std::set<QString> wellNames = std::set<QString>(simWellNames.begin(), simWellNames.end());

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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::calculateValueOptionsForTimeSteps(const QString& wellName, QList<caf::PdmOptionItemInfo>& options)
{
    std::map<QDateTime, std::set<RimWellRftAddress>> displayTimeStepsMap, obsAndRftTimeStepsMap, gridTimeStepsMap;
    const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCases = eclipseCasesContainingPressure(wellName);
    auto rftCases = rftCasesFromEclipseCases(eclipseCases);
    auto gridCases = gridCasesFromEclipseCases(eclipseCases);
    auto observedWellPaths = wellPathsContainingPressure(m_wellName);

    for (const auto& selection : m_selectedSources())
    {
        if (selection.sourceType() == RimWellRftAddress(RftSourceType::RFT))
        {
            for (const auto& rftCase : rftCases)
            {
                addTimeStepsToMap(obsAndRftTimeStepsMap, timeStepsFromRftCase(rftCase));
            }
        }
        else if (selection.sourceType() == RimWellRftAddress(RftSourceType::GRID))
        {
            for (const auto& gridCase : gridCases)
            {
                addTimeStepsToMap(gridTimeStepsMap, timeStepsFromGridCase(gridCase));
            }
        }
        else if (selection.sourceType() == RimWellRftAddress(RftSourceType::OBSERVED))
        {
            addTimeStepsToMap(obsAndRftTimeStepsMap, timeStepsFromWellPaths(observedWellPaths));
        }
    }

    if (isOnlyGridSourcesSelected())
    {
        displayTimeStepsMap = gridTimeStepsMap;
    }
    else
    {
        for (const auto& timeStepPair : obsAndRftTimeStepsMap)
        {
            const auto gridTimeStepsVector = std::vector<std::pair<QDateTime, std::set<RimWellRftAddress>>>(gridTimeStepsMap.begin(), gridTimeStepsMap.end());
            const auto& adjTimeSteps = adjacentTimeSteps(gridTimeStepsVector, timeStepPair);
            addTimeStepsToMap(displayTimeStepsMap, adjTimeSteps);
        }

        // Add already selected time steps
        for (const auto& timeStep : m_selectedTimeSteps())
        {
            if (m_timeStepsToAddresses.count(timeStep) > 0)
            {
                auto sourceAddresses = m_timeStepsToAddresses[timeStep];
                if (isAnySourceAddressSelected(sourceAddresses))
                {
                    displayTimeStepsMap.insert(std::make_pair(timeStep, m_timeStepsToAddresses[timeStep]));
                }
            }
        }
    }

    for (const auto& timeStepPair : displayTimeStepsMap)
    {
        options.push_back(caf::PdmOptionItemInfo(timeStepPair.first.toString(RimTools::dateFormatString()), timeStepPair.first));
    }
    addTimeStepsToMap(m_timeStepsToAddresses, displayTimeStepsMap);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setDescription(const QString& description)
{
    m_userName = description;

    updateWidgetTitleWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellRftPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::loadDataAndUpdate()
{
    updateMdiWindowVisibility();
    m_wellLogPlot->loadDataAndUpdate();
    updateEditorsFromCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellRftPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_wellLogPlotWidget = new RiuWellRftPlot(this, mainWindowParent);
    return m_wellLogPlotWidget;
}

