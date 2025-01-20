#include <opm/input/eclipse/Parser/ParserKeywords/Z.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_ZCORN() { return ZCORN(); }
const ::Opm::ParserKeyword Builtin::get_ZFACT1() { return ZFACT1(); }
const ::Opm::ParserKeyword Builtin::get_ZFACT1S() { return ZFACT1S(); }
const ::Opm::ParserKeyword Builtin::get_ZFACTOR() { return ZFACTOR(); }
const ::Opm::ParserKeyword Builtin::get_ZFACTORS() { return ZFACTORS(); }
const ::Opm::ParserKeyword Builtin::get_ZIPP2OFF() { return ZIPP2OFF(); }
const ::Opm::ParserKeyword Builtin::get_ZIPPY2() { return ZIPPY2(); }
const ::Opm::ParserKeyword Builtin::get_ZMF() { return ZMF(); }
const ::Opm::ParserKeyword Builtin::get_ZMFVD() { return ZMFVD(); }

void Builtin::emplaceZ() const {
    this->keywords.emplace("ZCORN", ZCORN());
    this->keywords.emplace("ZFACT1", ZFACT1());
    this->keywords.emplace("ZFACT1S", ZFACT1S());
    this->keywords.emplace("ZFACTOR", ZFACTOR());
    this->keywords.emplace("ZFACTORS", ZFACTORS());
    this->keywords.emplace("ZIPP2OFF", ZIPP2OFF());
    this->keywords.emplace("ZIPPY2", ZIPPY2());
    this->keywords.emplace("ZMF", ZMF());
    this->keywords.emplace("ZMFVD", ZMFVD());
}
} }
