/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "Cloud/RiaSumoConnector.h"
#include "Cloud/RiaSumoDefines.h"

#include "RifRoffFileTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include <QEventLoop>
#include <QTimer>

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
/// Aborts any transfers this reader still has in flight, see RiaSumoConnector::cancelGroup.
///
/// cancelGroup() can make Qt deliver an aborted reply's finished() synchronously, re-entering this
/// reader's own completion lambdas while this destructor is still on the stack. Those lambdas bail
/// out once m_lifetimeToken has expired, so it must be invalidated *before* calling cancelGroup(),
/// not left to expire implicitly afterwards - otherwise a reentrant callback could still touch this
/// or m_caseData while both are mid-destruction.
//--------------------------------------------------------------------------------------------------
RifReaderSumoGridProperty::~RifReaderSumoGridProperty()
{
    void* lifetimeTokenKey = m_lifetimeToken.get();
    m_lifetimeToken.reset();
    if ( m_connector ) m_connector->cancelGroup( lifetimeTokenKey );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::setPendingChangedCallback( PendingChangedCallback callback )
{
    m_onPendingChanged = std::move( callback );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::setTimeStepArrivedCallback( TimeStepArrivedCallback callback )
{
    m_onTimeStepArrived = std::move( callback );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::setBatchProgressChangedCallback( BatchProgressChangedCallback callback )
{
    m_onBatchProgressChanged = std::move( callback );
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
/// A displayed dynamic property is read one time step at a time. Fires a batch request for every missing
/// time step of this property alongside the one asked for, then blocks the calling thread - showing a
/// progress bar - until the requested step itself has arrived (or the transfer times out), so the caller
/// gets back real values instead of a placeholder, the same contract a disk-based reader already has.
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

    // Skip straight to the result if this exact step is already resolved (loaded, or permanently failed) and
    // nothing is in flight for it: requestTimeStepsAsync marks its primary stepIndex pending unconditionally,
    // with no "already loaded" check of its own (only the other look-ahead steps get that check), so without
    // this guard a later call for an already-resolved step - e.g. from a redraw after a sibling step arrives -
    // would mark it pending again and re-fetch it from the network every time.
    const bool stepAlreadyPending  = m_pending.count( PendingKey{ result, stepIndex } ) > 0;
    auto*      preExistingSlot     = resultValueSlot( result, stepIndex );
    const bool stepAlreadyResolved = !stepAlreadyPending && preExistingSlot && !preExistingSlot->empty();

    if ( !stepAlreadyResolved )
    {
        // Always try to fold in the rest of this property's missing time steps alongside the one asked for -
        // requestTimeStepsAsync/timeStepsToFetch already skip whatever is already pending or loaded, so this
        // is safe to call whenever this step itself is not resolved yet.
        requestTimeStepsAsync( result, timestamps, timeStepsToFetch( result, timestamps, stepIndex ) );
    }

    // Block on the requested step: the caller (a view, calculation, contour map, etc.) is not designed to
    // cope with partially-loaded Sumo data, so the simplest correct contract is to make this call behave
    // like a disk-based reader and hand back real values once they exist, at the cost of blocking the GUI
    // behind a progress bar while they arrive.
    waitForTimeStepToArrive( result, stepIndex );

    // Either the wait above delivered the values, or the transfer timed out / is still pending: read
    // whatever is in the slot now (the real values, or still the placeholder), so a stalled transfer draws
    // blank instead of hanging the GUI forever.
    if ( auto* slot = resultValueSlot( result, stepIndex ); slot && !slot->empty() )
    {
        *values = *slot;
        return true;
    }

    return fillWithUndefinedValues( values );
}

//--------------------------------------------------------------------------------------------------
/// See header. Writes into the case's result storage the same way an arrived async time step would, so a
/// later normal read finds it already there.
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::prefetchDynamicResult( const QString& propertyName, const std::vector<size_t>& stepIndices )
{
    if ( !m_caseData || stepIndices.empty() ) return false;

    auto it = m_dynamicTimestamps.find( propertyName );
    if ( it == m_dynamicTimestamps.end() ) return false;
    const std::vector<QString>& timestamps = it->second;

    auto* cellResults = m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !cellResults ) return false;

    const RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, propertyName );
    if ( !cellResults->hasResultEntry( resultAddress ) ) return false;

    auto* timeStepValues = cellResults->modifiableCellScalarResultTimesteps( resultAddress );
    if ( !timeStepValues ) return false;

    // Size to the full time series, matching normal on-demand loading, so other time steps stay in bounds.
    if ( timeStepValues->size() < timestamps.size() ) timeStepValues->resize( timestamps.size() );

    // Only ask for what is not already there. requestTimeStepsAsync() itself skips whatever is already
    // pending (e.g. requested by a live view of the same realization), so this is safe to call even when
    // some of these steps are already on their way.
    std::vector<size_t> stepsToRequest;
    for ( size_t step : stepIndices )
    {
        if ( step >= timestamps.size() || timestamps[step].isEmpty() ) continue; // No data at this step.
        if ( !( *timeStepValues )[step].empty() ) continue; // Already there.

        stepsToRequest.push_back( step );
    }

    if ( !stepsToRequest.empty() ) requestTimeStepsAsync( propertyName, timestamps, stepsToRequest );

    // Block until every requested step is resolved. All of them are already in flight together (or were
    // already pending/loaded before this call), so waiting on them one at a time here does not serialize the
    // transfers themselves - only this call's return.
    bool allResolved = true;
    for ( size_t step : stepIndices )
    {
        if ( step >= timestamps.size() || timestamps[step].isEmpty() ) continue;

        waitForTimeStepToArrive( propertyName, step );

        if ( ( *timeStepValues )[step].empty() ) allResolved = false;
    }

    // None of the requested steps has any Sumo data at all for this property (e.g. this realization's own
    // reporting dates do not include any of the statistics' selected global dates for it), so nothing above
    // was requested or written. Leaving every slot genuinely empty would leave
    // RigCaseCellResultsData::isDataPresent() seeing this property as never having been looked at, so the
    // next read of it (e.g. RigEclipseContourMapProjection::generateResults()'s own
    // ensureKnownResultLoaded() call right after this) falls back to eagerly loading this property's entire
    // time series instead of accepting "no data at the steps that were asked for" - exactly the unbounded
    // fetch this function exists to avoid. Record that this property was genuinely looked at for this case by
    // placing one real (if blank) placeholder, the same one an arrived-but-empty transfer would leave behind.
    if ( stepsToRequest.empty() &&
         std::none_of( timeStepValues->begin(), timeStepValues->end(), []( const std::vector<double>& v ) { return !v.empty(); } ) )
    {
        for ( size_t step : stepIndices )
        {
            if ( step < timeStepValues->size() )
            {
                fillWithUndefinedValues( &( *timeStepValues )[step] );
                break;
            }
        }
    }

    return allResolved;
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
    // are written either way; a duplicate arriving later finds nothing pending and drops itself. Only bump
    // the batch total when this call is the one actually claiming it, so a duplicate arrival does not count
    // the same step twice.
    if ( m_pending.insert( PendingKey{ propertyName, stepIndex } ).second ) m_batchProgressTotal++;

    onTimeStepArrived( propertyName, stepIndex, isoDateOrInterval, contents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::markTimeStepPending( const QString& propertyName, size_t stepIndex )
{
    if ( !m_pending.insert( PendingKey{ propertyName, stepIndex } ).second ) return;

    m_batchProgressTotal++;
    updateBatchProgress();
    notifyPendingChanged();
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

    // No cap on how many are requested together: QNetworkAccessManager already limits how many of these
    // actually run at once per host and queues the rest, so this reader only needs to avoid asking for the
    // same step twice, not throttle the batch itself.
    std::vector<QString> isoDatesOrIntervals;
    std::vector<size_t>  requestedSteps;
    for ( size_t step : steps )
    {
        if ( step >= timestamps.size() || timestamps[step].isEmpty() ) continue;
        if ( !m_pending.insert( PendingKey{ propertyName, step } ).second ) continue; // Already on its way.

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

    // One dialog for the whole batch instead of one per waited-for step: grow the running total by however
    // many genuinely new steps this call just added, then let updateBatchProgress open or resize the dialog.
    m_batchProgressTotal += requestedSteps.size();
    updateBatchProgress();

    // Maps a delivered timestamp back to the time step it was requested for. Should a property report the
    // same timestamp twice, the first one is the step that was asked for.
    std::map<QString, size_t> stepByTimestamp;
    for ( size_t i = 0; i < requestedSteps.size(); i++ )
    {
        stepByTimestamp.try_emplace( isoDatesOrIntervals[i], requestedSteps[i] );
    }

    notifyPendingChanged();

    std::weak_ptr<bool> isAlive = m_lifetimeToken;

    m_connector->grid().propertyDataBatchAsync(
        SumoCaseId( m_caseId ),
        m_ensembleName,
        m_gridName,
        m_realization,
        propertyName,
        isoDatesOrIntervals,
        [this, isAlive, propertyName, stepByTimestamp]( const QString& isoDateOrInterval, const QByteArray& contents )
        {
            // The reader may be gone: a realization can be closed while its
            // transfers are still running.
            if ( isAlive.expired() ) return;

            auto it = stepByTimestamp.find( isoDateOrInterval );
            if ( it == stepByTimestamp.end() ) return;

            onTimeStepArrived( propertyName, it->second, isoDateOrInterval, contents );
        },
        m_lifetimeToken.get() );
}

//--------------------------------------------------------------------------------------------------
/// Blocks the calling thread until the given time step is no longer pending or the transfer times out
/// (RiaSumoDefines::gridPropertyTransferTimeoutMillis()), pumping a local QEventLoop so the transfer itself
/// (which runs through Qt's own event loop on this same thread) can still make progress while this call
/// waits for it. A no-op if the step is not pending, e.g. it already arrived or failed before this call.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::waitForTimeStepToArrive( const QString& propertyName, size_t stepIndex )
{
    // Refresh here as well: a transfer started before the case was open (see markTimeStepPending) was
    // registered while the view had no viewer yet, so this is the first point at which the owner can
    // actually show it.
    notifyPendingChanged();

    if ( m_pending.count( PendingKey{ propertyName, stepIndex } ) == 0 ) return;

    QEventLoop          loop;
    std::weak_ptr<bool> isAlive = m_lifetimeToken;

    QTimer pollTimer;
    QObject::connect( &pollTimer,
                      &QTimer::timeout,
                      [&]()
                      {
                          if ( isAlive.expired() || m_pending.count( PendingKey{ propertyName, stepIndex } ) == 0 ) loop.quit();
                      } );
    pollTimer.start( 20 );

    QTimer giveUpTimer;
    giveUpTimer.setSingleShot( true );
    QObject::connect( &giveUpTimer, &QTimer::timeout, &loop, &QEventLoop::quit );
    giveUpTimer.start( RiaSumoDefines::gridPropertyTransferTimeoutMillis() );

    loop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );
}

//--------------------------------------------------------------------------------------------------
/// One time step has arrived, on the GUI thread. Write it into the case results over the placeholder, then
/// tell the owner via m_onTimeStepArrived, successfully or not, so it can redraw.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::onTimeStepArrived( const QString&    propertyName,
                                                   size_t            stepIndex,
                                                   const QString&    isoDateOrInterval,
                                                   const QByteArray& contents )
{
    // No longer pending means it was abandoned while in flight, so these values are not wanted.
    if ( m_pending.erase( PendingKey{ propertyName, stepIndex } ) == 0 ) return;

    // Counts toward the single shared batch dialog whether this step succeeded or failed, and closes it once
    // nothing is left pending.
    m_batchProgressCompleted++;
    updateBatchProgress();

    if ( contents.isEmpty() )
    {
        RiaLogging::warning( QString( "Failed to load '%1' (time '%2') for realization %3 from Sumo." )
                                 .arg( propertyName, isoDateOrInterval )
                                 .arg( m_realization )
                                 .toStdString() );

        // The placeholder is left in place. A non-empty slot is not read again, so a failed step is not
        // retried on every redraw, which would turn one failure into a flood of requests. The cells stay
        // blank until the case is reloaded.
        notifyPendingChanged();
        notifyTimeStepArrived( propertyName, stepIndex, false );
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

    notifyPendingChanged();
    notifyTimeStepArrived( propertyName, stepIndex, true );
}

//--------------------------------------------------------------------------------------------------
/// Builds the "Loading X from Sumo..." message from pendingDataDescription() and reports it via
/// m_onPendingChanged. No GUI code here: the owner decides where and how to show it.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::notifyPendingChanged() const
{
    // Regression tests are left alone, matching how RiuMainWindow treats its own status messages.
    if ( !m_onPendingChanged || RiaRegressionTestRunner::instance()->isRunningRegressionTests() ) return;

    const QString pending = pendingDataDescription();
    const QString message = pending.isEmpty() ? QString() : QString( "Loading %1 from Sumo..." ).arg( pending );

    m_onPendingChanged( message );
}

//--------------------------------------------------------------------------------------------------
/// Reports how many of a batch's time steps are done via m_onBatchProgressChanged, instead of owning any
/// progress bar itself: called after requestTimeStepsAsync/markTimeStepPending/acceptFetchedTimeStep grow
/// the batch total, and after onTimeStepArrived counts one more step done. Reports total 0 once m_pending is
/// empty, so the owner knows the whole batch is over and can close whatever it is showing for it.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::updateBatchProgress()
{
    if ( m_pending.empty() )
    {
        // Whole batch done: tell the owner nothing is left, and reset the counters so the next batch starts
        // counting from zero instead of continuing this one's total.
        m_batchProgressTotal     = 0;
        m_batchProgressCompleted = 0;
        if ( m_onBatchProgressChanged ) m_onBatchProgressChanged( 0, 0, QString() );
        return;
    }

    if ( m_onBatchProgressChanged )
    {
        m_onBatchProgressChanged( m_batchProgressCompleted, m_batchProgressTotal, pendingDataDescription() );
    }
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
/// Simply forwards to m_onTimeStepArrived. No redraw logic here: the owner decides what showing this data
/// means and how to guard against re-entrancy from a redraw that reads cell results and arrives back here.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::notifyTimeStepArrived( const QString& propertyName, size_t stepIndex, bool ok ) const
{
    if ( m_onTimeStepArrived ) m_onTimeStepArrived( propertyName, stepIndex, ok );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t>
    RifReaderSumoGridProperty::timeStepsToFetch( const QString& propertyName, const std::vector<QString>& timestamps, size_t stepIndex )
{
    std::vector<size_t> steps{ stepIndex };

    if ( !m_connector ) return steps;

    // Request every other time step of this property not already loaded or on its way, alongside the one
    // asked for, in the same batch transfer. QNetworkAccessManager caps how many of these actually run at
    // once per host and queues the rest, so there is no need for this reader to also throttle how much is
    // asked for - it only needs to avoid asking for the same step twice.
    for ( size_t step = 0; step < timestamps.size(); step++ )
    {
        if ( step == stepIndex ) continue;
        if ( timestamps[step].isEmpty() ) continue; // no data at this time step for this property
        if ( m_pending.count( PendingKey{ propertyName, step } ) > 0 ) continue; // already on its way

        auto* slot = resultValueSlot( propertyName, step );
        if ( slot && !slot->empty() ) continue; // already loaded

        steps.push_back( step );
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

    return true;
}
