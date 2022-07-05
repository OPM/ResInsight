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


#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>
#include <chrono>

#include <fmt/format.h>

#include <opm/io/eclipse/ERsm.hpp>
#include <opm/io/eclipse/ESmry.hpp>
#include <opm/common/utility/FileSystem.hpp>
#include <opm/common/utility/String.hpp>
#include <opm/common/utility/numeric/cmp.hpp>
#include <opm/common/utility/TimeService.hpp>
#include <opm/output/eclipse/WStat.hpp>

namespace Opm {
namespace EclIO {

namespace {

constexpr std::size_t num_columns  = 10;
constexpr std::size_t column_width = 13;


std::deque<std::string> load(const std::string& fname) {
    std::deque<std::string> lines;
    std::ifstream is(fname.c_str());
    if (!is.good())
        throw std::invalid_argument("Can not open: " + fname + " for reading");

    std::string line;
    while(std::getline(is, line)) {
        if (line.back() == '\r')
            lines.push_back(line.substr(0, line.size() - 1));
        else
            lines.push_back(line);
    }

    return lines;
}

std::vector<std::string> split_line(const std::string& line) {
    std::vector<std::string> tokens;
    for (std::size_t column = 0; column < num_columns; column++) {
        if (column * column_width >= line.size())
            break;
        tokens.push_back( trim_copy(line.substr(column*column_width, column_width) ));
    }
    return tokens;
}


bool block_start(const std::string& line) {
    if (line.empty())
        return false;

    if (line[0] != '1')
        return false;

    if (line.find_first_not_of(' ', 1) != std::string::npos)
        return false;

    return true;
}

std::string pop_return(std::deque<std::string>& lines) {
    auto front = lines.front();
    lines.pop_front();
    return front;
}

void pop_separator(std::deque<std::string>& lines) {
    lines.pop_front();
}

int make_num(const std::string& nums_string) {
    if (nums_string.empty())
        return 0;

    return std::stoi(nums_string);
}

TimeStampUTC make_timestamp(const std::string& date_string) {
    const auto& month_index = TimeService ::eclipseMonthIndices();
    auto dash_pos1 = date_string.find('-');
    auto dash_pos2 = date_string.rfind('-');
    auto day = std::stoi( date_string.substr(0, dash_pos1 ) );
    auto year = std::stoi( date_string.substr(dash_pos2 + 1) );
    auto month_name = date_string.substr( dash_pos1 + 1, 3);

    return TimeStampUTC(year, month_index.at(month_name), day);
}


/*
  The multiplier line is optional; so when looking for the multiplier line in
  the text we must make sure that line we are looking at is not the WGNAMES line
  - including the possibility of a totally empty WGNAMES line.
*/
std::vector<double> make_multiplier(std::deque<std::string>& lines) {
    std::vector<double> multiplier = {1,1,1,1,1,1,1,1,1,1};
    if (lines.front().find_first_not_of("-0123456789* ") != std::string::npos)
        return multiplier;

    if (lines.front().find_first_not_of(" ") == std::string::npos)
        return multiplier;

    auto mult_list = split_line(pop_return(lines));
    for (std::size_t index=0; index < mult_list.size(); index++) {
        const auto& mult_string = mult_list[index];
        if (mult_string.empty())
            continue;

        auto power_pos = mult_string.find("**");
        if (power_pos == std::string::npos)
            throw std::invalid_argument("Multiplier item wrong format: " + mult_string);

        double power = std::stod(mult_string.c_str() + power_pos + 2);
        multiplier[index] = std::pow(10, power);
    }

    return multiplier;
}

double convert_wstat(const std::string& symbolic_wstat) {
    static const std::unordered_map<std::string, int> wstat_map = {
        {Opm::WStat::symbolic::UNKNOWN, Opm::WStat::numeric::UNKNOWN},
        {Opm::WStat::symbolic::PROD,    Opm::WStat::numeric::PROD},
        {Opm::WStat::symbolic::INJ,     Opm::WStat::numeric::INJ},
        {Opm::WStat::symbolic::SHUT,    Opm::WStat::numeric::SHUT},
        {Opm::WStat::symbolic::STOP,    Opm::WStat::numeric::STOP},
        {Opm::WStat::symbolic::PSHUT,   Opm::WStat::numeric::PSHUT},
        {Opm::WStat::symbolic::PSTOP,   Opm::WStat::numeric::PSTOP},
    };
    return static_cast<double>(wstat_map.at(symbolic_wstat));
}

}

void ERsm::load_block(std::deque<std::string>& lines, std::size_t& vector_length) {
    if (!block_start(lines.front()))
        throw std::invalid_argument("Block should start with '1' in first column");

    lines.pop_front();
    pop_separator(lines);
    lines.pop_front();
    pop_separator(lines);

    auto kw_list = split_line(pop_return(lines));
    auto unit_list = split_line(pop_return(lines));
    auto mult_list = make_multiplier(lines);
    auto wgnames = split_line(pop_return(lines));
    auto nums_list = split_line(pop_return(lines));
    pop_separator(lines);
    std::size_t num_rows = std::count_if(kw_list.begin(), kw_list.end(), [](const std::string& kw) { return !kw.empty();}) - 1;

    if (vector_length == 0) {
        if (kw_list[0] == "DATE")
            this->time = std::vector<TimeStampUTC>();
        else if (kw_list[0] == "TIME") {
            if (unit_list[0] != "DAYS")
                throw std::invalid_argument("Only days is supported as time unit");
            this->time = std::vector<double>();
        }
        else
            throw std::invalid_argument("The first column must be DATE or TIME");
    }

    std::vector<ERsm::Vector> block_data;
    for (std::size_t kw_index = 1; kw_index < kw_list.size(); kw_index++) {
        auto node = SummaryNode{ kw_list[kw_index],
                                 SummaryNode::category_from_keyword(kw_list[kw_index]),
                                 SummaryNode::Type::Undefined,
                                 wgnames[kw_index],
                                 make_num(nums_list[kw_index]),
                                 "",
                                 {}
        };
        block_data.emplace_back( node, vector_length );
    }

    std::size_t block_size = 0;
    while (true) {
        if (lines.empty())
            break;

        if (block_start(lines.front()))
            break;

        auto data_row = split_line(pop_return(lines));
        for (std::size_t data_index = 0; data_index < num_rows; data_index++) {
            const auto& keyword = kw_list[data_index + 1];
            double value;
            if (keyword == "WSTAT")
                value = convert_wstat(data_row[data_index + 1]);
            else
                value = std::stod(data_row[data_index + 1]) * mult_list[data_index + 1];
            block_data[data_index].data.push_back(value);
        }

        if (vector_length == 0) {
            if (std::holds_alternative<std::vector<double>>(this->time)) {
                double d = std::stod(data_row[0]) * mult_list[0];
                std::get<std::vector<double>>( this->time ).push_back( d );
            } else {
                TimeStampUTC ts = make_timestamp(data_row[0]);
                std::get<std::vector<TimeStampUTC>>( this->time ).push_back( ts );
            }
        }
        block_size += 1;
    }
    if (vector_length == 0)
        vector_length = block_size;

    if (vector_length != block_size)
        throw std::invalid_argument("Block size error");

    for (auto& v : block_data)
        this->vectors.insert(std::make_pair<std::string, ERsm::Vector>( v.header.unique_key(), std::move(v)));
}


const std::vector<TimeStampUTC>& ERsm::dates() const {
    if (std::holds_alternative<std::vector<double>>(this->time))
        throw std::invalid_argument("This ERsm instance has time given as days");

    return std::get<std::vector<TimeStampUTC>>( this->time );
}

const std::vector<double>& ERsm::days() const {
    if (!std::holds_alternative<std::vector<double>>(this->time))
        throw std::invalid_argument("This ERsm instance has time given as dates");

    return std::get<std::vector<double>>( this->time );
}

const std::vector<double>& ERsm::get(const std::string& key) const {
    const auto& vector = this->vectors.at(key);
    return vector.data;
}

bool ERsm::has_dates() const {
    return std::holds_alternative<std::vector<TimeStampUTC>>(this->time);
}

bool ERsm::has(const std::string& key) const {
    return this->vectors.count(key) == 1;
}

ERsm::ERsm(const std::string& fname) {
    auto lines = load(fname);
    std::size_t vector_length = 0;
    while (!lines.empty())
        load_block(lines, vector_length);
}



bool cmp(const ESmry& smry, const ERsm& rsm) {
    const auto& summary_dates = smry.dates();
    if (rsm.has_dates()) {
        const auto& rsm_dates = rsm.dates();
        if (rsm_dates.size() != summary_dates.size()) {
            fmt::print(stderr, "len(summary) = {}    len(rsm) = {}", summary_dates.size(), rsm_dates.size());
            return false;
        }

        for (std::size_t time_index = 0; time_index < rsm_dates.size(); time_index++) {
            const auto smry_ts = TimeStampUTC( std::chrono::system_clock::to_time_t(summary_dates[time_index]) );
            const auto rsm_ts = rsm_dates[time_index];

            if (smry_ts.year() != rsm_ts.year()) {
                fmt::print(stderr, "time_index: {}  summary.year: {}   rsm.year: {}\n", time_index, smry_ts.year(), rsm_ts.year());
                return false;
            }

            if (smry_ts.month() != rsm_ts.month()) {
                fmt::print(stderr, "time_index: {}  summary.month: {}   rsm.month: {}\n", time_index, smry_ts.month(), rsm_ts.month());
                return false;
            }

            if (smry_ts.day() != rsm_ts.day()) {
                fmt::print(stderr, "time_index: {}  summary.day: {}   rsm.day: {}\n", time_index, smry_ts.day(), rsm_ts.day());
                return false;
            }
        }
    } else {
        const auto& rsm_days = rsm.days();
        if (rsm_days.size() != summary_dates.size())
            return false;

        for (std::size_t time_index = 0; time_index < rsm_days.size(); time_index++) {
            auto smry_days = std::chrono::duration_cast<std::chrono::seconds>(summary_dates[time_index] - smry.startdate()).count() / 86400.0 ;

            if (!cmp::scalar_equal(smry_days, rsm_days[time_index])) {
                fmt::print(stderr, "time_index: {}  summary.days: {}   rsm.days: {}\n", time_index, smry_days, rsm_days[time_index]);
                return false;
            }
        }
    }

    for (const auto& node : smry.summaryNodeList()) {
        const auto& key = node.unique_key();

        if (key == "TIME")
            continue;

        if (key == "DAY")
            continue;

        if (key == "MONTH")
            continue;

        if (key == "YEAR")
            continue;

        /*
          The ESmry class and the ERsm class treat block vector keys
          differently, the ESmry class uses only the key Bxxx:i,j,k whereas the
          ERsm class uses only the key Bxxxx:g. Therefor the ESmry lookup is
          based on nodes, whereas the ERsm lookup is based on key.
        */
        const auto& smry_vector = smry.get(node);
        const auto& rsm_vector = rsm.get(key);
        for (std::size_t index = 0; index < smry_vector.size(); index++) {
            const auto smry_value = static_cast<double>(smry_vector[index]);
            const auto rsm_value = rsm_vector[index];
            const double diff = std::fabs(smry_value - rsm_value);
            if (std::fabs(rsm_value) < 1e-3) {
                const double zero_eps = 1e-4;
                if (diff > zero_eps) {
                    fmt::print(stderr, "time_index: {}  key: {}  summary: {}   rsm: {}\n", index, key, smry_value, rsm_value);
                    return false;
                }
            } else {
                const double eps  = 5e-5;
                const double sum = std::fabs(smry_value) + std::fabs(rsm_value);

                if (diff > eps * std::max(1.0, sum)) {
                    fmt::print(stderr, "time_index: {}  key: {}  summary: {}   rsm: {}\n", index, key, smry_value, rsm_value);
                    return false;
                }
            }
        }
    }

    return true;
}

}
}
