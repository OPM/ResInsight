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


#ifndef PAVE_CALC_HPP
#define PAVE_CALC_HPP

#include <functional>
#include <map>
#include <optional>
#include <vector>

#include <opm/input/eclipse/Schedule/Well/PAvg.hpp>
#include <opm/input/eclipse/Schedule/Well/Connection.hpp>

namespace Opm {

class WellConnections;
class EclipseGrid;
class Serializer;
class PAvgCalculator {
public:

    PAvgCalculator(const std::string& well, double well_ref_depth, const EclipseGrid& grid, const std::vector<double>& porv, const WellConnections& connections, const PAvg& pavg);

    enum class WBPMode {
        WBP,
        WBP4,
        WBP5,
        WBP9
    };


    struct Neighbour {
        Neighbour(double porv_arg, std::size_t index_arg) :
            porv(porv_arg),
            global_index(index_arg)
        {}

        double porv;
        std::size_t global_index;
    };


    struct Connection {
        Connection(double porv_arg, double cf, ::Opm::Connection::Direction dir_arg, std::size_t index_arg) :
            porv(porv_arg),
            cfactor(cf),
            dir(dir_arg),
            global_index(index_arg)
        {
        }

        double porv;
        double cfactor;
        ::Opm::Connection::Direction dir;
        std::size_t global_index;
        std::vector<Neighbour> rect_neighbours;
        std::vector<Neighbour> diag_neighbours;
    };


    const std::string& wname() const;
    double wbp() const;
    double wbp4() const;
    double wbp5() const;
    double wbp9() const;
    double wbp(WBPMode mode) const;
    bool add_pressure(std::size_t global_index, double pressure);
    void update(Serializer& serializer);
    const std::vector< std::size_t >& index_list() const;
    std::pair< std::reference_wrapper<const std::vector<double>>, std::reference_wrapper<const std::vector<bool>> > data() const;
    void serialize(Serializer& serializer) const;

private:
    void update(const std::vector<double>& p, const std::vector<char>& m);
    void add_connection(const PAvgCalculator::Connection& conn);
    void add_neighbour(std::size_t global_index, std::optional<PAvgCalculator::Neighbour> neighbour, bool rect_neighbour);
    double get_pressure(std::size_t global_index) const;
    double cf_avg(const std::vector<std::optional<double>>& block_pressure) const;
    std::pair<double,double> porv_pressure(std::size_t global_index) const;
    std::vector<std::optional<double>> block_pressures(PAvgCalculator::WBPMode mode) const;

    std::string well_name;
    PAvg m_pavg;
    std::vector<Connection> m_connections;
    std::map<std::size_t, std::size_t> m_index_map;
    std::vector<std::size_t> m_index_list;
    std::vector<double> pressure;
    std::vector<char> valid_pressure;
    double ref_depth;
};

}
#endif
