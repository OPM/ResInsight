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

#include <cstdint>
#include <opm/common/OpmLog/LogBackend.hpp>

namespace Opm {

    LogBackend::LogBackend( int64_t mask ) :
        m_mask(mask)
    {
    }


    bool LogBackend::includeMessage(int64_t messageFlag) {
        if (((messageFlag & m_mask) == messageFlag) &&
            (messageFlag > 0))
            return true;
        else
            return false;
    }

    int64_t LogBackend::getMask() const {
        return m_mask;
    }


}
