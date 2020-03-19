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

//==================================================================================================
///
//==================================================================================================
enum class DerivedSummaryOperator
{
    DERIVED_OPERATOR_SUB,
    DERIVED_OPERATOR_ADD
};

//==================================================================================================
//
//==================================================================================================
class RimDerivedSummaryCase : public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimDerivedSummaryCase();
    ~RimDerivedSummaryCase() override;

    void setInUse( bool inUse );
    bool isInUse() const;
    void setSummaryCases( RimSummaryCase* sumCase1, RimSummaryCase* sumCase2 );
    void setOperator( DerivedSummaryOperator oper );

    bool                       needsCalculation( const RifEclipseSummaryAddress& address ) const;
    const std::vector<time_t>& timeSteps( const RifEclipseSummaryAddress& address ) const;
    const std::vector<double>& values( const RifEclipseSummaryAddress& address ) const;

    void calculate( const RifEclipseSummaryAddress& address );

    void                       createSummaryReaderInterface() override;
    RifSummaryReaderInterface* summaryReader() override;
    void updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath ) override;

protected:
    QString caseName() const override;

private:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void clearData( const RifEclipseSummaryAddress& address );
    void updateNameFromInputCases();

private:
    caf::PdmPtrField<RimSummaryCase*>                   m_summaryCase1;
    caf::PdmPtrField<RimSummaryCase*>                   m_summaryCase2;
    caf::PdmField<caf::AppEnum<DerivedSummaryOperator>> m_operator;

    bool                                      m_inUse;
    std::unique_ptr<RifDerivedEnsembleReader> m_reader;

    std::map<RifEclipseSummaryAddress, std::pair<std::vector<time_t>, std::vector<double>>> m_dataCache;
};
