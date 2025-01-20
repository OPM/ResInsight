#include <opm/input/eclipse/Parser/ParserKeywords/N.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_NARROW() { return NARROW(); }
const ::Opm::ParserKeyword Builtin::get_NCOMPS() { return NCOMPS(); }
const ::Opm::ParserKeyword Builtin::get_NCONSUMP() { return NCONSUMP(); }
const ::Opm::ParserKeyword Builtin::get_NEFAC() { return NEFAC(); }
const ::Opm::ParserKeyword Builtin::get_NETBALAN() { return NETBALAN(); }
const ::Opm::ParserKeyword Builtin::get_NETCOMPA() { return NETCOMPA(); }
const ::Opm::ParserKeyword Builtin::get_NETWORK() { return NETWORK(); }
const ::Opm::ParserKeyword Builtin::get_NETWORK_PROBE() { return NETWORK_PROBE(); }
const ::Opm::ParserKeyword Builtin::get_NEWTRAN() { return NEWTRAN(); }
const ::Opm::ParserKeyword Builtin::get_NEXTSTEP() { return NEXTSTEP(); }
const ::Opm::ParserKeyword Builtin::get_NEXTSTPL() { return NEXTSTPL(); }
const ::Opm::ParserKeyword Builtin::get_NINENUM() { return NINENUM(); }
const ::Opm::ParserKeyword Builtin::get_NINEPOIN() { return NINEPOIN(); }
const ::Opm::ParserKeyword Builtin::get_NMATOPTS() { return NMATOPTS(); }
const ::Opm::ParserKeyword Builtin::get_NMATRIX() { return NMATRIX(); }
const ::Opm::ParserKeyword Builtin::get_NMESSAGE() { return NMESSAGE(); }
const ::Opm::ParserKeyword Builtin::get_NNC() { return NNC(); }
const ::Opm::ParserKeyword Builtin::get_NNEWTF() { return NNEWTF(); }
const ::Opm::ParserKeyword Builtin::get_NOCASC() { return NOCASC(); }
const ::Opm::ParserKeyword Builtin::get_NODEPROP() { return NODEPROP(); }
const ::Opm::ParserKeyword Builtin::get_NODPPM() { return NODPPM(); }
const ::Opm::ParserKeyword Builtin::get_NOECHO() { return NOECHO(); }
const ::Opm::ParserKeyword Builtin::get_NOGGF() { return NOGGF(); }
const ::Opm::ParserKeyword Builtin::get_NOGRAV() { return NOGRAV(); }
const ::Opm::ParserKeyword Builtin::get_NOHMD() { return NOHMD(); }
const ::Opm::ParserKeyword Builtin::get_NOHMO() { return NOHMO(); }
const ::Opm::ParserKeyword Builtin::get_NOHYST() { return NOHYST(); }
const ::Opm::ParserKeyword Builtin::get_NOINSPEC() { return NOINSPEC(); }
const ::Opm::ParserKeyword Builtin::get_NOMONITO() { return NOMONITO(); }
const ::Opm::ParserKeyword Builtin::get_NONNC() { return NONNC(); }
const ::Opm::ParserKeyword Builtin::get_NORSSPEC() { return NORSSPEC(); }
const ::Opm::ParserKeyword Builtin::get_NOSIM() { return NOSIM(); }
const ::Opm::ParserKeyword Builtin::get_NOWARN() { return NOWARN(); }
const ::Opm::ParserKeyword Builtin::get_NOWARNEP() { return NOWARNEP(); }
const ::Opm::ParserKeyword Builtin::get_NRSOUT() { return NRSOUT(); }
const ::Opm::ParserKeyword Builtin::get_NSTACK() { return NSTACK(); }
const ::Opm::ParserKeyword Builtin::get_NTG() { return NTG(); }
const ::Opm::ParserKeyword Builtin::get_NUMRES() { return NUMRES(); }
const ::Opm::ParserKeyword Builtin::get_NUPCOL() { return NUPCOL(); }
const ::Opm::ParserKeyword Builtin::get_NWATREM() { return NWATREM(); }
const ::Opm::ParserKeyword Builtin::get_NXFIN() { return NXFIN(); }
const ::Opm::ParserKeyword Builtin::get_NYFIN() { return NYFIN(); }
const ::Opm::ParserKeyword Builtin::get_NZFIN() { return NZFIN(); }

void Builtin::emplaceN() const {
    this->keywords.emplace("NARROW", NARROW());
    this->keywords.emplace("NCOMPS", NCOMPS());
    this->keywords.emplace("NCONSUMP", NCONSUMP());
    this->keywords.emplace("NEFAC", NEFAC());
    this->keywords.emplace("NETBALAN", NETBALAN());
    this->keywords.emplace("NETCOMPA", NETCOMPA());
    this->keywords.emplace("NETWORK", NETWORK());
    this->keywords.emplace("NETWORK_PROBE", NETWORK_PROBE());
    this->keywords.emplace("NEWTRAN", NEWTRAN());
    this->keywords.emplace("NEXTSTEP", NEXTSTEP());
    this->keywords.emplace("NEXTSTPL", NEXTSTPL());
    this->keywords.emplace("NINENUM", NINENUM());
    this->keywords.emplace("NINEPOIN", NINEPOIN());
    this->keywords.emplace("NMATOPTS", NMATOPTS());
    this->keywords.emplace("NMATRIX", NMATRIX());
    this->keywords.emplace("NMESSAGE", NMESSAGE());
    this->keywords.emplace("NNC", NNC());
    this->keywords.emplace("NNEWTF", NNEWTF());
    this->keywords.emplace("NOCASC", NOCASC());
    this->keywords.emplace("NODEPROP", NODEPROP());
    this->keywords.emplace("NODPPM", NODPPM());
    this->keywords.emplace("NOECHO", NOECHO());
    this->keywords.emplace("NOGGF", NOGGF());
    this->keywords.emplace("NOGRAV", NOGRAV());
    this->keywords.emplace("NOHMD", NOHMD());
    this->keywords.emplace("NOHMO", NOHMO());
    this->keywords.emplace("NOHYST", NOHYST());
    this->keywords.emplace("NOINSPEC", NOINSPEC());
    this->keywords.emplace("NOMONITO", NOMONITO());
    this->keywords.emplace("NONNC", NONNC());
    this->keywords.emplace("NORSSPEC", NORSSPEC());
    this->keywords.emplace("NOSIM", NOSIM());
    this->keywords.emplace("NOWARN", NOWARN());
    this->keywords.emplace("NOWARNEP", NOWARNEP());
    this->keywords.emplace("NRSOUT", NRSOUT());
    this->keywords.emplace("NSTACK", NSTACK());
    this->keywords.emplace("NTG", NTG());
    this->keywords.emplace("NUMRES", NUMRES());
    this->keywords.emplace("NUPCOL", NUPCOL());
    this->keywords.emplace("NWATREM", NWATREM());
    this->keywords.emplace("NXFIN", NXFIN());
    this->keywords.emplace("NYFIN", NYFIN());
    this->keywords.emplace("NZFIN", NZFIN());
}
} }
