/*
  Copyright 2020 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cmath>
#include <stdexcept>

#include <opm/io/eclipse/rst/header.hpp>
#include <opm/io/eclipse/rst/connection.hpp>
#include <opm/output/eclipse/VectorItems/connection.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>


namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace Opm {
namespace RestartIO {

namespace {

template <typename T>
T from_int(int);

template <>
Connection::State from_int(int int_state) {
    if (int_state == 1)
        return Connection::State::OPEN;

    return Connection::State::SHUT;
}

template <>
Connection::Direction from_int(int int_dir) {
    switch (int_dir) {
    case 1:
        return Connection::Direction::X;
    case 2:
        return Connection::Direction::Y;
    case 3:
        return Connection::Direction::Z;
    default:
        throw std::invalid_argument("Can not convert: " + std::to_string(int_dir) + " to string");
    }
}

/*
  That the CTFKind variable comes from a float looks extremely suspicious; but
  it has been double checked ...
*/
Connection::CTFKind from_float(float float_kind) {
    if (float_kind == 0)
        return Connection::CTFKind::Defaulted;

    return Connection::CTFKind::DeckValue;
}
}

double RstConnection::inverse_peaceman(double cf, double kh, double rw, double skin) {
    auto alpha = 3.14159265 * 2 * kh / cf - skin;
    return rw * std::exp(alpha);
}

using M  = ::Opm::UnitSystem::measure;

RstConnection::RstConnection(const ::Opm::UnitSystem& unit_system, std::size_t rst_index_, int nsconz, const int* icon, const float* scon, const double* xcon) :
    rst_index(                                               rst_index_),
    ijk(                                                    {icon[VI::IConn::CellI] - 1, icon[VI::IConn::CellJ] - 1, icon[VI::IConn::CellK] - 1}),
    state(                                                   from_int<Connection::State>(icon[VI::IConn::ConnStat])),
    drain_sat_table(                                         icon[VI::IConn::Drainage]),
    imb_sat_table(                                           icon[VI::IConn::Imbibition]),
    completion(                                              icon[VI::IConn::ComplNum]),
    dir(                                                     from_int<Connection::Direction>(icon[VI::IConn::ConnDir])),
    segment(                                                 icon[VI::IConn::Segment]),
    cf_kind(                                                 from_float(1.0)),
    skin_factor(                                             scon[VI::SConn::SkinFactor]),
    cf(            unit_system.to_si(M::transmissibility,    scon[VI::SConn::ConnTrans])),
    depth(         unit_system.to_si(M::length,              scon[VI::SConn::Depth])),
    diameter(      unit_system.to_si(M::length,              scon[VI::SConn::Diameter])),
    kh(            unit_system.to_si(M::effective_Kh,        scon[VI::SConn::EffectiveKH])),
    segdist_end(   unit_system.to_si(M::length,              scon[VI::SConn::SegDistEnd])),
    segdist_start( unit_system.to_si(M::length,              scon[VI::SConn::SegDistStart])),
    oil_rate(      unit_system.to_si(M::liquid_surface_rate, xcon[VI::XConn::OilRate])),
    water_rate(    unit_system.to_si(M::liquid_surface_rate, xcon[VI::XConn::WaterRate])),
    gas_rate(      unit_system.to_si(M::gas_surface_rate,    xcon[VI::XConn::GasRate])),
    pressure(      unit_system.to_si(M::pressure,            xcon[VI::XConn::Pressure])),
    resv_rate(     unit_system.to_si(M::rate,                xcon[VI::XConn::ResVRate])),
    r0( RstConnection::inverse_peaceman(this->cf, this->kh, this->diameter/2, this->skin_factor) )
    /*
      r0: The r0 quantity is currently not written or read from the restart
          file. If the r0 value is given explicitly in the deck it is possible
          to give a value which is not consistent with the Peaceman formula -
          that value will be lost when loading back from a restart file.
    */
{
    if (static_cast<std::size_t>(nsconz) > VI::SConn::CFInDeck)
        this->cf_kind = from_float(scon[VI::SConn::CFInDeck]);
}

}
}
