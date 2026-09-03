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

#include "RimNamedObject.h"

#include "Cloud/RiaSumoConnector.h"

#include "cafPdmProxyValueField.h"

//==================================================================================================
//
// Common data source describing a single Sumo ensemble. Holds the information required both for
// summary ensembles (vector names) and for grid case ensembles (asset, grid names). The available
// ensemble realizations - fetched from the realizations endpoint - are the source of truth, and the
// user selects a subset of them ("ensemble selection"). Both summary and grid consumers listen to
// the selected realization id subset. All values are populated by RimCloudDataSourceCollection,
// which owns the RiaSumoConnector; this object does not talk to the connector directly.
//
//==================================================================================================

class RimSumoDataSource : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSumoDataSource();

    SumoCaseId caseId() const;
    void       setCaseId( const SumoCaseId& caseId );

    QString assetName() const;
    void    setAssetName( const QString& assetName );

    QString caseName() const;
    void    setCaseName( const QString& caseName );

    QString ensembleName() const;
    void    setEnsembleName( const QString& ensembleName );

    // All realizations available for the ensemble (the source of truth).
    std::vector<QString> availableRealizationIds() const;
    void                 setAvailableRealizationIds( const std::vector<QString>& realizationIds );

    // Whether the realizations have been fetched from Sumo. They are not written to the project file, so a
    // data source read back from one starts out without them, see
    // RimCloudDataSourceCollection::refreshDataSourcesFromSumo. Ask this rather than testing
    // availableRealizationIds() for emptiness: an empty list cannot tell "not fetched" from "fetched, and the
    // ensemble has none", and reading the first as the second removes the realization cases of every ensemble
    // created from this data source.
    bool hasFetchedRealizations() const;

    // The subset of realizations matching the realization filter. Both summary and grid creation listen to this.
    std::vector<QString> selectedRealizationIds() const;

    // Available summary vectors for the ensemble. Not shown in the UI, but used to populate the
    // ensemble's available result addresses (RimSummaryEnsembleSumo::updateResultAddresses).
    std::vector<QString> vectorNames() const;
    void                 setVectorNames( const std::vector<QString>& vectorNames );

    std::vector<QString> gridNames() const;
    void                 setGridNames( const std::vector<QString>& gridNames );

    QString selectedGridName() const;
    void    setSelectedGridName( const QString& gridName );

    bool doComputeMobileVolumeWeightedMean() const;

    void updateName();

private:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onRealizationFilterChanged();
    void updateGridCaseEnsembles();

    QString realizationFilterInfoText() const;
    QString availableRealizationsRangeText() const;

private:
    caf::PdmField<QString> m_caseId;
    caf::PdmField<QString> m_assetName;
    caf::PdmField<QString> m_caseName;
    caf::PdmField<QString> m_ensembleName;
    caf::PdmField<QString> m_customName;

    caf::PdmField<std::vector<QString>> m_availableRealizationIds;

    // Runtime only, deliberately not a PdmField: it describes whether this session has fetched the
    // realizations, which is never true for a data source just read from a project file.
    bool                             m_hasFetchedRealizations = false;
    caf::PdmField<QString>           m_realizationFilter;
    caf::PdmProxyValueField<QString> m_realizationFilterInfo;

    caf::PdmField<std::vector<QString>> m_vectorNames;

    caf::PdmField<std::vector<QString>> m_gridNames;
    caf::PdmField<QString>              m_selectedGridName;
    caf::PdmField<bool>                 m_doComputeMobileVolumeWeightedMean;
};
