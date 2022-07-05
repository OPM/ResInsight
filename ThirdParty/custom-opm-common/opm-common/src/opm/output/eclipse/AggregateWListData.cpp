/*
  Copyright 2019-2020 Equinor ASA
  Copyright 2018 Statoil ASA

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

#include <opm/output/eclipse/AggregateWListData.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>

#include <opm/output/data/Wells.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/Well/WList.hpp>

#include <cstddef>
#include <cstring>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <string>
#include <fmt/format.h>

namespace VI = Opm::RestartIO::Helpers::VectorItems;

// #####################################################################
// Class Opm::RestartIO::Helpers::AggregateWellData
// ---------------------------------------------------------------------

namespace {
std::size_t maxNoWells(const std::vector<int>& inteHead)
{
    return inteHead[VI::intehead::NWMAXZ];
}

std::size_t maxNoOfWellListsPrWell(const std::vector<int>& inteHead)
{
    return inteHead[VI::intehead::MXWLSTPRWELL];
}

template <typename T>
std::optional<int> findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
    // Find given element in vector
    auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

    return (it != vecOfElements.end()) ? std::optional<int> {std::distance(vecOfElements.begin(), it)} :
           std::nullopt;
}

std::vector<std::vector<std::size_t>> wellOrderInWList(const Opm::Schedule&   sched,
                                                                const std::size_t sim_step,
                                                                const std::vector<int>& inteHead ) {
    const auto& wells = sched.wellNames(sim_step);
    const auto& wlmngr = sched[sim_step].wlist_manager.get();

    std::vector<std::vector<std::size_t>> curWelOrd;
    std::size_t iwlst;
    Opm::WList wlist;
    std::vector<std::size_t> well_order;
    well_order.resize(maxNoOfWellListsPrWell(inteHead), 0);

    // loop over wells and establish map
    //
    for (const auto& wname : wells) {
        if (wlmngr.hasWList(wname)) {
            const auto& wListNames = wlmngr.getWListNames(wname);
            // loop over well lists for well - and assign well sequence in the different well lists
            //
            iwlst = 0;
            for ( const auto& wlst_name : wListNames) {
                if (wlmngr.hasList(wlst_name)) {
                    wlist = wlmngr.getList(wlst_name);
                    auto well_no = findInVector<std::string>(wlist.wells(), wname);
                    if (well_no) well_order[iwlst] = well_no.value() + 1;
                    iwlst += 1;
                } else {
                    auto msg = fmt::format("Well List Manager does not contain WLIST: {} ", wlst_name);
                    throw std::logic_error(msg);
                }
            }
        }
        //store vector in map - and reset vector values to zero
        curWelOrd.push_back(well_order);
        std::fill(well_order.begin(), well_order.end(), 0);
    }
    return curWelOrd;
}

template <typename WellOp>
void wellLoop(const std::vector<std::string>& wells,
              const Opm::Schedule&            sched,
              const std::size_t               simStep,
              WellOp&&                        wellOp)
{
    for (const auto& wname : wells) {
        const auto& well = sched.getWell(wname, simStep);
        wellOp(well, well.seqIndex());
    }
}

namespace IWls {

Opm::RestartIO::Helpers::WindowedArray<int>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

    return WV {
        WV::NumWindows{ maxNoWells(inteHead) },
        WV::WindowSize{ maxNoOfWellListsPrWell(inteHead) }
    };
}


template <class IWlsArray>
void staticContrib(const Opm::Well&                well,
                   const std::vector<std::vector<std::size_t>> &  welOrdLst,
                   IWlsArray&                     iWls)
{
    const auto& well_index = well.seqIndex();

    // set values for iWls to the well order for all Wlists
    //
    std::size_t output_index = 0;
    for (const auto& wlist_index : welOrdLst[well_index]) {
        iWls[output_index] = wlist_index;
        output_index += 1;
    }
}

} // IWls

namespace ZWls {

Opm::RestartIO::Helpers::WindowedArray<
Opm::EclIO::PaddedOutputString<8>
>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<
               Opm::EclIO::PaddedOutputString<8>
               >;

    return WV {
        WV::NumWindows{ maxNoWells(inteHead) },
        WV::WindowSize{ maxNoOfWellListsPrWell(inteHead) }
    };
}

template <class ZWlsArray>
void staticContrib(const Opm::Well& well,
                   const std::vector<std::vector<std::size_t>> &  welOrdLst,
                   const Opm::WListManager& wlmngr,
                   ZWlsArray& zWls)
{
    const auto& seq_ind = well.seqIndex();

    // set values if zWls to the well list name for wells with well order > 0
    //
    if (wlmngr.hasWList(well.name())) {
        const auto& wListNames = wlmngr.getWListNames(well.name());
        const auto& wln = wListNames;
        std::size_t ind = 0;
        for (const auto& wlist_vec : welOrdLst[seq_ind]) {
            if (wlist_vec > 0) zWls[ind] = wln[ind];
            ind += 1;
        }
    }
}

} // ZWls
} // Anonymous

// =====================================================================

Opm::RestartIO::Helpers::AggregateWListData::
AggregateWListData(const std::vector<int>& inteHead)
    : iWls_ (IWls::allocate(inteHead))
    , zWls_ (ZWls::allocate(inteHead))

{}

// ---------------------------------------------------------------------

void
Opm::RestartIO::Helpers::AggregateWListData::
captureDeclaredWListData(const Schedule&   sched,
                         const std::size_t sim_step,
                         const std::vector<int>& inteHead)
{
    const auto& wells = sched.wellNames(sim_step);
    const auto& wlmngr = sched[sim_step].wlist_manager.get();
    if (wlmngr.WListSize() > 0) {

        const auto welOrdLst = wellOrderInWList(sched, sim_step, inteHead );

        // Static contributions to ZWLS array.

        {
            // Static contributions to ZWLS array.
            wellLoop(wells, sched, sim_step, [&welOrdLst, wlmngr, this]
                    (const Well& well, const std::size_t wellID) -> void
            {
                auto zw = this->zWls_[wellID];
                ZWls::staticContrib(well, welOrdLst, wlmngr, zw);
            });
        }
        // Static contributions to IWLS array.
        {
            wellLoop(wells, sched, sim_step, [&welOrdLst, this]
                    (const Well& well, const std::size_t wellID) -> void
            {
                auto iw   = this->iWls_[wellID];

                IWls::staticContrib(well, welOrdLst, iw);
            });
        }

    }
}


