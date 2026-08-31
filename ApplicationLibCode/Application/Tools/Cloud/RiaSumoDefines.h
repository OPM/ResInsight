/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "nonstd/type.hpp"

#include <QString>

#include <cstddef>

using SumoAssetId = nonstd::ordered<QString, struct sumo_asset_tag>;
using SumoCaseId  = nonstd::ordered<QString, struct sumo_case_tag>;

namespace RiaSumoDefines
{
QString tokenPath();
int     requestTimeoutMillis();

// The timeout of a request nothing is waiting for. Only there so a request that never answers is eventually
// given up on, and generous because a summary vector that has not been aggregated yet is produced on demand
// by the request asking for it. Nothing is blocked while it runs, so waiting longer costs nothing.
int asyncRequestTimeoutMillis();

// Deadline for the small requests that resolve which blob to fetch and where it lives. Shorter than the
// transfer that follows: they move almost no data, so taking minutes means the answer is not coming.
int blobLookupTimeoutMillis();

// Deadline for one grid property transfer. Long enough that a large blob on a slow link is not cut off,
// and short enough that a stalled transfer fails instead of leaving the time step blank for the rest of
// the session. A failed step is not retried, so erring on the generous side is the cheaper mistake.
int gridPropertyTransferTimeoutMillis();

// The number of grid property time steps in flight at once. Bounds the concurrent transfers, and with them
// the data pulled in for time steps that may not be needed. The step being displayed is always fetched, so
// this caps the look ahead rather than the total.
size_t gridPropertyPrefetchBatchSize();

// Prefetch a new batch of grid property time steps when fewer than this many of the following time steps
// are loaded. Keeps the look ahead full instead of trickling in one or two time steps at a time.
size_t gridPropertyPrefetchLowWaterMark();

// How long to wait for the local cloud API service to answer before giving up on a request. Long enough
// to cover a cold uvicorn boot, short enough that a service that will never come up does not hang the UI.
int serviceReadyTimeoutMillis();
}; // namespace RiaSumoDefines
