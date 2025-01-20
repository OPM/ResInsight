#include <opm/input/eclipse/Parser/ParserKeywords/L.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_LAB() { return LAB(); }
const ::Opm::ParserKeyword Builtin::get_LANGMPL() { return LANGMPL(); }
const ::Opm::ParserKeyword Builtin::get_LANGMUIR() { return LANGMUIR(); }
const ::Opm::ParserKeyword Builtin::get_LANGSOLV() { return LANGSOLV(); }
const ::Opm::ParserKeyword Builtin::get_LCUNIT() { return LCUNIT(); }
const ::Opm::ParserKeyword Builtin::get_LGR() { return LGR(); }
const ::Opm::ParserKeyword Builtin::get_LGRCOPY() { return LGRCOPY(); }
const ::Opm::ParserKeyword Builtin::get_LGRFREE() { return LGRFREE(); }
const ::Opm::ParserKeyword Builtin::get_LGRLOCK() { return LGRLOCK(); }
const ::Opm::ParserKeyword Builtin::get_LGROFF() { return LGROFF(); }
const ::Opm::ParserKeyword Builtin::get_LGRON() { return LGRON(); }
const ::Opm::ParserKeyword Builtin::get_LICENSES() { return LICENSES(); }
const ::Opm::ParserKeyword Builtin::get_LIFTOPT() { return LIFTOPT(); }
const ::Opm::ParserKeyword Builtin::get_LINCOM() { return LINCOM(); }
const ::Opm::ParserKeyword Builtin::get_LINKPERM() { return LINKPERM(); }
const ::Opm::ParserKeyword Builtin::get_LIVEOIL() { return LIVEOIL(); }
const ::Opm::ParserKeyword Builtin::get_LKRO() { return LKRO(); }
const ::Opm::ParserKeyword Builtin::get_LKRORG() { return LKRORG(); }
const ::Opm::ParserKeyword Builtin::get_LKRORW() { return LKRORW(); }
const ::Opm::ParserKeyword Builtin::get_LKRW() { return LKRW(); }
const ::Opm::ParserKeyword Builtin::get_LKRWR() { return LKRWR(); }
const ::Opm::ParserKeyword Builtin::get_LOAD() { return LOAD(); }
const ::Opm::ParserKeyword Builtin::get_LOWSALT() { return LOWSALT(); }
const ::Opm::ParserKeyword Builtin::get_LPCW() { return LPCW(); }
const ::Opm::ParserKeyword Builtin::get_LSALTFNC() { return LSALTFNC(); }
const ::Opm::ParserKeyword Builtin::get_LSLTWNUM() { return LSLTWNUM(); }
const ::Opm::ParserKeyword Builtin::get_LSNUM() { return LSNUM(); }
const ::Opm::ParserKeyword Builtin::get_LSOGCR() { return LSOGCR(); }
const ::Opm::ParserKeyword Builtin::get_LSOWCR() { return LSOWCR(); }
const ::Opm::ParserKeyword Builtin::get_LSWCR() { return LSWCR(); }
const ::Opm::ParserKeyword Builtin::get_LSWL() { return LSWL(); }
const ::Opm::ParserKeyword Builtin::get_LSWLPC() { return LSWLPC(); }
const ::Opm::ParserKeyword Builtin::get_LSWU() { return LSWU(); }
const ::Opm::ParserKeyword Builtin::get_LTOSIGMA() { return LTOSIGMA(); }
const ::Opm::ParserKeyword Builtin::get_LWKRO() { return LWKRO(); }
const ::Opm::ParserKeyword Builtin::get_LWKRORG() { return LWKRORG(); }
const ::Opm::ParserKeyword Builtin::get_LWKRORW() { return LWKRORW(); }
const ::Opm::ParserKeyword Builtin::get_LWKRW() { return LWKRW(); }
const ::Opm::ParserKeyword Builtin::get_LWKRWR() { return LWKRWR(); }
const ::Opm::ParserKeyword Builtin::get_LWPCW() { return LWPCW(); }
const ::Opm::ParserKeyword Builtin::get_LWSLTNUM() { return LWSLTNUM(); }
const ::Opm::ParserKeyword Builtin::get_LWSNUM() { return LWSNUM(); }
const ::Opm::ParserKeyword Builtin::get_LWSOGCR() { return LWSOGCR(); }
const ::Opm::ParserKeyword Builtin::get_LWSOWCR() { return LWSOWCR(); }
const ::Opm::ParserKeyword Builtin::get_LWSWCR() { return LWSWCR(); }
const ::Opm::ParserKeyword Builtin::get_LWSWL() { return LWSWL(); }
const ::Opm::ParserKeyword Builtin::get_LWSWLPC() { return LWSWLPC(); }
const ::Opm::ParserKeyword Builtin::get_LWSWU() { return LWSWU(); }
const ::Opm::ParserKeyword Builtin::get_LX() { return LX(); }
const ::Opm::ParserKeyword Builtin::get_LXFIN() { return LXFIN(); }
const ::Opm::ParserKeyword Builtin::get_LY() { return LY(); }
const ::Opm::ParserKeyword Builtin::get_LYFIN() { return LYFIN(); }
const ::Opm::ParserKeyword Builtin::get_LZ() { return LZ(); }
const ::Opm::ParserKeyword Builtin::get_LZFIN() { return LZFIN(); }

