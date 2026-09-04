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

#include "RimEclipseCase.h"

#include "Cloud/RiaSumoGrid.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

#include <QPointer>
#include <QString>

#include <memory>
#include <optional>
#include <utility>

class RiaSumoConnector;
class RifReaderSumoGridProperty;
class RimSumoDataSource;

//==================================================================================================
//
// Eclipse grid case backed by a roff grid stored on Sumo. The grid geometry is downloaded as a
// blob through RiaSumoConnector and parsed in memory, so there is no grid file on disk.
//
//==================================================================================================
class RimRoffCaseSumo : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimRoffCaseSumo();
    ~RimRoffCaseSumo() override;

    // Create a grid case for a single realization of the given grid, linked back to the data source
    // so the case can be updated when the data source realization filter changes.
    static RimRoffCaseSumo* createFromDataSource( RimSumoDataSource* dataSource, const QString& gridName, int realization );

    void setSumoDataSource( RimSumoDataSource* dataSource );
    void setSumoCaseId( const QString& sumoCaseId );
    void setEnsembleName( const QString& ensembleName );
    void setGridName( const QString& gridName );
    void setRealization( int realization );

    QString gridName() const;
    int     realization() const;

    bool openEclipseGridFile() override;

    void closeReservoirCase() override;

    // Aborts any transfers still in flight for this realization, without discarding already loaded grid and
    // result data. Used when switching away from this case in a view while it may still be shown elsewhere,
    // so a still loading realization does not keep competing for bandwidth with a newly selected one, while a
    // fully loaded realization is not forced through a full reload if switched back to.
    void cancelPendingTransfers();

    // Fetches and stores one time step of a dynamic property, synchronously, without pulling in the whole
    // time series. See RifReaderSumoGridProperty::prefetchDynamicResult.
    bool prefetchDynamicResult( const QString& resultName, size_t stepIndex ) override;

    QString locationOnDisc() const override;

    QString dataLoadingText() const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    // Download the roff grid blob for this realization and parse it into the case data.
    bool downloadAndParseGrid();

    // Register the results and attach the property reader. Shared by the two ways the grid can end up on
    // this case: parsed and kept, or parsed and replaced by the grid shared with the other realizations.
    void finalizeCaseSetup();

    // Discover the available grid properties from Sumo, register them as cell results and attach a reader
    // that fetches the property data on demand. Static properties only in this first version.
    void registerSumoGridProperties();

    // Start the transfer of the property time step about to be displayed. Called before the grid download so
    // the two run in parallel; only the decode needs the grid.
    //
    // The values go to the reader, which does not exist yet at this point. That is safe: arrivals come through
    // the event loop, and runOnTransferThreadBlocking holds the grid download on a semaphore without
    // dispatching events, so nothing lands before the case is set up.
    void startPropertyFetch();

    // The property and time step a view of this case is about to show. False when no view has picked one yet,
    // leaving the on demand path to fetch whatever is asked for.
    bool propertyToFetch( QString& propertyName, size_t& stepIndex, QString& isoDateOrInterval ) const;

private:
    caf::PdmPtrField<RimSumoDataSource*> m_sumoDataSource;
    caf::PdmField<QString>               m_sumoCaseId;
    caf::PdmField<QString>               m_ensembleName;
    caf::PdmField<QString>               m_gridName;
    caf::PdmField<int>                   m_realization;

    QPointer<RiaSumoConnector> m_sumoConnector;

    // The same reader the cell results hold, set in registerSumoGridProperties. Time steps fetched before it
    // existed are handed to it.
    cvf::ref<RifReaderSumoGridProperty> m_propertyReader;

    // The properties Sumo reports for this realization. Fetched before the grid and reused when they are
    // registered, so the request is made once.
    std::vector<SumoGridPropertyInfo> m_propertyInfos;

    // The time step startPropertyFetch put in flight, until the reader exists and takes it over. Handed to
    // the reader as pending so it is reported to the user and not requested a second time.
    std::optional<std::pair<QString, size_t>> m_fetchInFlight;

    // Identifies the transfer startPropertyFetch issues before the reader (and its own lifetime token) exists,
    // so it can still be cancelled from closeReservoirCase. Recreated on every close, so a transfer left over
    // from a previous open is never cancelled by a later one.
    std::shared_ptr<bool> m_lifetimeToken;
};
