/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimSummaryCase.h"

#include "RigSummaryCaseData.h"
#include "RimEclipseCase.h"
#include "RimSummaryCaseCollection.h"

#include <QFileInfo>
#include "RimProject.h"
#include "RimSummaryPlotCollection.h"
#include "RimMainPlotCollection.h"

CAF_PDM_ABSTRACT_SOURCE_INIT(RimSummaryCase,"SummaryCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase::RimSummaryCase()
{
    CAF_PDM_InitObject("Summary Case",":/SummaryCase48x48.png","","");

    CAF_PDM_InitField(&m_shortName, "ShortName", QString("Display Name"), "Display Name", "", "", "");
    CAF_PDM_InitField(&m_useAutoShortName, "AutoShortyName", false, "Use Auto Display Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryHeaderFilename, "SummaryHeaderFilename", "Summary Header File", "", "", "");
    m_summaryHeaderFilename.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase::~RimSummaryCase()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::loadCase()
{
    if (m_summaryCaseData.isNull()) m_summaryCaseData = new RigSummaryCaseData(this->summaryHeaderFilename());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSummaryCaseData* RimSummaryCase::caseData()
{
    return m_summaryCaseData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_useAutoShortName)
    {
        this->updateAutoShortName();
    }
    else if (changedField == &m_shortName)
    {
        updateTreeItemName();
    }

    RimProject* proj = NULL;
    this->firstAncestorOrThisOfType(proj);
    
    RimMainPlotCollection* mainPlotColl = proj->mainPlotCollection();
    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();

    summaryPlotColl->updateSummaryNameHasChanged();

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateOptionSensitivity()
{
    m_shortName.uiCapability()->setUiReadOnly(m_useAutoShortName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateTreeItemName()
{
    if (caseName() != shortName())
        this->setUiName(caseName() + " (" + shortName() +")");
    else
        this->setUiName(caseName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::shortName() const
{
    return m_shortName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::initAfterRead()
{
    updateOptionSensitivity();

    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateAutoShortName()
{
    if(m_useAutoShortName)
    {
        RimSummaryCaseCollection* summaryCaseCollection = NULL;
        this->firstAncestorOrThisOfType(summaryCaseCollection);
        CVF_ASSERT(summaryCaseCollection);

        m_shortName =  summaryCaseCollection->uniqueShortNameForCase(this);
        updateTreeItemName();
    }
    else if (m_shortName() == QString("Display Name"))
    {
        m_shortName =  caseName();
        updateTreeItemName();
    }
}
