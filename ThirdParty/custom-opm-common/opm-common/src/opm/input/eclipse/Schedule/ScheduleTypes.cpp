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
#include <stdexcept>

#include <opm/input/eclipse/Schedule/ScheduleTypes.hpp>

namespace Opm {

namespace ecl {

static constexpr int producer       = 1;
static constexpr int oil_injector   = 2;
static constexpr int water_injector = 3;
static constexpr int gas_injector   = 4;


static constexpr int oil_phase      = 1;
static constexpr int water_phase    = 2;
static constexpr int gas_phase      = 3;
static constexpr int liquid_phase   = 4;


Phase from_ecl_phase(int ecl_phase) {
    switch(ecl_phase) {
    case ecl::oil_phase:
        return Phase::OIL;
    case ecl::water_phase:
        return Phase::WATER;
    case ecl::gas_phase:
        return Phase::GAS;
    case ecl::liquid_phase:
        throw std::logic_error("Sorry wells with preferred phase:LIQUID is not supported");
    default:
        throw std::invalid_argument("Invalid integer well phase");
    }
}
}


namespace {

Phase from_injector_type(InjectorType injector_type) {
    switch (injector_type) {
    case InjectorType::WATER:
        return Phase::WATER;
    case InjectorType::GAS:
        return Phase::GAS;
    case InjectorType::OIL:
        return Phase::OIL;
    default:
        throw std::logic_error("Unhandled injector type");
    }
}

}


bool WellType::producer(int ecl_wtype) {
    return ecl_wtype == ecl::producer;
}

bool WellType::oil_injector(int ecl_wtype) {
    return ecl_wtype == ecl::oil_injector;
}

bool WellType::water_injector(int ecl_wtype) {
    return ecl_wtype == ecl::water_injector;
}

bool WellType::gas_injector(int ecl_wtype) {
    return ecl_wtype == ecl::gas_injector;
}

Phase WellType::injection_phase() const {
    if (this->m_producer)
        throw std::logic_error("Asked for injection phase in a producer");

    return this->m_injection_phase;
}



WellType::WellType(int ecl_wtype, int ecl_phase) :
    m_injection_phase(ecl::from_ecl_phase(ecl_phase)),
    m_welspecs_phase(ecl::from_ecl_phase(ecl_phase))
{
    this->m_producer = false;
    switch (ecl_wtype) {
    case ecl::producer:
        this->m_producer = true;
        break;
    case ecl::oil_injector:
        this->m_injection_phase = Phase::OIL;
        break;
    case ecl::water_injector:
        this->m_injection_phase = Phase::WATER;
        break;
    case ecl::gas_injector:
        this->m_injection_phase = Phase::GAS;
        break;
    default:
        throw std::invalid_argument("Invalid integer well type ID");
    }

}

WellType::WellType(bool producer, Phase phase) :
    m_producer(producer),
    m_injection_phase(phase),
    m_welspecs_phase(phase)
{}

WellType::WellType(Phase phase) :
    WellType(true, phase)
{}

WellType WellType::serializeObject()
{
    WellType result;
    result.m_producer = true;
    result.m_injection_phase = Phase::OIL;
    result.m_welspecs_phase = Phase::WATER;

    return result;
}

bool WellType::update(bool producer_arg) {
    if (this->m_producer != producer_arg) {
        this->m_producer = producer_arg;
        return true;
    } else
        return false;
}

bool WellType::update(InjectorType injector_type) {
    bool ret_value = false;
    if (this->m_producer) {
        this->m_producer = false;
        ret_value = true;
    }

    auto inj_phase = from_injector_type(injector_type);
    if (this->m_injection_phase != inj_phase) {
        this->m_injection_phase = inj_phase;
        ret_value = true;
    }

    return ret_value;
}

bool WellType::producer() const {
    return this->m_producer;
}

bool WellType::injector() const {
    return !this->m_producer;
}

int WellType::ecl_wtype() const {
    if (this->m_producer)
        return ecl::producer;

    switch (this->m_injection_phase) {
    case Phase::OIL:
        return ecl::oil_injector;
    case Phase::WATER:
        return ecl::water_injector;
    case Phase::GAS:
        return ecl::gas_injector;
    default:
        throw std::logic_error("Internal error - should not be here");
    }
}

/*
  The enum Runspec::Phase is maybe not very well suited; it has lots of 'extra'
  phases like ENERGY and BRINE, and at the same time it is missing the phase
  LIQUID which should map to ecl value 4.
*/

int WellType::ecl_phase() const {
    switch (this->m_welspecs_phase) {
    case Phase::OIL:
        return ecl::oil_phase;
    case Phase::WATER:
        return ecl::water_phase;
    case Phase::GAS:
        return ecl::gas_phase;
    default:
        throw std::logic_error("Member has invalid phase");
    }
}


Phase WellType::preferred_phase() const {
    return this->injector() ? this->m_injection_phase : this->m_welspecs_phase;
}


bool WellType::operator==(const WellType& other) const {
    return this->m_welspecs_phase == other.m_welspecs_phase &&
           this->m_injection_phase == other.m_injection_phase &&
           this->m_producer == other.m_producer;
}


InjectorType WellType::injector_type() const {
    if (this->producer())
        throw std::invalid_argument("Asked for injector type for a well which is a producer");

    switch (this->m_injection_phase) {
    case Phase::OIL:
        return InjectorType::OIL;
    case Phase::WATER:
        return InjectorType::WATER;
    case Phase::GAS:
        return InjectorType::GAS;
    default:
        throw std::logic_error("Member has invalid phase");
    }

}


const std::string InjectorType2String( InjectorType enumValue ) {
    switch( enumValue ) {
    case InjectorType::OIL:
        return "OIL";
    case InjectorType::GAS:
        return "GAS";
    case InjectorType::WATER:
        return "WATER";
    case InjectorType::MULTI:
        return "MULTI";
    default:
        throw std::invalid_argument("unhandled enum value");
    }
}

InjectorType InjectorTypeFromString( const std::string& stringValue ) {
    if (stringValue == "OIL")
        return InjectorType::OIL;
    else if (stringValue == "WATER")
        return InjectorType::WATER;
    else if (stringValue == "WAT")
        return InjectorType::WATER;
    else if (stringValue == "GAS")
        return InjectorType::GAS;
    else if (stringValue == "MULTI")
        return InjectorType::MULTI;
    else
        throw std::invalid_argument("Unknown enum state string: " + stringValue );
}}
