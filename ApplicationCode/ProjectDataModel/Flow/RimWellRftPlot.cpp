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
#include "RigSimWellData.h"
#include "RigWellPath.h"

#include "RimWellPlotTools.h"
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

#include <algorithm>
#include <iterator>
#include <tuple>

CAF_PDM_SOURCE_INIT(RimWellRftPlot, "WellRftPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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
    m_wellLogPlot->setDepthType(RimWellLogPlot::TRUE_VERTICAL_DEPTH);
    m_wellLogPlot.uiCapability()->setUiTreeHidden(true);
    m_wellLogPlot.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellPathNameOrSimWellName, "WellName", "WellName", "", "", "");
    CAF_PDM_InitField(&m_branchIndex, "BranchIndex", 0, "BranchIndex", "", "", "");

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
    m_selectedSourcesOrTimeStepsFieldsChanged = false;
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
    const RiaRftPltCurveDefinition& newCurveDef = RimWellPlotTools::curveDefFromCurve(newCurve);

    std::vector<cvf::Color3f> colorTable;
    RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector(&colorTable);

    std::vector<RimPlotCurve::PointSymbolEnum> symbolTable =
    {
        RimPlotCurve::SYMBOL_ELLIPSE,
        RimPlotCurve::SYMBOL_RECT,
        RimPlotCurve::SYMBOL_DIAMOND,
        RimPlotCurve::SYMBOL_TRIANGLE,
        RimPlotCurve::SYMBOL_CROSS,
        RimPlotCurve::SYMBOL_XCROSS
    };

    // State variables
    static size_t  defaultColorTableIndex = 0;
    static size_t  defaultSymbolTableIndex = 0;

    cvf::Color3f                    currentColor;
    RimPlotCurve::PointSymbolEnum   currentSymbol = symbolTable.front();
    RimPlotCurve::LineStyleEnum     currentLineStyle = RimPlotCurve::STYLE_SOLID;
    bool                            isCurrentColorSet = false;
    bool                            isCurrentSymbolSet = false;

    std::set<cvf::Color3f>                  assignedColors;
    std::set<RimPlotCurve::PointSymbolEnum> assignedSymbols;

    // Used colors and symbols
    for (RimWellLogCurve* const curve : m_wellLogPlot->trackByIndex(0)->curvesVector())
    {
        if (curve == newCurve) continue;

        RiaRftPltCurveDefinition cDef = RimWellPlotTools::curveDefFromCurve(curve);
        if (cDef.address() == newCurveDef.address())
        {
            currentColor = curve->color();
            isCurrentColorSet = true;
        }
        if (cDef.timeStep() == newCurveDef.timeStep())
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
            currentColor = colorTable[defaultColorTableIndex];
            if (++defaultColorTableIndex == colorTable.size())
                defaultColorTableIndex = 0;

        }
    }

    // Assign symbol
    if (!isCurrentSymbolSet)
    {
        for (const auto& symbol : symbolTable)
        {
            if (assignedSymbols.count(symbol) == 0)
            {
                currentSymbol = symbol;
                isCurrentSymbolSet = true;
                break;
            }
        }
        if (!isCurrentSymbolSet)
        {
            currentSymbol = symbolTable[defaultSymbolTableIndex];
            if (++defaultSymbolTableIndex == symbolTable.size())
                defaultSymbolTableIndex = 0;
        }
    }

    // Observed data
    currentLineStyle = newCurveDef.address().sourceType() == RifWellRftAddress::OBSERVED
        ? RimPlotCurve::STYLE_NONE : RimPlotCurve::STYLE_SOLID;

    newCurve->setColor(currentColor);
    newCurve->setSymbol(currentSymbol);
    newCurve->setLineStyle(currentLineStyle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateSelectedTimeStepsFromSelectedSources()
{
    std::vector<QDateTime> newTimeStepsSelections;
    std::vector<RifWellRftAddress> selectedSourcesVector = selectedSources();
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
void RimWellRftPlot::updateFormationsOnPlot() const
{
    if (m_selectedTimeSteps().empty())
    {
        for (size_t i = 0; i < m_wellLogPlot->trackCount(); i++)
        {
            m_wellLogPlot->trackByIndex(i)->setAndUpdateWellPathFormationNamesData(nullptr, nullptr);
        }
        return;
    }

    RimProject* proj = RiaApplication::instance()->project();
    RimOilField* oilField = proj->activeOilField();

    RimWellPathCollection* wellPathCollection = oilField->wellPathCollection();
    RimWellPath* wellPath = wellPathCollection->wellPathByName(m_wellPathNameOrSimWellName);

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
        if (trajectoryType == RimWellLogTrack::SIMULATION_WELL)
        {
            m_wellLogPlot->trackByIndex(0)->setAndUpdateSimWellFormationNamesData(rimCase, associatedSimWellName(), m_branchIndex);
        }
        else if (trajectoryType == RimWellLogTrack::WELL_PATH)
        {
            m_wellLogPlot->trackByIndex(0)->setAndUpdateWellPathFormationNamesData(rimCase, wellPath);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellRftPlot::associatedSimWellName() const
{
    return RimWellPlotTools::simWellName(m_wellPathNameOrSimWellName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::applyInitialSelections()
{
    std::vector<RifWellRftAddress> sourcesToSelect;
    std::set<QDateTime> rftTimeSteps;
    std::set<QDateTime> observedTimeSteps;
    std::set<QDateTime> gridTimeSteps;
    const QString simWellName = associatedSimWellName();

    for(RimEclipseResultCase* const rftCase : RimWellPlotTools::rftCasesForWell(simWellName))
    {
        sourcesToSelect.push_back(RifWellRftAddress(RifWellRftAddress::RFT, rftCase));
        RimWellPlotTools::appendSet(rftTimeSteps, RimWellPlotTools::timeStepsFromRftCase(rftCase, simWellName));
    }
    
    for (RimEclipseResultCase* const gridCase : RimWellPlotTools::gridCasesForWell(simWellName))
    {
        sourcesToSelect.push_back(RifWellRftAddress(RifWellRftAddress::GRID, gridCase));
        RimWellPlotTools::appendSet(gridTimeSteps, RimWellPlotTools::timeStepsFromGridCase(gridCase));
    }
    
    std::vector<RimWellLogFile*> wellLogFiles = RimWellPlotTools::wellLogFilesContainingPressure(simWellName);
    if(wellLogFiles.size() > 0)
    {
        sourcesToSelect.push_back(RifWellRftAddress(RifWellRftAddress::OBSERVED));
        for (RimWellLogFile* const wellLogFile : wellLogFiles)
        {
            observedTimeSteps.insert(RimWellPlotTools::timeStepFromWellLogFile(wellLogFile));
        }
    }

    m_selectedSources = sourcesToSelect;
    
    std::set<QDateTime> timeStepsToSelect;
    for (const QDateTime& timeStep : rftTimeSteps)
    {
        timeStepsToSelect.insert(timeStep);
    }
    for (const QDateTime& timeStep : observedTimeSteps)
    {
        timeStepsToSelect.insert(timeStep);
    }
    if (gridTimeSteps.size() > 0)
        timeStepsToSelect.insert(*gridTimeSteps.begin());

    m_selectedTimeSteps = std::vector<QDateTime>(timeStepsToSelect.begin(), timeStepsToSelect.end());

    syncCurvesFromUiSelection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateEditorsFromCurves()
{
    std::set<RifWellRftAddress>                         selectedSources;
    std::set<QDateTime>                                 selectedTimeSteps;
    std::map<QDateTime, std::set<RifWellRftAddress>>    selectedTimeStepsMap;

    for (const RiaRftPltCurveDefinition& curveDef : curveDefsFromCurves())
    {
        if (curveDef.address().sourceType() == RifWellRftAddress::OBSERVED)
            selectedSources.insert(RifWellRftAddress(RifWellRftAddress::OBSERVED));
        else
            selectedSources.insert(curveDef.address());

        auto newTimeStepMap = std::map<QDateTime, std::set<RifWellRftAddress>>
        {
            { curveDef.timeStep(), std::set<RifWellRftAddress> { curveDef.address()} }
        };
        RimWellPlotTools::addTimeStepsToMap(selectedTimeStepsMap, newTimeStepMap);
        selectedTimeSteps.insert(curveDef.timeStep());
    }

    m_selectedSources = std::vector<RifWellRftAddress>(selectedSources.begin(), selectedSources.end());
    m_selectedTimeSteps = std::vector<QDateTime>(selectedTimeSteps.begin(), selectedTimeSteps.end());
    RimWellPlotTools::addTimeStepsToMap(m_timeStepsToAddresses, selectedTimeStepsMap);
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
    RimWellLogTrack* plotTrack = m_wellLogPlot->trackByIndex(0);
    const std::set<RiaRftPltCurveDefinition>& allCurveDefs = selectedCurveDefs();
    const std::set<RiaRftPltCurveDefinition>& curveDefsInPlot = curveDefsFromCurves();
    std::set<RimWellLogCurve*> curvesToDelete;
    std::set<RiaRftPltCurveDefinition> newCurveDefs;

    if (allCurveDefs.size() < curveDefsInPlot.size())
    {
        // Determine which curves to delete from plot
        std::set<RiaRftPltCurveDefinition> deleteCurveDefs;
        std::set_difference(curveDefsInPlot.begin(), curveDefsInPlot.end(),
                            allCurveDefs.begin(), allCurveDefs.end(),
                            std::inserter(deleteCurveDefs, deleteCurveDefs.end()));

        for (RimWellLogCurve* const curve : plotTrack->curvesVector())
        {
            RiaRftPltCurveDefinition curveDef = RimWellPlotTools::curveDefFromCurve(curve);
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
std::set < RiaRftPltCurveDefinition> RimWellRftPlot::selectedCurveDefs() const
{
    std::set<RiaRftPltCurveDefinition> curveDefs;
    const QString simWellName = associatedSimWellName();

    const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell(simWellName);
    const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell(simWellName);

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
                        curveDefs.insert(RiaRftPltCurveDefinition(addr, timeStep));
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
                        curveDefs.insert(RiaRftPltCurveDefinition(addr, timeStep));
                    }
                }
            }
            else if (addr.sourceType() == RifWellRftAddress::OBSERVED)
            {
                if (addr.wellLogFile() != nullptr)
                {
                    const QDateTime wellLogFileTimeStep = RimWellPlotTools::timeStepFromWellLogFile(addr.wellLogFile());
                    if (wellLogFileTimeStep == timeStep)
                    {
                        curveDefs.insert(RiaRftPltCurveDefinition(RifWellRftAddress(RifWellRftAddress::OBSERVED, addr.wellLogFile()), timeStep));
                    }
                }
            }
        }
    }
    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RiaRftPltCurveDefinition> RimWellRftPlot::curveDefsFromCurves() const
{
    std::set<RiaRftPltCurveDefinition> curveDefs;

    RimWellLogTrack* const plotTrack = m_wellLogPlot->trackByIndex(0);
    for (RimWellLogCurve* const curve : plotTrack->curvesVector())
    {
        curveDefs.insert(RimWellPlotTools::curveDefFromCurve(curve));
    }
    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateCurvesInPlot(const std::set<RiaRftPltCurveDefinition>& allCurveDefs,
                                        const std::set<RiaRftPltCurveDefinition>& curveDefsToAdd,
                                        const std::set<RimWellLogCurve*>& curvesToDelete)
{
    const QString simWellName = associatedSimWellName();
    RimWellLogTrack* const plotTrack = m_wellLogPlot->trackByIndex(0);

    // Delete curves
    for (RimWellLogCurve* const curve : curvesToDelete)
    {
        plotTrack->removeCurve(curve);
    }

    // Add new curves
    for (const RiaRftPltCurveDefinition& curveDefToAdd : curveDefsToAdd)
    {
        if (curveDefToAdd.address().sourceType() == RifWellRftAddress::RFT)
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve(curve);

            auto rftCase = curveDefToAdd.address().eclCase();
            curve->setEclipseResultCase(dynamic_cast<RimEclipseResultCase*>(rftCase));

            RifEclipseRftAddress address(simWellName, curveDefToAdd.timeStep(), RifEclipseRftAddress::PRESSURE);
            curve->setRftAddress(address);
            curve->setZOrder(1);

            applyCurveAppearance(curve);
        }
        else if (curveDefToAdd.address().sourceType() == RifWellRftAddress::GRID)
        {
            auto curve = new RimWellLogExtractionCurve();
            plotTrack->addCurve(curve);

            cvf::Color3f curveColor = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f(plotTrack->curveCount());
            curve->setColor(curveColor);
            curve->setFromSimulationWellName(simWellName, m_branchIndex);

            // Fetch cases and time steps
            auto gridCase = curveDefToAdd.address().eclCase();
            if (gridCase != nullptr)
            {
                std::pair<size_t, QString> resultDataInfo = RimWellPlotTools::pressureResultDataInfo(gridCase->eclipseCaseData());

                // Case
                curve->setCase(gridCase);

                // Result definition
                RimEclipseResultDefinition* resultDef = new RimEclipseResultDefinition();
                resultDef->setResultVariable(resultDataInfo.second);
                curve->setEclipseResultDefinition(resultDef);

                // Time step
                const std::set<QDateTime>& timeSteps = RimWellPlotTools::timeStepsFromGridCase(gridCase);
                auto currentTimeStepItr = std::find_if(timeSteps.begin(), timeSteps.end(), 
                                                       [curveDefToAdd](const QDateTime& timeStep) {return timeStep == curveDefToAdd.timeStep(); });
                auto currentTimeStepIndex = std::distance(timeSteps.begin(), currentTimeStepItr);
                curve->setCurrentTimeStep(currentTimeStepIndex);
                curve->setZOrder(0);

                applyCurveAppearance(curve);
            }
        }
        else if (curveDefToAdd.address().sourceType() == RifWellRftAddress::OBSERVED)
        {
            RimWellLogFile* const wellLogFile = curveDefToAdd.address().wellLogFile();
            RimWellPath* const wellPath = RimWellPlotTools::wellPathFromWellLogFile(wellLogFile);
            if(wellLogFile!= nullptr)
            {
                RimWellLogFileChannel* pressureChannel = RimWellPlotTools::getPressureChannelFromWellFile(wellLogFile);
                auto curve = new RimWellLogFileCurve();
                plotTrack->addCurve(curve);
                curve->setWellPath(wellPath);
                curve->setWellLogFile(wellLogFile);
                curve->setWellLogChannelName(pressureChannel->name());
                curve->setZOrder(2);

                applyCurveAppearance(curve);
            }
        }
    }

    m_wellLogPlot->loadDataAndUpdate();
    m_wellLogPlot->zoomAll();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::isOnlyGridSourcesSelected() const
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
bool RimWellRftPlot::isAnySourceAddressSelected(const std::set<RifWellRftAddress>& addresses) const
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
std::vector<RifWellRftAddress> RimWellRftPlot::selectedSources() const
{
    std::vector<RifWellRftAddress> sources;
    for (const RifWellRftAddress& addr : m_selectedSources())
    {
        if (addr.sourceType() == RifWellRftAddress::OBSERVED)
        {
            for (RimWellLogFile* const wellLogFile : RimWellPlotTools::wellLogFilesContainingPressure(associatedSimWellName()))
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
void RimWellRftPlot::setSimWellOrWellPathName(const QString& currWellName)
{
    m_wellPathNameOrSimWellName = currWellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellRftPlot::simWellOrWellPathName() const
{
    return m_wellPathNameOrSimWellName;
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
QList<caf::PdmOptionItemInfo> RimWellRftPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    const QString simWellName = associatedSimWellName();

    if (fieldNeedingOptions == &m_wellPathNameOrSimWellName)
    {
        calculateValueOptionsForWells(options);
    }
    else if (fieldNeedingOptions == &m_selectedSources)
    {
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

        if (RimWellPlotTools::wellLogFilesContainingPressure(simWellName).size() > 0)
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(RifWellRftAddress::sourceTypeUiText(RifWellRftAddress::OBSERVED), true));

            auto addr = RifWellRftAddress(RifWellRftAddress::OBSERVED);
            auto item = caf::PdmOptionItemInfo("Observed Data", QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
        }
    }
    else if (fieldNeedingOptions == &m_selectedTimeSteps)
    {
        calculateValueOptionsForTimeSteps(options);
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        RimProject* proj = RiaApplication::instance()->project();

        size_t branchCount = proj->simulationWellBranches(simWellName).size();

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

    if (changedField == &m_wellPathNameOrSimWellName)
    {
        setDescription(QString(plotNameFormatString()).arg(m_wellPathNameOrSimWellName));
    }

    if (changedField == &m_wellPathNameOrSimWellName || changedField == &m_branchIndex)
    {
        RimWellLogTrack* const plotTrack = m_wellLogPlot->trackByIndex(0);
        for (RimWellLogCurve* const curve : plotTrack->curvesVector())
        {
            plotTrack->removeCurve(curve);
        }
        m_timeStepsToAddresses.clear();
        updateEditorsFromCurves();
        updateFormationsOnPlot();
    }
    else if (changedField == &m_selectedSources)
    {
        // Update time steps selections based on source selections
        updateSelectedTimeStepsFromSelectedSources();
    }

    if (changedField == &m_selectedSources ||
        changedField == &m_selectedTimeSteps)
    {
        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
        m_selectedSourcesOrTimeStepsFieldsChanged = true;
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
    if (!m_selectedSourcesOrTimeStepsFieldsChanged)
    {
        updateEditorsFromCurves();
    }
    m_selectedSourcesOrTimeStepsFieldsChanged = false;

    uiOrdering.add(&m_userName);
    uiOrdering.add(&m_wellPathNameOrSimWellName);
    
    RimProject* proj = RiaApplication::instance()->project();
    if (proj->simulationWellBranches(associatedSimWellName()).size() > 1)
    {
        uiOrdering.add(&m_branchIndex);
    }

    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword("Sources", "Sources");
    sourcesGroup->add(&m_selectedSources);

    caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroupWithKeyword("Time Steps", "TimeSteps");
    timeStepsGroup->add(&m_selectedTimeSteps);

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
void RimWellRftPlot::calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options)
{
    RimProject * proj = RiaApplication::instance()->project();

    if (proj != nullptr)
    {
        const std::vector<QString> simWellNames = proj->simulationWellNames();
        std::set<QString> simWellsAssociatedWithWellPath;
        std::set<QString> wellNames;

        // Observed wells
        for (RimWellPath* const wellPath : proj->allWellPaths())
        {
            wellNames.insert(wellPath->name());

            if (!wellPath->associatedSimulationWell().isEmpty())
            {
                simWellsAssociatedWithWellPath.insert(wellPath->associatedSimulationWell());
            }
        }

        // Sim wells not associated with well path
        for (const QString& simWellName : simWellNames)
        {
            if (simWellsAssociatedWithWellPath.count(simWellName) == 0)
            {
                wellNames.insert(simWellName);
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
void RimWellRftPlot::calculateValueOptionsForTimeSteps(QList<caf::PdmOptionItemInfo>& options)
{
    const QString simWellName = associatedSimWellName();

    std::map<QDateTime, std::set<RifWellRftAddress>> displayTimeStepsMap, obsAndRftTimeStepsMap, gridTimeStepsMap;
    const std::vector<RimEclipseResultCase*> rftCases = RimWellPlotTools::rftCasesForWell(simWellName);
    const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell(simWellName);

    for (const RifWellRftAddress& selection : selectedSources())
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
void RimWellRftPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    updateFormationsOnPlot();
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

