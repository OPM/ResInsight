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

class RimSumoDataSource;

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
    // Sent when data asked for earlier has arrived, for views the reload below cannot reach.
    caf::Signal<> summaryDataLoaded;

public:
    RimSummaryEnsembleSumo();

    void setSumoDataSource( RimSumoDataSource* sumoDataSource );

    void onRealizationSelectionChanged();

    void                               loadSummaryData( const RifEclipseSummaryAddress& resultAddress );
    void                               loadSummaryData( const std::vector<RifEclipseSummaryAddress>& resultAddresses );
    std::string                        unitName( const RifEclipseSummaryAddress& resultAddress );
    RiaDefines::EclipseUnitSystem      unitSystem() const;
    std::set<RifEclipseSummaryAddress> allResultAddresses() const;

    std::pair<std::string, std::string> nameKeys() const override;
    void                                updateName( const std::set<QString>& existingEnsembleNames ) override;
    void                                prefetchSummaryData( const std::vector<RifEclipseSummaryAddress>& resultAddresses ) override;
    bool                                isSummaryDataPending( const std::vector<RifEclipseSummaryAddress>& resultAddresses ) const override;

protected:
    void onLoadDataAndUpdate() override;

private:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void updateResultAddresses();
    void clearCachedData();

    void distributeDataToRealizations( const RifEclipseSummaryAddress& resultAddress, std::shared_ptr<arrow::Table> table );
    void buildMetaData();

    void distributeParametersDataToRealizations( std::shared_ptr<arrow::Table> table );
    void redistributeCachedDataToRealizations();
    void loadEnsembleParameters();

    void onVectorDataReceived( const ParquetKey& parquetKey, const QByteArray& contents );
    void onParameterDataReceived( const ParquetKey& parquetKey, const QByteArray& contents );
    void updatePlotsUsingThisEnsemble();

    static std::shared_ptr<arrow::Table> readParquetTable( const QByteArray& contents, const QString& messageTag );

private:
    caf::PdmPtrField<RimSumoDataSource*> m_sumoDataSource;

    QPointer<RiaSumoConnector> m_sumoConnector;

    std::set<RifEclipseSummaryAddress>                  m_resultAddresses;
    std::map<ParquetKey, std::shared_ptr<arrow::Table>> m_parquetTable;

    // The vectors requested but not yet arrived, and the address each belongs to. A vector in here is not
    // requested again, and is reported as having no data yet rather than being waited for.
    std::map<ParquetKey, RifEclipseSummaryAddress> m_pendingVectors;

    // Held by the callbacks of requests still on their way. They check it before touching this object, so a
    // reply arriving after the ensemble is gone is dropped instead of writing into freed memory.
    std::shared_ptr<bool> m_lifetimeToken;

    bool m_isUpdatingPlots     = false;
    bool m_hasMissedPlotUpdate = false;
};
