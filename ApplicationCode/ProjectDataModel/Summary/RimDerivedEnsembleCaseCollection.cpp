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

#include "RiaApplication.h"
#include "RiaTimeHistoryCurveMerger.h"

#include "SummaryPlotCommands/RicNewDerivedEnsembleFeature.h"

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimDerivedEnsembleCase.h"
#include "RimProject.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"

#include "RifSummaryReaderInterface.h"

#include <cafPdmUiPushButtonEditor.h>

#include <cmath>

namespace caf
{
    template<>
    void caf::AppEnum<DerivedEnsembleOperator>::setUp()
    {
        addItem(DERIVED_ENSEMBLE_SUB, "Sub", "-");
        addItem(DERIVED_ENSEMBLE_ADD, "Add", "+");
        setDefault(DERIVED_ENSEMBLE_SUB);
    }
}

CAF_PDM_SOURCE_INIT(RimDerivedEnsembleCaseCollection, "RimDerivedEnsembleCaseCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDerivedEnsembleCaseCollection::RimDerivedEnsembleCaseCollection()
{
    CAF_PDM_InitObject("Derived Ensemble", ":/SummaryEnsemble16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_ensemble1, "Ensemble1", "Ensemble 1", "", "", "");
    m_ensemble1.uiCapability()->setUiTreeChildrenHidden(true);
    m_ensemble1.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_ensemble2, "Ensemble2", "Ensemble 2", "", "", "");
    m_ensemble1.uiCapability()->setUiTreeChildrenHidden(true);
    m_ensemble2.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_operator, "Operator", "Operator", "", "", "");

    CAF_PDM_InitField(&m_swapEnsemblesButton, "SwapEnsembles", false, "SwapEnsembles", "", "", "");
    m_swapEnsemblesButton.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_swapEnsemblesButton.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_swapEnsemblesButton.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_caseCount, "CaseCount", QString(""), "Matching Cases", "", "", "");
    m_caseCount.uiCapability()->setUiReadOnly(true);

    // Do not show child cases
    uiCapability()->setUiTreeChildrenHidden(true);

    // Do not store child cases to project file
    m_cases.xmlCapability()->disableIO();

    setNameAsReadOnly();
    setName("Derived Ensemble");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDerivedEnsembleCaseCollection::~RimDerivedEnsembleCaseCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::setEnsemble1(RimSummaryCaseCollection* ensemble)
{
    m_ensemble1 = ensemble;
    updateAutoName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::setEnsemble2(RimSummaryCaseCollection* ensemble)
{
    m_ensemble2 = ensemble;
    updateAutoName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimDerivedEnsembleCaseCollection::allSummaryCases() const
{
    std::vector<RimSummaryCase*> cases;
    for (auto sumCase : allDerivedCases(true)) cases.push_back(sumCase);
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimDerivedEnsembleCaseCollection::calculateUnionOfSummaryAddresses() const
{
    std::set<RifEclipseSummaryAddress> addresses;
    if (!m_ensemble1 || !m_ensemble2) return addresses;

    addresses = m_ensemble1->calculateUnionOfSummaryAddresses();
    auto addrs2 = m_ensemble2->calculateUnionOfSummaryAddresses();
    addresses.insert(addrs2.begin(), addrs2.end());
    return addresses;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::updateDerivedEnsembleCases()
{
    if (!m_ensemble1 || !m_ensemble2) return;

    setAllCasesNotInUse();

    const auto cases1 = m_ensemble1->allSummaryCases();
    const auto cases2 = m_ensemble2->allSummaryCases();

    for (auto& sumCase1 : cases1)
    {
        auto crp = sumCase1->caseRealizationParameters();
        if (!crp) continue;

        const auto& sumCase2 = findCaseByParametersHash(cases2, crp->parametersHash());
        if (!sumCase2) continue;

        auto derivedCase = firstCaseNotInUse();
        derivedCase->createSummaryReaderInterface();
        derivedCase->setSummaryCases(sumCase1, sumCase2);
        derivedCase->setCaseRealizationParameters(crp);
        derivedCase->setInUse(true);
    }

    // If other derived ensembles are referring to this ensemble, update their cases as well
    for (auto referring : findReferringEnsembles())
    {
        referring->updateDerivedEnsembleCases();
    }

    deleteCasesNoInUse();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimDerivedEnsembleCaseCollection::hasCaseReference(const RimSummaryCase* sumCase) const
{
    for (auto currCase : m_ensemble1->allSummaryCases())
    {
        if (currCase == sumCase) return true;
    }
    for (auto currCase : m_ensemble2->allSummaryCases())
    {
        if (currCase == sumCase) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::onLoadDataAndUpdate()
{
    updateDerivedEnsembleCases();
    updateReferringCurveSets();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimDerivedEnsembleCaseCollection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_ensemble1 || fieldNeedingOptions == &m_ensemble2)
    {
        for (auto ensemble : allEnsembles())
        {
            if(ensemble != this) options.push_back(caf::PdmOptionItemInfo(ensemble->name(), ensemble));
        }
    }
    else if (fieldNeedingOptions == &m_caseCount)
    {
        size_t caseCount1 = m_ensemble1 ? m_ensemble1->allSummaryCases().size() : 0;
        size_t caseCount2 = m_ensemble2 ? m_ensemble2->allSummaryCases().size() : 0;

        m_caseCount = QString("%1 / %2").arg((int)m_cases.size()).arg(std::max(caseCount1, caseCount2));
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimSummaryCaseCollection::defineUiOrdering(uiConfigName, uiOrdering);

    uiOrdering.add(&m_caseCount);
    uiOrdering.add(&m_ensemble1);
    uiOrdering.add(&m_operator);
    uiOrdering.add(&m_ensemble2);
    uiOrdering.add(&m_swapEnsemblesButton);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    bool doUpdate = false;
    bool doUpdateCases = false;
    bool doClearAllData = false;
    bool doShowDialog = false;

    if (changedField == &m_ensemble1 || changedField == &m_ensemble2)
    {
        doUpdate = true;
        doUpdateCases = true;
        doShowDialog = true;
    }
    else if (changedField == &m_operator)
    {
        doUpdate = true;
        doUpdateCases = true;
        doShowDialog = false;
    }
    else if (changedField == &m_swapEnsemblesButton)
    {
        m_swapEnsemblesButton = false;
        auto temp = m_ensemble1();
        m_ensemble1 = m_ensemble2();
        m_ensemble2 = temp;

        doUpdate = true;
        doUpdateCases = true;
        doShowDialog = false;
    }

    if (doUpdate)
    {
        updateAutoName();

        if (doUpdateCases)
        {
            updateDerivedEnsembleCases();
            updateConnectedEditors();

            if (doShowDialog && m_ensemble1 != nullptr && m_ensemble2 != nullptr && allSummaryCases().empty())
            {
                RicNewDerivedEnsembleFeature::showWarningDialog();
            }
        }

        updateReferringCurveSets();

        // If other derived ensembles are referring to this ensemble, update their cases as well
        for (auto refering : findReferringEnsembles())
        {
            refering->updateReferringCurveSets();
        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_swapEnsemblesButton)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Swap Ensembles";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::setAllCasesNotInUse()
{
    for (auto derCase : allDerivedCases(true)) derCase->setInUse(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::deleteCasesNoInUse()
{
    std::vector<RimDerivedEnsembleCase*> inactiveCases;
    auto allCases = allDerivedCases(false);
    std::copy_if(allCases.begin(), allCases.end(), std::back_inserter(inactiveCases), [](RimDerivedEnsembleCase* derCase) { return !derCase->isInUse(); });

    for (auto derCase : inactiveCases)
    {
        removeCase(derCase);
        delete derCase;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDerivedEnsembleCase* RimDerivedEnsembleCaseCollection::firstCaseNotInUse()
{
    auto allCases = allDerivedCases(false);
    auto itr = std::find_if(allCases.begin(), allCases.end(), [](RimDerivedEnsembleCase* derCase) { return !derCase->isInUse(); });
    if (itr != allCases.end())
    {
        return *itr;
    }

    // If no active case was found, add a new case to the collection
    auto newCase = new RimDerivedEnsembleCase();
    m_cases.push_back(newCase);
    return newCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimDerivedEnsembleCase*> RimDerivedEnsembleCaseCollection::allDerivedCases(bool activeOnly) const
{
    std::vector<RimDerivedEnsembleCase*> activeCases;
    for (auto sumCase : RimSummaryCaseCollection::allSummaryCases())
    {
        auto derivedCase = dynamic_cast<RimDerivedEnsembleCase*>(sumCase);
        if (derivedCase && (!activeOnly || derivedCase->isInUse()))
        {
            activeCases.push_back(derivedCase);
        }
    }
    return activeCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDerivedEnsembleCaseCollection::updateAutoName()
{
    QString op = caf::AppEnum<DerivedEnsembleOperator>::uiText(m_operator());

    auto derivedEnsemble1 = dynamic_cast<RimDerivedEnsembleCaseCollection*>(m_ensemble1());
    auto derivedEnsemble2 = dynamic_cast<RimDerivedEnsembleCaseCollection*>(m_ensemble2());
    bool isDerived1 = derivedEnsemble1 != nullptr;
    bool isDerived2 = derivedEnsemble2 != nullptr;

    QString name =
        (isDerived1 ? "(" : "") +
        (m_ensemble1 ? m_ensemble1->name() : "") +
        (isDerived1 ? ")" : "") +
        " " + op + " " +
        (isDerived2 ? "(" : "") +
        (m_ensemble2 ? m_ensemble2->name() : "") +
        (isDerived2 ? ")" : "");
    setName(name);

    // If other derived ensembles are referring to this ensemble, update theirs name as well
    for (auto refering : findReferringEnsembles())
    {
        refering->updateAutoName();
        refering->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimDerivedEnsembleCaseCollection::findCaseByParametersHash(const std::vector<RimSummaryCase*>& cases, size_t hash) const
{
    for (auto sumCase : cases)
    {
        auto ensembleParameters = sumCase->caseRealizationParameters();
        if (ensembleParameters && ensembleParameters->parametersHash() == hash) return sumCase;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimDerivedEnsembleCaseCollection*> RimDerivedEnsembleCaseCollection::findReferringEnsembles() const
{
    std::vector<RimDerivedEnsembleCaseCollection*> referringEnsembles;
    RimSummaryCaseMainCollection* mainColl;

    firstAncestorOrThisOfType(mainColl);
    if (mainColl)
    {
        for (auto group : mainColl->summaryCaseCollections())
        {
            auto derivedEnsemble = dynamic_cast<RimDerivedEnsembleCaseCollection*>(group);
            if (derivedEnsemble)
            {
                if (derivedEnsemble->m_ensemble1() == this || derivedEnsemble->m_ensemble2() == this)
                {
                    referringEnsembles.push_back(derivedEnsemble);
                }
            }
        }
    }
    return referringEnsembles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCaseCollection*> RimDerivedEnsembleCaseCollection::allEnsembles() const
{
    std::vector<RimSummaryCaseCollection*> ensembles;

    auto project = RiaApplication::instance()->project();

    for (auto group : project->summaryGroups())
    {
        if (group == this) continue;

        if (!group->isEnsemble()) continue;

        auto derivedEnsemble = dynamic_cast<const RimDerivedEnsembleCaseCollection*>(group);
        if (derivedEnsemble && !derivedEnsemble->isValid()) continue;

        ensembles.push_back(group);
    }
    return ensembles;
}
