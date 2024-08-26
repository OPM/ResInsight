/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimSummaryEnsemble.h"

#include "Cloud/RiaSumoConnector.h"

#include "cafPdmPtrField.h"

#include <QPointer>

class RimSummarySumoDataSource;

//==================================================================================================
//
//
//
//==================================================================================================

struct ParquetKey
{
    SumoCaseId caseId;
    QString    ensembleId;
    QString    vectorName;
    bool       isSensitivityParameters;

    auto operator<=>( const ParquetKey& other ) const
    {
        return std::tie( caseId, ensembleId, vectorName, isSensitivityParameters ) <=>
               std::tie( other.caseId, other.ensembleId, other.vectorName, other.isSensitivityParameters );
    }
};

namespace arrow
{
class Table;
}

class RimSummaryEnsembleSumo : public RimSummaryEnsemble
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryEnsembleSumo();

    void setSumoDataSource( RimSummarySumoDataSource* sumoDataSource );
    void updateName();

    void                               loadSummaryData( const RifEclipseSummaryAddress& resultAddress );
    std::string                        unitName( const RifEclipseSummaryAddress& resultAddress );
    RiaDefines::EclipseUnitSystem      unitSystem() const;
    std::set<RifEclipseSummaryAddress> allResultAddresses() const;

protected:
    void onLoadDataAndUpdate() override;

private:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void updateResultAddresses();
    void clearCachedData();

    QByteArray loadParquetData( const ParquetKey& parquetKey );

    void distributeDataToRealizations( const RifEclipseSummaryAddress& resultAddress, std::shared_ptr<arrow::Table> table );
    void buildMetaData();

    void distributeParametersDataToRealizations( std::shared_ptr<arrow::Table> table );

    static std::shared_ptr<arrow::Table> readParquetTable( const QByteArray& contents, const QString& messageTag );

private:
    caf::PdmPtrField<RimSummarySumoDataSource*> m_sumoDataSource;

    QPointer<RiaSumoConnector> m_sumoConnector;

    std::set<RifEclipseSummaryAddress>                  m_resultAddresses;
    std::map<ParquetKey, std::shared_ptr<arrow::Table>> m_parquetTable;
};
