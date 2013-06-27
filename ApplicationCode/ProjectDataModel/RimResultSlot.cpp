/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "RimResultSlot.h"
#include "RimLegendConfig.h"
#include "RimReservoirView.h"
#include "RimCase.h"
#include "RiuMainWindow.h"
#include "RimUiTreeModelPdm.h"


#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"

#include "RimReservoirCellResultsCacher.h"

CAF_PDM_SOURCE_INIT(RimResultSlot, "ResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultSlot::RimResultSlot()
{
    CAF_PDM_InitObject("Result Slot", "", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_legendConfigData, "ResultVarLegendDefinitionList", "", "", "", "");
    m_legendConfigData.setUiHidden(true);

    legendConfig = new RimLegendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultSlot::~RimResultSlot()
{
    delete legendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultSlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_resultVariable)
    {
        if (oldValue != newValue)
        {
           changeLegendConfig(this->resultVariable());
        }

        if (newValue != RimDefines::undefinedResultName())
        {
            if (m_reservoirView) m_reservoirView->animationMode = true;
        }
    }

    RimResultDefinition::fieldChangedByUi(changedField, oldValue, newValue);

    if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultSlot::changeLegendConfig(QString resultVarNameOfNewLegend)
{
    if (this->legendConfig()->resultVariableName() == resultVarNameOfNewLegend) return;

    std::list<caf::PdmPointer<RimLegendConfig> >::iterator it;
    bool found = false;
    for (it = m_legendConfigData.v().begin(); it != m_legendConfigData.v().end(); ++it)
    {
        if ((*it)->resultVariableName() == resultVarNameOfNewLegend)
        {
            RimLegendConfig* newLegend = *it;
          
            m_legendConfigData.v().erase(it);
            m_legendConfigData.v().push_back(this->legendConfig());
            this->legendConfig = newLegend;
            RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(this);
            found = true;
            break;
        }
    }

    // Not found ?
    if (!found)
    {
         RimLegendConfig* newLegend = new RimLegendConfig;
         newLegend->setReservoirView(m_reservoirView);
         newLegend->resultVariableName = resultVarNameOfNewLegend;
         m_legendConfigData.v().push_back(this->legendConfig());
         this->legendConfig = newLegend;
         RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(this);

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultSlot::initAfterRead()
{
    RimResultDefinition::initAfterRead();

    if (this->legendConfig()->resultVariableName == "")
    {
        this->legendConfig()->resultVariableName = this->resultVariable();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultSlot::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    this->legendConfig()->setReservoirView(ownerReservoirView);
    std::list<caf::PdmPointer<RimLegendConfig> >::iterator it;
    for (it = m_legendConfigData.v().begin(); it != m_legendConfigData.v().end(); ++it)
    {
        (*it)->setReservoirView(ownerReservoirView);
    }
}
