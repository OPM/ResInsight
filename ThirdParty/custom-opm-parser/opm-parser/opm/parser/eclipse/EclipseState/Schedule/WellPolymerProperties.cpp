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
#include <opm/parser/eclipse/EclipseState/Schedule/WellPolymerProperties.hpp>

#include <string>
#include <vector>

namespace Opm {

    WellPolymerProperties::WellPolymerProperties() {
        m_polymerConcentration = 0.0;
        m_saltConcentration = 0.0;
    }

    bool WellPolymerProperties::operator==(const WellPolymerProperties& other) const {
        if ((m_polymerConcentration == other.m_polymerConcentration) &&
            (m_saltConcentration == other.m_saltConcentration))
            return true;
        else
            return false;

    }

    bool WellPolymerProperties::operator!=(const WellPolymerProperties& other) const {
        return !(*this == other);
    }
}
