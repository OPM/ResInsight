#include <opm/input/eclipse/Parser/ParserKeywords/U.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_UDADIMS() { return UDADIMS(); }
const ::Opm::ParserKeyword Builtin::get_UDQ() { return UDQ(); }
const ::Opm::ParserKeyword Builtin::get_UDQDIMS() { return UDQDIMS(); }
const ::Opm::ParserKeyword Builtin::get_UDQPARAM() { return UDQPARAM(); }
const ::Opm::ParserKeyword Builtin::get_UDT() { return UDT(); }
const ::Opm::ParserKeyword Builtin::get_UDTDIMS() { return UDTDIMS(); }
const ::Opm::ParserKeyword Builtin::get_UNCODHMD() { return UNCODHMD(); }
const ::Opm::ParserKeyword Builtin::get_UNIFIN() { return UNIFIN(); }
const ::Opm::ParserKeyword Builtin::get_UNIFOUT() { return UNIFOUT(); }
const ::Opm::ParserKeyword Builtin::get_UNIFOUTS() { return UNIFOUTS(); }
const ::Opm::ParserKeyword Builtin::get_UNIFSAVE() { return UNIFSAVE(); }
const ::Opm::ParserKeyword Builtin::get_USECUPL() { return USECUPL(); }
const ::Opm::ParserKeyword Builtin::get_USEFLUX() { return USEFLUX(); }
const ::Opm::ParserKeyword Builtin::get_USENOFLO() { return USENOFLO(); }

void Builtin::emplaceU() const {
    this->keywords.emplace("UDADIMS", UDADIMS());
    this->keywords.emplace("UDQ", UDQ());
    this->keywords.emplace("UDQDIMS", UDQDIMS());
    this->keywords.emplace("UDQPARAM", UDQPARAM());
    this->keywords.emplace("UDT", UDT());
    this->keywords.emplace("UDTDIMS", UDTDIMS());
    this->keywords.emplace("UNCODHMD", UNCODHMD());
    this->keywords.emplace("UNIFIN", UNIFIN());
    this->keywords.emplace("UNIFOUT", UNIFOUT());
    this->keywords.emplace("UNIFOUTS", UNIFOUTS());
    this->keywords.emplace("UNIFSAVE", UNIFSAVE());
    this->keywords.emplace("USECUPL", USECUPL());
    this->keywords.emplace("USEFLUX", USEFLUX());
    this->keywords.emplace("USENOFLO", USENOFLO());
}
} }
