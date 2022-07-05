/*
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

#include <opm/output/eclipse/AggregateConnectionData.hpp>

#include <opm/output/eclipse/VectorItems/connection.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/output/data/Wells.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>

namespace VI = Opm::RestartIO::Helpers::VectorItems;

// #####################################################################
// Class Opm::RestartIO::Helpers::AggregateConnectionData
// ---------------------------------------------------------------------

namespace {
    std::size_t numWells(const std::vector<int>& inteHead)
    {
        return inteHead[VI::intehead::NWELLS];
    }

    std::size_t maxNumConn(const std::vector<int>& inteHead)
    {
        return inteHead[VI::intehead::NCWMAX];
    }

    template <class ConnOp>
    void connectionLoop(const Opm::EclipseGrid& grid,
                        const Opm::Well&        well,
                        const Opm::data::Well*  wellRes,
                        ConnOp&&                connOp)
    {
        const auto& wellName = well.name();
        const auto  wellID   = well.seqIndex();
        const auto  isProd   = well.isProducer();

        std::size_t connID = 0;
        for (const auto* connPtr : well.getConnections().output(grid)) {
            const auto* dynConnRes = (wellRes == nullptr)
                ? nullptr : wellRes->find_connection(connPtr->global_index());

            connOp(wellName, wellID, isProd, *connPtr, connID,
                   connPtr->global_index(), dynConnRes);

            ++connID;
        }
    }

    template <class ConnOp>
    void wellConnectionLoop(const Opm::Schedule&    sched,
                            const std::size_t       sim_step,
                            const Opm::EclipseGrid& grid,
                            const Opm::data::Wells& xw,
                            ConnOp&&                connOp)
    {
        for (const auto& wname : sched.wellNames(sim_step)) {
            const auto  well_iter = xw.find(wname);
            const auto* wellRes   = (well_iter == xw.end())
                ? nullptr : &well_iter->second;

            connectionLoop(grid, sched.getWell(wname, sim_step),
                           wellRes, std::forward<ConnOp>(connOp));
        }
    }

    namespace IConn {
        std::size_t entriesPerConn(const std::vector<int>& inteHead)
        {
            return inteHead[VI::intehead::NICONZ];
        }

        Opm::RestartIO::Helpers::WindowedMatrix<int>
        allocate(const std::vector<int>& inteHead)
        {
            using WM = Opm::RestartIO::Helpers::WindowedMatrix<int>;

            return WM {
                WM::NumRows   { numWells(inteHead) },
                WM::NumCols   { maxNumConn(inteHead) },
                WM::WindowSize{ entriesPerConn(inteHead) }
            };
        }

        template <class IConnArray>
        void staticContrib(const Opm::Connection& conn,
                           const std::size_t      connID,
                           IConnArray&            iConn)
        {
            using ConnState = ::Opm::Connection::State;
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::IConn::index;

            iConn[Ix::SeqIndex] = connID + 1;

            iConn[Ix::CellI] = conn.getI() + 1;
            iConn[Ix::CellJ] = conn.getJ() + 1;
            iConn[Ix::CellK] = conn.getK() + 1;

            iConn[Ix::ConnStat] = (conn.state() == ConnState::OPEN)
                ? 1 : 0;

            iConn[Ix::Drainage] = conn.getDefaultSatTabId()
                ? 0 : conn.satTableId();

            // Don't support differing sat-func tables for
            // draining and imbibition curves at connections.
            iConn[Ix::Imbibition] = iConn[Ix::Drainage];

            //complnum is(1 too large): 1 - based while icon is 0 - based?
            iConn[Ix::ComplNum] = conn.complnum();
            //iConn[Ix::ComplNum] = iConn[Ix::SeqIndex];

            iConn[Ix::ConnDir] = static_cast<int>(conn.dir());
            iConn[Ix::Segment] = conn.attachedToSegment()
                ? conn.segment() : 0;
        }
    } // IConn

    namespace SConn {
        std::size_t entriesPerConn(const std::vector<int>& inteHead)
        {
            return inteHead[VI::intehead::NSCONZ];
        }

        Opm::RestartIO::Helpers::WindowedMatrix<float>
        allocate(const std::vector<int>& inteHead)
        {
            using WM = Opm::RestartIO::Helpers::WindowedMatrix<float>;

            return WM {
                WM::NumRows   { numWells(inteHead) },
                WM::NumCols   { maxNumConn(inteHead) },
                WM::WindowSize{ entriesPerConn(inteHead) }
            };
        }

        template <class SConnArray>
        void staticContrib(const Opm::Connection& conn,
                           const Opm::UnitSystem& units,
                           SConnArray&            sConn)
        {
            using M  = ::Opm::UnitSystem::measure;
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::SConn::index;

            auto scprop = [&units](const M u, const double x) -> float
            {
                return static_cast<float>(units.from_si(u, x));
            };

            sConn[Ix::ConnTrans] =
                scprop(M::transmissibility, conn.CF());

            sConn[Ix::Depth]    = scprop(M::length, conn.depth());
            sConn[Ix::Diameter] = scprop(M::length, 2*conn.rw());

            sConn[Ix::EffectiveKH] =
                scprop(M::effective_Kh, conn.Kh());

            sConn[Ix::SkinFactor] = conn.skinFactor();

            sConn[Ix::item12] = sConn[Ix::ConnTrans];


            if (conn.attachedToSegment()) {
                const auto& [start, end] = *conn.perf_range();
                sConn[Ix::SegDistStart] = scprop(M::length, start);
                sConn[Ix::SegDistEnd]   = scprop(M::length, end);
            }

            sConn[Ix::item30] = -1.0e+20f;
            sConn[Ix::item31] = -1.0e+20f;
            sConn[Ix::CFInDeck] = (conn.ctfAssignedFromInput()) ? 1 : 0;
        }

        template <class SConnArray>
        void dynamicContrib(const Opm::data::Connection& xconn,
                            const Opm::UnitSystem&       units,
                            SConnArray&                  sConn)
        {
            using M  = ::Opm::UnitSystem::measure;
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::SConn::index;

            auto scprop = [&units](const M u, const double x) -> float
            {
                return static_cast<float>(units.from_si(u, x));
            };

            sConn[Ix::item12] = sConn[Ix::ConnTrans] =
                scprop(M::transmissibility, xconn.trans_factor);
        }
    } // SConn

    namespace XConn {
        std::size_t entriesPerConn(const std::vector<int>& inteHead)
        {
            return inteHead[VI::intehead::NXCONZ];
        }

        Opm::RestartIO::Helpers::WindowedMatrix<double>
        allocate(const std::vector<int>& inteHead)
        {
            using WM = Opm::RestartIO::Helpers::WindowedMatrix<double>;

            return WM {
                WM::NumRows   { numWells(inteHead) },
                WM::NumCols   { maxNumConn(inteHead) },
                WM::WindowSize{ entriesPerConn(inteHead) }
            };
        }

        template <class XConnArray>
        void dynamicContrib(const std::string&       well_name,
                            const bool               is_producer,
                            const std::size_t        global_index,
                            const Opm::SummaryState& summary_state,
                            XConnArray&              xConn)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XConn::index;

            auto get = [global_index, &well_name, &summary_state]
                (const std::string& var)
            {
                return summary_state
                    .get_conn_var(well_name, var, global_index + 1, 0.0);
            };

            auto connRate = [is_producer, &get](const char phase) -> double
            {
                const auto var =
                    fmt::format("C{}{}R", phase, is_producer ? 'P' : 'I');

                const auto val = get(var);

                // Note: Production rates are positive but injection rates
                // are reported as negative values in XCON.
                return is_producer ? val : -val;
            };

            auto connTotal = [&get](const char phase, const char direction)
            {
                return get(fmt::format("C{}{}T", phase, direction));
            };

            xConn[Ix::Pressure] = get("CPR");

            xConn[Ix::OilRate]   = connRate('O');
            xConn[Ix::WaterRate] = connRate('W');
            xConn[Ix::GasRate]   = connRate('G');
            xConn[Ix::ResVRate]  = connRate('V');

            xConn[Ix::OilPrTotal]  = connTotal('O', 'P');
            xConn[Ix::WatPrTotal]  = connTotal('W', 'P');
            xConn[Ix::GasPrTotal]  = connTotal('G', 'P');
            xConn[Ix::VoidPrTotal] = connTotal('V', 'P');

            xConn[Ix::OilInjTotal]  = connTotal('O', 'I');
            xConn[Ix::WatInjTotal]  = connTotal('W', 'I');
            xConn[Ix::GasInjTotal]  = connTotal('G', 'I');
            xConn[Ix::VoidInjTotal] = connTotal('V', 'I');

            xConn[Ix::GORatio] = get("CGOR");

            xConn[Ix::OilRate_Copy] = xConn[Ix::OilRate];
            xConn[Ix::GasRate_Copy] = xConn[Ix::GasRate];
            xConn[Ix::WaterRate_Copy] = xConn[Ix::WaterRate];

            // Pad the connection array with tracer values
            std::fill(xConn.begin() + Ix::TracerOffset, xConn.end(), 0);
        }
    } // XConn
} // Anonymous

Opm::RestartIO::Helpers::AggregateConnectionData::
AggregateConnectionData(const std::vector<int>& inteHead)
    : iConn_(IConn::allocate(inteHead))
    , sConn_(SConn::allocate(inteHead))
    , xConn_(XConn::allocate(inteHead))
{}

// ---------------------------------------------------------------------

void
Opm::RestartIO::Helpers::AggregateConnectionData::
captureDeclaredConnData(const Schedule&        sched,
                        const EclipseGrid&     grid,
                        const UnitSystem&      units,
                        const data::Wells& xw,
                        const SummaryState&    summary_state,
                        const std::size_t      sim_step)
{
    wellConnectionLoop(sched, sim_step, grid, xw, [&units, &summary_state, this]
        (const std::string&      wellName,
         const std::size_t       wellID,
         const bool              is_producer,
         const Connection&       conn,
         const std::size_t       connID,
         const std::size_t       global_index,
         const data::Connection* dynConnRes) -> void
    {
        auto ic = this->iConn_(wellID, connID);
        auto sc = this->sConn_(wellID, connID);

        IConn::staticContrib(conn, connID, ic);
        SConn::staticContrib(conn, units, sc);

        if (dynConnRes != nullptr) {
            // Simulator provides dynamic connection results such as flow
            // rates and PI-adjusted transmissibility factors.

            SConn::dynamicContrib(*dynConnRes, units, sc);
        }

        auto xc = this->xConn_(wellID, connID);
        XConn::dynamicContrib(wellName, is_producer,
                              global_index, summary_state, xc);
    });
}
