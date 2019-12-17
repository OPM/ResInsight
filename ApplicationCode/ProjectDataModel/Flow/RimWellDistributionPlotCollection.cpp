/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellDistributionPlotCollection.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimFlowDiagSolution.h"
#include "RimWellDistributionPlot.h"

#include "RigEclipseCaseData.h"
#include "RigTofWellDistributionCalculator.h"

#include "RiaColorTools.h"

#include "RiuQwtPlotTools.h"

#include "qwt_plot.h"
#include "qwt_legend.h"
#include "qwt_plot_curve.h"

#include <QWidget>
#include <QGridLayout>
#include <QTextBrowser>

//#include "cvfBase.h"
//#include "cvfTrace.h"
//#include "cvfDebugTimer.h"


//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellDistributionPlotCollection, "WellDistributionPlotCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlotCollection::RimWellDistributionPlotCollection()
:   RimMultiPlotWindow(true)
{
    //cvf::Trace::show("RimWellDistributionPlotCollection::RimWellDistributionPlotCollection()");

    CAF_PDM_InitObject("Well Distribution Plots", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "Case", "Case", "", "", "");
    CAF_PDM_InitField(&m_timeStepIndex, "TimeStepIndex", -1, "Time Step", "", "", "");
    CAF_PDM_InitField(&m_wellName, "WellName", QString("None"), "Well", "", "", "");
    CAF_PDM_InitField(&m_groupSmallContributions, "GroupSmallContributions", true, "Group Small Contributions", "", "", "");
    CAF_PDM_InitField(&m_smallContributionsRelativeThreshold, "SmallContributionsRelativeThreshold", 0.005, "Relative Threshold [0, 1]", "", "", "");

    m_plotWindowTitle = "Well Distribution Plots";
    m_columnCountEnum = RimMultiPlotWindow::COLUMNS_UNLIMITED;

    m_showPlotLegends = false;
    m_showWindow = false;

    setAcceptDrops(false);
    setAsPlotMdiWindow();

    addPlot(new RimWellDistributionPlot(RiaDefines::OIL_PHASE));
    addPlot(new RimWellDistributionPlot(RiaDefines::GAS_PHASE));
    addPlot(new RimWellDistributionPlot(RiaDefines::WATER_PHASE));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlotCollection::~RimWellDistributionPlotCollection() 
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::onLoadDataAndUpdate()
{
    //cvf::Trace::show("RimWellDistributionPlotCollection::onLoadDataAndUpdate()");

    RimMultiPlotWindow::onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_case);
    uiOrdering.add(&m_timeStepIndex);
    uiOrdering.add(&m_wellName);
    uiOrdering.add(&m_groupSmallContributions);
    uiOrdering.add(&m_smallContributionsRelativeThreshold);

    m_smallContributionsRelativeThreshold.uiCapability()->setUiReadOnly(m_groupSmallContributions == false);

    //RimMultiPlotWindow::defineUiOrdering(uiConfigName, uiOrdering);
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellDistributionPlotCollection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options = RimMultiPlotWindow::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (fieldNeedingOptions == &m_case)
    {
        RimProject* ownerProj = nullptr;
        firstAncestorOrThisOfType(ownerProj);
        if (ownerProj)
        {
            std::vector<RimEclipseResultCase*> caseArr;
            ownerProj->descendantsIncludingThisOfType(caseArr);
            for (RimEclipseResultCase* c : caseArr)
            {
                options.push_back(caf::PdmOptionItemInfo(c->caseUserDescription(), c, true, c->uiIconProvider()));
            }
        }
    }

    else if (fieldNeedingOptions == &m_timeStepIndex)
    {
        if (m_case && m_case->eclipseCaseData())
        {
            const QStringList timeStepNames = m_case->timeStepStrings();
            for (int i = 0; i < timeStepNames.size(); i++)
            {
                options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
            }
        }

        if (options.size() == 0)
        {
            options.push_back(caf::PdmOptionItemInfo("None", -1));
        }
    }

    else if (fieldNeedingOptions == &m_wellName)
    {
        if (m_case && m_case->eclipseCaseData())
        {
            caf::QIconProvider simWellIcon(":/Well.png");
            const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
            for (const QString& name : sortedWellNameSet)
            {
                options.push_back(caf::PdmOptionItemInfo(name, name, true, simWellIcon));
            }
        }

        if (options.size() == 0)
        {
            options.push_back(caf::PdmOptionItemInfo("None", QVariant()));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_case)
    {
        fixupDependentFieldsAfterCaseChange();
    }

    bool shouldRecalculatePlotData = false;
    if (changedField == &m_case ||
        changedField == &m_timeStepIndex ||
        changedField == &m_wellName ||
        changedField == &m_groupSmallContributions ||
        changedField == &m_smallContributionsRelativeThreshold)
    {
        applyPlotParametersToContainedPlots();
        shouldRecalculatePlotData = true;
    }

    RimMultiPlotWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (shouldRecalculatePlotData)
    {
        loadDataAndUpdate();
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::applyPlotParametersToContainedPlots()
{
    const size_t numPlots = plotCount();
    for (size_t i = 0; i < numPlots; i++)
    {
        // Dirty usage of dyn_cast, but type is lost when adding the plots to our base class
        RimWellDistributionPlot* aPlot = dynamic_cast<RimWellDistributionPlot*>(plotByIndex(i));
        if (aPlot)
        {
            aPlot->setDataSourceParameters(m_case, m_timeStepIndex, m_wellName);
            aPlot->setPlotOptions(m_groupSmallContributions, m_smallContributionsRelativeThreshold);
        }
    }

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::fixupDependentFieldsAfterCaseChange()
{
    int newTimeStepIndex = -1;
    QString newWellName;

    if (m_case)
    {
        const int timeStepCount = m_case->timeStepStrings().size();
        if (timeStepCount > 0)
        {
            newTimeStepIndex = timeStepCount - 1;
        }

        const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
        if (sortedWellNameSet.size() > 0)
        {
            newWellName = *sortedWellNameSet.begin();
        }
    }

    m_timeStepIndex = newTimeStepIndex;
    m_wellName = newWellName;
}


