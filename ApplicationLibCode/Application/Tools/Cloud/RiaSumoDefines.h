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

// The number of grid property time steps fetched concurrently when prefetching. Bounds both the number of
// requests in flight and the amount of data pulled in for time steps that may not be needed.
size_t gridPropertyPrefetchBatchSize();

// Prefetch a new batch of grid property time steps when fewer than this many of the following time steps
// are loaded. Keeps the look ahead full instead of trickling in one or two time steps at a time.
size_t gridPropertyPrefetchLowWaterMark();

// How long to wait for the local cloud API service to answer before giving up on a request. Long enough
// to cover a cold uvicorn boot, short enough that a service that will never come up does not hang the UI.
int serviceReadyTimeoutMillis();
}; // namespace RiaSumoDefines
