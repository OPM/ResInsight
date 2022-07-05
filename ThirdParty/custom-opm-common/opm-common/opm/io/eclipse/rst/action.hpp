/*
  Copyright 2021 Equinor ASA.

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

#ifndef RST_ACTIONX
#define RST_ACTIONX

#include <ctime>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <opm/input/eclipse/Schedule/Action/Enums.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

namespace Opm {

namespace RestartIO {

struct RstAction {
    struct Quantity {
        std::variant<std::string, double> quantity;
        std::optional<std::string> wgname;

        Quantity() = default;
        Quantity(const std::string * zacn, double sacn_value);
        Quantity(const std::string& quantity);
        Quantity(double value);
    };


    struct Condition {
        static bool valid(const std::string * zacn, const int * iacn);
        Condition(const std::string * zacn, const int * iacn, const double * sacn);
        Action::Logical logic;
        Action::Comparator cmp_op;
        Quantity lhs;
        Quantity rhs;
        bool left_paren{false};
        bool right_paren{false};

        std::vector<std::string> tokens() const;
    };


    RstAction(const std::string& name_arg, int max_run_arg, int run_count_arg, double min_wait_arg, std::time_t start_time, std::time_t last_run, std::vector<Condition> conditions_arg);

    std::string name;
    int max_run;
    int run_count;
    double min_wait;
    std::time_t start_time;
    std::optional<std::time_t> last_run;
    std::vector<Condition> conditions;
    std::vector<DeckKeyword> keywords;
};

}
}



#endif