void Builtin::emplaceL() const {
    this->keywords.emplace("LAB", LAB());
    this->keywords.emplace("LANGMPL", LANGMPL());
    this->keywords.emplace("LANGMUIR", LANGMUIR());
    this->keywords.emplace("LANGSOLV", LANGSOLV());
    this->keywords.emplace("LCUNIT", LCUNIT());
    this->keywords.emplace("LGR", LGR());
    this->keywords.emplace("LGRCOPY", LGRCOPY());
    this->keywords.emplace("LGRFREE", LGRFREE());
    this->keywords.emplace("LGRLOCK", LGRLOCK());
    this->keywords.emplace("LGROFF", LGROFF());
    this->keywords.emplace("LGRON", LGRON());
    this->keywords.emplace("LICENSES", LICENSES());
    this->keywords.emplace("LIFTOPT", LIFTOPT());
    this->keywords.emplace("LINCOM", LINCOM());
    this->keywords.emplace("LINKPERM", LINKPERM());
    this->keywords.emplace("LIVEOIL", LIVEOIL());
    this->keywords.emplace("LKRO", LKRO());
    this->keywords.emplace("LKRORG", LKRORG());
    this->keywords.emplace("LKRORW", LKRORW());
    this->keywords.emplace("LKRW", LKRW());
    this->keywords.emplace("LKRWR", LKRWR());
    this->keywords.emplace("LOAD", LOAD());
    this->keywords.emplace("LOWSALT", LOWSALT());
    this->keywords.emplace("LPCW", LPCW());
    this->keywords.emplace("LSALTFNC", LSALTFNC());
    this->keywords.emplace("LSLTWNUM", LSLTWNUM());
    this->keywords.emplace("LSNUM", LSNUM());
    this->keywords.emplace("LSOGCR", LSOGCR());
    this->keywords.emplace("LSOWCR", LSOWCR());
    this->keywords.emplace("LSWCR", LSWCR());
    this->keywords.emplace("LSWL", LSWL());
    this->keywords.emplace("LSWLPC", LSWLPC());
    this->keywords.emplace("LSWU", LSWU());
    this->keywords.emplace("LTOSIGMA", LTOSIGMA());
    this->keywords.emplace("LWKRO", LWKRO());
    this->keywords.emplace("LWKRORG", LWKRORG());
    this->keywords.emplace("LWKRORW", LWKRORW());
    this->keywords.emplace("LWKRW", LWKRW());
    this->keywords.emplace("LWKRWR", LWKRWR());
    this->keywords.emplace("LWPCW", LWPCW());
    this->keywords.emplace("LWSLTNUM", LWSLTNUM());
    this->keywords.emplace("LWSNUM", LWSNUM());
    this->keywords.emplace("LWSOGCR", LWSOGCR());
    this->keywords.emplace("LWSOWCR", LWSOWCR());
    this->keywords.emplace("LWSWCR", LWSWCR());
    this->keywords.emplace("LWSWL", LWSWL());
    this->keywords.emplace("LWSWLPC", LWSWLPC());
    this->keywords.emplace("LWSWU", LWSWU());
    this->keywords.emplace("LX", LX());
    this->keywords.emplace("LXFIN", LXFIN());
    this->keywords.emplace("LY", LY());
    this->keywords.emplace("LYFIN", LYFIN());
    this->keywords.emplace("LZ", LZ());
    this->keywords.emplace("LZFIN", LZFIN());
}
} }
