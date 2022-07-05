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

#ifndef OPM_AGGREGATE_NETWORK_DATA_HPP
#define OPM_AGGREGATE_NETWORK_DATA_HPP

#include <opm/output/eclipse/WindowedArray.hpp>

#include <opm/io/eclipse/PaddedOutputString.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Network/ExtNetwork.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include <map>

namespace Opm {
    class EclipseState;
    class Schedule;
    class SummaryState;
    //class Group;
    class UnitSystem;
} // Opm

namespace Opm { namespace RestartIO { namespace Helpers {

class AggregateNetworkData
{
public:
    explicit AggregateNetworkData(const std::vector<int>& inteHead);

    void captureDeclaredNetworkData(const Opm::EclipseState&             es,
                                    const Opm::Schedule&                 sched,
                                    const Opm::UnitSystem&               units,
                                    const std::size_t                    lookup_step,
                                    const Opm::SummaryState&             sumState,
                                    const std::vector<int>&              inteHead);

    const std::vector<int>& getINode() const
    {
        return this->iNode_.data();
    }

    const std::vector<int>& getIBran() const
    {
        return this->iBran_.data();
    }

    const std::vector<int>& getINobr() const
    {
        return this->iNobr_.data();
    }


    const std::vector<double>& getRNode() const
    {
        return this->rNode_.data();
    }

    const std::vector<double>& getRBran() const
    {
        return this->rBran_.data();
    }


    const std::vector<EclIO::PaddedOutputString<8>>& getZNode() const
    {
        return this->zNode_.data();
    }


private:
    /// Aggregate 'INODE' array (Integer) for all nodes
    WindowedArray<int> iNode_;

    /// Aggregate 'IBRAN' array (Integer) for all branches
    WindowedArray<int> iBran_;

    /// Aggregate 'INOBR' array (Integer) for all nodes
    WindowedArray<int> iNobr_;

    /// Aggregate 'RNODE' array (Real) for all nodes.
    WindowedArray<double> rNode_;

    /// Aggregate 'RBRAN' array (Real) for all branches.
    WindowedArray<double> rBran_;

    /// Aggregate 'ZNODE' array (Character) for all wells.
    WindowedArray<EclIO::PaddedOutputString<8>> zNode_;

    /// Maximum number of wells in a group.
    //int nWGMax_;

    /// Maximum number of groups
    //int nGMaxz_;
};

}}} // Opm::RestartIO::Helpers

#endif // OPM_AGGREGATE_NETWORK_DATA_HPP
