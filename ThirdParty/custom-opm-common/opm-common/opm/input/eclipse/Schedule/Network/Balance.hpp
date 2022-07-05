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

#ifndef NETWORK_BALANCE_HPP
#define NETWORK_BALANCE_HPP

#include <cstddef>
#include <optional>

namespace Opm {
    class DeckKeyword;
    class UnitSystem;
} // namespace Opm

namespace Opm { namespace Network {

class Balance
{
public:
    enum class CalcMode {
        None = 0,
        TimeInterval = 1,
        TimeStepStart = 2,
        NUPCOL = 3,
    };

    Balance();
    explicit Balance(const DeckKeyword& keyword);
    explicit Balance(bool network_active);

    static Balance serializeObject();

    CalcMode mode() const;
    double interval() const;
    double pressure_tolerance() const;
    std::size_t pressure_max_iter() const;
    double thp_tolerance() const;
    std::size_t thp_max_iter() const;
    const std::optional<double>& target_balance_error() const;
    const std::optional<double>& max_balance_error() const;
    const std::optional<double>& min_tstep() const;

    bool operator==(const Balance& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(this->calc_mode);
        serializer(this->calc_interval);
        serializer(this->ptol);
        serializer(this->m_pressure_max_iter);
        serializer(this->m_thp_tolerance);
        serializer(this->m_thp_max_iter);
        serializer(this->target_branch_balance_error);
        serializer(this->max_branch_balance_error);
        serializer(this->m_min_tstep);
    }

private:
    CalcMode calc_mode{CalcMode::None};
    double calc_interval;
    double ptol;
    std::size_t m_pressure_max_iter;

    double m_thp_tolerance;
    std::size_t m_thp_max_iter;

    std::optional<double> target_branch_balance_error{};
    std::optional<double> max_branch_balance_error{};
    std::optional<double> m_min_tstep{};
};

}} // Opm::Network

#endif  // NETWORK_BALANCE_HPP
