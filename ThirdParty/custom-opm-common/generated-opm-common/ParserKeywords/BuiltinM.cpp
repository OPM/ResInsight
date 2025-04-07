#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_MAPAXES() { return MAPAXES(); }
const ::Opm::ParserKeyword Builtin::get_MAPUNITS() { return MAPUNITS(); }
const ::Opm::ParserKeyword Builtin::get_MASSFLOW() { return MASSFLOW(); }
const ::Opm::ParserKeyword Builtin::get_MATCORR() { return MATCORR(); }
const ::Opm::ParserKeyword Builtin::get_MAXVALUE() { return MAXVALUE(); }
const ::Opm::ParserKeyword Builtin::get_MECH() { return MECH(); }
const ::Opm::ParserKeyword Builtin::get_MEMORY() { return MEMORY(); }
const ::Opm::ParserKeyword Builtin::get_MESSAGE() { return MESSAGE(); }
const ::Opm::ParserKeyword Builtin::get_MESSAGES() { return MESSAGES(); }
const ::Opm::ParserKeyword Builtin::get_MESSOPTS() { return MESSOPTS(); }
const ::Opm::ParserKeyword Builtin::get_MESSSRVC() { return MESSSRVC(); }
const ::Opm::ParserKeyword Builtin::get_METRIC() { return METRIC(); }
const ::Opm::ParserKeyword Builtin::get_MICP() { return MICP(); }
const ::Opm::ParserKeyword Builtin::get_MICPPARA() { return MICPPARA(); }
const ::Opm::ParserKeyword Builtin::get_MINNNCT() { return MINNNCT(); }
const ::Opm::ParserKeyword Builtin::get_MINNPCOL() { return MINNPCOL(); }
const ::Opm::ParserKeyword Builtin::get_MINPORV() { return MINPORV(); }
const ::Opm::ParserKeyword Builtin::get_MINPV() { return MINPV(); }
const ::Opm::ParserKeyword Builtin::get_MINPVV() { return MINPVV(); }
const ::Opm::ParserKeyword Builtin::get_MINVALUE() { return MINVALUE(); }
const ::Opm::ParserKeyword Builtin::get_MISC() { return MISC(); }
const ::Opm::ParserKeyword Builtin::get_MISCIBLE() { return MISCIBLE(); }
const ::Opm::ParserKeyword Builtin::get_MISCNUM() { return MISCNUM(); }
const ::Opm::ParserKeyword Builtin::get_MLANG() { return MLANG(); }
const ::Opm::ParserKeyword Builtin::get_MLANGSLV() { return MLANGSLV(); }
const ::Opm::ParserKeyword Builtin::get_MONITOR() { return MONITOR(); }
const ::Opm::ParserKeyword Builtin::get_MPFANUM() { return MPFANUM(); }
const ::Opm::ParserKeyword Builtin::get_MPFNNC() { return MPFNNC(); }
const ::Opm::ParserKeyword Builtin::get_MSFN() { return MSFN(); }
const ::Opm::ParserKeyword Builtin::get_MSGFILE() { return MSGFILE(); }
const ::Opm::ParserKeyword Builtin::get_MSUM_PROBE() { return MSUM_PROBE(); }
const ::Opm::ParserKeyword Builtin::get_MULSGGD() { return MULSGGD(); }
const ::Opm::ParserKeyword Builtin::get_MULSGGDV() { return MULSGGDV(); }
const ::Opm::ParserKeyword Builtin::get_MULTFLT() { return MULTFLT(); }
const ::Opm::ParserKeyword Builtin::get_MULTIN() { return MULTIN(); }
const ::Opm::ParserKeyword Builtin::get_MULTIPLY() { return MULTIPLY(); }
const ::Opm::ParserKeyword Builtin::get_MULTIREG() { return MULTIREG(); }
const ::Opm::ParserKeyword Builtin::get_MULTNUM() { return MULTNUM(); }
const ::Opm::ParserKeyword Builtin::get_MULTOUT() { return MULTOUT(); }
const ::Opm::ParserKeyword Builtin::get_MULTOUTS() { return MULTOUTS(); }
const ::Opm::ParserKeyword Builtin::get_MULTPV() { return MULTPV(); }
const ::Opm::ParserKeyword Builtin::get_MULTREAL() { return MULTREAL(); }
const ::Opm::ParserKeyword Builtin::get_MULTREGD() { return MULTREGD(); }
const ::Opm::ParserKeyword Builtin::get_MULTREGH() { return MULTREGH(); }
const ::Opm::ParserKeyword Builtin::get_MULTREGP() { return MULTREGP(); }
const ::Opm::ParserKeyword Builtin::get_MULTREGT() { return MULTREGT(); }
const ::Opm::ParserKeyword Builtin::get_MULTSIG() { return MULTSIG(); }
const ::Opm::ParserKeyword Builtin::get_MULTSIGV() { return MULTSIGV(); }
const ::Opm::ParserKeyword Builtin::get_MULT_XYZ() { return MULT_XYZ(); }
const ::Opm::ParserKeyword Builtin::get_MW() { return MW(); }
const ::Opm::ParserKeyword Builtin::get_MWS() { return MWS(); }

