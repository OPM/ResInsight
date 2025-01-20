#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_ACF() { return ACF(); }
const ::Opm::ParserKeyword Builtin::get_ACTCO2S() { return ACTCO2S(); }
const ::Opm::ParserKeyword Builtin::get_ACTDIMS() { return ACTDIMS(); }
const ::Opm::ParserKeyword Builtin::get_ACTION() { return ACTION(); }
const ::Opm::ParserKeyword Builtin::get_ACTIONG() { return ACTIONG(); }
const ::Opm::ParserKeyword Builtin::get_ACTIONR() { return ACTIONR(); }
const ::Opm::ParserKeyword Builtin::get_ACTIONS() { return ACTIONS(); }
const ::Opm::ParserKeyword Builtin::get_ACTIONW() { return ACTIONW(); }
const ::Opm::ParserKeyword Builtin::get_ACTIONX() { return ACTIONX(); }
const ::Opm::ParserKeyword Builtin::get_ACTNUM() { return ACTNUM(); }
const ::Opm::ParserKeyword Builtin::get_ACTPARAM() { return ACTPARAM(); }
const ::Opm::ParserKeyword Builtin::get_ADD() { return ADD(); }
const ::Opm::ParserKeyword Builtin::get_ADDREG() { return ADDREG(); }
const ::Opm::ParserKeyword Builtin::get_ADDZCORN() { return ADDZCORN(); }
const ::Opm::ParserKeyword Builtin::get_ADSALNOD() { return ADSALNOD(); }
const ::Opm::ParserKeyword Builtin::get_ADSORP() { return ADSORP(); }
const ::Opm::ParserKeyword Builtin::get_AITS() { return AITS(); }
const ::Opm::ParserKeyword Builtin::get_AITSOFF() { return AITSOFF(); }
const ::Opm::ParserKeyword Builtin::get_ALKADS() { return ALKADS(); }
const ::Opm::ParserKeyword Builtin::get_ALKALINE() { return ALKALINE(); }
const ::Opm::ParserKeyword Builtin::get_ALKROCK() { return ALKROCK(); }
const ::Opm::ParserKeyword Builtin::get_ALL() { return ALL(); }
const ::Opm::ParserKeyword Builtin::get_ALPOLADS() { return ALPOLADS(); }
const ::Opm::ParserKeyword Builtin::get_ALSURFAD() { return ALSURFAD(); }
const ::Opm::ParserKeyword Builtin::get_ALSURFST() { return ALSURFST(); }
const ::Opm::ParserKeyword Builtin::get_AMALGAM() { return AMALGAM(); }
const ::Opm::ParserKeyword Builtin::get_API() { return API(); }
const ::Opm::ParserKeyword Builtin::get_APIGROUP() { return APIGROUP(); }
const ::Opm::ParserKeyword Builtin::get_APILIM() { return APILIM(); }
const ::Opm::ParserKeyword Builtin::get_APIVD() { return APIVD(); }
const ::Opm::ParserKeyword Builtin::get_AQANCONL() { return AQANCONL(); }
const ::Opm::ParserKeyword Builtin::get_AQANNC() { return AQANNC(); }
const ::Opm::ParserKeyword Builtin::get_AQANTRC() { return AQANTRC(); }
const ::Opm::ParserKeyword Builtin::get_AQUALIST() { return AQUALIST(); }
const ::Opm::ParserKeyword Builtin::get_AQUANCON() { return AQUANCON(); }
const ::Opm::ParserKeyword Builtin::get_AQUCHGAS() { return AQUCHGAS(); }
const ::Opm::ParserKeyword Builtin::get_AQUCHWAT() { return AQUCHWAT(); }
const ::Opm::ParserKeyword Builtin::get_AQUCON() { return AQUCON(); }
const ::Opm::ParserKeyword Builtin::get_AQUCT() { return AQUCT(); }
const ::Opm::ParserKeyword Builtin::get_AQUCWFAC() { return AQUCWFAC(); }
const ::Opm::ParserKeyword Builtin::get_AQUDIMS() { return AQUDIMS(); }
const ::Opm::ParserKeyword Builtin::get_AQUFET() { return AQUFET(); }
const ::Opm::ParserKeyword Builtin::get_AQUFETP() { return AQUFETP(); }
const ::Opm::ParserKeyword Builtin::get_AQUFLUX() { return AQUFLUX(); }
const ::Opm::ParserKeyword Builtin::get_AQUIFER_PROBE_ANALYTIC() { return AQUIFER_PROBE_ANALYTIC(); }
const ::Opm::ParserKeyword Builtin::get_AQUIFER_PROBE_ANALYTIC_NAMED() { return AQUIFER_PROBE_ANALYTIC_NAMED(); }
const ::Opm::ParserKeyword Builtin::get_AQUIFER_PROBE_NUMERIC() { return AQUIFER_PROBE_NUMERIC(); }
const ::Opm::ParserKeyword Builtin::get_AQUNNC() { return AQUNNC(); }
const ::Opm::ParserKeyword Builtin::get_AQUNUM() { return AQUNUM(); }
const ::Opm::ParserKeyword Builtin::get_AQUTAB() { return AQUTAB(); }
const ::Opm::ParserKeyword Builtin::get_AUTOCOAR() { return AUTOCOAR(); }
const ::Opm::ParserKeyword Builtin::get_AUTOREF() { return AUTOREF(); }

