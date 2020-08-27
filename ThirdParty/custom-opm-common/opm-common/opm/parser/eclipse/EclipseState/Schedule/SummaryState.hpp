/*
  Copyright 2016 Statoil ASA.

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

#ifndef SUMMARY_STATE_H
#define SUMMARY_STATE_H

#include <chrono>
#include <iosfwd>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Opm{


/*
  The purpose of this class is to serve as a small container object for
  computed, ready to use summary values. The values will typically be used by
  the UDQ, WTEST and ACTIONX calculations. Observe that all value *have been
  converted to the correct output units*.

  The main key used to access the content of this container is the eclipse style
  colon separated string - i.e. 'WWCT:OPX' to get the watercut in well 'OPX'.
  The main usage of the SummaryState class is a temporary holding ground while
  assembling data for the summary output, but it is also used as a context
  object when evaulating the condition in ACTIONX keywords. For that reason some
  of the data is duplicated both in the general structure and a specialized
  structure:

      SummaryState st;

      st.add_well_var("OPX", "WWCT", 0.75);
      st.add("WGOR:OPY", 120);

      // The WWCT:OPX key has been added with the specialized add_well_var()
      // method and this data is available both with the general
      // st.has("WWCT:OPX") and the specialized st.has_well_var("OPX", "WWCT");
      st.has("WWCT:OPX") => True
      st.has_well_var("OPX", "WWCT") => True


      // The WGOR:OPY key is added with the general add("WGOR:OPY") and is *not*
      // accessible through the specialized st.has_well_var("OPY", "WGOR").
      st.has("WGOR:OPY") => True
      st.has_well_var("OPY", "WGOR") => False
*/

class UDQSet;

class SummaryState {
public:
    typedef std::unordered_map<std::string, double>::const_iterator const_iterator;
    explicit SummaryState(std::chrono::system_clock::time_point sim_start_arg);

    /*
      The set() function has to be retained temporarily to support updating of
      cumulatives from restart files.
    */
    void set(const std::string& key, double value);

    bool erase(const std::string& key);
    bool erase_well_var(const std::string& well, const std::string& var);
    bool erase_group_var(const std::string& group, const std::string& var);

    bool has(const std::string& key) const;
    bool has_well_var(const std::string& well, const std::string& var) const;
    bool has_group_var(const std::string& group, const std::string& var) const;

    void update(const std::string& key, double value);
    void update_well_var(const std::string& well, const std::string& var, double value);
    void update_group_var(const std::string& group, const std::string& var, double value);
    void update_elapsed(double delta);
    void update_udq(const UDQSet& udq_set);

    double get(const std::string&) const;
    double get(const std::string&, double) const;
    double get_elapsed() const;
    double get_well_var(const std::string& well, const std::string& var) const;
    double get_group_var(const std::string& group, const std::string& var) const;
    double get_well_var(const std::string& well, const std::string& var, double) const;
    double get_group_var(const std::string& group, const std::string& var, double) const;

    const std::vector<std::string>& wells() const;
    std::vector<std::string> wells(const std::string& var) const;
    const std::vector<std::string>& groups() const;
    std::vector<std::string> groups(const std::string& var) const;
    std::vector<char> serialize() const;
    void deserialize(const std::vector<char>& buffer);
    const_iterator begin() const;
    const_iterator end() const;
    std::size_t num_wells() const;
    std::size_t size() const;
private:
    std::chrono::system_clock::time_point sim_start;
    double elapsed = 0;
    std::unordered_map<std::string,double> values;

    // The first key is the variable and the second key is the well.
    std::unordered_map<std::string, std::unordered_map<std::string, double>> well_values;
    std::unordered_set<std::string> m_wells;
    mutable std::optional<std::vector<std::string>> well_names;

    // The first key is the variable and the second key is the group.
    std::unordered_map<std::string, std::unordered_map<std::string, double>> group_values;
    std::unordered_set<std::string> m_groups;
    mutable std::optional<std::vector<std::string>> group_names;
};


std::ostream& operator<<(std::ostream& stream, const SummaryState& st);

}
#endif
