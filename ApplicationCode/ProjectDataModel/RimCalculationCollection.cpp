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

#include "RimCalculationCollection.h"

#include "RimCalculation.h"
#include "RimCalculatedSummaryCase.h"

#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeSelectionEditor.h"


CAF_PDM_SOURCE_INIT(RimCalculationCollection, "RimCalculationCollection");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculationCollection::RimCalculationCollection()
{
    CAF_PDM_InitObject("Calculation Collection", ":/chain.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_calcuations, "Calculations", "Calculations", "", "", "");
    m_calcuations.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_calcuationSummaryCase, "CalculationsSummaryCase", "Calculations Summary Case", "", "", "");
    m_calcuationSummaryCase.xmlCapability()->disableIO();
    
    ensureSummaryCaseIsCreated();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculation* RimCalculationCollection::addCalculation()
{
    RimCalculation* calculation = new RimCalculation;
    calculation->setDescription(QString("Calculation %1").arg(m_calcuations.size()));

    m_calcuations.push_back(calculation);

    ensureSummaryCaseIsCreated();
    m_calcuationSummaryCase->buildMetaData();

    return calculation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculationCollection::deleteCalculation(RimCalculation* calculation)
{
    m_calcuations.removeChildObject(calculation);

    ensureSummaryCaseIsCreated();
    m_calcuationSummaryCase->buildMetaData();

    delete calculation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimCalculation*> RimCalculationCollection::calculations() const
{
    std::vector<RimCalculation*> calcs;

    for (auto c : m_calcuations)
    {
        calcs.push_back(c);
    }

    return calcs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimCalculationCollection::calculationSummaryCase()
{
    ensureSummaryCaseIsCreated();

    return m_calcuationSummaryCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculationCollection::deleteAllContainedObjects()
{
    RimCalculatedSummaryCase* sumCase = m_calcuationSummaryCase();
    if (sumCase)
    {
        m_calcuationSummaryCase.removeChildObject(sumCase);

        delete sumCase;
    }

    m_calcuations.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculationCollection::ensureSummaryCaseIsCreated()
{
    if (!m_calcuationSummaryCase())
    {
        m_calcuationSummaryCase = new RimCalculatedSummaryCase;
    }
}

