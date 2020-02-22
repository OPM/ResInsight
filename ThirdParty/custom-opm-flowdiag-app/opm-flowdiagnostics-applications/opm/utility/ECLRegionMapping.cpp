/*
  Copyright 2017 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/utility/ECLRegionMapping.hpp>

#include <algorithm>
#include <exception>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace {

    std::vector<int>
    makeRegionSubset(const std::vector<int>::size_type n,
                     std::vector<int>                  regSubset)
    {
        auto ret = std::move(regSubset);

        if (ret.empty()) {
            ret.resize(n);

            std::iota(std::begin(ret), std::end(ret), 0);
        }

        return ret;
    }

} // Anonymous

Opm::ECLRegionMapping::
ECLRegionMapping(const std::vector<int>& region,
                 const std::vector<int>& regSubset)
    : regSubset_(makeRegionSubset(region.size(), regSubset))
{
    {
        auto i = 0;
        for (const auto& ix : this->regSubset_) {
            this->regionSubsetIndex_
                .addConnection(this->activeID(region[ix]), i++);
        }
    }

    if (this->next_ == this->start_) {
        // No active region IDs.  Typically empty 'region' or 'regSubset'.
        // Invalid nonetheless so we can't continue here.
        throw std::invalid_argument {
            "Requested Region/Index Vector Pair "
            "Does not Form Valid Subset"
        };
    }

    // Recall: next_ represents the dense, linear ID that would be assigned
    // to the "next" unseen region ID in a "start_"-based index sequence.
    //
    // Therefore next_ - start_ is the number of linear IDs assigned during
    // this construction process which is also the number of rows in the
    // sparse mapping matrix.
    this->regionSubsetIndex_.compress(this->next_ - this->start_);
}

const std::vector<int>&
Opm::ECLRegionMapping::regionSubset() const
{
    return this->regSubset_;
}

std::vector<int>
Opm::ECLRegionMapping::activeRegions() const
{
    auto areg = std::vector<int>{};
    areg.reserve(activeID_.size());

    for (const auto& reg : this->activeID_) {
        areg.push_back(reg.first);
    }

    std::sort(std::begin(areg), std::end(areg));

    return areg;
}

Opm::ECLRegionMapping::IndexView
Opm::ECLRegionMapping::getRegionIndices(const int region) const
{
    const auto areg = this->activeID(region);

    if (areg < 0) {
        throw std::logic_error {
            "Region ID not in configured subset"
        };
    }

    const auto& start = this->regionSubsetIndex_.startPointers();
    const auto& ix    = this->regionSubsetIndex_.neighbourhood();

    auto begin = std::begin(ix) + start[areg + 0];
    auto end   = std::begin(ix) + start[areg + 1];

    return { begin, end };
}

int Opm::ECLRegionMapping::activeID(const int regID)
{
    auto& areg = this->activeID_[regID];

    if (areg == 0) {
        // Region ID 'regID' not previously seen.  Assign new active ID.
        areg = this->next_++;
    }

    return areg - this->start_;
}

int Opm::ECLRegionMapping::activeID(const int regID) const
{
    auto i = this->activeID_.find(regID);

    if (i == std::end(this->activeID_)) {
        // Region ID 'regID' not in configured subset of ID set defined on
        // active cells.
        return -1;
    }

    return i->second - this->start_;
}
