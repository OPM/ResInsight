/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "RimSummaryCase.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <memory>

class RifEclipseSummaryAddress;
class RifSummaryReaderInterface;
class RifDerivedEnsembleReader;
class RimDerivedEnsembleCaseCollection;

//==================================================================================================
///  
//==================================================================================================
enum DerivedEnsembleOperator
{
    DERIVED_ENSEMBLE_SUB,
    DERIVED_ENSEMBLE_ADD
};

//==================================================================================================
//
//==================================================================================================

class RimDerivedEnsembleCase : public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;

    static const std::vector<time_t> EMPTY_TIME_STEPS_VECTOR;
    static const std::vector<double> EMPTY_VALUES_VECTOR;

public:
    RimDerivedEnsembleCase();
    ~RimDerivedEnsembleCase();

    void                            setInUse(bool inUse);
    bool                            isInUse() const;
    void                            setSummaryCases(RimSummaryCase* sumCase1, RimSummaryCase* sumCase2);
    bool                            needsCalculation(const RifEclipseSummaryAddress& address) const;
    const std::vector<time_t>&      timeSteps(const RifEclipseSummaryAddress& address) const;
    const std::vector<double>&      values(const RifEclipseSummaryAddress& address) const;

    void                            calculate(const RifEclipseSummaryAddress& address);

    virtual QString caseName() override;
    virtual void createSummaryReaderInterface() override;
    virtual RifSummaryReaderInterface* summaryReader() override;
    virtual void updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) override;

    RimDerivedEnsembleCaseCollection*           parentEnsemble() const;

private:
    std::pair<std::vector<time_t>, std::vector<double>> lookupCachedData(const RifEclipseSummaryAddress& address);
    void                                        clearData(const RifEclipseSummaryAddress& address);

    std::unique_ptr<RifDerivedEnsembleReader>   m_reader;

    bool                                        m_inUse;
    caf::PdmPtrField<RimSummaryCase*>           m_summaryCase1;
    caf::PdmPtrField<RimSummaryCase*>           m_summaryCase2;
    std::map<RifEclipseSummaryAddress, std::pair<std::vector<time_t>, std::vector<double>>> m_data;
};
