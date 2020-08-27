/*
  Copyright 2020 Equinor ASA.

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

#ifndef AICD_HPP_HEADER_INCLUDED
#define AICD_HPP_HEADER_INCLUDED

#include <map>
#include <utility>
#include <vector>
#include <string>

#include <opm/parser/eclipse/EclipseState/Schedule/MSW/SICD.hpp>

namespace Opm {

    class DeckRecord;
    class DeckKeyword;

    class AutoICD : public SICD {
    public:
        AutoICD() = default;
        AutoICD(const DeckRecord& record);

        static AutoICD serializeObject();

        // the function will return a map
        // [
        //     "WELL1" : [<seg1, aicd1>, <seg2, aicd2> ...]
        //     ....
        static std::map<std::string, std::vector<std::pair<int, AutoICD> > >
        fromWSEGAICD(const DeckKeyword& wsegaicd);

        bool operator==(const AutoICD& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            AutoICD::serializeOp(serializer);
        }

    private:
        double m_flow_rate_exponent;
        double m_visc_exponent;
        double m_oil_density_exponent;
        double m_water_density_exponent;
        double m_gas_density_exponent;
        double m_oil_viscosity_exponent;
        double m_water_viscosity_exponent;
        double m_gas_viscosity_exponent;
    };
}

#endif
