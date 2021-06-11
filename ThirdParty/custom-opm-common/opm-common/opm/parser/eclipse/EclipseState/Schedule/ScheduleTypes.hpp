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

#ifndef OPM_SCHEDULE_TYPES_HPP
#define OPM_SCHEDULE_TYPES_HPP

#include <string>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>

namespace Opm {

enum class InjectorType {
    WATER = 1,
    GAS = 2,
    OIL = 3,
    MULTI = 4
};
const std::string InjectorType2String( InjectorType enumValue );
InjectorType InjectorTypeFromString( const std::string& stringValue );


class WellType {
public:
    WellType(int ecl_wtype, int welspecs_phase);
    WellType(bool producer, Phase welspecs_phase);
    explicit WellType(Phase welspecs_phase);
    WellType() = default;

    static WellType serializeObject();

    bool injector() const;
    bool producer() const;
    bool update(InjectorType injector_type);
    bool update(bool producer);

    static bool oil_injector(int ecl_wtype);
    static bool gas_injector(int ecl_wtype);
    static bool water_injector(int ecl_wtype);
    static bool producer(int ecl_wtype);

    int   ecl_wtype() const;
    int   ecl_phase() const;
    Phase preferred_phase() const;
    InjectorType injector_type() const;
    bool operator==(const WellType& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_producer);
        serializer(injection_phase);
        serializer(m_welspecs_phase);
    }

private:
    bool  m_producer;
    /*
      The injection_phase member is updated during the course of the simulation;
      following each WCONINJE keyword the injection phase is updated. If an
      producer is specified in the constructor the injection_phase is
      initialzied to the welspecs phase. This is not wildly random - but the
      injection_phase will not be meaningfull before an update(Phase) call has been
      issued.

      The welspecs_phase is the preferred phase specified when the well is
      defined with the WELSPECS keyword. This member is immutable, and it is only
      used when initializing the well equations for a producer.
    */

    Phase injection_phase;
    Phase m_welspecs_phase;
};

}



#endif
