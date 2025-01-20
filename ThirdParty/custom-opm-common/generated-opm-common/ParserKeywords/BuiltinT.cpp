#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_TABDIMS() { return TABDIMS(); }
const ::Opm::ParserKeyword Builtin::get_TBLK() { return TBLK(); }
const ::Opm::ParserKeyword Builtin::get_TCRIT() { return TCRIT(); }
const ::Opm::ParserKeyword Builtin::get_TEMP() { return TEMP(); }
const ::Opm::ParserKeyword Builtin::get_TEMPI() { return TEMPI(); }
const ::Opm::ParserKeyword Builtin::get_TEMPNODE() { return TEMPNODE(); }
const ::Opm::ParserKeyword Builtin::get_TEMPTVD() { return TEMPTVD(); }
const ::Opm::ParserKeyword Builtin::get_TEMPVD() { return TEMPVD(); }
const ::Opm::ParserKeyword Builtin::get_THCGAS() { return THCGAS(); }
const ::Opm::ParserKeyword Builtin::get_THCO2MIX() { return THCO2MIX(); }
const ::Opm::ParserKeyword Builtin::get_THCOIL() { return THCOIL(); }
const ::Opm::ParserKeyword Builtin::get_THCONR() { return THCONR(); }
const ::Opm::ParserKeyword Builtin::get_THCONSF() { return THCONSF(); }
const ::Opm::ParserKeyword Builtin::get_THCROCK() { return THCROCK(); }
const ::Opm::ParserKeyword Builtin::get_THCWATER() { return THCWATER(); }
const ::Opm::ParserKeyword Builtin::get_THELCOEF() { return THELCOEF(); }
const ::Opm::ParserKeyword Builtin::get_THERMAL() { return THERMAL(); }
const ::Opm::ParserKeyword Builtin::get_THERMEXR() { return THERMEXR(); }
const ::Opm::ParserKeyword Builtin::get_THPRES() { return THPRES(); }
const ::Opm::ParserKeyword Builtin::get_THPRESFT() { return THPRESFT(); }
const ::Opm::ParserKeyword Builtin::get_TIGHTEN() { return TIGHTEN(); }
const ::Opm::ParserKeyword Builtin::get_TIGHTENP() { return TIGHTENP(); }
const ::Opm::ParserKeyword Builtin::get_TIME() { return TIME(); }
const ::Opm::ParserKeyword Builtin::get_TITLE() { return TITLE(); }
const ::Opm::ParserKeyword Builtin::get_TLMIXPAR() { return TLMIXPAR(); }
const ::Opm::ParserKeyword Builtin::get_TLPMIXPA() { return TLPMIXPA(); }
const ::Opm::ParserKeyword Builtin::get_TNUM() { return TNUM(); }
const ::Opm::ParserKeyword Builtin::get_TOLCRIT() { return TOLCRIT(); }
const ::Opm::ParserKeyword Builtin::get_TOPS() { return TOPS(); }
const ::Opm::ParserKeyword Builtin::get_TPAMEPS() { return TPAMEPS(); }
const ::Opm::ParserKeyword Builtin::get_TPAMEPSS() { return TPAMEPSS(); }
const ::Opm::ParserKeyword Builtin::get_TRACER() { return TRACER(); }
const ::Opm::ParserKeyword Builtin::get_TRACERKM() { return TRACERKM(); }
const ::Opm::ParserKeyword Builtin::get_TRACERKP() { return TRACERKP(); }
const ::Opm::ParserKeyword Builtin::get_TRACERS() { return TRACERS(); }
const ::Opm::ParserKeyword Builtin::get_TRACITVD() { return TRACITVD(); }
const ::Opm::ParserKeyword Builtin::get_TRACTVD() { return TRACTVD(); }
const ::Opm::ParserKeyword Builtin::get_TRADS() { return TRADS(); }
const ::Opm::ParserKeyword Builtin::get_TRANGL() { return TRANGL(); }
const ::Opm::ParserKeyword Builtin::get_TRANR() { return TRANR(); }
const ::Opm::ParserKeyword Builtin::get_TRANTHT() { return TRANTHT(); }
const ::Opm::ParserKeyword Builtin::get_TRANX() { return TRANX(); }
const ::Opm::ParserKeyword Builtin::get_TRANY() { return TRANY(); }
const ::Opm::ParserKeyword Builtin::get_TRANZ() { return TRANZ(); }
const ::Opm::ParserKeyword Builtin::get_TRDCY() { return TRDCY(); }
const ::Opm::ParserKeyword Builtin::get_TRDIF() { return TRDIF(); }
const ::Opm::ParserKeyword Builtin::get_TRDIS() { return TRDIS(); }
const ::Opm::ParserKeyword Builtin::get_TREF() { return TREF(); }
const ::Opm::ParserKeyword Builtin::get_TREFS() { return TREFS(); }
const ::Opm::ParserKeyword Builtin::get_TRKPF() { return TRKPF(); }
const ::Opm::ParserKeyword Builtin::get_TRNHD() { return TRNHD(); }
const ::Opm::ParserKeyword Builtin::get_TRPLPORO() { return TRPLPORO(); }
const ::Opm::ParserKeyword Builtin::get_TRROCK() { return TRROCK(); }
const ::Opm::ParserKeyword Builtin::get_TSTEP() { return TSTEP(); }
const ::Opm::ParserKeyword Builtin::get_TUNING() { return TUNING(); }
const ::Opm::ParserKeyword Builtin::get_TUNINGDP() { return TUNINGDP(); }
const ::Opm::ParserKeyword Builtin::get_TUNINGH() { return TUNINGH(); }
const ::Opm::ParserKeyword Builtin::get_TUNINGL() { return TUNINGL(); }
const ::Opm::ParserKeyword Builtin::get_TUNINGS() { return TUNINGS(); }
const ::Opm::ParserKeyword Builtin::get_TVDP() { return TVDP(); }
const ::Opm::ParserKeyword Builtin::get_TZONE() { return TZONE(); }

