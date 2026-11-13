/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifGroupNetworkSummaryData.h"
#include "RigGroupNetwork.h"

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

#include <atomic>
#include <ctime>
#include <future>
#include <map>
#include <memory>
#include <vector>

class RimSummaryCase;
class RimSummaryEnsemble;
class RifGroupNetworkEnsembleStatistics;

//==================================================================================================
/// A production network tree read from a deck (GRUPTREE or BRANPROP), with node and edge values from
/// a summary case or a summary ensemble. This is a data source, shown in the data source tree. The
/// network is shown by one or more RimGroupNetworkPlot objects.
///
/// For an ensemble, the network is read from the deck of the first realization, and the values are
/// taken from a realization or from a statistic of all realizations. The networks of the other
/// realizations are compared in the background, and a warning is logged if they differ.
//==================================================================================================
class RimGroupNetworkData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SourceType
    {
        SUMMARY_CASE,
        SUMMARY_ENSEMBLE
    };

    // The values shown for an ensemble
    enum class EnsembleValue
    {
        REALIZATION,
        MEAN,
        P10,
        P50,
        P90,
        MINIMUM,
        MAXIMUM
    };

    using TreeTypeEnum      = caf::AppEnum<RigGroupNetwork::TreeType>;
    using SourceTypeEnum    = caf::AppEnum<SourceType>;
    using EnsembleValueEnum = caf::AppEnum<EnsembleValue>;

    caf::Signal<> dataChanged;

public:
    RimGroupNetworkData();
    ~RimGroupNetworkData() override;

    QString             name() const;
    RimSummaryCase*     summaryCase() const;
    void                setSummaryCase( RimSummaryCase* summaryCase );
    RimSummaryEnsemble* summaryEnsemble() const;
    void                setSummaryEnsemble( RimSummaryEnsemble* ensemble );
    bool                isEnsemble() const;

    // The realizations of the ensemble, and the realization used for the network and the units
    std::vector<RimSummaryCase*> realizations() const;
    RimSummaryCase*              referenceRealization() const;

    const RigGroupNetwork* network() const;

    // The values of a summary case, or of the given value of an ensemble. The first realization is used if no realization is given.
    const RifGroupNetworkSummaryData* summaryData( EnsembleValue   value       = EnsembleValue::REALIZATION,
                                                   RimSummaryCase* realization = nullptr ) const;
    const std::vector<std::time_t>& timeSteps( EnsembleValue value = EnsembleValue::REALIZATION, RimSummaryCase* realization = nullptr ) const;

    RigGroupNetwork::TreeType            treeType() const;
    QString                              statusText( const RigGroupNetwork::TreeRevision* shownTree ) const;
    const RigGroupNetwork::TreeRevision* treeAtDate( std::time_t date ) const;

    void reload();

    static QString dateText( std::time_t date );

private:
    caf::PdmFieldHandle* userDescriptionField() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          initAfterRead() override;
    void                          appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void ensureLoaded() const;
    void clearCache();
    void clearTreeTypeCache();
    void updateDeckFile( const QString& previousDeckFile );
    void startTopologyCheck() const;
    void stopTopologyCheck() const;

    QString                                     deckFile() const;
    QString                                     defaultDeckFile() const;
    std::unique_ptr<RifGroupNetworkSummaryData> createCaseData( RimSummaryCase* summaryCase ) const;

private:
    caf::PdmField<QString>                m_name;
    caf::PdmField<SourceTypeEnum>         m_sourceType;
    caf::PdmPtrField<RimSummaryCase*>     m_summaryCase;
    caf::PdmPtrField<RimSummaryEnsemble*> m_summaryEnsemble;
    caf::PdmField<caf::FilePath>          m_deckFile;
    caf::PdmField<TreeTypeEnum>           m_treeType;

    mutable bool                                        m_isLoaded = false;
    mutable QString                                     m_loadError;
    mutable std::unique_ptr<RigGroupNetwork>            m_network;
    mutable std::unique_ptr<RifGroupNetworkSummaryData> m_summaryData;
    mutable std::vector<std::time_t>                    m_networkDates;

    // Ensemble data, created on demand
    struct RealizationData
    {
        caf::PdmPointer<RimSummaryCase>             summaryCase;
        std::unique_ptr<RifGroupNetworkSummaryData> data;
    };
    mutable std::map<const RimSummaryCase*, RealizationData>                     m_realizationData;
    mutable std::shared_ptr<RifGroupNetworkEnsembleStatistics>                   m_ensembleStatistics;
    mutable std::map<EnsembleValue, std::unique_ptr<RifGroupNetworkSummaryData>> m_statisticData;

    mutable std::shared_ptr<std::atomic<bool>> m_topologyCheckCancelled;
    mutable std::future<void>                  m_topologyCheck;
};
