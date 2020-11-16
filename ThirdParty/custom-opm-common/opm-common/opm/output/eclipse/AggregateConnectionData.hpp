/*
  Copyright (c) 2018 Equinor ASA

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

#ifndef OPM_AGGREGATE_CONNECTION_DATA_HPP
#define OPM_AGGREGATE_CONNECTION_DATA_HPP

#include <opm/output/eclipse/WindowedArray.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace Opm {
    class EclipseGrid;
    class Schedule;
    class UnitSystem;
} // Opm

namespace Opm { namespace data {
    class WellRates;
}}

namespace Opm { namespace RestartIO { namespace Helpers {

    class AggregateConnectionData
    {
    public:
        explicit AggregateConnectionData(const std::vector<int>& inteHead);

        void captureDeclaredConnData(const Opm::Schedule&        sched,
                                     const Opm::EclipseGrid&     grid,
                                     const Opm::UnitSystem&      units,
                                     const Opm::data::WellRates& xw,
                                     const std::size_t           sim_step);

        const std::vector<int>& getIConn() const
        {
            return this->iConn_.data();
        }

        const std::vector<float>& getSConn() const
        {
            return this->sConn_.data();
        }

        const std::vector<double>& getXConn() const
        {
            return this->xConn_.data();
        }

    private:
        WindowedMatrix<int> iConn_;
        WindowedMatrix<float> sConn_;
        WindowedMatrix<double> xConn_;
    };

}}} // Opm::RestartIO::Helpers

#endif // OPM_AGGREGATE_CONNECTION_DATA_HPP
