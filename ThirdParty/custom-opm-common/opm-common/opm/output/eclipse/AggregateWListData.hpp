/*
  Copyright (c) 2018 Statoil ASA

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

#ifndef OPM_AGGREGATE_WLIST_DATA_HPP
#define OPM_AGGREGATE_WLIST_DATA_HPP

#include <opm/output/eclipse/WindowedArray.hpp>
#include <opm/io/eclipse/PaddedOutputString.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace Opm {
    class Schedule;
} // Opm

namespace Opm { namespace data {
    class Wells;
}} // Opm::data

namespace Opm { namespace RestartIO { namespace Helpers {

    class AggregateWListData
    {
    public:
        explicit AggregateWListData(const std::vector<int>& inteHead);

        void captureDeclaredWListData(const Schedule&   sched,
                                    const std::size_t sim_step,
                                    const std::vector<int>& inteHead);



        /// Retrieve Integer WLIST Data Array.
        const std::vector<int>& getIWls() const
        {
            return this->iWls_.data();
        }

        /// Retrieve Character WLIST Data Array.
        const std::vector<EclIO::PaddedOutputString<8>>& getZWls() const
        {
            return this->zWls_.data();
        }



    private:
        /// Aggregate 'IWLS' array (Integer) for all wells.
        WindowedArray<int> iWls_;

        /// Aggregate 'ZWLS' array (Character) for all wells.
        WindowedArray<EclIO::PaddedOutputString<8>> zWls_;

    };

}}} // Opm::RestartIO::Helpers

#endif // OPM_AGGREGATE_WLIST_DATA_HPP
