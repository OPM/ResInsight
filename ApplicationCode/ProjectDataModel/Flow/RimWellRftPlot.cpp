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

#include "RigAccWellFlowCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigSingleWellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"
#include "RimTotalWellAllocationPlot.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimTofAccumulatedPhaseFractionsPlot.h"
#include "RimOilField.h"
#include "RiuMainPlotWindow.h"
#include "RiuWellRftPlot.h"
#include "RiuWellLogTrack.h"
#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "SummaryPlotCommands/RicSummaryCurveCreatorUiKeywords.h"
#include "cafPdmChildArrayField.h"
#include "RimWellRftAddress.h"
#include "RiaDateStringParser.h"
#include "RimTools.h"
#include "RimWellLogFileCurve.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RigEclipseCaseData.h"
#include "RigCaseCellResultsData.h"
#include "RimWellLogExtractionCurve.h"
#include "RiaColorTables.h"
#include "RifReaderEclipseRft.h"
#include "RimWellLogRftCurve.h"
#include "RigWellPath.h"
#include "RimWellLogPlotCollection.h"
#include "RimMainPlotCollection.h"
#include <tuple>

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
    std::set<RimWellRftAddress> selectedSources;
    std::set<QDateTime>         selectedTimeSteps;

    const auto& curveDefs = curveDefsFromCurves();
    for (const auto& curveDef : curveDefs)
    {
        selectedSources.insert(curveDef.first);
        selectedTimeSteps.insert(curveDef.second);
    }

    m_selectedSources = std::vector<RimWellRftAddress>(selectedSources.begin(), selectedSources.end());
    m_selectedTimeSteps = std::vector<QDateTime>(selectedTimeSteps.begin(), selectedTimeSteps.end());
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
std::vector<QDateTime> RimWellRftPlot::timeStepsFromRftCase(RimEclipseResultCase* rftCase) const
{
    auto reader = rftCase->rftReader();
    if (reader != nullptr)
    {
        return reader->availableTimeSteps(m_wellName, RifEclipseRftAddress::PRESSURE);
    }
    return std::vector<QDateTime>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimWellRftPlot::timeStepsFromGridCase(const RimEclipseCase* gridCase) const
{
    auto eclipseCaseData = gridCase->eclipseCaseData();
    size_t resultIndex = eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->
        findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, PRESSURE_DATA_NAME);
    return eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->timeStepDates(resultIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimWellRftPlot::timeStepsFromWellPaths(const std::vector<RimWellPath*> wellPaths) const
{
    std::set<QDateTime> dates;
    for (const auto& wellPath : wellPaths)
    {
        auto wellLogFile = wellPath->wellLogFile();
        dates.insert(RiaDateStringParser::parseDateString(wellLogFile->date()));
    }
    return std::vector<QDateTime>(dates.begin(), dates.end());
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

    for (const auto& selectedDate : m_selectedTimeSteps())
    {
        for (const auto& rftAddr : m_selectedSources())
        {
            if (rftAddr.sourceType() == RftSourceType::RFT)
            {
                for (const auto& rftCase : rftCases)
                {
                    const auto& timeSteps = timeStepsFromRftCase(rftCase);
                    if (std::find(timeSteps.begin(), timeSteps.end(), selectedDate) != timeSteps.end())
                    {
                        curveDefs.insert(std::make_pair(rftAddr, selectedDate));
                    }
                }
            }
            else if (rftAddr.sourceType() == RftSourceType::GRID)
            {
                for (const auto& gridCase : gridCases)
                {
                    const auto& timeSteps = timeStepsFromGridCase(gridCase);
                    if (std::find(timeSteps.begin(), timeSteps.end(), selectedDate) != timeSteps.end())
                    {
                        curveDefs.insert(std::make_pair(rftAddr, selectedDate));
                    }
                }
            }
            else if (rftAddr.sourceType() == RftSourceType::OBSERVED)
            {
                const auto& timeSteps = timeStepsFromWellPaths(wellPaths);
                if (std::find(timeSteps.begin(), timeSteps.end(), selectedDate) != timeSteps.end())
                {
                    curveDefs.insert(std::make_pair(rftAddr, selectedDate));
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
            auto timeSteps = timeStepsFromGridCase(gridCase);
            if (timeStepIndex < timeSteps.size())
            {
                return std::make_pair(RimWellRftAddress(RftSourceType::GRID, gridCase->caseId),
                                      timeSteps[timeStepIndex]);
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
                auto currentTimeStepIt = std::find(timeSteps.begin(), timeSteps.end(), curveDefToAdd.second);
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
    else if (changedField == &m_selectedSources ||
             changedField == &m_selectedTimeSteps ||
             changedField == &m_branchIndex)
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
    std::set<QDateTime> dates;
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
                for (const auto& date : timeStepsFromRftCase(rftCase))
                {
                    if (date.isValid())
                    {
                        dates.insert(date);
                    }
                }
            }
        }
        else if (selection.sourceType() == RimWellRftAddress(RftSourceType::GRID))
        {
            for (const auto& gridCase : gridCases)
            {
                for (const auto& date : timeStepsFromGridCase(gridCase))
                {
                    if (date.isValid())
                    {
                        dates.insert(date);
                    }
                }
            }
        }
        else if (selection.sourceType() == RimWellRftAddress(RftSourceType::OBSERVED))
        {
            for (const auto& date : timeStepsFromWellPaths(observedWellPaths))
            {
                if (date.isValid())
                {
                    dates.insert(date);
                }
            }
        }
    }

    for (const auto& date : dates)
    {
        options.push_back(caf::PdmOptionItemInfo(date.toString(RimTools::dateFormatString()), date));
    }
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

