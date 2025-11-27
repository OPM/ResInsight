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

#include "RimDeltaSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <vector>

class RimSummaryCase;

//==================================================================================================
///
//==================================================================================================
class RimDeltaSummaryEnsemble : public RimSummaryEnsemble
{
    CAF_PDM_HEADER_INIT;

public:
    enum class FixedTimeStepMode
    {
        FIXED_TIME_STEP_NONE,
        FIXED_TIME_STEP_CASE_1,
        FIXED_TIME_STEP_CASE_2
    };

public:
    RimDeltaSummaryEnsemble();
    ~RimDeltaSummaryEnsemble() override;

    void setEnsemble1( RimSummaryEnsemble* ensemble );
    void setEnsemble2( RimSummaryEnsemble* ensemble );

    std::vector<RimSummaryCase*>       allSummaryCases() const override;
    std::set<RifEclipseSummaryAddress> ensembleSummaryAddresses() const override;

    bool hasCaseReference( const RimSummaryCase* sumCase ) const;

    void onLoadDataAndUpdate() override;

    void onSourceEnsembleChanged();
    void createDerivedEnsembleCases();

    bool discardMissingOrIncompleteRealizations() const;

    std::pair<std::string, std::string> nameKeys() const override;
    QString                             nameTemplateText() const override;

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void                              setAllCasesNotInUse();
    void                              deleteCasesNoInUse();
    RimDeltaSummaryCase*              firstCaseNotInUse();
    std::vector<RimDeltaSummaryCase*> allDerivedCases( bool activeOnly ) const;

    void updateDerivedEnsembleCases();
    bool isValid() const;

    static RimSummaryCase* findCaseByParametersHash( const std::vector<RimSummaryCase*>& cases, size_t hash );
    static RimSummaryCase* findCaseByRealizationNumber( const std::vector<RimSummaryCase*>& cases, int realizationNumber );

    std::vector<RimDeltaSummaryEnsemble*> findReferringEnsembles() const;

    std::vector<RimSummaryEnsemble*> allEnsembles() const;

private:
    caf::PdmPtrField<RimSummaryEnsemble*>               m_ensemble1;
    caf::PdmPtrField<RimSummaryEnsemble*>               m_ensemble2;
    caf::PdmField<caf::AppEnum<DerivedSummaryOperator>> m_operator;
    caf::PdmField<bool>                                 m_swapEnsemblesButton;
    caf::PdmField<QString>                              m_caseCount;
    caf::PdmField<bool>                                 m_matchOnParameters;
    caf::PdmField<bool>                                 m_discardMissingOrIncompleteRealizations;

    caf::PdmField<caf::AppEnum<FixedTimeStepMode>> m_useFixedTimeStep;
    caf::PdmField<int>                             m_fixedTimeStepIndex;
};
