/*
  Copyright 2020 Equinor ASA.

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
#include <fmt/format.h>
#include <numeric>


#include <opm/common/utility/Serializer.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Schedule/Well/PAvg.hpp>
#include <opm/input/eclipse/Schedule/Well/PAvgCalculator.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

namespace Opm {

namespace {
std::optional<PAvgCalculator::Neighbour> make_neighbour(const EclipseGrid& grid, std::size_t i, std::size_t j, std::size_t k) {

    if (i >= grid.getNX())
        return {};

    if (j >= grid.getNY())
        return {};

    if (k >= grid.getNZ())
        return {};

    if (!grid.cellActive(i,j,k))
        return {};

    double porv = -1;
    return PAvgCalculator::Neighbour(porv, grid.getGlobalIndex(i,j,k));
}
}


const std::string& PAvgCalculator::wname() const {
    return this->well_name;
}


PAvgCalculator::PAvgCalculator(const std::string& well, double well_ref_depth, const EclipseGrid& grid, const std::vector<double>& porv, const WellConnections& connections, const PAvg& pavg) :
    well_name(well),
    m_pavg(pavg),
    ref_depth(well_ref_depth)
{
    if (porv.size() != grid.getCartesianSize())
        throw std::logic_error("Should pass a GLOBAL porv vector");

    for (const auto& conn : connections) {
        if (conn.state() == ::Opm::Connection::State::OPEN || !this->m_pavg.open_connections()) {
            Connection wp_conn(porv[conn.global_index()], conn.CF(), conn.dir(), conn.global_index());
            this->add_connection(wp_conn);
        }
    }

    auto size = this->m_connections.size();
    for (std::size_t index = 0; index < size; index++) {
        auto& conn = this->m_connections[index];
        auto [i,j,k] = grid.getIJK(conn.global_index);
        if (conn.dir == ::Opm::Connection::Direction::X) {
            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j  ,k+1), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j  ,k-1), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j+1,k),   true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j-1,k),   true);

            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j+1,k+1), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j+1,k-1), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j-1,k+1), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i,j-1,k-1), false);
        }

        if (conn.dir == ::Opm::Connection::Direction::Y) {
            this->add_neighbour(conn.global_index, make_neighbour(grid, i+1,j,k), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i-1,j,k), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i  ,j,k+1), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i  ,j,k-1), true);

            this->add_neighbour(conn.global_index, make_neighbour(grid, i+1,j,k+1), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i-1,j,k+1), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i+1,j,k-1), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i-1,j,k-1), false);
        }

        if (conn.dir == ::Opm::Connection::Direction::Z) {
            this->add_neighbour(conn.global_index, make_neighbour(grid, i+1,j  ,k), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i-1,j  ,k), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i  ,j+1,k), true);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i  ,j-1,k), true);

            this->add_neighbour(conn.global_index, make_neighbour(grid, i+1,j+1,k), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i-1,j+1,k), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i+1,j-1,k), false);
            this->add_neighbour(conn.global_index, make_neighbour(grid, i-1,j-1,k), false);
        }
    }
    for (const auto& [ijk, _] : this->m_index_map) {
        (void)_;
        this->m_index_list.push_back(ijk);
    }

    this->pressure.resize( this->m_index_list.size() );
    this->valid_pressure.resize( this->m_index_list.size(), 0 );
}


void PAvgCalculator::add_connection(const PAvgCalculator::Connection& conn) {
    this->m_index_map.insert(std::make_pair( conn.global_index, this->m_index_map.size()));
    this->m_connections.push_back(conn);
}

void PAvgCalculator::add_neighbour(std::size_t global_index, std::optional<PAvgCalculator::Neighbour> neighbour, bool rect_neighbour) {
    if (neighbour) {
        this->m_index_map.insert(std::make_pair( neighbour->global_index, this->m_index_map.size()));
        auto& conn = this->m_connections[ this->m_index_map[global_index] ];
        if (rect_neighbour)
            conn.rect_neighbours.push_back(neighbour.value());
        else
            conn.diag_neighbours.push_back(neighbour.value());
    }
}


const std::vector< std::size_t >& PAvgCalculator::index_list() const {
    return this->m_index_list;
}

bool PAvgCalculator::add_pressure(std::size_t global_index, double block_pressure) {
    auto index_iter = this->m_index_map.find(global_index);
    if (index_iter == this->m_index_map.end())
        return false;

    auto storage_index = index_iter->second;
    this->pressure[storage_index] = block_pressure;
    this->valid_pressure[storage_index] = 1;
    return true;
}


double PAvgCalculator::get_pressure(std::size_t global_index) const {
    auto storage_index = this->m_index_map.at(global_index);
    if (this->valid_pressure[storage_index])
        return this->pressure[storage_index];

    auto msg = fmt::format("Tried to access pressure in invalid cell: {}", global_index);
    throw std::runtime_error(msg);
}



double PAvgCalculator::cf_avg(const std::vector<std::optional<double>>& block_pressure) const {
    double pressure_sum = 0;
    double cf_sum = 0;

    for (std::size_t index = 0; index < block_pressure.size(); index++) {
        if (block_pressure[index]) {
            const auto& conn = this->m_connections[index];
            pressure_sum += block_pressure[index].value() * conn.cfactor;
            cf_sum += conn.cfactor;
        }
    }

    if (cf_sum == 0)
        return 0;
    return pressure_sum / cf_sum;
}

double PAvgCalculator::wbp() const {
    return this->wbp(PAvgCalculator::WBPMode::WBP);
}

double PAvgCalculator::wbp4() const {
    return this->wbp(PAvgCalculator::WBPMode::WBP4);
}

double PAvgCalculator::wbp5() const {
    return this->wbp(PAvgCalculator::WBPMode::WBP5);
}

double PAvgCalculator::wbp9() const {
    return this->wbp(PAvgCalculator::WBPMode::WBP9);
}


std::pair<double,double> PAvgCalculator::porv_pressure(std::size_t global_index) const {
    auto storage_index = this->m_index_map.at(global_index);
    const auto& conn = this->m_connections[storage_index];
    return std::make_pair(conn.porv, this->get_pressure(global_index));
}


static double weighted_avg(const std::vector<std::pair<double, double>>& wp) {
    if (wp.empty())
        return 0;

    double pressure_sum = 0;
    double weight_sum = 0;

    for (const auto& [weight, pressure] : wp) {
        weight_sum += weight;
        pressure_sum += weight * pressure;
    }

    return pressure_sum / weight_sum;
}



std::vector<std::optional<double>> PAvgCalculator::block_pressures(PAvgCalculator::WBPMode mode) const{
    std::vector<std::optional<double>> block_pressure;
    for (const auto& conn : this->m_connections) {
        const double F1 = this->m_pavg.inner_weight();
        if (F1 >= 0) {
            std::optional<double> central_pressure;
            double neighbour_pressure = 0;
            std::size_t neighbour_count = 0;

            if (mode != PAvgCalculator::WBPMode::WBP4)
                central_pressure = this->get_pressure(conn.global_index);

            if (mode != PAvgCalculator::WBPMode::WBP) {
                for (const auto& neighbour : conn.rect_neighbours) {
                    neighbour_pressure += this->get_pressure(neighbour.global_index);
                    neighbour_count += 1;
                }

                if (mode == PAvgCalculator::WBPMode::WBP9) {
                    for (const auto& neighbour : conn.diag_neighbours) {
                        neighbour_pressure += this->get_pressure(neighbour.global_index);
                        neighbour_count += 1;
                    }
                }
            }
            if (neighbour_count == 0) {
                if (central_pressure)
                    block_pressure.push_back( central_pressure.value() );
            } else {
                if (central_pressure)
                    block_pressure.push_back( F1 * central_pressure.value() + (1 - F1) * neighbour_pressure / neighbour_count );
                else
                    block_pressure.push_back( neighbour_pressure / neighbour_count );
            }
        } else {
            std::vector<std::pair<double, double>> wp;
            if (mode != PAvgCalculator::WBPMode::WBP4)
                wp.emplace_back(this->porv_pressure(conn.global_index));

            if (mode != PAvgCalculator::WBPMode::WBP) {
                for (const auto& neighbour : conn.rect_neighbours)
                    wp.push_back( this->porv_pressure(neighbour.global_index));

                if (mode == PAvgCalculator::WBPMode::WBP9) {
                    for (const auto& neighbour : conn.diag_neighbours)
                        wp.push_back( this->porv_pressure(neighbour.global_index));
                }
            }
            if (!wp.empty())
                block_pressure.push_back( weighted_avg(wp) );
        }
    }
    return block_pressure;
}



double PAvgCalculator::wbp(PAvgCalculator::WBPMode mode) const {
    const double F2 = this->m_pavg.conn_weight();
    double conn_pressure = 0;
    if (F2 > 0) {
        auto block_pressure = this->block_pressures(mode);
        conn_pressure = this->cf_avg(block_pressure);
    }

    double porv_pressure = 0;
    if (F2 < 1) {
        std::vector<std::pair<double, double>> wp;
        for (const auto& conn : this->m_connections) {

            if (mode != PAvgCalculator::WBPMode::WBP4)
                wp.emplace_back(this->porv_pressure(conn.global_index));

            if (mode != PAvgCalculator::WBPMode::WBP) {
                for (const auto& neighbour : conn.rect_neighbours)
                    wp.push_back( this->porv_pressure(neighbour.global_index));

                if (mode == PAvgCalculator::WBPMode::WBP9) {
                    for (const auto& neighbour : conn.diag_neighbours)
                        wp.push_back( this->porv_pressure(neighbour.global_index));
                }
            }
        }
        porv_pressure = weighted_avg(wp);
    }

    return F2 * conn_pressure + (1 - F2) * porv_pressure;
}

void PAvgCalculator::update(const std::vector<double>& p, const std::vector<char>& m) {
    if (p.size() != this->pressure.size())
        std::logic_error("Wrong size in update");

    for (std::size_t index=0; index < p.size(); index++) {
        if (m[index]) {
            if (this->valid_pressure[index])
                std::logic_error("Internal error - trying to update already valid pressure element");

            this->pressure[index] = p[index];
            this->valid_pressure[index] = 1;
        }
    }
}


void PAvgCalculator::serialize(Serializer& serializer) const {
    serializer.put_vector( this->pressure );
    serializer.put_vector( this->valid_pressure );
}

void PAvgCalculator::update(Serializer& serializer) {
    std::vector<double> p = serializer.get_vector<double>();
    std::vector<char>   v = serializer.get_vector<char>();
    this->update(p,v);
}


}
