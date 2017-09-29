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
    //this->setAsPlotMdiWindow();
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
        calculateValueOptionsForObservedData(options);

    }
    else if (fieldNeedingOptions == &m_selectedTimeSteps)
    {
        options.push_back(caf::PdmOptionItemInfo("TimeStep 1", "TimeStep 1"));
        options.push_back(caf::PdmOptionItemInfo("TimeStep 2", "TimeStep 2"));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellRftPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    //RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    //if (changedField == &m_userName ||
    //    changedField == &m_showPlotTitle)
    //{
    //    updateWidgetTitleWindowTitle();
    //}
    //else if ( changedField == &m_case)
    //{
    //    if ( m_flowDiagSolution && m_case )
    //    {
    //        m_flowDiagSolution = m_case->defaultFlowDiagSolution();
    //    }
    //    else
    //    {
    //        m_flowDiagSolution = nullptr;
    //    }

    //    if (!m_case) m_timeStep = 0;
    //    else if (m_timeStep >= static_cast<int>(m_case->timeStepDates().size()))
    //    {
    //        m_timeStep = std::max(0, ((int)m_case->timeStepDates().size()) - 1);
    //    }

    //    std::set<QString> sortedWellNames = findSortedWellNames();
    //    if (!sortedWellNames.size()) m_wellName = "";
    //    else if ( sortedWellNames.count(m_wellName()) == 0 ){ m_wellName = *sortedWellNames.begin();}

    //    loadDataAndUpdate();
    //}
    //else if (   changedField == &m_wellName
    //         || changedField == &m_timeStep
    //         || changedField == &m_flowDiagSolution
    //         || changedField == &m_groupSmallContributions
    //         || changedField == &m_smallContributionsThreshold
    //         || changedField == &m_flowType )
    //{
    //    loadDataAndUpdate();
    //}
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
void RimWellRftPlot::calculateValueOptionsForObservedData(QList<caf::PdmOptionItemInfo>& options)
{
    auto project = RiaApplication::instance()->project();

    for (RimOilField* oilField : project->oilFields)
    {
        auto wellPathColl = oilField->wellPathCollection();
        for (const auto& wellPath : wellPathColl->wellPaths)
        {
            const auto& wellLogFile = wellPath->wellLogFile();
            const auto& channels = wellLogFile->wellLogChannelNames();

            for (const auto& channel : *channels)
            {
                auto name = channel->name();

                if (QString::compare(name, "GR") == 0)  // Test code
                {
                    options.push_back(caf::PdmOptionItemInfo(name, name));
                }
            }
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