void Builtin::emplaceA() const {
    this->keywords.emplace("ACF", ACF());
    this->keywords.emplace("ACTCO2S", ACTCO2S());
    this->keywords.emplace("ACTDIMS", ACTDIMS());
    this->keywords.emplace("ACTION", ACTION());
    this->keywords.emplace("ACTIONG", ACTIONG());
    this->keywords.emplace("ACTIONR", ACTIONR());
    this->keywords.emplace("ACTIONS", ACTIONS());
    this->keywords.emplace("ACTIONW", ACTIONW());
    this->keywords.emplace("ACTIONX", ACTIONX());
    this->keywords.emplace("ACTNUM", ACTNUM());
    this->keywords.emplace("ACTPARAM", ACTPARAM());
    this->keywords.emplace("ADD", ADD());
    this->keywords.emplace("ADDREG", ADDREG());
    this->keywords.emplace("ADDZCORN", ADDZCORN());
    this->keywords.emplace("ADSALNOD", ADSALNOD());
    this->keywords.emplace("ADSORP", ADSORP());
    this->keywords.emplace("AITS", AITS());
    this->keywords.emplace("AITSOFF", AITSOFF());
    this->keywords.emplace("ALKADS", ALKADS());
    this->keywords.emplace("ALKALINE", ALKALINE());
    this->keywords.emplace("ALKROCK", ALKROCK());
    this->keywords.emplace("ALL", ALL());
    this->keywords.emplace("ALPOLADS", ALPOLADS());
    this->keywords.emplace("ALSURFAD", ALSURFAD());
    this->keywords.emplace("ALSURFST", ALSURFST());
    this->keywords.emplace("AMALGAM", AMALGAM());
    this->keywords.emplace("API", API());
    this->keywords.emplace("APIGROUP", APIGROUP());
    this->keywords.emplace("APILIM", APILIM());
    this->keywords.emplace("APIVD", APIVD());
    this->keywords.emplace("AQANCONL", AQANCONL());
    this->keywords.emplace("AQANNC", AQANNC());
    this->keywords.emplace("AQANTRC", AQANTRC());
    this->keywords.emplace("AQUALIST", AQUALIST());
    this->keywords.emplace("AQUANCON", AQUANCON());
    this->keywords.emplace("AQUCHGAS", AQUCHGAS());
    this->keywords.emplace("AQUCHWAT", AQUCHWAT());
    this->keywords.emplace("AQUCON", AQUCON());
    this->keywords.emplace("AQUCT", AQUCT());
    this->keywords.emplace("AQUCWFAC", AQUCWFAC());
    this->keywords.emplace("AQUDIMS", AQUDIMS());
    this->keywords.emplace("AQUFET", AQUFET());
    this->keywords.emplace("AQUFETP", AQUFETP());
    this->keywords.emplace("AQUFLUX", AQUFLUX());
    this->keywords.emplace("AQUIFER_PROBE_ANALYTIC", AQUIFER_PROBE_ANALYTIC());
    this->keywords.emplace("AQUIFER_PROBE_ANALYTIC_NAMED", AQUIFER_PROBE_ANALYTIC_NAMED());
    this->keywords.emplace("AQUIFER_PROBE_NUMERIC", AQUIFER_PROBE_NUMERIC());
    this->keywords.emplace("AQUNNC", AQUNNC());
    this->keywords.emplace("AQUNUM", AQUNUM());
    this->keywords.emplace("AQUTAB", AQUTAB());
    this->keywords.emplace("AUTOCOAR", AUTOCOAR());
    this->keywords.emplace("AUTOREF", AUTOREF());
}
} }
