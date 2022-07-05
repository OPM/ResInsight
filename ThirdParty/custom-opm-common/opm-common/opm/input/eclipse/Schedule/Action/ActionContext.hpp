/*
  Copyright 2018 Equinor ASA.

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


#ifndef ActionContext_HPP
#define ActionContext_HPP

#include <string>
#include <map>

#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Well/WListManager.hpp>

namespace Opm {
namespace Action {

/*
  The Action::Context class is used as context when the ACTIONX condition is
  evaluated. The Action::Context class is mainly just a thin wrapper around the
  SummaryState class.
*/

class Context {
public:
    explicit Context(const SummaryState& summary_state, const WListManager& wlm);

    /*
      The get methods will first check the internal storage in the 'values' map
      and then subsequently query the SummaryState member.
    */
    double get(const std::string& func, const std::string& arg) const;
    void   add(const std::string& func, const std::string& arg, double value);

    double get(const std::string& func) const;
    void   add(const std::string& func, double value);

    std::vector<std::string> wells(const std::string& func) const;
    const WListManager& wlist_manager() const;

private:
    const SummaryState& summary_state;
    const WListManager& wlm;
    std::map<std::string, double> values;
};
}
}
#endif
