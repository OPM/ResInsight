#include <opm/input/eclipse/Parser/ParserKeywords/O.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_OCOMPIDX() { return OCOMPIDX(); }
const ::Opm::ParserKeyword Builtin::get_OFM() { return OFM(); }
const ::Opm::ParserKeyword Builtin::get_OIL() { return OIL(); }
const ::Opm::ParserKeyword Builtin::get_OILAPI() { return OILAPI(); }
const ::Opm::ParserKeyword Builtin::get_OILCOMPR() { return OILCOMPR(); }
const ::Opm::ParserKeyword Builtin::get_OILDENT() { return OILDENT(); }
const ::Opm::ParserKeyword Builtin::get_OILJT() { return OILJT(); }
const ::Opm::ParserKeyword Builtin::get_OILMW() { return OILMW(); }
const ::Opm::ParserKeyword Builtin::get_OILVISCT() { return OILVISCT(); }
const ::Opm::ParserKeyword Builtin::get_OILVTIM() { return OILVTIM(); }
const ::Opm::ParserKeyword Builtin::get_OLDTRAN() { return OLDTRAN(); }
const ::Opm::ParserKeyword Builtin::get_OLDTRANR() { return OLDTRANR(); }
const ::Opm::ParserKeyword Builtin::get_OPERATE() { return OPERATE(); }
const ::Opm::ParserKeyword Builtin::get_OPERATER() { return OPERATER(); }
const ::Opm::ParserKeyword Builtin::get_OPERNUM() { return OPERNUM(); }
const ::Opm::ParserKeyword Builtin::get_OPTIONS() { return OPTIONS(); }
const ::Opm::ParserKeyword Builtin::get_OPTIONS3() { return OPTIONS3(); }
const ::Opm::ParserKeyword Builtin::get_OUTRAD() { return OUTRAD(); }
const ::Opm::ParserKeyword Builtin::get_OUTSOL() { return OUTSOL(); }
const ::Opm::ParserKeyword Builtin::get_OVERBURD() { return OVERBURD(); }

void Builtin::emplaceO() const {
    this->keywords.emplace("OCOMPIDX", OCOMPIDX());
    this->keywords.emplace("OFM", OFM());
    this->keywords.emplace("OIL", OIL());
    this->keywords.emplace("OILAPI", OILAPI());
    this->keywords.emplace("OILCOMPR", OILCOMPR());
    this->keywords.emplace("OILDENT", OILDENT());
    this->keywords.emplace("OILJT", OILJT());
    this->keywords.emplace("OILMW", OILMW());
    this->keywords.emplace("OILVISCT", OILVISCT());
    this->keywords.emplace("OILVTIM", OILVTIM());
    this->keywords.emplace("OLDTRAN", OLDTRAN());
    this->keywords.emplace("OLDTRANR", OLDTRANR());
    this->keywords.emplace("OPERATE", OPERATE());
    this->keywords.emplace("OPERATER", OPERATER());
    this->keywords.emplace("OPERNUM", OPERNUM());
    this->keywords.emplace("OPTIONS", OPTIONS());
    this->keywords.emplace("OPTIONS3", OPTIONS3());
    this->keywords.emplace("OUTRAD", OUTRAD());
    this->keywords.emplace("OUTSOL", OUTSOL());
    this->keywords.emplace("OVERBURD", OVERBURD());
}
} }
