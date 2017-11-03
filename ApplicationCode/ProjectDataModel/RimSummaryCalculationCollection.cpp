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

#include "RimSummaryCalculationCollection.h"

#include "RimCalculatedSummaryCase.h"
#include "RimSummaryCalculation.h"

#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeSelectionEditor.h"


CAF_PDM_SOURCE_INIT(RimSummaryCalculationCollection, "RimSummaryCalculationCollection");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationCollection::RimSummaryCalculationCollection()
{
    CAF_PDM_InitObject("Calculation Collection", ":/chain.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_calcuations, "Calculations", "Calculations", "", "", "");
    m_calcuations.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_calcuationSummaryCase, "CalculationsSummaryCase", "Calculations Summary Case", "", "", "");
    m_calcuationSummaryCase.xmlCapability()->disableIO();
    m_calcuationSummaryCase = new RimCalculatedSummaryCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* RimSummaryCalculationCollection::addCalculation()
{
    RimSummaryCalculation* calculation = new RimSummaryCalculation;

    QString varName = QString("Calculation_%1").arg(m_calcuations.size() + 1);
    calculation->setDescription(varName);
    calculation->setDefaultExpression(varName + " := a + b");

    m_calcuations.push_back(calculation);

    rebuildCaseMetaData();

    return calculation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::deleteCalculation(RimSummaryCalculation* calculation)
{
    m_calcuations.removeChildObject(calculation);

    rebuildCaseMetaData();

    delete calculation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCalculation*> RimSummaryCalculationCollection::calculations() const
{
    std::vector<RimSummaryCalculation*> calcs;

    for (auto c : m_calcuations)
    {
        calcs.push_back(c);
    }

    return calcs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCalculationCollection::calculationSummaryCase()
{
    return m_calcuationSummaryCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::deleteAllContainedObjects()
{
    m_calcuations.deleteAllChildObjects();

    rebuildCaseMetaData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::rebuildCaseMetaData()
{
    m_calcuationSummaryCase->buildMetaData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::initAfterRead()
{
    rebuildCaseMetaData();
}

