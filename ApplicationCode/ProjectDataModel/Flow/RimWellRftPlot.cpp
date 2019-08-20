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
#include "RiaColorTools.h"
#include "RiaDateStringParser.h"
#include "RiaSimWellBranchTools.h"

#include "RifReaderEclipseRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimObservedFmuRftData.h"
#include "RimOilField.h"
#include "RimProject.h"
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

#include "RiuWellLogTrack.h"
#include "RiuWellRftPlot.h"

#include "cafPdmUiTreeOrdering.h"
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
    CAF_PDM_InitObject("Well Allocation Plot", ":/RFTPlot16x16.png", "", "");

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("RFT Plot"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");
    CAF_PDM_InitField(&m_showStatisticsCurves, "ShowStatisticsCurves", true, "Show Statistics Curves", "", "", "");
    CAF_PDM_InitField(&m_showEnsembleCurves, "ShowEnsembleCurves", true, "Show Ensemble Curves", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogPlot, "WellLog", "Well Log", "", "", "");
    m_wellLogPlot.uiCapability()->setUiHidden(true);
    m_wellLogPlot = new RimWellLogPlot();
    m_wellLogPlot->setDepthType(RimWellLogPlot::TRUE_VERTICAL_DEPTH);

    CAF_PDM_InitFieldNoDefault(&m_wellPathNameOrSimWellName, "WellName", "Well Name", "", "", "");
    CAF_PDM_InitField(&m_branchIndex, "BranchIndex", 0, "Branch Index", "", "", "");
    CAF_PDM_InitField(&m_branchDetection,
                      "BranchDetection",
                      true,
                      "Branch Detection",
                      "",
                      "Compute branches based on how simulation well cells are organized",
                      "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSources, "Sources", "Sources", "", "", "");
    m_selectedSources.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSources.xmlCapability()->disableIO();
    m_selectedSources.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_selectedTimeSteps, "TimeSteps", "Time Steps", "", "", "");
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedTimeSteps.xmlCapability()->disableIO();
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedTimeSteps.uiCapability()->setAutoAddingOptionFromValue(false);

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
    RiaRftPltCurveDefinition newCurveDef = RimWellPlotTools::curveDefFromCurve(newCurve);

    RiuQwtPlotCurve::LineStyleEnum currentLineStyle = RiuQwtPlotCurve::STYLE_SOLID;

    cvf::Color3f currentColor;
    if (newCurveDef.address().sourceType() == RifDataSourceForRftPlt::SUMMARY_RFT)
    {
        RifDataSourceForRftPlt sourceAddress(RifDataSourceForRftPlt::ENSEMBLE_RFT, newCurveDef.address().ensemble());
        currentColor = m_dataSourceColors[sourceAddress];
        if (m_showStatisticsCurves)
        {
            cvf::Color3f backgroundColor =
                RiaColorTools::fromQColorTo3f(m_wellLogPlot->trackByIndex(0)->viewer()->canvasBackground().color());
            currentColor = RiaColorTools::blendCvfColors(backgroundColor, currentColor, 2, 1);
        }
    }
    else
    {
        currentColor = m_dataSourceColors[newCurveDef.address()];
    }

    RiuQwtSymbol::PointSymbolEnum currentSymbol = RiuQwtSymbol::SYMBOL_NONE;
    if (newCurveDef.address().sourceType() != RifDataSourceForRftPlt::ENSEMBLE_RFT)
    {
        currentSymbol = m_timeStepSymbols[newCurveDef.timeStep()];
    }

	bool isObservedData = newCurveDef.address().sourceType() == RifDataSourceForRftPlt::OBSERVED ||
                          newCurveDef.address().sourceType() == RifDataSourceForRftPlt::OBSERVED_FMU_RFT;
    // Observed data
    currentLineStyle = isObservedData ? RiuQwtPlotCurve::STYLE_NONE : RiuQwtPlotCurve::STYLE_SOLID;

    newCurve->setColor(currentColor);
    newCurve->setSymbolEdgeColor(currentColor);
    newCurve->setSymbol(currentSymbol);
    newCurve->setLineStyle(currentLineStyle);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateFormationsOnPlot() const
{
    if (m_wellLogPlot->trackCount() > 0)
    {
        RimProject*  proj     = RiaApplication::instance()->project();
        RimWellPath* wellPath = proj->wellPathByName(m_wellPathNameOrSimWellName);

        RimCase* formationNamesCase = m_wellLogPlot->trackByIndex(0)->formationNamesCase();

        if (!formationNamesCase)
        {
            /// Set default case. Todo : Use the first of the selected cases in the plot
            std::vector<RimCase*> cases;
            proj->allCases(cases);

            if (!cases.empty())
            {
                formationNamesCase = cases[0];
            }
        }

        if (wellPath)
        {
            m_wellLogPlot->trackByIndex(0)->setAndUpdateWellPathFormationNamesData(formationNamesCase, wellPath);
        }
        else
        {
            m_wellLogPlot->trackByIndex(0)->setAndUpdateSimWellFormationNamesAndBranchData(
                formationNamesCase, associatedSimWellName(), m_branchIndex, m_branchDetection);
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
    std::vector<RifDataSourceForRftPlt> sourcesToSelect;
    const QString                       simWellName = associatedSimWellName();

    for (RimEclipseResultCase* const rftCase : RimWellPlotTools::rftCasesForWell(simWellName))
    {
        sourcesToSelect.push_back(RifDataSourceForRftPlt(RifDataSourceForRftPlt::RFT, rftCase));
    }

    for (RimEclipseResultCase* const gridCase : RimWellPlotTools::gridCasesForWell(simWellName))
    {
        sourcesToSelect.push_back(RifDataSourceForRftPlt(RifDataSourceForRftPlt::GRID, gridCase));
    }

    for (RimSummaryCaseCollection* const ensemble : RimWellPlotTools::rftEnsemblesForWell(simWellName))
    {
        sourcesToSelect.push_back(RifDataSourceForRftPlt(RifDataSourceForRftPlt::ENSEMBLE_RFT, ensemble));
    }

    std::vector<RimWellLogFile*> wellLogFiles = RimWellPlotTools::wellLogFilesContainingPressure(m_wellPathNameOrSimWellName);
    if (!wellLogFiles.empty())
    {
        for (RimWellLogFile* const wellLogFile : wellLogFiles)
        {
            sourcesToSelect.push_back(RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED, wellLogFile));
        }
    }

    for (RimObservedFmuRftData* const observedFmuRftData : RimWellPlotTools::observedFmuRftData())
    {
        sourcesToSelect.push_back(RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED_FMU_RFT, observedFmuRftData));
    }

    m_selectedSources = sourcesToSelect;

    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::rftPlotChannelTypes();

        auto relevantTimeSteps =
            RimWellPlotTools::calculateRelevantTimeStepsFromCases(associatedSimWellName(), m_selectedSources, channelTypesToUse);

        std::vector<QDateTime> timeStepVector;
        for (const auto& item : relevantTimeSteps)
            timeStepVector.push_back(item.first);

        m_selectedTimeSteps = timeStepVector;
    }

    syncCurvesFromUiSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::updateEditorsFromCurves()
{
    std::set<RifDataSourceForRftPlt>                      selectedSources;
    std::set<QDateTime>                                   selectedTimeSteps;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> selectedTimeStepsMap;

    for (const RiaRftPltCurveDefinition& curveDef : curveDefsFromCurves())
    {
        if (curveDef.address().sourceType() == RifDataSourceForRftPlt::OBSERVED)
            selectedSources.insert(RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED));
        else
            selectedSources.insert(curveDef.address());

        auto newTimeStepMap = std::map<QDateTime, std::set<RifDataSourceForRftPlt>>{
            {curveDef.timeStep(), std::set<RifDataSourceForRftPlt>{curveDef.address()}}};
        RimWellPlotTools::addTimeStepsToMap(selectedTimeStepsMap, newTimeStepMap);
        selectedTimeSteps.insert(curveDef.timeStep());
    }

    m_selectedSources   = std::vector<RifDataSourceForRftPlt>(selectedSources.begin(), selectedSources.end());
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
    RimWellLogTrack* plotTrack = m_wellLogPlot->trackByIndex(0);

    const std::set<RiaRftPltCurveDefinition>& allCurveDefs    = selectedCurveDefs();
    const std::set<RiaRftPltCurveDefinition>& curveDefsInPlot = curveDefsFromCurves();

    std::set<RimWellLogCurve*>         curvesToDelete;
    std::set<RiaRftPltCurveDefinition> newCurveDefs;

    if (allCurveDefs.size() < curveDefsInPlot.size())
    {
        // Determine which curves to delete from plot
        std::set<RiaRftPltCurveDefinition> deleteCurveDefs;

        std::set_difference(curveDefsInPlot.begin(),
                            curveDefsInPlot.end(),
                            allCurveDefs.begin(),
                            allCurveDefs.end(),
                            std::inserter(deleteCurveDefs, deleteCurveDefs.end()));

        for (RimWellLogCurve* const curve : plotTrack->curvesVector())
        {
            RiaRftPltCurveDefinition curveDef = RimWellPlotTools::curveDefFromCurve(curve);
            if (deleteCurveDefs.count(curveDef) > 0)
            {
                curvesToDelete.insert(curve);
            }
        }
    }
    else
    {
        // Determine which curves are new since last time
        std::set_difference(allCurveDefs.begin(),
                            allCurveDefs.end(),
                            curveDefsInPlot.begin(),
                            curveDefsInPlot.end(),
                            std::inserter(newCurveDefs, newCurveDefs.end()));
    }

    updateCurvesInPlot(allCurveDefs, newCurveDefs, curvesToDelete);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaRftPltCurveDefinition> RimWellRftPlot::selectedCurveDefs() const
{
    std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::rftPlotChannelTypes();

    return RimWellPlotTools::curveDefsFromTimesteps(
        associatedSimWellName(), m_selectedTimeSteps.v(), true, selectedSourcesExpanded(), channelTypesToUse);
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
                                        const std::set<RimWellLogCurve*>&         curvesToDelete)
{
    const QString          simWellName = associatedSimWellName();
    RimWellLogTrack* const plotTrack   = m_wellLogPlot->trackByIndex(0);

    // Delete curves
    plotTrack->deleteAllCurves();

    defineCurveColorsAndSymbols(allCurveDefs);

    // Add new curves
    for (const RiaRftPltCurveDefinition& curveDefToAdd : allCurveDefs)
    {
        if (curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::RFT)
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve(curve);

            auto rftCase = curveDefToAdd.address().eclCase();
            curve->setEclipseResultCase(dynamic_cast<RimEclipseResultCase*>(rftCase));

            RifEclipseRftAddress address(simWellName, curveDefToAdd.timeStep(), RifEclipseRftAddress::PRESSURE);
            curve->setRftAddress(address);
            curve->setZOrder(1);
            curve->setSimWellBranchData(m_branchDetection, m_branchIndex);

            applyCurveAppearance(curve);
        }
        else if (curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::OBSERVED_FMU_RFT)
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve(curve);

			auto observedFmuRftData = curveDefToAdd.address().observedFmuRftData();
            curve->setObservedFmuRftData(observedFmuRftData);
            RifEclipseRftAddress address(simWellName, curveDefToAdd.timeStep(), RifEclipseRftAddress::PRESSURE);
            curve->setRftAddress(address);
            curve->setZOrder(RiuQwtPlotCurve::Z_SINGLE_CURVE_OBSERVED);
            applyCurveAppearance(curve);
        }
        else if (m_showEnsembleCurves && curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::SUMMARY_RFT)
        {
            auto curve = new RimWellLogRftCurve();
            plotTrack->addCurve(curve);
            auto rftCase = curveDefToAdd.address().summaryCase();
            curve->setSummaryCase(rftCase);
            RifEclipseRftAddress address(simWellName, curveDefToAdd.timeStep(), RifEclipseRftAddress::PRESSURE);
            curve->setRftAddress(address);
            curve->setZOrder(1);
            applyCurveAppearance(curve);
        }
        else if (m_showStatisticsCurves && curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::ENSEMBLE_RFT)
        {
            RimSummaryCaseCollection*      ensemble = curveDefToAdd.address().ensemble();
            std::set<RifEclipseRftAddress> rftAddresses =
                ensemble->rftStatisticsReader()->eclipseRftAddresses(simWellName, curveDefToAdd.timeStep());
            for (auto rftAddress : rftAddresses)
            {
                if (rftAddress.wellLogChannel() != RifEclipseRftAddress::TVD)
                {
                    auto curve = new RimWellLogRftCurve();
                    plotTrack->addCurve(curve);
                    curve->setEnsemble(ensemble);
                    curve->setRftAddress(rftAddress);
                    curve->setZOrder(RiuQwtPlotCurve::Z_ENSEMBLE_STAT_CURVE);
                    applyCurveAppearance(curve);
                    auto                        symbol   = statisticsCurveSymbolFromAddress(rftAddress);
                    RiuQwtSymbol::LabelPosition labelPos = statisticsLabelPosFromAddress(rftAddress);
                    curve->setSymbol(symbol);
                    curve->setSymbolLabelPosition(labelPos);
                    curve->setSymbolSize(curve->symbolSize() + 3);
                    curve->setSymbolSkipDistance(150);
                    curve->setLineStyle(RiuQwtPlotCurve::STYLE_SOLID);
                    QString uiText =
                        caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(rftAddress.wellLogChannel());
                    QString label = uiText.replace("Pressure", "");
                    curve->setSymbolLabel(label);
                }
            }
        }

        else if (curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::GRID)
        {
            auto curve = new RimWellLogExtractionCurve();
            plotTrack->addCurve(curve);

            cvf::Color3f curveColor = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f(plotTrack->curveCount());
            curve->setColor(curveColor);
            curve->setFromSimulationWellName(simWellName, m_branchIndex, m_branchDetection);

            // Fetch cases and time steps
            auto gridCase = curveDefToAdd.address().eclCase();
            if (gridCase != nullptr)
            {
                // Case
                curve->setCase(gridCase);
                curve->setEclipseResultVariable("PRESSURE");

                // Time step

                std::vector<QDateTime> timeSteps =
                    gridCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->timeStepDates();
                int currentTimeStepIndex = -1;
                for (size_t tsIdx = 0; tsIdx < timeSteps.size(); ++tsIdx)
                {
                    if (timeSteps[tsIdx] == curveDefToAdd.timeStep())
                    {
                        currentTimeStepIndex = static_cast<int>(tsIdx);
                        break;
                    }
                }

                curve->setCurrentTimeStep(currentTimeStepIndex);
                curve->setZOrder(0);

                applyCurveAppearance(curve);
            }
        }
        else if (curveDefToAdd.address().sourceType() == RifDataSourceForRftPlt::OBSERVED)
        {
            RimWellLogFile* const wellLogFile = curveDefToAdd.address().wellLogFile();
            RimWellPath* const    wellPath    = RimWellPlotTools::wellPathFromWellLogFile(wellLogFile);
            if (wellLogFile != nullptr)
            {
                RimWellLogFileChannel* pressureChannel = RimWellPlotTools::getPressureChannelFromWellFile(wellLogFile);
                auto                   curve           = new RimWellLogFileCurve();

                plotTrack->addCurve(curve);
                curve->setWellPath(wellPath);
                curve->setWellLogFile(wellLogFile);
                curve->setWellLogChannelName(pressureChannel->name());
                curve->setZOrder(2);

                applyCurveAppearance(curve);
            }
        }
    }

    if (m_wellLogPlot->depthType() == RimWellLogPlot::MEASURED_DEPTH)
    {
        assignWellPathToExtractionCurves();
    }

    m_wellLogPlot->loadDataAndUpdate();
    if (plotTrack->curveCount())
    {
        m_wellLogPlot->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifDataSourceForRftPlt> RimWellRftPlot::selectedSourcesExpanded() const
{
    std::vector<RifDataSourceForRftPlt> sources;
    for (const RifDataSourceForRftPlt& addr : m_selectedSources())
    {
        if (addr.sourceType() == RifDataSourceForRftPlt::OBSERVED)
        {
            for (RimWellLogFile* const wellLogFile :
                 RimWellPlotTools::wellLogFilesContainingPressure(m_wellPathNameOrSimWellName))
            {
                sources.push_back(RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED, wellLogFile));
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
const QString& RimWellRftPlot::simWellOrWellPathName() const
{
    return m_wellPathNameOrSimWellName.v();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::setSimWellOrWellPathName(const QString& currWellName)
{
    m_wellPathNameOrSimWellName = currWellName;
    if (m_wellPathNameOrSimWellName().isEmpty())
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
void RimWellRftPlot::deleteCurvesAssosicatedWithObservedData(const RimObservedFmuRftData* observedFmuRftData)
{
	for (auto track : m_wellLogPlot->tracks())
	{
        auto curves = track->curvesVector();
		for (auto curve : curves)
		{
            RimWellLogRftCurve* rftCurve = dynamic_cast<RimWellLogRftCurve*>(curve);
			if (rftCurve && rftCurve->observedFmuRftData() == observedFmuRftData)
			{
                track->takeOutCurve(rftCurve);
                delete rftCurve;
			}
		}		
	}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellRftPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                    bool*                      useOptionsOnly)
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
        if (!rftCases.empty())
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(
                RifDataSourceForRftPlt::sourceTypeUiText(RifDataSourceForRftPlt::RFT), true));

            for (const auto& rftCase : rftCases)
            {
                auto addr = RifDataSourceForRftPlt(RifDataSourceForRftPlt::RFT, rftCase);
                auto item = caf::PdmOptionItemInfo(rftCase->caseUserDescription(), QVariant::fromValue(addr));
                item.setLevel(1);
                options.push_back(item);
            }
        }

        const std::vector<RimSummaryCaseCollection*> rftEnsembles = RimWellPlotTools::rftEnsemblesForWell(simWellName);
        if (!rftEnsembles.empty())
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(
                RifDataSourceForRftPlt::sourceTypeUiText(RifDataSourceForRftPlt::ENSEMBLE_RFT), true));

            for (RimSummaryCaseCollection* rftEnsemble : rftEnsembles)
            {
                auto addr = RifDataSourceForRftPlt(RifDataSourceForRftPlt::ENSEMBLE_RFT, rftEnsemble);
                auto item = caf::PdmOptionItemInfo(rftEnsemble->name(), QVariant::fromValue(addr));
                item.setLevel(1);
                options.push_back(item);
            }
        }

        const std::vector<RimEclipseResultCase*> gridCases = RimWellPlotTools::gridCasesForWell(simWellName);
        if (!gridCases.empty())
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(
                RifDataSourceForRftPlt::sourceTypeUiText(RifDataSourceForRftPlt::GRID), true));

            for (const auto& gridCase : gridCases)
            {
                auto addr = RifDataSourceForRftPlt(RifDataSourceForRftPlt::GRID, gridCase);
                auto item = caf::PdmOptionItemInfo(gridCase->caseUserDescription(), QVariant::fromValue(addr));
                item.setLevel(1);
                options.push_back(item);
            }
        }

        if (!RimWellPlotTools::wellLogFilesContainingPressure(m_wellPathNameOrSimWellName).empty())
        {
            options.push_back(caf::PdmOptionItemInfo::createHeader(
                RifDataSourceForRftPlt::sourceTypeUiText(RifDataSourceForRftPlt::OBSERVED), true));

            auto addr = RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED);
            auto item = caf::PdmOptionItemInfo("Observed Data", QVariant::fromValue(addr));
            item.setLevel(1);
            options.push_back(item);
        }

		const std::vector<RimObservedFmuRftData*> observedFmuRftCases = RimWellPlotTools::observedFmuRftDataForWell(simWellName);            
		if (!observedFmuRftCases.empty())
		{
            options.push_back(caf::PdmOptionItemInfo::createHeader(
                RifDataSourceForRftPlt::sourceTypeUiText(RifDataSourceForRftPlt::OBSERVED_FMU_RFT), true));

			for (const auto& observedFmuRftCase : observedFmuRftCases)
			{
                auto addr = RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED_FMU_RFT, observedFmuRftCase);
                auto item = caf::PdmOptionItemInfo(observedFmuRftCase->name(), QVariant::fromValue(addr));
                item.setLevel(1);
                options.push_back(item);
			}
		}

    }
    else if (fieldNeedingOptions == &m_selectedTimeSteps)
    {
        std::set<RifEclipseRftAddress::RftWellLogChannelType> channelTypesToUse = RifEclipseRftAddress::rftPlotChannelTypes();

        RimWellPlotTools::calculateValueOptionsForTimeSteps(
            associatedSimWellName(), selectedSourcesExpanded(), channelTypesToUse, options);
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        auto simulationWellBranches = RiaSimWellBranchTools::simulationWellBranches(simWellName, m_branchDetection);

        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField(simulationWellBranches);
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

        m_branchIndex = 0;

        RimWellLogTrack* const plotTrack = m_wellLogPlot->trackByIndex(0);
        if (plotTrack)
        {
            plotTrack->deleteAllCurves();
        }

        updateEditorsFromCurves();
        updateFormationsOnPlot();
    }
    else if (changedField == &m_branchIndex || changedField == &m_branchDetection)
    {
        const QString simWellName = associatedSimWellName();
        m_branchIndex             = RiaSimWellBranchTools::clampBranchIndex(simWellName, m_branchIndex, m_branchDetection);

        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
    }
    else if (changedField == &m_selectedSources || changedField == &m_selectedTimeSteps)
    {
        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
        this->updateConnectedEditors();
    }
    else if (changedField == &m_showStatisticsCurves || changedField == &m_showEnsembleCurves)
    {
        updateFormationsOnPlot();
        syncCurvesFromUiSelection();
    }

    else if (changedField == &m_showPlotTitle)
    {
        // m_wellLogPlot->setShowDescription(m_showPlotTitle);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    uiTreeOrdering.skipRemainingChildren(true);
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
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_userName);
    uiOrdering.add(&m_wellPathNameOrSimWellName);
    uiOrdering.add(&m_showStatisticsCurves);
    uiOrdering.add(&m_showEnsembleCurves);

    bool showingEnsembleData = false;
    for (const RifDataSourceForRftPlt& dataSource : m_selectedSources())
    {
        if (dataSource.sourceType() == RifDataSourceForRftPlt::ENSEMBLE_RFT)
        {
            showingEnsembleData = true;
            break;
        }
    }
    m_showStatisticsCurves.uiCapability()->setUiReadOnly(!showingEnsembleData);
    m_showEnsembleCurves.uiCapability()->setUiReadOnly(!showingEnsembleData);

    RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromWellName(
        &uiOrdering, m_wellPathNameOrSimWellName, m_branchDetection, m_branchIndex);

    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword("Sources", "Sources");
    sourcesGroup->add(&m_selectedSources);

    caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroupWithKeyword("Time Steps", "TimeSteps");
    timeStepsGroup->add(&m_selectedTimeSteps);

    if (m_wellLogPlot && m_wellLogPlot->trackCount() > 0)
    {
        RimWellLogTrack* track = m_wellLogPlot->trackByIndex(0);

        track->uiOrderingForRftPltFormations(uiOrdering);

        caf::PdmUiGroup* legendAndAxisGroup = uiOrdering.addNewGroup("Legend and Axis");
        legendAndAxisGroup->setCollapsedByDefault(true);

        m_wellLogPlot->uiOrderingForPlotSettings(*legendAndAxisGroup);

        track->uiOrderingForXAxisSettings(*legendAndAxisGroup);

        m_wellLogPlot->uiOrderingForDepthAxis(*legendAndAxisGroup);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options)
{
    RimProject* proj = RiaApplication::instance()->project();

    options.push_back(caf::PdmOptionItemInfo("None", "None"));

    if (proj != nullptr)
    {
        const std::vector<QString>                                 simWellNames = proj->simulationWellNames();
        std::set<QString>                                          simWellsAssociatedWithWellPath;
        std::set<std::pair<QString /*uitext*/, QString /*value*/>> wellNames;

        // Observed wells
        for (RimWellPath* const wellPath : proj->allWellPaths())
        {
            wellNames.insert(std::make_pair(wellPath->name() + "  (Well Path)", wellPath->name()));

            if (!wellPath->associatedSimulationWellName().isEmpty())
            {
                simWellsAssociatedWithWellPath.insert(wellPath->associatedSimulationWellName());
            }
        }

        // Sim wells not associated with well path
        for (const QString& simWellName : simWellNames)
        {
            if (simWellsAssociatedWithWellPath.count(simWellName) == 0)
            {
                wellNames.insert(std::make_pair(simWellName, simWellName));
            }
        }

        // Ensemble RFT wells
        const std::vector<RimSummaryCaseCollection*> rftEnsembles = RimWellPlotTools::rftEnsembles();
        if (!rftEnsembles.empty())
        {
            for (RimSummaryCaseCollection* summaryCaseColl : rftEnsembles)
            {
                std::set<QString> wellsWithRftData = summaryCaseColl->wellsWithRftData();
                for (QString wellName : wellsWithRftData)
                {
                    wellNames.insert(std::make_pair(wellName, wellName));
                }
            }
        }

		// Observer FMU RFT wells
        const std::vector<RimObservedFmuRftData*> allRftFmuData = RimWellPlotTools::observedFmuRftData();
		if (!allRftFmuData.empty())
		{
			for (RimObservedFmuRftData* rftFmuData : allRftFmuData)
			{
				for (QString wellName : rftFmuData->wells())
				{
                    wellNames.insert(std::make_pair(wellName, wellName));
				}
			}
		}

        for (const auto& wellName : wellNames)
        {
            options.push_back(caf::PdmOptionItemInfo(wellName.first, wellName.second));
        }
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

    if (m_wellLogPlot->depthType() == RimWellLogPlot::MEASURED_DEPTH)
    {
        assignWellPathToExtractionCurves();
    }

    m_wellLogPlot->loadDataAndUpdate();

    updateEditorsFromCurves();
    updateWidgetTitleWindowTitle();

    // applyInitialSelections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::assignWellPathToExtractionCurves()
{
    RimProject*  proj     = RiaApplication::instance()->project();
    RimWellPath* wellPath = proj->wellPathByName(m_wellPathNameOrSimWellName);

    if (wellPath)
    {
        for (RimWellLogCurve* curve : m_wellLogPlot->trackByIndex(0)->curvesVector())
        {
            auto extractionCurve = dynamic_cast<RimWellLogExtractionCurve*>(curve);
            if (extractionCurve)
            {
                extractionCurve->setTrajectoryType(RimWellLogExtractionCurve::WELL_PATH);
                extractionCurve->setWellPath(wellPath);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellRftPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_wellLogPlotWidget = new RiuWellRftPlot(this, mainWindowParent);

    return m_wellLogPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum RimWellRftPlot::statisticsCurveSymbolFromAddress(const RifEclipseRftAddress& address)
{
    switch (address.wellLogChannel())
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
RiuQwtSymbol::LabelPosition RimWellRftPlot::statisticsLabelPosFromAddress(const RifEclipseRftAddress& address)
{
    switch (address.wellLogChannel())
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
void RimWellRftPlot::defineCurveColorsAndSymbols(const std::set<RiaRftPltCurveDefinition>& allCurveDefs)
{
    m_dataSourceColors.clear();
    m_timeStepSymbols.clear();

    std::vector<cvf::Color3f> colorTable;
    RiaColorTables::summaryCurveDefaultPaletteColors().color3fArray().toStdVector(&colorTable);

    std::vector<RiuQwtSymbol::PointSymbolEnum> symbolTable = {RiuQwtSymbol::SYMBOL_ELLIPSE,
                                                              RiuQwtSymbol::SYMBOL_RECT,
                                                              RiuQwtSymbol::SYMBOL_DIAMOND,
                                                              RiuQwtSymbol::SYMBOL_CROSS,
                                                              RiuQwtSymbol::SYMBOL_XCROSS,
                                                              RiuQwtSymbol::SYMBOL_STAR1};

    // Add new curves
    for (const RiaRftPltCurveDefinition& curveDefToAdd : allCurveDefs)
    {
        auto colorTableIndex  = m_dataSourceColors.size();
        auto symbolTableIndex = m_timeStepSymbols.size();

        RifDataSourceForRftPlt address = curveDefToAdd.address();

        if (address.sourceType() != RifDataSourceForRftPlt::SUMMARY_RFT)
        {
            if (!m_dataSourceColors.count(curveDefToAdd.address()))
            {
                colorTableIndex                             = colorTableIndex % colorTable.size();
                m_dataSourceColors[curveDefToAdd.address()] = colorTable[colorTableIndex];
            }
        }

        if (address.sourceType() != RifDataSourceForRftPlt::ENSEMBLE_RFT)
        {
            if (!m_timeStepSymbols.count(curveDefToAdd.timeStep()))
            {
                symbolTableIndex                            = symbolTableIndex % symbolTable.size();
                m_timeStepSymbols[curveDefToAdd.timeStep()] = symbolTable[symbolTableIndex];
            }
        }
    }
}
