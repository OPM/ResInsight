/*
  Copyright 2015 Statoil ASA.

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


#ifndef OPM_LOGBACKEND_HPP
#define OPM_LOGBACKEND_HPP

#include <cstdint>
#include <string>

namespace Opm {

class LogBackend {

public:
    LogBackend( int64_t mask );
    virtual ~LogBackend() { };
    virtual void addMessage(int64_t , const std::string& ) { };

    int64_t getMask() const;
    bool    includeMessage(int64_t messageFlag);

private:
    int64_t m_mask;
};
}


#endif
