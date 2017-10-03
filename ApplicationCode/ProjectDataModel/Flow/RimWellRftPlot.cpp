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

CAF_PDM_SOURCE_INIT(RimWellRftPlot, "WellRftPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

//namespace caf
//{
//
//template<>
//void AppEnum<RimWellRftPlot::FlowType>::setUp()
//{
//    addItem(RimWellRftPlot::ACCUMULATED, "ACCUMULATED", "Accumulated");
//    addItem(RimWellRftPlot::INFLOW, "INFLOW", "Inflow Rates");
//    setDefault(RimWellRftPlot::ACCUMULATED);
//
//}
//}

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

    CAF_PDM_InitFieldNoDefault(&m_selectedSources, "Sources", "Sources", "", "", "");
    m_selectedSources.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSources.xmlCapability()->setIOReadable(false);
    m_selectedSources.xmlCapability()->setIOWritable(false);
    m_selectedSources.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_selectedTimeSteps, "TimeSteps", "TimeSteps", "", "", "");
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedTimeSteps.xmlCapability()->setIOReadable(false);
    m_selectedTimeSteps.xmlCapability()->setIOWritable(false);
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

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
void RimWellRftPlot::updateFromWell()
{
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
void RimWellRftPlot::loadDataAndUpdatePlot()
{
    auto wellLogFiles = getWellLogFilesWithPressure();

    for (const auto& wellLogFile : wellLogFiles)
    {

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellRftPlot::isPressureChannel(RimWellLogFileChannel* channel)
{
    // Todo: read pressure channel names from config/defines
    return QString::compare(channel->name(), "PRESSURE") == 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellRftPlot::getWellLogFilesWithPressure() const
{
    std::vector<RimWellLogFile*> wellLogFiles;
    auto project = RiaApplication::instance()->project();

    for (RimOilField* oilField : project->oilFields)
    {
        auto wellPathColl = oilField->wellPathCollection();
        for (const auto& wellPath : wellPathColl->wellPaths)
        {
            bool hasPressure = false;
            const auto& wellLogFile = wellPath->wellLogFile();
            const auto& wellLogChannels = wellLogFile->wellLogChannelNames();

            for (const auto& wellLogChannel : *wellLogChannels)
            {
                // Todo: add more criterias
                if (isPressureChannel(wellLogChannel))
                {
                    hasPressure = true;
                    break;
                }
            }
            if (hasPressure)
                wellLogFiles.push_back(wellLogFile);
        }
    }
    return wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFileChannel*> RimWellRftPlot::getPressureChannelsFromWellLogFile(const RimWellLogFile* wellLogFile) const
{
    std::vector<RimWellLogFileChannel*> channels;

    for (const auto& wellLogFile : getWellLogFilesWithPressure())
    {
        for (const auto& channel : *wellLogFile->wellLogChannelNames())
        {
            // Todo: add more criterias
            if (isPressureChannel(channel))
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
QList<caf::PdmOptionItemInfo> RimWellRftPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_wellName)
    {
        calculateValueOptionsForWells(options);
    }
    else if (fieldNeedingOptions == &m_selectedSources)
    {
        options.push_back(caf::PdmOptionItemInfo::createHeader(RimWellRftAddress::sourceTypeUiText(RftSourceType::RFT), true));

        options.push_back(caf::PdmOptionItemInfo::createHeader(RimWellRftAddress::sourceTypeUiText(RftSourceType::GRID), true));

        options.push_back(caf::PdmOptionItemInfo("Test", "Test"));

        options.push_back(caf::PdmOptionItemInfo::createHeader(RimWellRftAddress::sourceTypeUiText(RftSourceType::OBSERVED), true));
        calculateValueOptionsForObservedData(options, 1);

    }
    else if (fieldNeedingOptions == &m_selectedTimeSteps)
    {
        for (const auto& selection : m_selectedSources())
        {
            if (selection == RimWellRftAddress(RftSourceType::OBSERVED))
            {
                for (const auto& wellLogFile : getWellLogFilesWithPressure())
                {
                    auto item = caf::PdmOptionItemInfo(wellLogFile->date(), wellLogFile->date());
                    options.push_back(item);
                }
            }
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

    if (changedField == &m_selectedSources)
    {
        loadDataAndUpdatePlot();
    }
    else if (changedField == &m_selectedTimeSteps)
    {
        loadDataAndUpdatePlot();
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

    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword("Sources", "Sources");
    sourcesGroup->add(&m_selectedSources);

    caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroupWithKeyword("Time Steps", "TimeSteps");
    timeStepsGroup->add(&m_selectedTimeSteps);

    uiOrdering.add(&m_showPlotTitle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options)
{
    auto project = RiaApplication::instance()->project();

    for (RimOilField* oilField : project->oilFields)
    {
        auto wellPathColl = oilField->wellPathCollection();
        for (const auto& wellPath : wellPathColl->wellPaths)
        {
            options.push_back(caf::PdmOptionItemInfo(wellPath->name(), wellPath->name()));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::calculateValueOptionsForObservedData(QList<caf::PdmOptionItemInfo>& options, int level)
{
    if (getWellLogFilesWithPressure().size() > 0)
    {
        auto addr = RimWellRftAddress(RftSourceType::OBSERVED);
        auto item = caf::PdmOptionItemInfo(addr.uiText(), QVariant::fromValue(addr));
        if (level > 0)
        {
            item.setLevel(level);
        }
        options.push_back(item);
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
    updateFromWell();
    m_wellLogPlot->loadDataAndUpdate();
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
cvf::Color3f RimWellRftPlot::getTracerColor(const QString& tracerName)
{

    if (tracerName == RIG_FLOW_OIL_NAME)   return cvf::Color3f::DARK_GREEN;
    if (tracerName == RIG_FLOW_GAS_NAME)   return cvf::Color3f::DARK_RED;
    if (tracerName == RIG_FLOW_WATER_NAME) return cvf::Color3f::BLUE;
    return cvf::Color3f::DARK_GRAY;
}
