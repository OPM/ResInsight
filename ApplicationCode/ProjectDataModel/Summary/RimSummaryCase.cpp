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

#include "RiaSummaryTools.h"
#include "RifSummaryReaderInterface.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCaseCollection.h"

#include "cvfAssert.h"

#include <QFileInfo>

CAF_PDM_ABSTRACT_SOURCE_INIT(RimSummaryCase,"SummaryCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase::RimSummaryCase()
{
    CAF_PDM_InitObject("Summary Case",":/SummaryCase16x16.png","","");

    CAF_PDM_InitField(&m_shortName, "ShortName", QString("Display Name"), "Display Name", "", "", "");
    CAF_PDM_InitField(&m_useAutoShortName, "AutoShortyName", false, "Use Auto Display Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryHeaderFilename, "SummaryHeaderFilename", "Summary Header File", "", "", "");
    m_summaryHeaderFilename.uiCapability()->setUiReadOnly(true);

    m_isObservedData = false;
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
QString RimSummaryCase::summaryHeaderFilename() const
{
    return m_summaryHeaderFilename();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setSummaryHeaderFileName(const QString& fileName)
{
    m_summaryHeaderFilename = fileName;

    this->updateAutoShortName();
    this->updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::isObservedData()
{
    return m_isObservedData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setCaseRealizationParameters(const std::shared_ptr<RigCaseRealizationParameters>& crlParameters)
{
    m_crlParameters = crlParameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigCaseRealizationParameters> RimSummaryCase::caseRealizationParameters() const
{
    return m_crlParameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::hasCaseRealizationParameters() const
{
    return m_crlParameters != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimSummaryCase::ensemble() const
{
    RimSummaryCaseCollection* e;
    firstAncestorOrThisOfType(e);
    return e && e->isEnsemble() ? e : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::copyFrom(const RimSummaryCase& rhs)
{
    m_shortName = rhs.m_shortName;
    m_useAutoShortName = rhs.m_useAutoShortName;
    m_summaryHeaderFilename = rhs.m_summaryHeaderFilename;
    m_isObservedData = rhs.m_isObservedData;

    this->updateTreeItemName();
    this->updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// Sorting operator for sets and maps. Sorts by summary case short name.
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::operator<(const RimSummaryCase& rhs) const
{
    return this->caseName() < rhs.caseName();
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

    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();
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
RifReaderRftInterface* RimSummaryCase::rftReader()
{
    return nullptr;
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
RiaEclipseUnitTools::UnitSystemType RimSummaryCase::unitsSystem()
{
    RifSummaryReaderInterface* reader = summaryReader();
    if (reader)
    {
        return reader->unitSystem();
    }
    return RiaEclipseUnitTools::UNITS_UNKNOWN;
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
        RimOilField* oilField = nullptr;
        this->firstAncestorOrThisOfType(oilField);
        CVF_ASSERT(oilField);

        m_shortName = oilField->uniqueShortNameForCase(this);
    }
    else if (m_shortName() == QString("Display Name"))
    {
        m_shortName =  caseName();
    }
    
    updateTreeItemName();
}
