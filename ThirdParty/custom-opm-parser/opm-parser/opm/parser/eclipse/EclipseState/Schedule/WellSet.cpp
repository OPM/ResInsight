/*
  Copyright 2013 Statoil ASA.

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

#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellSet.hpp>

namespace Opm {


    WellSet::WellSet() {
    }

    size_t WellSet::size() const {
        return m_wells.size();
    }


    bool WellSet::hasWell(const std::string& wellName) const {
        return (m_wells.find(wellName) != m_wells.end());
    }


    const Well* WellSet::getWell(const std::string& wellName) const {
        if (hasWell(wellName))
            return m_wells.find(wellName)->second;
        else
            throw std::invalid_argument("Do not have well '"+wellName+"'?!\n");
    }


    void WellSet::addWell( Well* well) {
        const std::string& wellName = well->name();
        if (!hasWell(wellName))
            m_wells[wellName] = well;
        else if (well != getWell(wellName))
            throw std::invalid_argument("Well has changed - internal fuckup");
    }


    void WellSet::delWell(const std::string& wellName) {
        if (hasWell(wellName))
            m_wells.erase( wellName );
        else
            throw std::invalid_argument("Cannot delete unknown well '"+wellName+"'");
    }


    WellSet * WellSet::shallowCopy() const {
        std::unique_ptr< WellSet > copy( new WellSet() );

        for( auto pair : this->m_wells )
            copy->addWell( pair.second );

        return copy.release();
    }

    WellSet::const_iterator WellSet::begin() const {
        return this->m_wells.begin();
    }

    WellSet::const_iterator WellSet::end() const {
        return this->m_wells.end();
    }
}
