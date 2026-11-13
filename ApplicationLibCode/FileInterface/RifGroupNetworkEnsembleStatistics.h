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

#include "RifEclipseSummaryAddress.h"

#include <array>
#include <ctime>
#include <functional>
#include <map>
#include <optional>
#include <vector>

class RifSummaryReaderInterface;

//==================================================================================================
/// Statistics of summary vectors across the realizations of an ensemble, for all time steps. The
/// time steps are the union of the time steps of the realizations. A realization without a value at
/// a time step, for example a realization that ends early, is left out of the statistics for that
/// time step. Values are taken from the next time step of the realization (step right), as for the
/// rates in the ensemble statistics curves.
///
/// The statistics are cached per address, so changing the time step does not read any data. Use
/// prefetch() to compute the statistics of many addresses, as the realizations are read in parallel.
/// P10 and P90 use the same convention as the ensemble statistics curves (P10 is the high value).
//==================================================================================================
class RifGroupNetworkEnsembleStatistics
{
public:
    enum class Statistic
    {
        MEAN,
        P10,
        P50,
        P90,
        MINIMUM,
        MAXIMUM
    };

    // Returns the readers of the realizations. A reader can be nullptr. The readers are owned by the summary cases, which can
    // replace them, so no reader pointer is used after a later call to the provider. The cache is cleared when the readers change.
    using ReadersProvider = std::function<std::vector<RifSummaryReaderInterface*>()>;

    explicit RifGroupNetworkEnsembleStatistics( ReadersProvider readersProvider );

    const std::vector<std::time_t>& timeSteps() const;
    size_t                          realizationCount() const;

    // Computes the statistics of the addresses that are not cached
    void prefetch( const std::vector<RifEclipseSummaryAddress>& addresses ) const;

    // The statistic for all time steps, or nullptr if no realization has the vector. Time steps without values are HUGE_VAL.
    const std::vector<double>* values( const RifEclipseSummaryAddress& address, Statistic statistic ) const;

private:
    static constexpr size_t statisticCount = 6;
    using Statistics                       = std::array<std::vector<double>, statisticCount>;

    struct Realization
    {
        RifSummaryReaderInterface* reader     = nullptr;
        size_t                     valueCount = 0;

        // The index of the value to use at each time step, or -1 if the realization has no value at the time step
        std::vector<int> valueIndices;
    };

    void updateRealizations() const;
    void computeStatistics( const std::vector<RifEclipseSummaryAddress>& addresses ) const;

    static std::optional<Statistics> statisticsForAddress( const std::vector<Realization>&         realizations,
                                                           const std::vector<std::vector<double>>& realizationValues,
                                                           size_t                                  timeStepCount );

private:
    ReadersProvider m_readersProvider;

    mutable std::vector<int>                                              m_serialNumbers;
    mutable std::vector<Realization>                                      m_realizations;
    mutable std::vector<std::time_t>                                      m_timeSteps;
    mutable std::map<RifEclipseSummaryAddress, std::optional<Statistics>> m_cache;
};