void Builtin::emplaceM() const {
    this->keywords.emplace("MAPAXES", MAPAXES());
    this->keywords.emplace("MAPUNITS", MAPUNITS());
    this->keywords.emplace("MASSFLOW", MASSFLOW());
    this->keywords.emplace("MATCORR", MATCORR());
    this->keywords.emplace("MAXVALUE", MAXVALUE());
    this->keywords.emplace("MECH", MECH());
    this->keywords.emplace("MEMORY", MEMORY());
    this->keywords.emplace("MESSAGE", MESSAGE());
    this->keywords.emplace("MESSAGES", MESSAGES());
    this->keywords.emplace("MESSOPTS", MESSOPTS());
    this->keywords.emplace("MESSSRVC", MESSSRVC());
    this->keywords.emplace("METRIC", METRIC());
    this->keywords.emplace("MICP", MICP());
    this->keywords.emplace("MICPPARA", MICPPARA());
    this->keywords.emplace("MINNNCT", MINNNCT());
    this->keywords.emplace("MINNPCOL", MINNPCOL());
    this->keywords.emplace("MINPORV", MINPORV());
    this->keywords.emplace("MINPV", MINPV());
    this->keywords.emplace("MINPVV", MINPVV());
    this->keywords.emplace("MINVALUE", MINVALUE());
    this->keywords.emplace("MISC", MISC());
    this->keywords.emplace("MISCIBLE", MISCIBLE());
    this->keywords.emplace("MISCNUM", MISCNUM());
    this->keywords.emplace("MLANG", MLANG());
    this->keywords.emplace("MLANGSLV", MLANGSLV());
    this->keywords.emplace("MONITOR", MONITOR());
    this->keywords.emplace("MPFANUM", MPFANUM());
    this->keywords.emplace("MPFNNC", MPFNNC());
    this->keywords.emplace("MSFN", MSFN());
    this->keywords.emplace("MSGFILE", MSGFILE());
    this->keywords.emplace("MSUM_PROBE", MSUM_PROBE());
    this->keywords.emplace("MULSGGD", MULSGGD());
    this->keywords.emplace("MULSGGDV", MULSGGDV());
    this->keywords.emplace("MULTFLT", MULTFLT());
    this->keywords.emplace("MULTIN", MULTIN());
    this->keywords.emplace("MULTIPLY", MULTIPLY());
    this->keywords.emplace("MULTIREG", MULTIREG());
    this->keywords.emplace("MULTNUM", MULTNUM());
    this->keywords.emplace("MULTOUT", MULTOUT());
    this->keywords.emplace("MULTOUTS", MULTOUTS());
    this->keywords.emplace("MULTPV", MULTPV());
    this->keywords.emplace("MULTREAL", MULTREAL());
    this->keywords.emplace("MULTREGD", MULTREGD());
    this->keywords.emplace("MULTREGH", MULTREGH());
    this->keywords.emplace("MULTREGP", MULTREGP());
    this->keywords.emplace("MULTREGT", MULTREGT());
    this->keywords.emplace("MULTSIG", MULTSIG());
    this->keywords.emplace("MULTSIGV", MULTSIGV());
    this->keywords.emplace("MULT_XYZ", MULT_XYZ());
    this->keywords.emplace("MW", MW());
    this->keywords.emplace("MWS", MWS());
}
} }
