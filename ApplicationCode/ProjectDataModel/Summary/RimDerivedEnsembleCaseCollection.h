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

#pragma once

#include "RifEclipseSummaryAddress.h"

#include "RimDerivedEnsembleCase.h"
#include "RimSummaryCaseCollection.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <vector>

class RimSummaryCase;

//==================================================================================================
///  
//==================================================================================================
class RimDerivedEnsembleCaseCollection : public RimSummaryCaseCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimDerivedEnsembleCaseCollection();
    ~RimDerivedEnsembleCaseCollection() override;

    RimSummaryCaseCollection*   ensemble1() const { return m_ensemble1; }
    RimSummaryCaseCollection*   ensemble2() const { return m_ensemble2; }
    DerivedEnsembleOperator     op() const { return m_operator(); }
    bool                        isValid() const { return m_ensemble1 != nullptr && m_ensemble2 != nullptr; }

    void                        setEnsemble1(RimSummaryCaseCollection* ensemble);
    void                        setEnsemble2(RimSummaryCaseCollection* ensemble);

    std::vector<RimSummaryCase*>    allSummaryCases() const override;
    std::set<RifEclipseSummaryAddress> ensembleSummaryAddresses() const override;
    void                                    updateDerivedEnsembleCases();
    bool                                    hasCaseReference(const RimSummaryCase* sumCase) const;

    void                            onLoadDataAndUpdate() override;

private:
    QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName,
                                                                  caf::PdmUiEditorAttribute* attribute) override;

    void                                    setAllCasesNotInUse();
    void                                    deleteCasesNoInUse();
    RimDerivedEnsembleCase*                 firstCaseNotInUse();
    std::vector<RimDerivedEnsembleCase*>    allDerivedCases(bool activeOnly) const;
    void                                    updateAutoName();
    RimSummaryCase*                         findCaseByParametersHash(const std::vector<RimSummaryCase*>& cases, size_t hash) const;
    std::vector<RimDerivedEnsembleCaseCollection*> findReferringEnsembles() const;

private:
    std::vector<RimSummaryCaseCollection*>  allEnsembles() const;

private:
    caf::PdmPtrField<RimSummaryCaseCollection*>             m_ensemble1;
    caf::PdmPtrField<RimSummaryCaseCollection*>             m_ensemble2;
    caf::PdmField<caf::AppEnum<DerivedEnsembleOperator>>    m_operator;
    caf::PdmField<bool>                                     m_swapEnsemblesButton;
    caf::PdmField<QString>                                  m_caseCount;
};
