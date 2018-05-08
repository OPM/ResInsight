/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RimSummaryCaseCollection.h"

#include "RimEnsembleCurveSet.h"
#include "RimGridSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"

CAF_PDM_SOURCE_INIT(RimSummaryCaseCollection, "SummaryCaseSubCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::RimSummaryCaseCollection()
{
    CAF_PDM_InitObject("Summary Case Group", ":/SummaryGroup16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_cases, "SummaryCases", "", "", "", "");
    m_cases.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_name, "SummaryCollectionName", QString("Case Group"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_nameAndItemCount, "NameCount", "Name", "", "", "");
    m_nameAndItemCount.registerGetMethod(this, &RimSummaryCaseCollection::nameAndItemCount);
    m_nameAndItemCount.uiCapability()->setUiReadOnly(true);
    m_nameAndItemCount.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_isEnsemble, "IsEnsemble", false, "Is Ensemble", "", "", "");
    m_isEnsemble.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::~RimSummaryCaseCollection()
{
    m_cases.deleteAllChildObjects();
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::removeCase(RimSummaryCase* summaryCase)
{
    m_cases.removeChildObject(summaryCase);
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::addCase(RimSummaryCase* summaryCase)
{
    m_cases.push_back(summaryCase);
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseCollection::allSummaryCases()
{
    return m_cases.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setName(const QString& name)
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseCollection::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCaseCollection::isEnsemble() const
{
    return m_isEnsemble();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setAsEnsemble(bool isEnsemble)
{
    m_isEnsemble = isEnsemble;
    updateIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCaseCollection::userDescriptionField()
{
    return &m_nameAndItemCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::updateReferringCurveSets() const
{
    // Update curve set referring to this group
    std::vector<PdmObjectHandle*> referringObjects;
    objectsWithReferringPtrFields(referringObjects);

    for (PdmObjectHandle* obj : referringObjects)
    {
        RimEnsembleCurveSet* curveSet = dynamic_cast<RimEnsembleCurveSet*>(obj);
        if (curveSet) curveSet->updateAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseCollection::nameAndItemCount() const
{
    size_t itemCount = m_cases.size();
    if (itemCount > 20)
    {
        return QString("%1 (%2)").arg(m_name()).arg(itemCount);
    }

    return m_name();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::updateIcon()
{
    if (m_isEnsemble) setUiIcon(QIcon(":/SummaryEnsemble16x16.png"));
    else              setUiIcon(QIcon(":/SummaryGroup16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::initAfterRead()
{
    updateIcon();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_isEnsemble)
    {
        updateIcon();
    }
}
