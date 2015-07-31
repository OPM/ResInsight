/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseCellColors.h"

#include "RimEclipseView.h"
#include "RimTernaryLegendConfig.h"
#include "RimUiTreeModelPdm.h"
#include "RiuMainWindow.h"

CAF_PDM_SOURCE_INIT(RimEclipseCellColors, "ResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::RimEclipseCellColors()
{
    CAF_PDM_InitObject("Result Slot", "", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();
    this->legendConfig.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
    this->legendConfig.capability<caf::PdmUiFieldHandle>()->setUiChildrenHidden(true);

    // MODTODO
    CAF_PDM_InitFieldNoDefault(&m_legendConfigData, "ResultVarLegendDefinitionList", "", "", "", "");
    m_legendConfigData.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
    m_legendConfigData.capability<caf::PdmUiFieldHandle>()->setUiChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Legend Definition", "", "", "");
    this->ternaryLegendConfig = new RimTernaryLegendConfig();
    this->ternaryLegendConfig.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
    this->ternaryLegendConfig.capability<caf::PdmUiFieldHandle>()->setUiChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::~RimEclipseCellColors()
{
    delete legendConfig();
    delete ternaryLegendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseResultDefinition::fieldChangedByUi(changedField, oldValue, newValue);

    // Update of legend config must happen after RimResultDefinition::fieldChangedByUi(), as this function modifies this->resultVariable()
    if (changedField == &m_resultVariableUiField)
    {
        if (oldValue != newValue)
        {
            changeLegendConfig(this->resultVariable());
        }

        if (newValue != RimDefines::undefinedResultName())
        {
            if (m_reservoirView) m_reservoirView->hasUserRequestedAnimation = true;
        }

        RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(this);
    }

    if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::changeLegendConfig(QString resultVarNameOfNewLegend)
{
    if (resultVarNameOfNewLegend == RimDefines::ternarySaturationResultName())
    {
        this->ternaryLegendConfig.capability<caf::PdmUiFieldHandle>()->setUiHidden(false);
        this->ternaryLegendConfig.capability<caf::PdmUiFieldHandle>()->setUiChildrenHidden(false);
        this->legendConfig.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
        this->legendConfig.capability<caf::PdmUiFieldHandle>()->setUiChildrenHidden(true);
    }
    else
    {
        this->ternaryLegendConfig.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
        this->ternaryLegendConfig.capability<caf::PdmUiFieldHandle>()->setUiChildrenHidden(true);

        if (this->legendConfig()->resultVariableName() != resultVarNameOfNewLegend)
        {
            bool found = false;
            for (size_t i = 0; i < m_legendConfigData.size(); i++)
            {
                if (m_legendConfigData[i]->resultVariableName() == resultVarNameOfNewLegend)
                {
                    RimLegendConfig* newLegend = m_legendConfigData[i];

                    m_legendConfigData.erase(i);
                    m_legendConfigData.push_back(this->legendConfig());
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
                 m_legendConfigData.push_back(this->legendConfig());
                 this->legendConfig = newLegend;
            }
        }
    
        this->legendConfig.capability<caf::PdmUiFieldHandle>()->setUiHidden(false);
        this->legendConfig.capability<caf::PdmUiFieldHandle>()->setUiChildrenHidden(false);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::initAfterRead()
{
    RimEclipseResultDefinition::initAfterRead();

    if (this->legendConfig()->resultVariableName == "")
    {
        this->legendConfig()->resultVariableName = this->resultVariable();
    }

    changeLegendConfig(this->resultVariable());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setReservoirView(RimEclipseView* ownerReservoirView)
{
    RimEclipseResultDefinition::setReservoirView(ownerReservoirView);

    m_reservoirView = ownerReservoirView;
    this->legendConfig()->setReservoirView(ownerReservoirView);
    std::list<caf::PdmPointer<RimLegendConfig> >::iterator it;

    // MODTODO
    //     for (it = m_legendConfigData.v().begin(); it != m_legendConfigData.v().end(); ++it)
//     {
//         (*it)->setReservoirView(ownerReservoirView);
//     }

    this->ternaryLegendConfig()->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setResultVariable(const QString& val)
{
    RimEclipseResultDefinition::setResultVariable(val);

    this->changeLegendConfig(val);
}

