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

#include "RicSelectSummaryPlotUI.h"

#include "RiaApplication.h"
#include "RiaSummaryTools.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"


CAF_PDM_SOURCE_INIT(RicSelectSummaryPlotUI, "RicSelectSummaryPlotUI");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSelectSummaryPlotUI::RicSelectSummaryPlotUI()
{
    CAF_PDM_InitObject("RicSelectSummaryPlotUI", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryPlot,  "SelectedSummaryPlot",                    "Select Plot", "", "", "");
    CAF_PDM_InitField(&m_createNewPlot,                 "CreateNewPlot", false,                   "Create New Plot", "", "", "");
    CAF_PDM_InitField(&m_newSummaryPlotName,            "NewViewName",   QString("Cell Results"), "New Plot Name", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectSummaryPlotUI::setDefaultSummaryPlot(RimSummaryPlot* summaryPlot)
{
    m_selectedSummaryPlot = summaryPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectSummaryPlotUI::setSuggestedPlotName(const QString& name)
{
    m_newSummaryPlotName = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSelectSummaryPlotUI::selectedSummaryPlot() const
{
    return m_selectedSummaryPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSelectSummaryPlotUI::isCreateNewPlotChecked() const
{
    return m_createNewPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSelectSummaryPlotUI::newPlotName() const
{
    return m_newSummaryPlotName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSelectSummaryPlotUI::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_selectedSummaryPlot)
    {
        RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

        summaryPlotColl->summaryPlotItemInfos(&options);
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectSummaryPlotUI::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if (RiaSummaryTools::summaryPlotCollection()->summaryPlots().size() == 0)
    {
        m_createNewPlot = true;
    }

    if (m_createNewPlot)
    {
        m_newSummaryPlotName.uiCapability()->setUiReadOnly(false);
        m_selectedSummaryPlot.uiCapability()->setUiReadOnly(true);
    }
    else
    {
        m_newSummaryPlotName.uiCapability()->setUiReadOnly(true);
        m_selectedSummaryPlot.uiCapability()->setUiReadOnly(false);
    }
}

