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

#include "RimResultSlot.h"

#include "RimReservoirView.h"
#include "RimTernaryLegendConfig.h"
#include "RimUiTreeModelPdm.h"
#include "RiuMainWindow.h"

CAF_PDM_SOURCE_INIT(RimResultSlot, "ResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultSlot::RimResultSlot()
{
    CAF_PDM_InitObject("Result Slot", "", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();
    this->legendConfig.setUiHidden(true);
    this->legendConfig.setUiChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_legendConfigData, "ResultVarLegendDefinitionList", "", "", "", "");
    m_legendConfigData.setUiHidden(true);
    m_legendConfigData.setUiChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Legend Definition", "", "", "");
    this->ternaryLegendConfig = new RimTernaryLegendConfig();
    this->ternaryLegendConfig.setUiHidden(true);
    this->ternaryLegendConfig.setUiChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultSlot::~RimResultSlot()
{
    delete legendConfig();
    delete ternaryLegendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultSlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimResultDefinition::fieldChangedByUi(changedField, oldValue, newValue);

    // Update of legend config must happen after RimResultDefinition::fieldChangedByUi(), as this function modifies this->resultVariable()
    if (changedField == &m_resultVariableUiField)
    {
        if (oldValue != newValue)
        {
            changeLegendConfig(this->resultVariable());
        }

        if (newValue != RimDefines::undefinedResultName())
        {
            if (m_reservoirView) m_reservoirView->animationMode = true;
        }

        RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(this);
    }

    if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultSlot::changeLegendConfig(QString resultVarNameOfNewLegend)
{
    if (resultVarNameOfNewLegend == RimDefines::ternarySaturationResultName())
    {
        this->ternaryLegendConfig.setUiHidden(false);
        this->ternaryLegendConfig.setUiChildrenHidden(false);
        this->legendConfig.setUiHidden(true);
        this->legendConfig.setUiChildrenHidden(true);
    }
    else
    {
        this->ternaryLegendConfig.setUiHidden(true);
        this->ternaryLegendConfig.setUiChildrenHidden(true);

        if (this->legendConfig()->resultVariableName() != resultVarNameOfNewLegend)
        {
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
            }
        }
    
        this->legendConfig.setUiHidden(false);
        this->legendConfig.setUiChildrenHidden(false);
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

    changeLegendConfig(this->resultVariable());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultSlot::setReservoirView(RimReservoirView* ownerReservoirView)
{
    RimResultDefinition::setReservoirView(ownerReservoirView);

    m_reservoirView = ownerReservoirView;
    this->legendConfig()->setReservoirView(ownerReservoirView);
    std::list<caf::PdmPointer<RimLegendConfig> >::iterator it;
    for (it = m_legendConfigData.v().begin(); it != m_legendConfigData.v().end(); ++it)
    {
        (*it)->setReservoirView(ownerReservoirView);
    }

    this->ternaryLegendConfig()->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultSlot::setResultVariable(const QString& val)
{
    RimResultDefinition::setResultVariable(val);

    this->changeLegendConfig(val);
}

