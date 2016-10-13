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
#ifndef WELLSET_HPP
#define WELLSET_HPP

#include <map>
#include <memory>
#include <string>

namespace Opm {

    class Well;

    class WellSet {
    public:
        using const_iterator = std::map< std::string, Well* >::const_iterator;

        WellSet();
        size_t size() const;
        bool hasWell(const std::string& wellName) const;
        const Well* getWell(const std::string& wellName) const;
        void addWell( Well* well);
        void delWell(const std::string& wellName);
        WellSet * shallowCopy() const;

        const_iterator begin() const;
        const_iterator end() const;

    private:
        std::map<std::string, Well* > m_wells;
    };
}

#endif
