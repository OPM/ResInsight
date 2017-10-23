/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimMainPlotCollection.h"

#include "RimFlowPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryPlotCollection.h"
#include "RimRftPlotCollection.h"
#include "RimPltPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "RiuMainWindow.h"
#include "RiuProjectPropertyView.h"
#include "RimFlowCharacteristicsPlot.h"

CAF_PDM_SOURCE_INIT(RimMainPlotCollection, "MainPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::RimMainPlotCollection()
{
    CAF_PDM_InitObject("Plots", "", "", "");

    CAF_PDM_InitField(&show, "Show", true, "Show 2D Plot Window", "", "", "");
    show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogPlotCollection, "WellLogPlotCollection", "",  "", "", "");
    m_wellLogPlotCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_rftPlotCollection, "RftPlotCollection", "", "", "", "");
    m_rftPlotCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_pltPlotCollection, "PltPlotCollection", "", "", "", "");
    m_pltPlotCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_summaryPlotCollection, "SummaryPlotCollection", "Summary Plots", "", "", "");
    m_summaryPlotCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_flowPlotCollection, "FlowPlotCollection", "Flow Diagnostics Plots", "", "", "");
    m_flowPlotCollection.uiCapability()->setUiHidden(true);

    m_wellLogPlotCollection = new RimWellLogPlotCollection();
    m_rftPlotCollection     = new RimRftPlotCollection();
    m_pltPlotCollection     = new RimPltPlotCollection();
    m_summaryPlotCollection = new RimSummaryPlotCollection();
    m_flowPlotCollection    = new RimFlowPlotCollection();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::~RimMainPlotCollection()
{
    if (m_wellLogPlotCollection())  delete m_wellLogPlotCollection();
    if (m_rftPlotCollection())      delete m_rftPlotCollection();
    if (m_pltPlotCollection())      delete m_pltPlotCollection();
    if (m_summaryPlotCollection())  delete m_summaryPlotCollection();
    if (m_flowPlotCollection())     delete m_flowPlotCollection();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMainPlotCollection::objectToggleField()
{
    return &show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RimMainPlotCollection::wellLogPlotCollection()
{
    return m_wellLogPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRftPlotCollection* RimMainPlotCollection::rftPlotCollection()
{
    return m_rftPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPltPlotCollection* RimMainPlotCollection::pltPlotCollection()
{
    return m_pltPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection* RimMainPlotCollection::summaryPlotCollection()
{
    return m_summaryPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection* RimMainPlotCollection::flowPlotCollection()
{
    return m_flowPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::deleteAllContainedObjects()
{
    m_wellLogPlotCollection()->wellLogPlots.deleteAllChildObjects();
    m_rftPlotCollection()->deleteAllPlots();
    m_pltPlotCollection()->deleteAllPlots();
    m_summaryPlotCollection()->summaryPlots.deleteAllChildObjects();

    m_flowPlotCollection()->closeDefaultPlotWindowAndDeletePlots();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::updateCurrentTimeStepInPlots()
{
    m_flowPlotCollection()->defaultFlowCharacteristicsPlot()->updateCurrentTimeStep();
}

