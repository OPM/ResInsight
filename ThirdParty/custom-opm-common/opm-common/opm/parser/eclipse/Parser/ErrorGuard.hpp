/*
  Copyright 2019 Joakim Hove/datagr

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


#ifndef ERROR_GUARD_HPP
#define ERROR_GUARD_HPP

#include <string>
#include <vector>

namespace Opm {

class ErrorGuard {
public:
    void addError(const std::string& errorKey, const std::string& msg);
    void addWarning(const std::string& errorKey, const std::string &msg);
    void clear();

    explicit operator bool() const { return !this->error_list.empty(); }

    /*
      Observe that this desctructor has a somewhat special semantics. If there
      are errors in the error list it will print all warnings and errors on
      stderr and throw std::runtime_error.
    */
    ~ErrorGuard();
    void terminate() const;
    void dump() const;

private:

    std::vector<std::pair<std::string, std::string>> error_list;
    std::vector<std::pair<std::string, std::string>> warning_list;
};

}

#endif
