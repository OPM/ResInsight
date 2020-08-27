/*
  Copyright 2019 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  but eliminates the memory saving DynamicState is intended to enable. You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ACTION_RESULT_HPP
#define ACTION_RESULT_HPP

#include <string>
#include <unordered_set>
#include <vector>
#include <memory>

namespace Opm {
namespace Action {

/*
  The Action::Result class is used to hold the boolean result of a ACTIONX
  condition like:

        WWCT > 0.75

   and also the final evaluation of an ACTIONX condition comes as a
   Action::Result instance.


   In addition to the overall truthness of the expression an Action::Result
   instance can optionally have a set of matching wells. For instance the
   the result of:

        FWCT > 0.75

   Will not contain any wells, whereas the result of:

        WWCT > 0.75

   will contain a set of all the wells matching the condition. The set of
   matching wells can be queries with the wells() method. When a result with
   wells and a result without wells are combined as:

        WWCT > 0.50 AND FPR > 1000

   The result will have all the wells corresponding to the first condition, when
   several results with wells are combined with logical operators AND and OR the
   set of matching wells are combined correspondingly. It is actually possible
   to have a result which evaluates to true and still have and zero matching
   wells - e.g.

       WWCT > 1.25 OR FOPT > 1

   If the condition evaluates to true the set of matching wells will be passed
   to the Schedule::applyAction() method, and will be used in place of '?' in
   keywords like WELOPEN.
*/


class WellSet {
public:
    WellSet() = default;
    explicit WellSet(const std::vector<std::string>& wells);
    void add(const std::string& well);

    std::size_t size() const;
    std::vector<std::string> wells() const;
    bool contains(const std::string& well) const;

    WellSet& intersect(const WellSet& other);
    WellSet& add(const WellSet& other);
private:
    std::unordered_set<std::string> well_set;
};



class Result {
public:
    explicit Result(bool result_arg);
    Result(bool result_arg, const std::vector<std::string>& wells);
    Result(bool result_arg, const WellSet& wells);
    Result(const Result& src);

    explicit operator bool() const;
    std::vector<std::string> wells() const;

    bool has_well(const std::string& well) const;

    void add_well(const std::string& well);

    Result& operator|=(const Result& other);
    Result& operator=(const Result& src);
    Result& operator&=(const Result& other);

private:
    void assign(bool value);
    bool result;
    /*
      The set of matching wells is implemented with pointer semantics to be able
      to differentiate between an empty set of wells - like all the wells with
      WWCT > 1, and a result set which does not have well information at all -
      e.g. FOPR > 0.
    */
    std::unique_ptr<WellSet> matching_wells;
};

}
}
#endif
