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

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>

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
    void connectionLoop(const std::vector<Opm::Well>& wells,
                        const Opm::EclipseGrid&       grid,
                        const Opm::data::WellRates&   xw,
                        ConnOp&&                      connOp)
    {
        for (auto nWell = wells.size(), wellID = 0*nWell;
             wellID < nWell; ++wellID)
        {
            const auto& well = wells[wellID];
            const auto well_iter = xw.find(well.name());
            const Opm::data::Well * well_rates = (well_iter == xw.end()) ? nullptr : &well_iter->second;
            const auto& connections = well.getConnections().output(grid);
            std::size_t connID = 0;
            for (const auto& conn : connections) {
                if (well_rates)
                    connOp(wellID, *conn, connID, well_rates->find_connection(conn->global_index()));
                else
                    connOp(wellID, *conn, connID, nullptr);
                connID++;
            }
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
        void dynamicContrib(const Opm::data::Connection& x,
                            const Opm::UnitSystem&       units,
                            XConnArray&                  xConn)
        {
            using M  = ::Opm::UnitSystem::measure;
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XConn::index;
            using R  = ::Opm::data::Rates::opt;

            xConn[Ix::Pressure] = units.from_si(M::pressure, x.pressure);

            // Note flow rate sign.  Treat production rates as positive.
            const auto& Q = x.rates;

            if (Q.has(R::oil)) {
                xConn[Ix::OilRate] =
                    - units.from_si(M::liquid_surface_rate, Q.get(R::oil));
            }

            if (Q.has(R::wat)) {
                xConn[Ix::WaterRate] =
                    - units.from_si(M::liquid_surface_rate, Q.get(R::wat));
            }

            if (Q.has(R::gas)) {
                xConn[Ix::GasRate] =
                    - units.from_si(M::gas_surface_rate, Q.get(R::gas));
            }

            xConn[Ix::ResVRate] = 0.0;

            if (Q.has(R::reservoir_oil)) {
                xConn[Ix::ResVRate] -=
                    units.from_si(M::rate, Q.get(R::reservoir_oil));
            }

            if (Q.has(R::reservoir_water)) {
                xConn[Ix::ResVRate] -=
                    units.from_si(M::rate, Q.get(R::reservoir_water));
            }

            if (Q.has(R::reservoir_gas)) {
                xConn[Ix::ResVRate] -=
                    units.from_si(M::rate, Q.get(R::reservoir_gas));
            }
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
                        const data::WellRates& xw,
                        const std::size_t      sim_step)
{
    const auto& wells = sched.getWells(sim_step);
    connectionLoop(wells, grid, xw, [&units, this]
        (const std::size_t wellID,
         const Connection& conn, const std::size_t connID,
         const Opm::data::Connection* conn_rates) -> void
    {
        auto ic = this->iConn_(wellID, connID);
        auto sc = this->sConn_(wellID, connID);

        IConn::staticContrib(conn, connID, ic);
        SConn::staticContrib(conn, units, sc);
        if (conn_rates) {
            auto xc = this->xConn_(wellID, connID);
            XConn::dynamicContrib(*conn_rates, units, xc);
        }
    });
}
