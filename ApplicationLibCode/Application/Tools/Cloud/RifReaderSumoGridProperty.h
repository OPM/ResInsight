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

#include "RifReaderInterface.h"

#include <QByteArray>
#include <QPointer>
#include <QString>

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

class RiaSumoConnector;

//==================================================================================================
//
// Lazily fetches grid cell properties for a single Sumo grid realization. ResInsight's result
// machinery calls staticResult()/dynamicResult() the first time a property is displayed; this reader
// then downloads the corresponding roff blob from Sumo and decodes the cell values.
//
//==================================================================================================
class RifReaderSumoGridProperty : public RifReaderInterface
{
public:
    RifReaderSumoGridProperty( RiaSumoConnector* connector,
                               const QString&    caseId,
                               const QString&    ensembleName,
                               const QString&    gridName,
                               int               realization );

    // Aborts any transfers still in flight for this reader, see RiaSumoConnector::cancelGroup, so they stop
    // consuming connections and logging results for a reader that is no longer there.
    ~RifReaderSumoGridProperty() override;

    void setStaticProperties( const std::vector<QString>& propertyNames );
    void setDynamicProperties( const std::map<QString, std::vector<QString>>& propertyNameToTimestamps );

    bool open( const QString& fileName, RigEclipseCaseData* eclipseCase ) override;

    // Take one time step fetched before this reader existed, see RimRoffCaseSumo::startPropertyFetch.
    void acceptFetchedTimeStep( const QString& propertyName, size_t stepIndex, const QString& isoDateOrInterval, const QByteArray& contents );

    // Record a transfer started before this reader existed as already on its way. Without this the reader
    // reports nothing in flight, so the user is told nothing while waiting and a second, duplicate transfer
    // is issued the first time the property is read.
    void markTimeStepPending( const QString& propertyName, size_t stepIndex );

    // What is being transferred right now, for display to the user. Empty when nothing is.
    QString pendingDataDescription() const;

    bool staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values ) override;
    bool dynamicResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, size_t stepIndex, std::vector<double>* values ) override;

private:
    bool fetchAndDecode( const QString& propertyName, const QString& isoDateOrInterval, std::vector<double>* values );
    bool decodeInto( const QByteArray& contents, const QString& propertyName, std::vector<double>* values );

    // One line per transferred time step, naming the realization and whether it came from a batch or from
    // a single request. This reader is per realization, so without the realization a run of single
    // requests from several readers is indistinguishable from one reader fetching the same steps twice.
    void logTransfer( const QString& propertyName, const QString& isoDateOrInterval, size_t byteCount, bool fromBatch ) const;

    // The time steps to download together with stepIndex: none when enough of the following time steps are
    // already loaded, otherwise the next batch of unloaded ones. Always contains stepIndex itself.
    std::vector<size_t> timeStepsToFetch( const QString& propertyName, const std::vector<QString>& timestamps, size_t stepIndex );

    // Where the case keeps the values of one time step of a dynamic property, or null when the property is
    // not registered as a result. Writing here is what makes a prefetched time step available without
    // holding on to the downloaded bytes.
    std::vector<double>* resultValueSlot( const QString& propertyName, size_t stepIndex );

    // One requested time step of one dynamic property.
    using PendingKey = std::pair<QString, size_t>;

    // Request the given time steps without waiting, marking them pending. Already pending steps are skipped,
    // so a redraw mid-transfer does not issue a second request.
    void requestTimeStepsAsync( const QString& propertyName, const std::vector<QString>& timestamps, const std::vector<size_t>& steps );

    // Called on the connector thread, which is the GUI thread, once per requested time step.
    void onTimeStepArrived( const QString& propertyName, size_t stepIndex, const QString& isoDateOrInterval, const QByteArray& contents );

    // Fill values with the undefined-cell value, sized to the grid, and report success, so a time step still
    // on its way draws blank instead of logging a load failure on every redraw.
    bool fillWithUndefinedValues( std::vector<double>* values ) const;

    // Redraw the views of this case, so arrived values become visible.
    void scheduleRedrawOfViews();

    // Refresh every surface reporting what is on its way: the banner in the view, the status bar, and
    // indirectly the info box via RimRoffCaseSumo::dataLoadingText.
    void updateLoadingIndicators() const;

private:
    QPointer<RiaSumoConnector> m_connector;
    QString                    m_caseId;
    QString                    m_ensembleName;
    QString                    m_gridName;
    int                        m_realization;

    RigEclipseCaseData* m_caseData; // set in open(); used for cell count and active cell masking

    std::vector<QString>                    m_staticProperties;
    std::map<QString, std::vector<QString>> m_dynamicTimestamps; // property name -> sorted iso timestamps

    // Requested and not yet arrived. Claimed before the request is issued, so it also answers "already on
    // its way?".
    std::set<PendingKey> m_pending;

    // Dropped when this reader dies. Callbacks hold a weak_ptr and return early once it expires, so a reply
    // outliving the reader cannot write into freed memory.
    std::shared_ptr<bool> m_lifetimeToken;

    // A redraw can read another time step and arrive back here. Coalesce instead of recursing.
    bool m_isRedrawing     = false;
    bool m_hasMissedRedraw = false;
};
