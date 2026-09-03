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

#include "RifReaderSumoGridProperty.h"

#include "RiaLogging.h"
#include "RiaRegressionTestRunner.h"
#include "RiaSumoConnector.h"
#include "RiaSumoDefines.h"

#include "RifRoffFileTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigNestedHybridGridResultTools.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include <QStatusBar>

#include <algorithm>
#include <cmath>
#include <sstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderSumoGridProperty::RifReaderSumoGridProperty( RiaSumoConnector* connector,
                                                      const QString&    caseId,
                                                      const QString&    ensembleName,
                                                      const QString&    gridName,
                                                      int               realization )
    : m_connector( connector )
    , m_caseId( caseId )
    , m_ensembleName( ensembleName )
    , m_gridName( gridName )
    , m_realization( realization )
    , m_caseData( nullptr )
    , m_lifetimeToken( std::make_shared<bool>( true ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::setStaticProperties( const std::vector<QString>& propertyNames )
{
    m_staticProperties = propertyNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::setDynamicProperties( const std::map<QString, std::vector<QString>>& propertyNameToTimestamps )
{
    m_dynamicTimestamps = propertyNameToTimestamps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::open( const QString& /*fileName*/, RigEclipseCaseData* eclipseCase )
{
    // The grid geometry is loaded elsewhere; only keep the case data for cell count and active cell masking.
    m_caseData = eclipseCase;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    if ( matrixOrFracture != RiaDefines::PorosityModelType::MATRIX_MODEL ) return false;

    // Only fetch properties this reader owns; other static results (e.g. computed DEPTH) are not on Sumo.
    if ( std::find( m_staticProperties.begin(), m_staticProperties.end(), result ) == m_staticProperties.end() ) return false;

    return fetchAndDecode( result, "", values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/// A displayed dynamic property is read one time step at a time, and each read is a blocking round trip to
/// Sumo. Download a batch of the following time steps together with the requested one, and decode them
/// straight into the case results, so the reads that follow find the values already loaded and never reach
/// this reader. Nothing is kept in a downloaded form: the decoded values are the only copy.
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::dynamicResult( const QString&                result,
                                               RiaDefines::PorosityModelType matrixOrFracture,
                                               size_t                        stepIndex,
                                               std::vector<double>*          values )
{
    if ( matrixOrFracture != RiaDefines::PorosityModelType::MATRIX_MODEL ) return false;

    auto it = m_dynamicTimestamps.find( result );
    if ( it == m_dynamicTimestamps.end() || stepIndex >= it->second.size() ) return false;

    // The timestamp list is aligned with the case's common time steps. An empty entry means this property has
    // no data at that time step, so report "no data" instead of fetching another step's values.
    const std::vector<QString>& timestamps        = it->second;
    const QString&              isoDateOrInterval = timestamps[stepIndex];
    if ( isoDateOrInterval.isEmpty() ) return false;

    // Already on its way. Hand back blank cells and let the arrival fill them in; requesting the same time
    // step again would only duplicate the transfer.
    if ( m_pending.count( PendingKey{ result, stepIndex } ) > 0 )
    {
        // Refresh here as well. A transfer started before the case was open was registered while the view had
        // no viewer yet, so this is the first point at which the banner can actually be put up.
        updateLoadingIndicators();

        return fillWithUndefinedValues( values );
    }

    requestTimeStepsAsync( result, timestamps, timeStepsToFetch( result, timestamps, stepIndex ) );

    // Nothing is waited for. The requested step draws blank until it arrives, which is what keeps the user
    // interface responsive while tens of megabytes are transferred.
    return fillWithUndefinedValues( values );
}

//--------------------------------------------------------------------------------------------------
/// Names what is on its way, so the user is told the cells are blank because data is being fetched and not
/// because there is none. A single time step is named outright; several are counted.
//--------------------------------------------------------------------------------------------------
QString RifReaderSumoGridProperty::pendingDataDescription() const
{
    if ( m_pending.empty() ) return {};

    std::set<QString> propertyNames;
    for ( const auto& [propertyName, stepIndex] : m_pending )
    {
        propertyNames.insert( propertyName );
    }

    if ( m_pending.size() == 1 )
    {
        const auto& [propertyName, stepIndex] = *m_pending.begin();

        QString isoDateOrInterval;
        if ( auto it = m_dynamicTimestamps.find( propertyName ); it != m_dynamicTimestamps.end() && stepIndex < it->second.size() )
        {
            isoDateOrInterval = it->second[stepIndex];
        }

        if ( !isoDateOrInterval.isEmpty() ) return QString( "%1 (%2)" ).arg( propertyName, isoDateOrInterval );

        return propertyName;
    }

    if ( propertyNames.size() == 1 )
    {
        return QString( "%1 time steps of %2" ).arg( m_pending.size() ).arg( *propertyNames.begin() );
    }

    return QString( "%1 time steps" ).arg( m_pending.size() );
}

//--------------------------------------------------------------------------------------------------
/// The transfer was issued by the case before this reader existed, so the time step was never marked pending
/// here. Claim it now, so the arrival handler treats it like any other.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::acceptFetchedTimeStep( const QString&    propertyName,
                                                       size_t            stepIndex,
                                                       const QString&    isoDateOrInterval,
                                                       const QByteArray& contents )
{
    // Whichever transfer arrives first wins. Claim the step if it is not already claimed, so these values
    // are written either way; a duplicate arriving later finds nothing pending and drops itself.
    m_pending.insert( PendingKey{ propertyName, stepIndex } );

    onTimeStepArrived( propertyName, stepIndex, isoDateOrInterval, contents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::markTimeStepPending( const QString& propertyName, size_t stepIndex )
{
    if ( !m_pending.insert( PendingKey{ propertyName, stepIndex } ).second ) return;

    updateLoadingIndicators();
}

//--------------------------------------------------------------------------------------------------
/// Start a transfer for the time steps not already on their way, marking them pending before the request is
/// issued so a redraw arriving in between does not start the same transfer twice.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::requestTimeStepsAsync( const QString&              propertyName,
                                                       const std::vector<QString>& timestamps,
                                                       const std::vector<size_t>&  steps )
{
    if ( !m_connector ) return;

    // The network manager serves only a few requests per host, so transfers beyond that sit queued. Keep the
    // look ahead from growing without limit as the user moves around the time series: each read would
    // otherwise add another batch on top of whatever is still running.
    const size_t maxInFlight = RiaSumoDefines::gridPropertyPrefetchBatchSize();

    std::vector<QString> isoDatesOrIntervals;
    std::vector<size_t>  requestedSteps;
    for ( size_t step : steps )
    {
        // The displayed step is requested whatever the count. Skipping it would still leave a placeholder in
        // its slot, and a non-empty slot is not read again, so those cells would stay blank for good.
        const bool isDisplayedStep = !steps.empty() && step == steps.front();
        if ( !isDisplayedStep && m_pending.size() >= maxInFlight ) break;

        if ( step >= timestamps.size() || timestamps[step].isEmpty() ) continue;
        if ( !m_pending.insert( PendingKey{ propertyName, step } ).second ) continue;

        requestedSteps.push_back( step );
        isoDatesOrIntervals.push_back( timestamps[step] );

        // Placeholder for every requested step, not just the displayed one: a failed prefetch step would
        // otherwise leave its slot empty and no longer pending, indistinguishable from never requested, and
        // get re-fetched on the next nearby redraw.
        if ( auto* slot = resultValueSlot( propertyName, step ); slot && slot->empty() )
        {
            fillWithUndefinedValues( slot );
        }
    }

    if ( isoDatesOrIntervals.empty() ) return;

    // Maps a delivered timestamp back to the time step it was requested for. Should a property report the
    // same timestamp twice, the first one is the step that was asked for.
    std::map<QString, size_t> stepByTimestamp;
    for ( size_t i = 0; i < requestedSteps.size(); i++ )
    {
        stepByTimestamp.try_emplace( isoDatesOrIntervals[i], requestedSteps[i] );
    }

    updateLoadingIndicators();

    std::weak_ptr<bool> isAlive = m_lifetimeToken;

    m_connector->grid().propertyDataBatchAsync( SumoCaseId( m_caseId ),
                                                m_ensembleName,
                                                m_gridName,
                                                m_realization,
                                                propertyName,
                                                isoDatesOrIntervals,
                                                [this, isAlive, propertyName, stepByTimestamp]( const QString&    isoDateOrInterval,
                                                                                                const QByteArray& contents )
                                                {
                                                    // The reader may be gone: a realization can be closed while its
                                                    // transfers are still running.
                                                    if ( isAlive.expired() ) return;

                                                    auto it = stepByTimestamp.find( isoDateOrInterval );
                                                    if ( it == stepByTimestamp.end() ) return;

                                                    onTimeStepArrived( propertyName, it->second, isoDateOrInterval, contents );
                                                } );
}

//--------------------------------------------------------------------------------------------------
/// One time step has arrived, on the GUI thread. Write it into the case results over the placeholder, and
/// redraw.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::onTimeStepArrived( const QString&    propertyName,
                                                   size_t            stepIndex,
                                                   const QString&    isoDateOrInterval,
                                                   const QByteArray& contents )
{
    // No longer pending means it was abandoned while in flight, so these values are not wanted.
    if ( m_pending.erase( PendingKey{ propertyName, stepIndex } ) == 0 ) return;

    if ( contents.isEmpty() )
    {
        RiaLogging::warning( QString( "Failed to load '%1' (time '%2') for realization %3 from Sumo." )
                                 .arg( propertyName, isoDateOrInterval )
                                 .arg( m_realization )
                                 .toStdString() );

        // The placeholder is left in place. A non-empty slot is not read again, so a failed step is not
        // retried on every redraw, which would turn one failure into a flood of requests. The cells stay
        // blank until the case is reloaded.
        updateLoadingIndicators();
        return;
    }

    // Overwrite the placeholder: a non-empty slot means RigCaseCellResultsData does not ask for this time
    // step again.
    auto* slot = resultValueSlot( propertyName, stepIndex );
    if ( !slot ) return;

    if ( !decodeInto( contents, propertyName, slot ) ) return;

    logTransfer( propertyName, isoDateOrInterval, contents.size(), true );

    // The placeholder pass produced a degenerate min/max, and the legend range is built from it. Throw the
    // cached statistics away so they are computed again from the values that just arrived.
    if ( auto* cellResults = m_caseData ? m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) : nullptr )
    {
        cellResults->recalculateStatistics( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, propertyName ) );
    }

    scheduleRedrawOfViews();

    updateLoadingIndicators();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::updateLoadingIndicators() const
{
    // Regression tests are left alone, matching how RiuMainWindow treats its own status messages.
    if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() ) return;

    const QString pending = pendingDataDescription();
    const QString message = pending.isEmpty() ? QString() : QString( "Loading %1 from Sumo..." ).arg( pending );

    // A banner in the view itself. The status bar is easy to miss and the info text is small and can be
    // switched off, while the wait is measured in seconds.
    if ( auto* ownerCase = m_caseData ? m_caseData->ownerCase() : nullptr )
    {
        for ( auto* view : ownerCase->reservoirViews() )
        {
            if ( !view || !view->viewer() ) continue;

            view->viewer()->setLoadingText( message );
            view->viewer()->showLoadingLabel( !message.isEmpty() );
        }
    }

    auto* mainWindow = RiuMainWindow::instance();
    if ( !mainWindow || !mainWindow->statusBar() ) return;

    if ( message.isEmpty() )
    {
        mainWindow->statusBar()->clearMessage();
        return;
    }

    mainWindow->statusBar()->showMessage( message );
}

//--------------------------------------------------------------------------------------------------
/// HUGE_VAL is the undefined-cell value the rest of the code already uses - the same value
/// RifRoffFileTools::propertyValuesFromStream writes for inactive cells - so a pending time step renders as
/// blank cells. A correctly sized vector is returned rather than an empty one, which the result accessors
/// would read out of range.
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::fillWithUndefinedValues( std::vector<double>* values ) const
{
    if ( !values || !m_caseData || !m_caseData->mainGrid() ) return false;

    values->assign( m_caseData->mainGrid()->cellCount(), HUGE_VAL );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::scheduleRedrawOfViews()
{
    // Redrawing reads cell results, which can start the next transfer and arrive back here. Coalesce rather
    // than recurse.
    if ( m_isRedrawing )
    {
        m_hasMissedRedraw = true;
        return;
    }

    m_isRedrawing = true;
    do
    {
        m_hasMissedRedraw = false;

        if ( auto* ownerCase = m_caseData ? m_caseData->ownerCase() : nullptr )
        {
            for ( auto* view : ownerCase->reservoirViews() )
            {
                if ( view ) view->scheduleCreateDisplayModelAndRedraw();
            }
        }
    } while ( m_hasMissedRedraw );
    m_isRedrawing = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t>
    RifReaderSumoGridProperty::timeStepsToFetch( const QString& propertyName, const std::vector<QString>& timestamps, size_t stepIndex )
{
    std::vector<size_t> steps{ stepIndex };

    if ( !m_connector ) return steps;

    const size_t batchSize    = RiaSumoDefines::gridPropertyPrefetchBatchSize();
    const size_t lowWaterMark = RiaSumoDefines::gridPropertyPrefetchLowWaterMark();

    // The values already in the case results are the look ahead: a time step loaded there is served without
    // reaching this reader. Only refill when it runs low, so the batches stay full instead of trickling in
    // one or two time steps at a time.
    //
    // A pending step holds a placeholder, so a non-empty slot does not by itself mean loaded. Counting those
    // as loaded would let the look ahead believe it is full while nothing has arrived; listing them as
    // unloaded would request them twice.
    std::vector<size_t> unloadedSteps;
    size_t              loadedAhead = 0;
    for ( size_t step = stepIndex + 1; step < timestamps.size(); step++ )
    {
        if ( timestamps[step].isEmpty() ) continue; // no data at this time step for this property

        if ( m_pending.count( PendingKey{ propertyName, step } ) > 0 ) continue; // on its way

        auto* slot = resultValueSlot( propertyName, step );
        if ( slot && !slot->empty() )
        {
            loadedAhead++;
            continue;
        }

        unloadedSteps.push_back( step );
    }

    if ( loadedAhead >= lowWaterMark ) return steps;

    for ( size_t step : unloadedSteps )
    {
        if ( steps.size() >= batchSize ) break;

        steps.push_back( step );
    }

    // Near the end of the series there is little or nothing left ahead, and a batch of one degrades to a
    // blocking round trip per time step. Fill the rest of the batch with the time steps just behind,
    // nearest first: those are the ones a view is most likely to be asked for next.
    size_t precedingStep = stepIndex;
    while ( precedingStep > 0 && steps.size() < batchSize )
    {
        precedingStep--;

        if ( timestamps[precedingStep].isEmpty() ) continue;

        if ( m_pending.count( PendingKey{ propertyName, precedingStep } ) > 0 ) continue;

        auto* slot = resultValueSlot( propertyName, precedingStep );
        if ( slot && !slot->empty() ) continue;

        steps.push_back( precedingStep );
    }

    return steps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RifReaderSumoGridProperty::resultValueSlot( const QString& propertyName, size_t stepIndex )
{
    if ( !m_caseData ) return nullptr;

    auto* cellResults = m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !cellResults ) return nullptr;

    const RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, propertyName );
    if ( !cellResults->hasResultEntry( resultAddress ) ) return nullptr;

    auto* timeStepValues = cellResults->modifiableCellScalarResultTimesteps( resultAddress );
    if ( !timeStepValues || stepIndex >= timeStepValues->size() ) return nullptr;

    return &( ( *timeStepValues )[stepIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::fetchAndDecode( const QString& propertyName, const QString& isoDateOrInterval, std::vector<double>* values )
{
    if ( !m_connector || !m_caseData || !values ) return false;

    QByteArray contents =
        m_connector->grid().propertyData( SumoCaseId( m_caseId ), m_ensembleName, m_gridName, m_realization, propertyName, isoDateOrInterval );

    logTransfer( propertyName, isoDateOrInterval, contents.size(), false );

    return decodeInto( contents, propertyName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::logTransfer( const QString& propertyName, const QString& isoDateOrInterval, size_t byteCount, bool fromBatch ) const
{
    RiaLogging::debug( std::format( "Sumo grid property '{}' realization {} (time '{}') [{}]: {} bytes",
                                    propertyName.toStdString(),
                                    m_realization,
                                    isoDateOrInterval.toStdString(),
                                    fromBatch ? "batch" : "single",
                                    byteCount ) );
}

//--------------------------------------------------------------------------------------------------
/// Decode a downloaded roff property blob into cell values. The same post processing as the on demand
/// path in RigCaseCellResultsData is applied, so a prefetched time step is indistinguishable from one
/// read when it was asked for.
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::decodeInto( const QByteArray& contents, const QString& propertyName, std::vector<double>* values )
{
    if ( contents.isEmpty() || !m_caseData || !values ) return false;

    std::string        buffer = contents.toStdString();
    std::istringstream stream( buffer, std::ios::binary );

    if ( !RifRoffFileTools::propertyValuesFromStream( stream, m_caseData, propertyName, values ) ) return false;

    RigNestedHybridGridResultTools::assignValuesToLgrs( m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ), *values );

    return true;
}
