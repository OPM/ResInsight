/*
  Copyright 2021 Equinor ASA.

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

#include <opm/input/eclipse/Schedule/WriteRestartFileEvents.hpp>

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
    constexpr auto nbits = sizeof(std::uint_fast64_t) * CHAR_BIT;
    constexpr auto zero  = std::uint_fast64_t{0};

    std::pair<std::vector<std::uint_fast64_t>::size_type, unsigned char>
    array_location(const std::size_t i)
    {
        return { i / nbits, static_cast<unsigned char>(i % nbits) };
    }

    void clear_bits(std::size_t start, std::uint_fast64_t& bits)
    {
        for (; start < nbits; ++start) {
            bits &= ~(1ULL << start);
        }
    }

    // In C++20 this is std::countl_zero().
    //
    // Using compiler intrinsics this is __builtin_clzl in GCC and Clang,
    // __lzcnt in MSVC, and _bit_scan_reverse in ICC.  POSIX has ffs but
    // only for 'int'.  Glibc additionally provides ffsl and ffsll, but
    // these require signed integer inputs.
    //
    // Note: There are more efficient approaches to computing this quantity
    // as evidenced by the existence of compiler intrinsics.  This binary
    // search is a reasonable compromise between efficiency, portability,
    // and readability outside Flow's hot path.
    std::uint_fast64_t count_leading_zeros(std::uint_fast64_t bits)
    {
        if (bits == zero) {
            return nbits;
        }

        auto count = zero;
        if ((bits & 0xFFFF'FFFF'0000'0000ULL) == zero) {
            count += 32;
            bits   = bits << 32;
        }

        if ((bits & 0xFFFF'0000'0000'0000ULL) == zero) {
            count += 16;
            bits   = bits << 16;
        }

        if ((bits & 0xFF00'0000'0000'0000ULL) == zero) {
            count += 8;
            bits   = bits << 8;
        }

        if ((bits & 0xF000'0000'0000'0000ULL) == zero) {
            count += 4;
            bits   = bits << 4;
        }

        if ((bits & 0xC000'0000'0000'0000ULL) == zero) {
            count += 2;
            bits   = bits << 2;
        }

        if ((bits & 0x8000'0000'0000'0000ULL) == zero) {
            ++count;
        }

        return count;
    }
}

void Opm::WriteRestartFileEvents::resize(const std::size_t numReportSteps)
{
    const auto loc = array_location(numReportSteps);

    if (loc.first < this->write_restart_file_.size()) {
        return;
    }

    this->write_restart_file_.resize(loc.first + 1, zero);
}

void Opm::WriteRestartFileEvents::clearRemainingEvents(const std::size_t start)
{
    const auto loc = array_location(start);

    if (loc.first >= this->write_restart_file_.size()) {
        return;
    }

    clear_bits(loc.second, this->write_restart_file_[loc.first]);

    auto begin = this->write_restart_file_.begin();
    std::advance(begin, loc.first + 1);
    std::fill(begin, this->write_restart_file_.end(), zero);
}

void Opm::WriteRestartFileEvents::addRestartOutput(const std::size_t reportStep)
{
    this->resize(reportStep + 1);

    const auto loc = array_location(reportStep);
    this->write_restart_file_[loc.first] |= (1ULL << loc.second);
}

bool Opm::WriteRestartFileEvents::writeRestartFile(const std::size_t reportStep) const
{
    const auto loc = array_location(reportStep);

    if (loc.first >= this->write_restart_file_.size()) {
        return false;
    }

    return (this->write_restart_file_[loc.first] & (1ULL << loc.second)) != zero;
}

std::optional<std::size_t>
Opm::WriteRestartFileEvents::lastRestartEventBefore(const std::size_t reportStep) const
{
    if (this->write_restart_file_.empty()) {
        return std::nullopt;
    }

    const auto loc = array_location(reportStep);
    if ((loc.first < this->write_restart_file_.size()) &&
        (loc.second > 0))
    {
        const auto prev = this->write_restart_file_[loc.first]
            & ((1ULL << loc.second) - 1);
        const auto zeros = count_leading_zeros(prev);
        if (zeros < nbits) {
            return { nbits*loc.first + nbits - zeros - 1 };
        }
    }

    auto bin = std::min(loc.first, this->write_restart_file_.size());
    auto zeros = nbits;
    while ((bin > zero) && (zeros == nbits)) {
        --bin;
        zeros = count_leading_zeros(this->write_restart_file_[bin]);
    }

    return (zeros == nbits)
        ? std::nullopt
        : std::optional<std::size_t>(nbits*bin + nbits - zeros - 1);
}

bool Opm::WriteRestartFileEvents::operator==(const WriteRestartFileEvents& that) const
{
    return this->write_restart_file_ == that.write_restart_file_;
}

Opm::WriteRestartFileEvents
Opm::WriteRestartFileEvents::serializeObject()
{
    auto events = WriteRestartFileEvents{};

    events.addRestartOutput(11);
    events.addRestartOutput(22);
    events.addRestartOutput(33);
    events.addRestartOutput(128);

    events.resize(256);

    return events;
}