void Builtin::emplaceT() const {
    this->keywords.emplace("TABDIMS", TABDIMS());
    this->keywords.emplace("TBLK", TBLK());
    this->keywords.emplace("TCRIT", TCRIT());
    this->keywords.emplace("TEMP", TEMP());
    this->keywords.emplace("TEMPI", TEMPI());
    this->keywords.emplace("TEMPNODE", TEMPNODE());
    this->keywords.emplace("TEMPTVD", TEMPTVD());
    this->keywords.emplace("TEMPVD", TEMPVD());
    this->keywords.emplace("THCGAS", THCGAS());
    this->keywords.emplace("THCO2MIX", THCO2MIX());
    this->keywords.emplace("THCOIL", THCOIL());
    this->keywords.emplace("THCONR", THCONR());
    this->keywords.emplace("THCONSF", THCONSF());
    this->keywords.emplace("THCROCK", THCROCK());
    this->keywords.emplace("THCWATER", THCWATER());
    this->keywords.emplace("THELCOEF", THELCOEF());
    this->keywords.emplace("THERMAL", THERMAL());
    this->keywords.emplace("THERMEXR", THERMEXR());
    this->keywords.emplace("THPRES", THPRES());
    this->keywords.emplace("THPRESFT", THPRESFT());
    this->keywords.emplace("TIGHTEN", TIGHTEN());
    this->keywords.emplace("TIGHTENP", TIGHTENP());
    this->keywords.emplace("TIME", TIME());
    this->keywords.emplace("TITLE", TITLE());
    this->keywords.emplace("TLMIXPAR", TLMIXPAR());
    this->keywords.emplace("TLPMIXPA", TLPMIXPA());
    this->keywords.emplace("TNUM", TNUM());
    this->keywords.emplace("TOLCRIT", TOLCRIT());
    this->keywords.emplace("TOPS", TOPS());
    this->keywords.emplace("TPAMEPS", TPAMEPS());
    this->keywords.emplace("TPAMEPSS", TPAMEPSS());
    this->keywords.emplace("TRACER", TRACER());
    this->keywords.emplace("TRACERKM", TRACERKM());
    this->keywords.emplace("TRACERKP", TRACERKP());
    this->keywords.emplace("TRACERS", TRACERS());
    this->keywords.emplace("TRACITVD", TRACITVD());
    this->keywords.emplace("TRACTVD", TRACTVD());
    this->keywords.emplace("TRADS", TRADS());
    this->keywords.emplace("TRANGL", TRANGL());
    this->keywords.emplace("TRANR", TRANR());
    this->keywords.emplace("TRANTHT", TRANTHT());
    this->keywords.emplace("TRANX", TRANX());
    this->keywords.emplace("TRANY", TRANY());
    this->keywords.emplace("TRANZ", TRANZ());
    this->keywords.emplace("TRDCY", TRDCY());
    this->keywords.emplace("TRDIF", TRDIF());
    this->keywords.emplace("TRDIS", TRDIS());
    this->keywords.emplace("TREF", TREF());
    this->keywords.emplace("TREFS", TREFS());
    this->keywords.emplace("TRKPF", TRKPF());
    this->keywords.emplace("TRNHD", TRNHD());
    this->keywords.emplace("TRPLPORO", TRPLPORO());
    this->keywords.emplace("TRROCK", TRROCK());
    this->keywords.emplace("TSTEP", TSTEP());
    this->keywords.emplace("TUNING", TUNING());
    this->keywords.emplace("TUNINGDP", TUNINGDP());
    this->keywords.emplace("TUNINGH", TUNINGH());
    this->keywords.emplace("TUNINGL", TUNINGL());
    this->keywords.emplace("TUNINGS", TUNINGS());
    this->keywords.emplace("TVDP", TVDP());
    this->keywords.emplace("TZONE", TZONE());
}
} }
